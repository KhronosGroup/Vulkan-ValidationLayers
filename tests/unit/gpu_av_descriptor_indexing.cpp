/*
 * Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
 * Copyright (c) 2020-2023 Google, Inc.
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

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBGraphics) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                                       &layout_createinfo_binding_flags,
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    uint32_t desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &desc_counts;

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                },
                                                0, &layout_createinfo_binding_flags, 0, &variable_count);
    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[6] = {};
    for (int i = 0; i < 6; i++) {
        image_info[i] = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, nullptr);
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, nullptr);

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    // Resources for buffer tests
    OneOffDescriptorSet descriptor_set_buffer(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr},
                                              },
                                              0, &layout_createinfo_binding_flags, 0);

    const vkt::PipelineLayout pipeline_layout_buffer(*m_device, {&descriptor_set_buffer.layout_});

    VkDescriptorBufferInfo buffer_test_buffer_info[7] = {};
    buffer_test_buffer_info[0].buffer = buffer0.handle();
    buffer_test_buffer_info[0].offset = 0;
    buffer_test_buffer_info[0].range = sizeof(uint32_t);

    for (int i = 1; i < 7; i++) {
        buffer_test_buffer_info[i].buffer = buffer1.handle();
        buffer_test_buffer_info[i].offset = 0;
        buffer_test_buffer_info[i].range = 4 * sizeof(float);
    }

    VkWriteDescriptorSet buffer_descriptor_writes[2] = {};
    buffer_descriptor_writes[0] = vku::InitStructHelper();
    buffer_descriptor_writes[0].dstSet = descriptor_set_buffer.set_;  // descriptor_set;
    buffer_descriptor_writes[0].dstBinding = 0;
    buffer_descriptor_writes[0].descriptorCount = 1;
    buffer_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    buffer_descriptor_writes[0].pBufferInfo = buffer_test_buffer_info;
    buffer_descriptor_writes[1] = vku::InitStructHelper();
    buffer_descriptor_writes[1].dstSet = descriptor_set_buffer.set_;  // descriptor_set;
    buffer_descriptor_writes[1].dstBinding = 1;
    buffer_descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    buffer_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_writes[1].pBufferInfo = &buffer_test_buffer_info[1];
    vk::UpdateDescriptorSets(m_device->device(), 2, buffer_descriptor_writes, 0, nullptr);

    // Shader programs for array OOB test in vertex stage:
    // - The vertex shader fetches the invalid index from the uniform buffer and uses it to make an invalid index into another
    // array.
    char const *vsSource_vert = R"glsl(
        #version 450

        layout(std140, set = 0, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));
        }
        )glsl";
    char const *fsSource_vert = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[0], vec2(0, 0));
        }
        )glsl";

    // Shader programs for array OOB test in fragment stage:
    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vsSource_frag = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fsSource_frag = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    char const *fsSource_frag_runtime = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    char const *fsSource_buffer = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) buffer foo { vec4 val; } colors[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = colors[index].val;
        }
        )glsl";
    char const *gsSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(triangles) in;
        layout(triangle_strip, max_vertices=3) out;
        layout(location=0) in VertexData { vec4 x; } gs_in[];
        layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;
        layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];
        void main() {
           gl_Position = gs_in[0].x + adds[uniform_index_buffer.index].val.x;
           EmitVertex();
        }
        )glsl";
    static const char vsSourceForGS[] = R"glsl(
        #version 450
        layout(location=0) out foo {vec4 val;} gs_out[3];
        void main() {
           gs_out[0].val = vec4(0);
           gl_Position = vec4(1);
        }
        )glsl";
    static const char *tesSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;
        layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];
        layout(triangles, equal_spacing, cw) in;
        void main() {
            gl_Position = adds[uniform_index_buffer.index].val;
        }
        )glsl";

    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        char const *geometry_source;
        char const *tess_ctrl_source;
        char const *tess_eval_source;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    // TODO - These errors only show with EShMsgDebugInfo - need to figure out if suppose to be here
    // tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 25,
    //                  "gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));"});
    // tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 25,
    //                  "uFragColor = texture(tex[index], vec2(0, 0));"});

    tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, &pipeline_layout, &descriptor_set, 5,
                     "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
    // Pick 6 below because it is less than the maximum specified, but more than the actual specified
    tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, &pipeline_layout_variable,
                     &descriptor_set_variable, 6, "(set = 0, binding = 1) Index of 6 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, &pipeline_layout_variable,
                     &descriptor_set_variable, 5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
    tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, &pipeline_layout_buffer, &descriptor_set_buffer, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, &pipeline_layout_buffer, &descriptor_set_buffer, 5,
                     "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
    if (m_device->phy().features().geometryShader) {
        // OOB Geometry
        tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 25, "UNASSIGNED-Descriptor index out of bounds"});
        // Uninitialized Geometry
        tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 5, "VUID-vkCmdDraw-None-08114"});
    }
    if (m_device->phy().features().tessellationShader) {
        tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource,
                         &pipeline_layout_buffer, &descriptor_set_buffer, 25, "UNASSIGNED-Descriptor index out of bounds"});
        tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource,
                         &pipeline_layout_buffer, &descriptor_set_buffer, 5, "VUID-vkCmdDraw-None-08114"});
    }

    for (const auto &iter : tests) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT);
        VkShaderObj *gs = nullptr;
        VkShaderObj *tcs = nullptr;
        VkShaderObj *tes = nullptr;

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
        if (iter.geometry_source) {
            gs = new VkShaderObj(this, iter.geometry_source, VK_SHADER_STAGE_GEOMETRY_BIT);
            pipe.shader_stages_.push_back(gs->GetStageCreateInfo());
        }
        VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                     VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
        VkPipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, VK_NULL_HANDLE,
            VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT};

        VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                                                   &tessellationDomainOriginStateInfo, 0, 3};
        if (iter.tess_ctrl_source && iter.tess_eval_source) {
            tcs = new VkShaderObj(this, iter.tess_ctrl_source, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
            tes = new VkShaderObj(this, iter.tess_eval_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
            pipe.shader_stages_.push_back(tcs->GetStageCreateInfo());
            pipe.shader_stages_.push_back(tes->GetStageCreateInfo());
            pipe.gp_ci_.pTessellationState = &tsci;
            pipe.gp_ci_.pInputAssemblyState = &iasci;
        }

        pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
        pipe.gp_ci_.layout = iter.pipeline_layout->handle();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, iter.pipeline_layout->handle(), 0, 1,
                                  &iter.descriptor_set->set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        uint32_t *data = (uint32_t *)buffer0.memory().map();
        data[0] = iter.index;
        buffer0.memory().unmap();

        m_default_queue->submit(*m_commandBuffer, false);
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();
        delete gs;
        delete tcs;
        delete tes;
    }

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer {
            uint data;
        } Data[];
        void main() {
            Data[(u_index.index - 1)].data = Data[u_index.index].data;
        }
        )glsl";

    VkShaderObj shader_module(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo stage = vku::InitStructHelper();
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout_buffer.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = stage;

    VkPipeline c_pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &hinfo;

    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_buffer.handle(), 0, 1,
                              &descriptor_set_buffer.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    // Uninitialized
    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 5;
    buffer0.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08114");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    // Out of Bounds
    data = (uint32_t *)buffer0.memory().map();
    data[0] = 25;
    buffer0.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-Descriptor index out of bounds");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    vk::DestroyPipeline(m_device->handle(), c_pipeline, nullptr);
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlyDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler.handle(), image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 1;
    buffer0.memory().unmap();

    // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
    // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
    // longer does QueueWaitIdle() in each submit call, we should also be able to detect
    // resource destruction while a submission is blocked on a semaphore as well.
    image.destroy();

    // UNASSIGNED-Descriptor destroyed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed.");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlySamplerDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler.handle(), image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
         )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
         )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 1;
    buffer0.memory().unmap();

    // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
    // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
    // longer does QueueWaitIdle() in each submit call, we should also be able to detect
    // resource destruction while a submission is blocked on a semaphore as well.
    sampler.destroy();

    // UNASSIGNED-Descriptor destroyed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed.");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ImageArrayDynamicIndexing) {
    TEST_DESCRIPTION("GPU validation: test that only dynamically used indices are validated");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 40;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
               VK_IMAGE_TILING_LINEAR);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler.handle(), image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorCount = kDescCount;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;

    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 35;
    buffer0.memory().unmap();

    // UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.");

    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, UpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    AddDisabledFeature(vkt::Feature::descriptorBindingUniformBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDescriptorBindingFlags flags[3] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    // Descriptor set has two bindings - only the second is update_after_bind
    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes[3] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
        {binding[2].descriptorType, binding[2].descriptorCount},
    };
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    dspci.poolSizeCount = 3;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);

    vkt::Buffer dynamic_uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer.handle();
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;

    VkWriteDescriptorSet descriptor_write[2] = {};
    descriptor_write[0] = vku::InitStructHelper();
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = buffInfo;
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 1;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    // Create a dummy pipeline, since VL inspects which bindings are actually used at draw time
    char const *fs_source = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform foo0 { float x0; } bar0;
        layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;
        layout(set=0, binding=2) buffer  foo2 { float x2; } bar2;
        void main(){
           color = vec4(bar0.x0 + bar1.x1 + bar2.x2);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // Make both bindings valid before binding to the command buffer
    vk::UpdateDescriptorSets(m_device->device(), 2, &descriptor_write[0], 0, nullptr);

    m_commandBuffer->begin();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds, 0,
                              nullptr);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    // Valid to update binding 1 after being bound
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[1], 0, nullptr);

    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08114");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, UpdateAfterBindImageViewTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}},
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &flags_create_info,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewType-07752");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, BasicHLSL) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 5;  // go past textures[4]
    buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[4] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
     %uint_4 = OpConstant %uint 4
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_4 = OpTypeArray %type_2d_image %uint_4
%_ptr_UniformConstant__arr_type_2d_image_uint_4 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_4
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_4 UniformConstant
       %main = OpFunction %void None %27
         %31 = OpLabel
         %32 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %33 = OpLoad %uint %32
         %34 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %33
         %35 = OpLoad %type_2d_image %34
         %36 = OpLoad %type_sampler %ss
         %37 = OpSampledImage %type_sampled_image %35 %36
         %38 = OpImageSampleExplicitLod %v4float %37 %18 Lod %float_0
         %39 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %39 %38
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-Descriptor index out of bounds");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, BasicHLSLRuntimeArray) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL via runtime array");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);
    // send index to select in image array
    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 7;
    buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    // indexing into textures[7], but don't fill it
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_EXT_descriptor_indexing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_type_2d_image = OpTypeRuntimeArray %type_2d_image
%_ptr_UniformConstant__runtimearr_type_2d_image = OpTypePointer UniformConstant %_runtimearr_type_2d_image
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__runtimearr_type_2d_image UniformConstant
       %main = OpFunction %void None %26
         %30 = OpLabel
         %31 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %32 = OpLoad %uint %31
         %33 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %32
         %34 = OpLoad %type_2d_image %33
         %35 = OpLoad %type_sampler %ss
         %36 = OpSampledImage %type_sampled_image %34 %35
         %37 = OpImageSampleExplicitLod %v4float %36 %18 Lod %float_0
         %38 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08114");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, PushConstant) {
    TEST_DESCRIPTION("Basic indexing from a push constant");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    std::vector<VkPushConstantRange> push_constant_ranges = {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)}};
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, push_constant_ranges);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(push_constant) uniform Input {
            uint index;
        } in_buffer;

        layout(set = 0, binding = 0) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();

    uint32_t index = 8;
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t),
                         &index);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-Descriptor index out of bounds");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleIndexes) {
    TEST_DESCRIPTION("Mis-index multiple times");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    // send index to select in image array
    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 3;
    data[1] = 0;  // valid
    data[2] = 5;
    buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index_0;
            uint index_1;
            uint index_2;
        } in_buffer;

        layout(set = 0, binding = 1) uniform sampler2D tex[];

        void main() {
           vec2 uv = vec2(0, 0);
           vec4 result = texture(tex[in_buffer.index_0], uv);
           result += texture(tex[in_buffer.index_1], uv); // valid
           result += texture(tex[in_buffer.index_2], uv);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-Descriptor index out of bounds");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}