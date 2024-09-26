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
#include "../layers/containers/range_vector.h"

class NegativeGpuAVBufferDeviceAddress : public GpuAVBufferDeviceAddressTest {};

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadBeforePointerPushConstant) {
    TEST_DESCRIPTION("Read before the valid pointer - use Push Constants to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    VkPushConstantRange push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(VkDeviceAddress)};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges = &push_constant_ranges;
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    char const *shader_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(push_constant) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4]; // 16 byte strides
            };
            void main() {
                for (int i=0; i < u_info.nWrites; ++i) {
                    u_info.data.a[i] = 42;
                }
            }
        )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.address();
    // Will dereference the wrong ptr address
    VkDeviceAddress push_constants[2] = {u_info_ptr - 16, 4};
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),
                         push_constants);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the other 3 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.memory().map());
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
    buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadAfterPointerPushConstant) {
    TEST_DESCRIPTION("Read after the valid pointer - use Push Constants to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    VkPushConstantRange push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT, 0, 2 * sizeof(VkDeviceAddress)};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges = &push_constant_ranges;
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    char const *shader_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(push_constant) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4]; // 16 byte stride
            };
            void main() {
                for (int i=0; i < u_info.nWrites; ++i) {
                    u_info.data.a[i] = 42;
                }
            }
        )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.address();
    // will go over a[4] by one
    VkDeviceAddress push_constants[2] = {u_info_ptr, 5};
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),
                         push_constants);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the first 4 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.memory().map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
    buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadBeforePointerDescriptor) {
    TEST_DESCRIPTION("Read 16 bytes before the valid pointer - use Descriptor to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer uniform_buffer(*m_device, 12, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.address();
    // Will dereference the wrong ptr address
    VkDeviceAddress invalid_buffer_address = u_info_ptr - 16;
    uint32_t n_writes = 4;

    uint8_t *uniform_buffer_ptr = (uint8_t *)uniform_buffer.memory().map();
    memcpy(uniform_buffer_ptr, &invalid_buffer_address, sizeof(VkDeviceAddress));
    memcpy(uniform_buffer_ptr + sizeof(VkDeviceAddress), &n_writes, sizeof(uint32_t));
    uniform_buffer.memory().unmap();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct data;
            int nWrites;
        } u_info;
        layout(buffer_reference, std140) buffer bufStruct {
            int a[4]; // 16 byte stride
        };
        void main() {
            for (int i=0; i < u_info.nWrites; ++i) {
                u_info.data.a[i] = 42;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the other 3 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.memory().map());
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
    buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadAfterPointerDescriptor) {
    TEST_DESCRIPTION("Read after the valid pointer - use Descriptor to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer uniform_buffer(*m_device, 12, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.address();
    // will go over a[4] by one
    uint32_t n_writes = 5;

    uint8_t *uniform_buffer_ptr = (uint8_t *)uniform_buffer.memory().map();
    memcpy(uniform_buffer_ptr, &u_info_ptr, sizeof(VkDeviceAddress));
    memcpy(uniform_buffer_ptr + sizeof(VkDeviceAddress), &n_writes, sizeof(uint32_t));
    uniform_buffer.memory().unmap();

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

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the first 4 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.memory().map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
    buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, UVec3Array) {
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

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Hold only 3 indices
    vkt::Buffer block_buffer(*m_device, 36, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.address();
    const uint32_t n_reads = 4;  // uvec3[0] to uvec3[3]

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &n_reads, sizeof(uint32_t));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, Maintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            vec4 indices[];
        };

        layout(set = 0, binding = 0) uniform foo {
            IndexBuffer data;
            int index;
        } in_buffer;

        layout(location = 0) out vec4 color;

        void main() {
            color = in_buffer.data.indices[in_buffer.index];
        }
    )glsl";

    std::vector<uint32_t> vert_shader;
    std::vector<uint32_t> frag_shader;
    this->GLSLtoSPV(&m_device->phy().limits_, VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl, vert_shader);
    this->GLSLtoSPV(&m_device->phy().limits_, VK_SHADER_STAGE_FRAGMENT_BIT, fs_source, frag_shader);

    VkShaderModuleCreateInfo module_create_info_vert = vku::InitStructHelper();
    module_create_info_vert.pCode = vert_shader.data();
    module_create_info_vert.codeSize = vert_shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci_vert = vku::InitStructHelper(&module_create_info_vert);
    stage_ci_vert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci_vert.module = VK_NULL_HANDLE;
    stage_ci_vert.pName = "main";

    VkShaderModuleCreateInfo module_create_info_frag = vku::InitStructHelper();
    module_create_info_frag.pCode = frag_shader.data();
    module_create_info_frag.codeSize = frag_shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci_frag = vku::InitStructHelper(&module_create_info_frag);
    stage_ci_frag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage_ci_frag.module = VK_NULL_HANDLE;
    stage_ci_frag.pName = "main";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {stage_ci_vert, stage_ci_frag};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.address();
    const uint32_t n_reads = 64;  // way too large

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &n_reads, sizeof(uint32_t));
    in_buffer.memory().unmap();

    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 6);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7462
