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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../layers/containers/range.h"

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
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.Address();
    // Will dereference the wrong ptr address
    VkDeviceAddress push_constants[2] = {u_info_ptr - 16, 4};
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants), push_constants);

    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the other 3 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.Memory().Map());
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
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
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.Address();
    // will go over a[4] by one
    VkDeviceAddress push_constants[2] = {u_info_ptr, 5};
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants), push_constants);

    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the first 4 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadBeforePointerDescriptor) {
    TEST_DESCRIPTION("Read 16 bytes before the valid pointer - use Descriptor to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer uniform_buffer(*m_device, 12, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.Address();
    // Will dereference the wrong ptr address
    VkDeviceAddress invalid_buffer_address = u_info_ptr - 16;
    uint32_t n_writes = 4;

    uint8_t *uniform_buffer_ptr = (uint8_t *)uniform_buffer.Memory().Map();
    memcpy(uniform_buffer_ptr, &invalid_buffer_address, sizeof(VkDeviceAddress));
    memcpy(uniform_buffer_ptr + sizeof(VkDeviceAddress), &n_writes, sizeof(uint32_t));

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
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the other 3 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.Memory().Map());
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ReadAfterPointerDescriptor) {
    TEST_DESCRIPTION("Read after the valid pointer - use Descriptor to set the value");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer uniform_buffer(*m_device, 12, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer buffer(*m_device, 64, 0, vkt::device_address);
    VkDeviceAddress u_info_ptr = buffer.Address();
    // will go over a[4] by one
    uint32_t n_writes = 5;

    uint8_t *uniform_buffer_ptr = (uint8_t *)uniform_buffer.Memory().Map();
    memcpy(uniform_buffer_ptr, &u_info_ptr, sizeof(VkDeviceAddress));
    memcpy(uniform_buffer_ptr + sizeof(VkDeviceAddress), &n_writes, sizeof(uint32_t));

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
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure we wrote the first 4 values
    auto *buffer_ptr = static_cast<uint32_t *>(buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*buffer_ptr, 42);
        buffer_ptr += 4;
    }
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

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Hold only 3 indices
    vkt::Buffer block_buffer(*m_device, 36, 0, vkt::device_address);
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
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
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl, vert_shader);
    GLSLtoSPV(m_device->Physical().limits_, VK_SHADER_STAGE_FRAGMENT_BIT, fs_source, frag_shader);

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
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.Address();
    const uint32_t n_reads = 64;  // way too large

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    memcpy(in_buffer_ptr + sizeof(VkDeviceAddress), &n_reads, sizeof(uint32_t));

    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds", 6);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ArrayOfStruct) {
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
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 32, 0, vkt::device_address);
    VkDeviceAddress block_ptr = block_buffer.Address();

    vkt::Buffer storage_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint8_t *buffer_ptr = (uint8_t *)storage_buffer.Memory().Map();
    const uint32_t index = 8;  // out of bounds
    memcpy(buffer_ptr, &index, sizeof(uint32_t));
    memcpy(buffer_ptr + (1 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));
    memcpy(buffer_ptr + (2 * sizeof(VkDeviceAddress)), &block_ptr, sizeof(VkDeviceAddress));

    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd140) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4]");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

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

    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    uniform_buffer_ptr[1] = 5;

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd140NumerousRanges) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4] - make sure it is detected even when there are numerous valid ranges");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

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

    // Get device address of buffer to write to
    using AddrRange = vvl::range<VkDeviceAddress>;
    const VkDeviceAddress storage_buffer_addr = storage_buffer.Address();
    const AddrRange shader_writes_range(storage_buffer_addr, storage_buffer_addr + storage_buffer_size + 4);

    // Create storage buffers for the sake of storing multiple device address ranges
    std::vector<vkt::Buffer> dummy_storage_buffers;
    for (int i = 0; i < 1024; ++i) {
        const VkDeviceAddress addr =
            dummy_storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).Address();
        const AddrRange addr_range(addr, addr + storage_buffer_size);
        // If new buffer address range overlaps with storage buffer range,
        // writes past its end may be valid, so remove dummy buffer
        if (shader_writes_range.intersects(addr_range)) {
            dummy_storage_buffers.resize(dummy_storage_buffers.size() - 1);
        }
    }

    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer_addr;
    uniform_buffer_ptr[1] = 5;  // Will provoke a 4 bytes write past buffer end

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(*storage_buffer_ptr, 42);
        storage_buffer_ptr += 4;
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd430) {
    TEST_DESCRIPTION("OOB read at u_info.data.a[4]");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    const uint32_t uniform_buffer_size = 8 + 4;  // 64 bits pointer + int
    vkt::Buffer uniform_buffer(*m_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

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

    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer.Address();
    uniform_buffer_ptr[1] = 5;

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written", 3);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote 42
    auto *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], 42);
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreRelaxedBlockLayout) {
    TEST_DESCRIPTION("OOB detected reading just past buffer end - use VK_KHR_relaxed_block_layout");
    AddRequiredExtensions(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // #version 450
    // #extension GL_EXT_buffer_reference : enable
    // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
    // layout(set = 0, binding = 0) uniform ufoo { bufStruct ptr; } ssbo;
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

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // only can fit 3 floats
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.Address();

    // Base buffer address is (storage_buffer_addr), so expect writing to `v.z` to cause an OOB access
    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer_addr;

    m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote to float
    auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.Memory().Map());
    ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreRelaxedBlockLayoutFront) {
    TEST_DESCRIPTION("OOB detected reading just front of buffer- use VK_KHR_relaxed_block_layout");
    AddRequiredExtensions(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // #version 450
    // #extension GL_EXT_buffer_reference : enable
    // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
    // layout(set = 0, binding = 0) uniform ufoo { bufStruct ptr; } ssbo;
    //
    // layout(buffer_reference, std430) buffer bufStruct {
    //     float f;
    //     vec3 v;
    // };
    // void main() {
    //     ssbo.ptr.f = 42.0;
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
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
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %ssbo %int_0
         %17 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %16
         %20 = OpAccessChain %_ptr_PhysicalStorageBuffer_float %17 %int_0
               OpStore %20 %float_42 Aligned 16
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

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // only can fit 3 floats
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.Address();

    // Base buffer address is (storage_buffer_addr - 16), so expect writing to `f` to cause an OOB access
    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    // The OpStore is aligned to 16 bytes, so need to substract by that
    uniform_buffer_ptr[0] = storage_buffer_addr - 16;

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
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

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // can only hold 3 floats, when SSBO uses 4
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.Address();

    // Base buffer address is (storage_buffer_addr), so expect writing to `v.z` to cause an OOB access
    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    uniform_buffer_ptr[0] = storage_buffer_addr;

    m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote to float
    auto *storage_buffer_ptr = static_cast<float *>(storage_buffer.Memory().Map());
    ASSERT_EQ(storage_buffer_ptr[0], 42.0f);
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreScalarBlockLayoutFront) {
    TEST_DESCRIPTION("OOB from front of buffer - use VK_EXT_scalar_block_layout");
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
        }
    )glsl";

    // Make a uniform buffer to be passed to the shader that contains the pointer
    const uint32_t uniform_buffer_size = 8;  // 64 bits pointers
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

    const uint32_t storage_buffer_size = 3 * sizeof(float);  // can only hold 3 floats, when SSBO uses 4
    vkt::Buffer storage_buffer(*m_device, storage_buffer_size, 0, vkt::device_address);
    const VkDeviceAddress storage_buffer_addr = storage_buffer.Address();

    // Base buffer address is (storage_buffer_addr - 16), so expect writing to `f` to cause an OOB access
    auto uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    // The OpStore is aligned to 16 bytes, so need to substract by that
    uniform_buffer_ptr[0] = storage_buffer_addr - 16;

    m_errorMonitor->SetDesiredError("Out of bounds access: 4 bytes written");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreStd430LinkedList) {
    TEST_DESCRIPTION("OOB writes in a linked list");
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

    constexpr size_t nodes_count = 3;

    // Make a list of storage buffers, each one holding a Node
    uint32_t storage_buffer_size = (4 * sizeof(float)) + sizeof(VkDeviceAddress);
    std::vector<vkt::Buffer> storage_buffers;
    auto *uniform_buffer_ptr = static_cast<VkDeviceAddress *>(uniform_buffer.Memory().Map());
    for (size_t i = 0; i < nodes_count; ++i) {
        // Last node's memory only holds 2 * sizeof(float), so writing to v.z is illegal
        if (i == nodes_count - 1) {
            storage_buffer_size = 2 * sizeof(float);
        }

        const VkDeviceAddress addr = storage_buffers.emplace_back(*m_device, storage_buffer_size, 0, vkt::device_address).Address();
        uniform_buffer_ptr[i] = addr;
    }

    m_errorMonitor->SetDesiredError("Out of bounds access: 12 bytes written");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Make sure shader wrote values to all nodes but the last
    for (auto [buffer_i, buffer] : vvl::enumerate(storage_buffers.data(), nodes_count - 1)) {
        auto storage_buffer_ptr = static_cast<float *>(buffer.Memory().Map());

        ASSERT_EQ(storage_buffer_ptr[0], float(3 * buffer_i + 1));
        ASSERT_EQ(storage_buffer_ptr[1], float(3 * buffer_i + 2));
        ASSERT_EQ(storage_buffer_ptr[2], float(3 * buffer_i + 3));
    }
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ProxyStructLoad) {
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

        void main() {
            restrict const RealCamera camera = cameraBuffer.camera;
            // glslang will do a load in the struct and OpCopyLogical what it wants after
            out_mat = camera.viewProjection * in_mat;

            // glslang will load just the mat4
            out_mat += cameraBuffer.camera.viewProjection * in_mat;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
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

    // One for each of the 2 access
    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ProxyStructLoad2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8073");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #extension GL_EXT_scalar_block_layout : require
        #extension GL_EXT_buffer_reference2 : require

        struct RealCamera {
            mat4 viewProjection; // [0, 63]
            vec4 frustum[6]; // [64, 160]
        };
        layout(buffer_reference, scalar, buffer_reference_align = 4) restrict readonly buffer CameraBuffer {
            RealCamera camera;
        };

        layout(binding = 0, set = 0) buffer OutData {
            CameraBuffer cameraBuffer;
            mat4 out_mat;
        };

        void main() {
            // While only the first 64 bytese are accessed, glslang loads the entire struct here and while the compiler may be smart, it is still possible it might load in the whole struct
            restrict const RealCamera camera = cameraBuffer.camera;
            out_mat = camera.viewProjection;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
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

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError(
        "This read corresponds to a full OpTypeStruct load. While not all members of the struct might be accessed, it is up to the "
        "source language or tooling to detect that and reflect it in the SPIR-V");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, ProxyStructLoadSlang) {
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Note this was with Slang 2025.2.2 but trying 2025.6.2 it is smart enough to only load a single int, but test as it is still
    // plausable SPIR-V to seee struct Foo {
    //     uint x;
    //     uint y; // never used
    // }
    // RWStructuredBuffer<uint> result;
    // struct Data{
    //     Foo* node;
    // };
    // [[vk::push_constant]] Data pc;
    // [shader("compute")]
    // void main(uint3 threadId : SV_DispatchThreadID) {
    //     Foo a = pc.node[0];
    //     result[0] = a.x;
    // }
    char const *shader_source = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %pc %result
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_ptr_PhysicalStorageBuffer_Foo_natural ArrayStride 8
               OpDecorate %Data_std430 Block
               OpMemberDecorate %Data_std430 0 Offset 0
               OpMemberDecorate %Foo_natural 0 Offset 0
               OpMemberDecorate %Foo_natural 1 Offset 4
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %result Binding 0
               OpDecorate %result DescriptorSet 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
         %12 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Foo_natural PhysicalStorageBuffer
%Data_std430 = OpTypeStruct %_ptr_PhysicalStorageBuffer_Foo_natural
%_ptr_PushConstant_Data_std430 = OpTypePointer PushConstant %Data_std430
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_52 = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_Foo_natural
%Foo_natural = OpTypeStruct %uint %uint
    %uint_15 = OpConstant %uint 15
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_PhysicalStorageBuffer_Foo_natural = OpTypePointer PhysicalStorageBuffer %Foo_natural
         %pc = OpVariable %_ptr_PushConstant_Data_std430 PushConstant
     %result = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
%_ptr_Function_uint = OpTypePointer Function %uint
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %12
         %13 = OpLabel
        %113 = OpVariable %_ptr_Function_uint Function
        %112 = OpVariable %_ptr_Function_uint Function
         %58 = OpAccessChain %_ptr_PushConstant_52 %pc %int_0
         %59 = OpLoad %_ptr_PhysicalStorageBuffer_Foo_natural %58
         %60 = OpPtrAccessChain %_ptr_PhysicalStorageBuffer_Foo_natural %59 %int_0
        %a_0 = OpLoad %Foo_natural %60 Aligned 4
         %96 = OpCompositeExtract %uint %a_0 0
         %97 = OpCompositeExtract %uint %a_0 1
               OpStore %112 %96
               OpStore %113 %97
         %75 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_0
               OpStore %75 %96
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 4, 0, vkt::device_address);
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
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
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
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
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress buffer_ptr = bda_buffer.Address() + 256;  // wrong
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, StoreAlignment) {
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) readonly buffer BlockBuffer {
            uvec4 data; // aligned to 16
        };

        layout(set = 0, binding = 0) uniform Input {
            BlockBuffer ptr;
        };

        void main() {
            ptr.data = uvec4(0.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 256, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();
    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = block_ptr + 4;

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-PhysicalStorageBuffer64-06315");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, LoadAlignment) {
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) readonly buffer BlockBuffer {
            uvec4 data; // aligned to 16
        };

        layout(set = 0, binding = 0) buffer Input {
            BlockBuffer ptr;
            uvec4 out_data;
        };

        void main() {
            out_data = ptr.data;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 256, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();
    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = block_ptr + 4;

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-PhysicalStorageBuffer64-06315");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, NonStructPointer) {
    TEST_DESCRIPTION("Slang allows BDA pointers to be with POD instead of a struct");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // Slang code
    // uniform uint* data_ptr; // only 256 bytes
    // [numthreads(1,1,1)]
    // void computeMain() {
    //    data_ptr[64] = 999;
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
    %int_64 = OpConstant %int 64
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
         %37 = OpPtrAccessChain %_ptr_PhysicalStorageBuffer_uint %36 %int_64
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, MultipleAccessChains) {
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
    //    uint a = bda->payload[0] * bda->payload[15];
    //    bda->x = a;
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
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
     %int_15 = OpConstant %int 15
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
         %30 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %29 %int_15
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, OpCopyObject) {
    TEST_DESCRIPTION("Use a OpCopyObject before the BDA OpLoad");
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // layout(buffer_reference) buffer BDA { uint a; uint b; };
    // layout(set = 0, binding = 0) uniform Uniforms { BDA ptr; };
    // void main() {
    //     ptr.a = ptr.b;
    // }
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %Uniforms Block
               OpMemberDecorate %Uniforms 0 Offset 0
               OpDecorate %BDA Block
               OpMemberDecorate %BDA 0 Offset 0
               OpMemberDecorate %BDA 1 Offset 4
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %bda_ptr PhysicalStorageBuffer
   %Uniforms = OpTypeStruct %bda_ptr
       %uint = OpTypeInt 32 0
        %BDA = OpTypeStruct %uint %uint
    %bda_ptr = OpTypePointer PhysicalStorageBuffer %BDA
%_ptr_Uniform_Uniforms = OpTypePointer Uniform %Uniforms
          %_ = OpVariable %_ptr_Uniform_Uniforms Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%uniform_ptr = OpTypePointer Uniform %bda_ptr
      %int_1 = OpConstant %int 1
%bda_uint_ptr = OpTypePointer PhysicalStorageBuffer %uint
       %main = OpFunction %void None %4
          %6 = OpLabel
         %16 = OpAccessChain %uniform_ptr %_ %int_0
         %17 = OpLoad %bda_ptr %16
         %18 = OpAccessChain %uniform_ptr %_ %int_0
         %19 = OpLoad %bda_ptr %18
  %load_copy = OpCopyObject %bda_ptr %19
         %22 = OpAccessChain %bda_uint_ptr %load_copy %int_1
    %ac_copy = OpCopyObject %bda_uint_ptr %22
         %23 = OpLoad %uint %ac_copy Aligned 4
         %24 = OpAccessChain %bda_uint_ptr %17 %int_0
               OpStore %24 %23 Aligned 16
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 4, 0, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    auto ubo_buffer_ptr = static_cast<VkDeviceAddress *>(ubo_buffer.Memory().Map());
    ubo_buffer_ptr[0] = bda_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, ubo_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, MemoryModelOperand) {
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
            coherent blockType b = node;
            b.x[16] = 2; // invalid store

            volatile blockType b2 = node;
            b2.x[32] = 3; // invalid store
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);  // too small
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, MemoryModelOperand2) {
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
                bool skip2 = node.x[128] == 0; // invalid load
                sharedSkip = skip && skip2;
            }
            skip = sharedSkip;
            if (!skip) {
                b = 1;
            }
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicLoad) {
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
        };

        void main() {
            uint a = atomicLoad(node.x[16], gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            node.x[0] = a;
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);  // too small
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicStore) {
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
        };

        void main() {
            atomicStore(node.x[16], 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);  // too small
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicExchange) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 460
        #pragma use_vulkan_memory_model
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer Node { uint x[]; };
        layout(set=0, binding=0) buffer SSBO {
            Node node;
        };

        void main() {
            // [16] will be atomic
            // [20] will be a normal OpLoad that also is invalid
            atomicExchange(node.x[16], node.x[20]);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);  // too small
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicAddValueOperand) {
    TEST_DESCRIPTION("Use BDA of the Value operand of Atomics");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer SSBO {
            uvec4 padding;
            uint x;
        };

        layout(set = 0, binding = 0) buffer SSBO_IN {
            SSBO a;
            uint b;
        };
        void main() {
            atomicAdd(b, a.x);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicAddPointerOperand) {
    TEST_DESCRIPTION("Use BDA of the Pointer operand of Atomics");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer SSBO {
            uvec4 padding;
            uint x;
        };

        layout(set = 0, binding = 0) buffer SSBO_IN {
            SSBO a;
        };
        void main() {
            atomicAdd(a.x, 1);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicsMaxMin) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer SSBO {
            uvec4 padding;
            uint x; // all invalid from here
            uint y;
            uint z;
        };

        layout(set = 0, binding = 0) buffer SSBO_IN {
            SSBO a;
        };
        void main() {
            atomicMax(a.x, a.y); // each produces 2 error
            atomicMin(a.x, a.z);
        }
    )glsl";

    vkt::Buffer bda_buffer(*m_device, 16, 0, vkt::device_address);
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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, PieceOfDataPointer) {
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
    //     result[0] = pc.node[64];
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
     %int_64 = OpConstant %int 64
     %result = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
         %pc = OpVariable %_ptr_PushConstant_Data_std430 PushConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_0
         %19 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint %pc %int_0
         %20 = OpLoad %_ptr_PhysicalStorageBuffer_uint %19
         %21 = OpPtrAccessChain %_ptr_PhysicalStorageBuffer_uint %20 %int_64
         %23 = OpLoad %uint %21 Aligned 4
               OpStore %9 %23
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, AtomicExchangeSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // struct Foo {
    //     uint pad_0;
    //     float3 pad_1;
    //     uint* a; // offset 48 (16 + 32)
    //     uint* b;
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

    vkt::Buffer bda_buffer(*m_device, 256, 0, vkt::device_address);
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
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, PieceOfDataPointerInStruct) {
    TEST_DESCRIPTION("Slang can have a BDA pointer of a int that is not wrapped in a struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUBufferDeviceAddress());

    // RWStructuredBuffer<uint> result;
    //
    // struct Bar {
    //   uint z;
    //   uint y;
    // }
    // struct Foo {
    //     uint pad_0;
    //     float3 pad_1;
    //     uint* a; // offset 48 (16 + 32)
    //     Bar* b; // also invalid, adds another indirection
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
    //     result[1] = pc.node.b->z;
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
               OpDecorate %_ptr_PhysicalStorageBuffer_Bar_natural ArrayStride 8
               OpMemberDecorate %Foo_std430 0 Offset 0
               OpMemberDecorate %Foo_std430 1 Offset 16
               OpMemberDecorate %Foo_std430 2 Offset 32
               OpMemberDecorate %Foo_std430 3 Offset 40
               OpDecorate %Data_std430 Block
               OpMemberDecorate %Data_std430 0 Offset 0
               OpMemberDecorate %Data_std430 1 Offset 16
               OpMemberDecorate %Bar_natural 0 Offset 0
               OpMemberDecorate %Bar_natural 1 Offset 4
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
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Bar_natural PhysicalStorageBuffer
 %Foo_std430 = OpTypeStruct %uint %v3float %_ptr_PhysicalStorageBuffer_uint %_ptr_PhysicalStorageBuffer_Bar_natural
%Data_std430 = OpTypeStruct %v4float %Foo_std430
%_ptr_PushConstant_Data_std430 = OpTypePointer PushConstant %Data_std430
      %int_1 = OpConstant %int 1
%_ptr_PushConstant_Foo_std430 = OpTypePointer PushConstant %Foo_std430
      %int_2 = OpConstant %int 2
%_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_uint
      %int_3 = OpConstant %int 3
%_ptr_PushConstant_21 = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_Bar_natural
%Bar_natural = OpTypeStruct %uint %uint
%_ptr_PhysicalStorageBuffer_Bar_natural = OpTypePointer PhysicalStorageBuffer %Bar_natural
     %result = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
         %pc = OpVariable %_ptr_PushConstant_Data_std430 PushConstant
       %main = OpFunction %void None %3
          %4 = OpLabel

          %9 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_0
         %26 = OpAccessChain %_ptr_PushConstant_Foo_std430 %pc %int_1
         %29 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_uint %26 %int_2
         %30 = OpLoad %_ptr_PhysicalStorageBuffer_uint %29
         %31 = OpLoad %uint %30 Aligned 4
               OpStore %9 %31

         %33 = OpAccessChain %_ptr_StorageBuffer_uint %result %int_0 %int_1
         %36 = OpAccessChain %_ptr_PushConstant_21 %26 %int_3
         %37 = OpLoad %_ptr_PhysicalStorageBuffer_Bar_natural %36
         %38 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %37 %int_0
         %39 = OpLoad %uint %38 Aligned 4
               OpStore %33 %39

               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer bda_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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
    VkDeviceAddress bda_buffer_addr = 0;  // invalid nullptr
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 48, sizeof(VkDeviceAddress),
                         &bda_buffer_addr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, PointerChainLastInvalid) {
    TEST_DESCRIPTION("Have BDA point to more BDA creating a chain, and the last pointer is bad");
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

        layout(set = 0, binding = 0) uniform UBO_IN {
            SSBO_B b;
        };

        void main() {
           b.a.x = 42;
        }
    )glsl";
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer ssbo_a_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ssbo_b_buffer(*m_device, 64, 0, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = (VkDeviceAddress *)in_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_b_buffer.Address();

    buffer_ptr = (VkDeviceAddress *)ssbo_b_buffer.Memory().Map();
    buffer_ptr[0] = 0xffffffffffffff00;  // bad pointer

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, PointerChainFirstInvalid) {
    TEST_DESCRIPTION("Have BDA point to more BDA creating a chain, and the first pointer is bad");
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

        layout(set = 0, binding = 0) uniform UBO_IN {
            SSBO_B b;
        };

        void main() {
           b.a.x = 42;
        }
    )glsl";
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer ssbo_a_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ssbo_b_buffer(*m_device, 64, 0, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    pipe.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = (VkDeviceAddress *)in_buffer.Memory().Map();
    buffer_ptr[0] = 0xffffffffffffff00;  // bad pointer

    buffer_ptr = (VkDeviceAddress *)ssbo_b_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_a_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // Because 'a' is null after detect 'b' is bad and causes a 2nd OOB access
    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVBufferDeviceAddress, PointerChainFirstInvalidAtomic) {
    TEST_DESCRIPTION("Have BDA point to more BDA creating a chain, and the first pointer is bad");
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

        layout(set = 0, binding = 0) buffer UBO_IN {
            SSBO_B b;
            uint z;
        };

        void main() {
            z = atomicAdd(b.a.x, 1u);
        }
    )glsl";
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer ssbo_a_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ssbo_b_buffer(*m_device, 64, 0, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    auto buffer_ptr = (VkDeviceAddress *)in_buffer.Memory().Map();
    buffer_ptr[0] = 0xffffffffffffff00;  // bad pointer

    buffer_ptr = (VkDeviceAddress *)ssbo_b_buffer.Memory().Map();
    buffer_ptr[0] = ssbo_a_buffer.Address();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // Because 'a' is null after detect 'b' is bad and causes a 2nd OOB access
    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
