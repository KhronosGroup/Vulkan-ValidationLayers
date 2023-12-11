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

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    bool descriptor_indexing = IsExtensionsEnabled(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    VkPhysicalDeviceMaintenance4Features maintenance4_features = vku::InitStructHelper();
    maintenance4_features.maintenance4 = true;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&maintenance4_features);
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    if (descriptor_indexing) {
        maintenance4_features.pNext = &indexing_features;
        GetPhysicalDeviceFeatures2(features2);

        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
            !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
            !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
            GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
        }
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        layout_createinfo_binding_flags[0] = vku::InitStructHelper();
        layout_createinfo_binding_flags[0].bindingCount = 2;
        layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
        layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        layout_pnext = layout_createinfo_binding_flags;
    }

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       layout_create_flags, layout_pnext, pool_create_flags);

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    uint32_t desc_counts;
    if (descriptor_indexing) {
        layout_create_flags = 0;
        pool_create_flags = 0;
        ds_binding_flags[1] =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
        variable_count = vku::InitStructHelper();
        variable_count.descriptorSetCount = 1;
        variable_count.pDescriptorCounts = &desc_counts;
        allocate_pnext = &variable_count;
    }

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                },
                                                layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
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
    if (descriptor_indexing)
        descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    else
        descriptor_writes[1].descriptorCount = 6;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = descriptor_set_variable.set_;
        descriptor_writes[1].dstSet = descriptor_set_variable.set_;
        vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);
    }

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

    // Resources for buffer tests
    OneOffDescriptorSet descriptor_set_buffer(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr},
                                              },
                                              0, layout_pnext, 0);

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

    if (descriptor_indexing) {
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
        vk::UpdateDescriptorSets(m_device->device(), 2, buffer_descriptor_writes, 0, NULL);
    }

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
        bool debug;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set, 25,
                     "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, true, &pipeline_layout, &descriptor_set, 25,
                     "gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));"});
    tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, true, &pipeline_layout, &descriptor_set, 25,
                     "uFragColor = texture(tex[index], vec2(0, 0));"});

    if (descriptor_indexing) {
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set,
                         25, "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set,
                         5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
        // Pick 6 below because it is less than the maximum specified, but more than the actual specified
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout_variable,
                         &descriptor_set_variable, 6,
                         "(set = 0, binding = 1) Index of 6 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout_variable,
                         &descriptor_set_variable, 5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 25,
                         "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
        if (m_device->phy().features().geometryShader) {
            // OOB Geometry
            tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, false, &pipeline_layout_buffer,
                             &descriptor_set_buffer, 25, "UNASSIGNED-Descriptor index out of bounds"});
            // Uninitialized Geometry
            tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, false, &pipeline_layout_buffer,
                             &descriptor_set_buffer, 5, "VUID-vkCmdDraw-None-08114"});
        }
        if (m_device->phy().features().tessellationShader) {
            tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 25, "UNASSIGNED-Descriptor index out of bounds"});
            tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 5, "VUID-vkCmdDraw-None-08114"});
        }
    }

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        VkShaderObj *gs = nullptr;
        VkShaderObj *tcs = nullptr;
        VkShaderObj *tes = nullptr;

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
        if (iter.geometry_source) {
            gs = new VkShaderObj(this, iter.geometry_source, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                 nullptr, "main", iter.debug);
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
            tcs = new VkShaderObj(this, iter.tess_ctrl_source, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, SPV_ENV_VULKAN_1_0,
                                  SPV_SOURCE_GLSL, nullptr, "main", iter.debug);
            tes = new VkShaderObj(this, iter.tess_eval_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, SPV_ENV_VULKAN_1_0,
                                  SPV_SOURCE_GLSL, nullptr, "main", iter.debug);
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

        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
        delete gs;
        delete tcs;
        delete tes;
    }

    if (m_device->compute_queues().empty()) {
        return;
    }
    if (descriptor_indexing) {
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
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
        // Out of Bounds
        data = (uint32_t *)buffer0.memory().map();
        data[0] = 25;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-Descriptor index out of bounds");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
        vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);
    }
    return;
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlyDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto maintenance4_features = vku::InitStruct<VkPhysicalDeviceMaintenance4Features>();
    maintenance4_features.maintenance4 = true;
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&maintenance4_features);
    auto indexing_features = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    maintenance4_features.pNext = &indexing_features;
    GetPhysicalDeviceFeatures2(features2);

    if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
        !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
        !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
        !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
        GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    layout_createinfo_binding_flags[0] = vku::InitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
    layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    layout_pnext = layout_createinfo_binding_flags;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    layout_create_flags = 0;
    pool_create_flags = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    variable_count = vku::InitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfo>();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;
    allocate_pnext = &variable_count;

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                },
                                                layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});
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
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

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
    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        bool debug;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;

    // UNASSIGNED-Descriptor destroyed
    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable, &descriptor_set_variable, 1,
                     "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed."});

    VkSubmitInfo submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
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

        // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
        // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
        // longer does QueueWaitIdle() in each submit call, we should also be able to detect
        // resource destruction while a submission is blocked on a semaphore as well.
        image.destroy();

        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
    }
    return;
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlySamplerDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto maintenance4_features = vku::InitStruct<VkPhysicalDeviceMaintenance4Features>();
    maintenance4_features.maintenance4 = true;
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&maintenance4_features);
    auto indexing_features = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    maintenance4_features.pNext = &indexing_features;
    GetPhysicalDeviceFeatures2(features2);

    if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
        !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
        !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
        !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
        GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    layout_createinfo_binding_flags[0] = vku::InitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
    layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    layout_pnext = layout_createinfo_binding_flags;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    layout_create_flags = 0;
    pool_create_flags = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    variable_count = vku::InitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfo>();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;
    allocate_pnext = &variable_count;

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                },
                                                layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});
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
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

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
    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        bool debug;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;

    // UNASSIGNED-Descriptor destroyed
    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable, &descriptor_set_variable, 1,
                     "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed."});

    VkSubmitInfo submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
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

        // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
        // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
        // longer does QueueWaitIdle() in each submit call, we should also be able to detect
        // resource destruction while a submission is blocked on a semaphore as well.
        sampler.destroy();

        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
    }
    return;
}