TEST_F(NegativeGpuAVBufferDeviceAddress, DISABLED_ArrayOfStruct) {
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
            uint pad;
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

    vkt::Buffer block_buffer(*m_device, 32, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.address();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *buffer_ptr = (uint8_t *)storage_buffer.memory().map();
    const uint32_t index = 8;  // out of bounds
    memcpy(buffer_ptr, &index, sizeof(uint32_t));
    memcpy(buffer_ptr + (1 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (2 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (3 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    storage_buffer.memory().unmap();

    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd140) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4]");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 16 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
    data[0] = storage_buffer.address();
    data[1] = 5;
    uniform_buffer.memory().unmap();

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.memory().map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
    storage_buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd140NumerousRanges) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4] - make sure it is detected even when there are numerous valid ranges");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 16 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    // Get device address of buffer to write to
    using AddrRange = sparse_container::range<VkDeviceAddress>;
    const VkDeviceAddress storage_buffer_addr = storage_buffer.address();
    const AddrRange shader_writes_range(storage_buffer_addr, storage_buffer_addr + storage_buffer_size + 4);

    // Create storage buffers for the sake of storing multiple device address ranges
    std::vector<vkt::Buffer> dummy_storage_buffers;
    for (int i = 0; i < 1024; ++i) {
        const VkDeviceAddress addr =
            dummy_storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).address();
        const AddrRange addr_range(addr, addr + storage_buffer_size);
        // If new buffer address range overlaps with storage buffer range,
        // writes past its end may be valid, so remove dummy buffer
        if (shader_writes_range.intersects(addr_range)) {
            dummy_storage_buffers.resize(dummy_storage_buffers.size() - 1);
        }
    }

    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
    data[0] = storage_buffer_addr;
    data[1] = 5;  // Will provoke a 4 bytes write past buffer end
    uniform_buffer.memory().unmap();

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.memory().map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
    storage_buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd430) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4]");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    // Make another buffer to write to
    const uint32_t storage_buffer_size = 4 * 4;
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);

    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
    data[0] = storage_buffer.address();
    data[1] = 5;
    uniform_buffer.memory().unmap();

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.memory().map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], 42);
    }
    storage_buffer.memory().unmap();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreRelaxedBlockLayout) {
    TEST_DESCRIPTION("OOB detected reading just past buffer end - use VK_KHR_relaxed_block_layout");
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
               OpStore %29 %27 Aligned 16
               OpReturn
               OpFunctionEnd
    )";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 8;  // 64 bits pointer
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.end();

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // only can fit 3 floats
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.address();

    // Base buffer address is (storage_buffer_addr), so expect writing to `v.z` to cause an OOB access
    {
        auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
        data[0] = storage_buffer_addr;
        uniform_buffer.memory().unmap();

        m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        // Make sure shader wrote to float
        auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.memory().map());
        ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
        storage_buffer.memory().unmap();
    }

    // Base buffer address is (storage_buffer_addr - 4), so expect writing to `f` to cause an OOB access
    {
        auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
        data[0] = storage_buffer_addr - sizeof(float);
        uniform_buffer.memory().unmap();

        m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        // Make sure shader wrote to vec3
        auto storage_buffer_ptr = static_cast<float *>(storage_buffer.memory().map());
        ASSERT_EQ(storage_buffer_ptr[0], 1.0f);
        ASSERT_EQ(storage_buffer_ptr[1], 2.0f);
        ASSERT_EQ(storage_buffer_ptr[2], 3.0f);
        storage_buffer.memory().unmap();
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreScalarBlockLayout) {
    TEST_DESCRIPTION("OOB detected - use VK_EXT_scalar_block_layout");

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
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.end();

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // can only hold 3 floats, when SSBO uses 4
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.address();

    // Base buffer address is (storage_buffer_addr), so expect writing to `v.z` to cause an OOB access
    {
        auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
        data[0] = storage_buffer_addr;
        uniform_buffer.memory().unmap();

        m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        // Make sure shader wrote to float
        auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.memory().map());
        ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
        storage_buffer.memory().unmap();
    }

    // Base buffer address is (storage_buffer_addr - 4), so expect writing to `f` to cause an OOB access
    {
        auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
        data[0] = storage_buffer_addr - sizeof(float);
        uniform_buffer.memory().unmap();

        m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        // Make sure shader wrote to vec3
        auto storage_buffer_ptr = static_cast<float *>(storage_buffer.memory().map());
        ASSERT_EQ(storage_buffer_ptr[0], 1.0f);
        ASSERT_EQ(storage_buffer_ptr[1], 2.0f);
        ASSERT_EQ(storage_buffer_ptr[2], 3.0f);
        storage_buffer.memory().unmap();
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd430LinkedList) {
    TEST_DESCRIPTION("OOB writes in a linked list");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 3 * sizeof(VkDeviceAddress);
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    pipeline.descriptor_set_->WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.end();

    constexpr size_t nodes_count = 3;

    // Make a list of storage buffers, each one holding a Node
    uint32_t storage_buffer_size = (4 * sizeof(float)) + sizeof(VkDeviceAddress);
    std::vector<vkt::Buffer> storage_buffers;
    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
    for (size_t i = 0; i < nodes_count; ++i) {
        // Last node's memory only holds 2 * sizeof(float), so writing to v.z is illegal
        if (i == nodes_count - 1) {
            storage_buffer_size = 2 * sizeof(float);
        }

        const VkDeviceAddress addr = storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).address();
        uniform_buffer_ptr[i] = addr;
    }

    uniform_buffer.memory().unmap();

    m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote values to all nodes but the last
    for (auto [buffer_i, buffer] : vvl::enumerate(storage_buffers.data(), nodes_count - 1)) {
        auto storage_buffer_ptr = static_cast<float *>(buffer->memory().map());

        ASSERT_EQ(storage_buffer_ptr[0], float(3 * buffer_i + 1));
        ASSERT_EQ(storage_buffer_ptr[1], float(3 * buffer_i + 2));
        ASSERT_EQ(storage_buffer_ptr[2], float(3 * buffer_i + 3));

        buffer->memory().unmap();
    }
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8089
TEST_F(NegativeGpuAVBufferDeviceAddress, DISABLED_ProxyStructLoad) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8073");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_scalar_block_layout : require
        #extension GL_EXT_buffer_reference2 : require

        struct RealCamera {
            vec4 frustum[6];
            mat4 viewProjection; // accessed but with large offset
        };
        layout(buffer_reference, scalar, buffer_reference_align = 4) restrict readonly buffer CameraBuffer {
            RealCamera camera;
        };

        layout(binding = 0, set = 0) buffer OutData {
            CameraBuffer cameraBuffer;
            mat4 in_mat;
            mat4 out_mat;
        };

        void foo() {
            out_mat += cameraBuffer.camera.viewProjection * in_mat;
        }

        void main() {
            restrict const RealCamera camera = cameraBuffer.camera;
            out_mat = camera.viewProjection * in_mat;
            foo();
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceAddress buffer_ptr = bda_buffer.address();
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    // One for each of the 2 access
    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ProxyStructLoadUint64) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8073");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_scalar_block_layout : require
        #extension GL_EXT_buffer_reference2 : require
        #extension GL_ARB_gpu_shader_int64 : require

        struct Test {
            float a;
        };

        layout(buffer_reference, std430, buffer_reference_align = 16) buffer TestBuffer {
            mat4 padding;
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

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceAddress buffer_ptr = bda_buffer.address();
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ProxyStructLoadBadAddress) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8073");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_scalar_block_layout : require
        #extension GL_EXT_buffer_reference2 : require
        #extension GL_ARB_gpu_shader_int64 : require

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

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceAddress buffer_ptr = bda_buffer.address() + 256;  // wrong
    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &buffer_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
