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

class PositiveGpuAVOOB : public GpuAVTest {};

TEST_F(PositiveGpuAVOOB, Basic) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    AddDisabledFeature(vkt::Feature::robustBufferAccess2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 1) {
                Data.data[0] = Null.data[40];
            }
            else if (u_index.index[0] == 2) {
                imageStore(s_buffer, 0, x);
            }
            else if (u_index.index[0] == 3) {
                x = imageLoad(s_buffer, 0);
            }
        }
    )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
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

    uint32_t *data = (uint32_t *)offset_buffer.memory().map();
    *data = 1;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.memory().map();
    *data = 2;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.memory().map();
    *data = 3;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, ImageLoadStoreTexelFetch) {
    TEST_DESCRIPTION("index into an image Load, Store, and texelFetch");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) uniform samplerBuffer u_buffer; // texel_buffer[5]
        layout(set = 0, binding = 1, r32f) uniform imageBuffer s_buffer;  // texel_buffer[6]

        void main() {
            vec4 x = texelFetch(u_buffer, 4);
            x *= imageLoad(s_buffer, 5);
           imageStore(s_buffer, 5, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_texel_buffer(*m_device, 20, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    vkt::Buffer storage_texel_buffer(*m_device, 24, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, mem_props);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferView(0, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, AtomicImageLoadStore) {
    TEST_DESCRIPTION("index into an atomic image Load and Store");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // Not sure if anyone actually support buffer texel on atomic
    if ((m_device->FormatFeaturesBuffer(VK_FORMAT_R32_SFLOAT) & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) == 0) {
        GTEST_SKIP() << "No atomic texel buffer support";
    }

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_atomic_float : enable

        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            float x = imageAtomicLoad(s_buffer, 3, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
            imageAtomicStore(s_buffer, 3, x, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferView(0, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, GPL) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    AddDisabledFeature(vkt::Feature::robustBufferAccess2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, reqs);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferView(4, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 1) {
                Data.data[0] = Null.data[40];
            }
            else if (u_index.index[0] == 1) {
                imageStore(s_buffer, 0, x);
            }
            else if (u_index.index[0] == 2) {
                x = imageLoad(s_buffer, 0);
            }
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vs_source);

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    uint32_t *data = (uint32_t *)offset_buffer.memory().map();
    *data = 1;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.memory().map();
    *data = 2;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.memory().map();
    *data = 3;
    offset_buffer.memory().unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, GPLNonInlined) {
    TEST_DESCRIPTION("Make sure GPL works when shader modules are not inlined at pipeline creation time with valid GPU-AV code");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t *offset_buffer_ptr = (uint32_t *)offset_buffer.memory().map();
    *offset_buffer_ptr = 8;
    offset_buffer.memory().unmap();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Uniform { uint offset_buffer[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint write_buffer[]; };
        void main() {
            uint index = offset_buffer[0];
            write_buffer[index] = 0xdeadca71;
        }
    )glsl";
    // Create VkShaderModule to pass in
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    CreatePipelineHelper pre_raster_lib(*this);
    {
        pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
        pre_raster_lib.vp_state_ci_.pViewports = &viewport;
        pre_raster_lib.vp_state_ci_.pScissors = &scissor;
        pre_raster_lib.gp_ci_.layout = pipeline_layout.handle();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
        frag_shader_lib.gp_ci_.layout = pipeline_layout.handle();
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout.handle();
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, GPLFragmentIndependentSets) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // Independent sets
    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto vs_layout = pipeline_layout_vs.handle();
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto fs_layout = pipeline_layout_fs.handle();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.memory().unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.memory().unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pre_raster_lib(*this);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.vp_state_ci_.pViewports = &viewport;
    pre_raster_lib.vp_state_ci_.pScissors = &scissor;
    pre_raster_lib.gp_ci_.layout = vs_layout;
    pre_raster_lib.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper frag_shader_lib(*this);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib.gp_ci_.layout = fs_layout;
    frag_shader_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline pipe(*m_device, exe_pipe_ci);

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, VertexFragmentMultiEntrypoint) {
    TEST_DESCRIPTION("Same as negative test, but buffer are large enough");
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, 256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer storage_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // layout(set = 0, binding = 0) uniform ufoo { uint index[]; };
    // layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
    // layout(location = 0) out vec4 c_out;
    // void vert_main() {
    //     data[0] = index[4];
    //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    // }
    // void frag_main() {
    //     data[4] = index[0];
    // }
    const char *shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %c_out
               OpEntryPoint Vertex %vert_main "vert_main" %2 %gl_VertexIndex
               OpExecutionMode %frag_main OriginUpperLeft
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
               OpDecorate %_arr_uint_uint_5 ArrayStride 16
               OpMemberDecorate %_struct_10 0 Offset 0
               OpDecorate %_struct_10 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %c_out Location 0
               OpMemberDecorate %_struct_12 0 BuiltIn Position
               OpMemberDecorate %_struct_12 1 BuiltIn PointSize
               OpMemberDecorate %_struct_12 2 BuiltIn ClipDistance
               OpMemberDecorate %_struct_12 3 BuiltIn CullDistance
               OpDecorate %_struct_12 Block
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
  %_struct_7 = OpTypeStruct %_runtimearr_uint
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
          %8 = OpVariable %_ptr_Uniform__struct_7 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
 %_struct_10 = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %11 = OpVariable %_ptr_Uniform__struct_10 Uniform
      %int_4 = OpConstant %int 4
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
 %_struct_12 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output__struct_12 = OpTypePointer Output %_struct_12
          %2 = OpVariable %_ptr_Output__struct_12 Output
    %v2float = OpTypeVector %float 2
     %uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %float_n1 = OpConstant %float -1
         %32 = OpConstantComposite %v2float %float_n1 %float_n1
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v2float %float_1 %float_n1
    %float_0 = OpConstant %float 0
         %36 = OpConstantComposite %v2float %float_0 %float_1
         %37 = OpConstantComposite %_arr_v2float_uint_3 %32 %34 %36
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
      %int_3 = OpConstant %int 3
%_ptr_Function__arr_v2float_uint_3 = OpTypePointer Function %_arr_v2float_uint_3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %c_out = OpVariable %_ptr_Output_v4float Output
          %vert_main = OpFunction %void None %14
         %43 = OpLabel
         %44 = OpVariable %_ptr_Function__arr_v2float_uint_3 Function
         %45 = OpAccessChain %_ptr_Uniform_uint %11 %int_0 %int_4
         %46 = OpLoad %uint %45
         %47 = OpAccessChain %_ptr_Uniform_uint %8 %int_0 %int_0
               OpStore %47 %46
         %48 = OpLoad %int %gl_VertexIndex
         %49 = OpSMod %int %48 %int_3
               OpStore %44 %37
         %50 = OpAccessChain %_ptr_Function_v2float %44 %49
         %51 = OpLoad %v2float %50
         %52 = OpCompositeExtract %float %51 0
         %53 = OpCompositeExtract %float %51 1
         %54 = OpCompositeConstruct %v4float %52 %53 %float_0 %float_1
         %55 = OpAccessChain %_ptr_Output_v4float %2 %int_0
               OpStore %55 %54
               OpReturn
               OpFunctionEnd
          %frag_main = OpFunction %void None %14
         %56 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_uint %11 %int_0 %int_0
         %58 = OpLoad %uint %57
         %59 = OpAccessChain %_ptr_Uniform_uint %8 %int_0 %int_4
               OpStore %59 %58
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "vert_main");
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "frag_main");

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, PartialBoundDescriptorSSBO) {
    TEST_DESCRIPTION("Only bound part of a SSBO, but only use that part so it is still valid");
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, PartialBoundDescriptorBuffer) {
    TEST_DESCRIPTION("Make large enough buffer, but only bound part of it to the SSBO that is used");
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    // only half of the buffer that is needed is
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, PartialBoundDescriptorCopy) {
    TEST_DESCRIPTION("Copy the partial bound buffer the descriptor that is used");
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    OneOffDescriptorSet descriptor_set_src(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set_src.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_src.UpdateDescriptorSets();

    OneOffDescriptorSet descriptor_set_dst(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_dst.layout_});

    VkCopyDescriptorSet descriptor_copy = vku::InitStructHelper();
    descriptor_copy.srcSet = descriptor_set_src.set_;
    descriptor_copy.srcBinding = 0;
    descriptor_copy.dstSet = descriptor_set_dst.set_;
    descriptor_copy.dstBinding = 0;
    descriptor_copy.dstArrayElement = 0;
    descriptor_copy.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_dst.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVOOB, TexelFetchArray) {
    TEST_DESCRIPTION("index into texelFetch OOB for an array of TexelBuffers");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
            vec4 b;
        } out_buffer;

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer[2];

        void main() {
            out_buffer.a = texelFetch(u_buffer[0], 4); // valid
            // Will not ever get into this check
            if (gl_WorkGroupID.x == 987) {
                out_buffer.b = texelFetch(u_buffer[1], 4); // invalid
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer out_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    vkt::Buffer uniform_texel_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView full_buffer_view(*m_device, bvci);
    bvci.range = 16;  // only fills 4, but are accessing index[4]
    vkt::BufferView partial_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, full_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, partial_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