TEST_F(NegativeGpuAVDescriptorIndexing, ImageArrayDynamicIndexing) {
    TEST_DESCRIPTION("GPU validation: test that only dynamically used indices are validated");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto maintenance4_features = vku::InitStruct<VkPhysicalDeviceMaintenance4Features>();
    maintenance4_features.maintenance4 = true;
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&maintenance4_features);
    auto indexing_features = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    maintenance4_features.pNext = &indexing_features;
    GetPhysicalDeviceFeatures2(features2);

    if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
        !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
        !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
        !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
        GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    RETURN_IF_SKIP(InitState(nullptr, &features2, pool_flags));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    layout_createinfo_binding_flags[0] = vku::InitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
    layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    layout_pnext = layout_createinfo_binding_flags;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    layout_create_flags = 0;
    pool_create_flags = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
    const uint32_t kDescCount = 40;
    variable_count = vku::InitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfo>();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;
    allocate_pnext = &variable_count;

    OneOffDescriptorSet descriptor_set_variable(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128, VK_SHADER_STAGE_ALL, nullptr},
        },
        layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});
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
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorCount = kDescCount;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;

    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

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
    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        bool debug;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;

    // UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout
    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable, &descriptor_set_variable, 35,
                     "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL."});

    VkSubmitInfo submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
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

        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
    }
    return;
}

TEST_F(NegativeGpuAVDescriptorIndexing, UpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    RETURN_IF_SKIP(InitGpuAvFramework());

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);

    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    if (!indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkDescriptorBindingFlagsEXT flags[3] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                                            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
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
    char const *vsSource = R"glsl(
        #version 450
        vec2 vertices[3];
        void main(){
            vertices[0] = vec2(-1.0, -1.0);
            vertices[1] = vec2( 1.0, -1.0);
            vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform foo0 { float x0; } bar0;
        layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;
        layout(set=0, binding=2) buffer  foo2 { float x2; } bar2;
        void main(){
           color = vec4(bar0.x0 + bar1.x1 + bar2.x2);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // Make both bindings valid before binding to the command buffer
    vk::UpdateDescriptorSets(m_device->device(), 2, &descriptor_write[0], 0, NULL);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->begin();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds, 0,
                              NULL);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    // Valid to update binding 1 after being bound
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[1], 0, NULL);

    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08114");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, UpdateAfterBindImageViewTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(indexing_features);
    if (!indexing_features.descriptorBindingSampledImageUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported)  descriptorBindingSampledImageUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &indexing_features));
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        vec2 vertices[3];
        void main(){
            vertices[0] = vec2(-1.0, -1.0);
            vertices[1] = vec2( 1.0, -1.0);
            vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBindingFlagsEXT binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewType-07752");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}
