/*
 * Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2022 Valve Corporation
 * Copyright (c) 2020-2022 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 */

#include "layer_validation_tests.h"

bool VkGpuAssistedLayerTest::InitGpuAssistedFramework(bool request_descriptor_indexing) {
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = LvlInitStruct<VkValidationFeaturesEXT>();
    features.enabledValidationFeatureCount = 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;

    if (request_descriptor_indexing) {
        return CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, &features,
                                                              m_errorMonitor);
    }

    InitFramework(m_errorMonitor, &features);
    return false;
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationArrayOOBGraphicsShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    bool descriptor_indexing = InitGpuAssistedFramework(true);

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    if (descriptor_indexing) {
        PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
            (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

        features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
            !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
            !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
            printf("Not all descriptor indexing features supported, skipping descriptor indexing tests\n");
            descriptor_indexing = false;
        }
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    // Make another buffer to populate the buffer array to be indexed
    VkBufferObj buffer1;
    buffer1.init(*m_device, bci, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        layout_createinfo_binding_flags[0] = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
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
        variable_count = LvlInitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfo>();
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

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    const VkPipelineLayoutObj pipeline_layout_variable(m_device, {&descriptor_set_variable.layout_});
    VkTextureObj texture(m_device, nullptr);
    VkSamplerObj sampler(m_device);

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[6] = {};
    for (int i = 0; i < 6; i++) {
        image_info[i] = texture.DescriptorImageInfo();
        image_info[i].sampler = sampler.handle();
        image_info[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = LvlInitStruct<VkWriteDescriptorSet>();
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

    const VkPipelineLayoutObj pipeline_layout_buffer(m_device, {&descriptor_set_buffer.layout_});

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
        buffer_descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
        buffer_descriptor_writes[0].dstSet = descriptor_set_buffer.set_;  // descriptor_set;
        buffer_descriptor_writes[0].dstBinding = 0;
        buffer_descriptor_writes[0].descriptorCount = 1;
        buffer_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        buffer_descriptor_writes[0].pBufferInfo = buffer_test_buffer_info;
        buffer_descriptor_writes[1] = LvlInitStruct<VkWriteDescriptorSet>();
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
    char const *vsSource_vert =
        "#version 450\n"
        "\n"
        "layout(std140, set = 0, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;\n"
        "layout(set = 0, binding = 1) uniform sampler2D tex[6];\n"
        "vec2 vertices[3];\n"
        "void main(){\n"
        "      vertices[0] = vec2(-1.0, -1.0);\n"
        "      vertices[1] = vec2( 1.0, -1.0);\n"
        "      vertices[2] = vec2( 0.0,  1.0);\n"
        "   gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);\n"
        "   gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));\n"
        "}\n";
    char const *fsSource_vert =
        "#version 450\n"
        "\n"
        "layout(set = 0, binding = 1) uniform sampler2D tex[6];\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "void main(){\n"
        "   uFragColor = texture(tex[0], vec2(0, 0));\n"
        "}\n";

    // Shader programs for array OOB test in fragment stage:
    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vsSource_frag =
        "#version 450\n"
        "\n"
        "layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;\n"
        "layout(location = 0) out flat uint index;\n"
        "vec2 vertices[3];\n"
        "void main(){\n"
        "      vertices[0] = vec2(-1.0, -1.0);\n"
        "      vertices[1] = vec2( 1.0, -1.0);\n"
        "      vertices[2] = vec2( 0.0,  1.0);\n"
        "   gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);\n"
        "   index = uniform_index_buffer.tex_index[0];\n"
        "}\n";
    char const *fsSource_frag =
        "#version 450\n"
        "\n"
        "layout(set = 0, binding = 1) uniform sampler2D tex[6];\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "layout(location = 0) in flat uint index;\n"
        "void main(){\n"
        "   uFragColor = texture(tex[index], vec2(0, 0));\n"
        "}\n";
    char const *fsSource_frag_runtime =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n"
        "\n"
        "layout(set = 0, binding = 1) uniform sampler2D tex[];\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "layout(location = 0) in flat uint index;\n"
        "void main(){\n"
        "   uFragColor = texture(tex[index], vec2(0, 0));\n"
        "}\n";
    char const *fsSource_buffer =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "\n"
        "layout(set = 0, binding = 1) buffer foo { vec4 val; } colors[];\n"
        "layout(location = 0) out vec4 uFragColor;\n"
        "layout(location = 0) in flat uint index;\n"
        "void main(){\n"
        "   uFragColor = colors[index].val;\n"
        "}\n";
    char const *gsSource =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "layout(triangles) in;\n"
        "layout(triangle_strip, max_vertices=3) out;\n"
        "layout(location=0) in VertexData { vec4 x; } gs_in[];\n"
        "layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;\n"
        "layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];\n"
        "void main() {\n"
        "   gl_Position = gs_in[0].x + adds[uniform_index_buffer.index].val.x;\n"
        "   EmitVertex();\n"
        "}\n";
    static const char vsSourceForGS[] = R"glsl(
        #version 450
        layout(location=0) out foo {vec4 val;} gs_out[3];
        void main() {
           gs_out[0].val = vec4(0);
           gl_Position = vec4(1);
        })glsl";
    static const char *tesSource =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;\n"
        "layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];\n"
        "layout(triangles, equal_spacing, cw) in;\n"
        "void main() {\n"
        "    gl_Position = adds[uniform_index_buffer.index].val;\n"
        "}\n";

    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        char const *geometry_source;
        char const *tess_ctrl_source;
        char const *tess_eval_source;
        bool debug;
        const VkPipelineLayoutObj *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
        char const *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set, 25,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set, 25,
                     "Index of 25 used to index descriptor array of length 6."});
#if !defined(ANDROID)
    // The Android test framework uses shaderc for online compilations.  Even when configured to compile with debug info,
    // shaderc seems to drop the OpLine instructions from the shader binary.  This causes the following two tests to fail
    // on Android platforms.  Skip these tests until the shaderc issue is understood/resolved.
    tests.push_back({vsSource_vert, fsSource_vert, nullptr, nullptr, nullptr, true, &pipeline_layout, &descriptor_set, 25,
                     "gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));"});
    tests.push_back({vsSource_frag, fsSource_frag, nullptr, nullptr, nullptr, true, &pipeline_layout, &descriptor_set, 25,
                     "uFragColor = texture(tex[index], vec2(0, 0));"});
#endif
    if (descriptor_indexing) {
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set,
                         25, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout, &descriptor_set,
                         5, "Descriptor index 5 is uninitialized"});
        // Pick 6 below because it is less than the maximum specified, but more than the actual specified
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout_variable,
                         &descriptor_set_variable, 6, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout_variable,
                         &descriptor_set_variable, 5, "Descriptor index 5 is uninitialized"});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 25, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 5, "Descriptor index 5 is uninitialized"});
        if (m_device->phy().features().geometryShader) {
            // OOB Geometry
            tests.push_back({vsSourceForGS, bindStateFragShaderText, gsSource, nullptr, nullptr, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 25, "Stage = Geometry"});
            // Uninitialized Geometry
            tests.push_back({vsSourceForGS, bindStateFragShaderText, gsSource, nullptr, nullptr, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 5, "Stage = Geometry"});
        }
        if (m_device->phy().features().tessellationShader) {
            tests.push_back({bindStateVertShaderText, bindStateFragShaderText, nullptr, bindStateTscShaderText, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 25, "Stage = Tessellation Eval"});
            tests.push_back({bindStateVertShaderText, bindStateFragShaderText, nullptr, bindStateTscShaderText, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 5, "Stage = Tessellation Eval"});
        }
    }

    VkViewport viewport = m_viewports[0];
    VkRect2D scissors = m_scissors[0];

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        VkResult err;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, iter.expected_error);
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        VkShaderObj *gs = nullptr;
        VkShaderObj *tcs = nullptr;
        VkShaderObj *tes = nullptr;
        VkPipelineObj pipe(m_device);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        if (iter.geometry_source) {
            gs = new VkShaderObj(this, iter.geometry_source, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                 nullptr, "main", iter.debug);
            pipe.AddShader(gs);
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
            pipe.AddShader(tcs);
            pipe.AddShader(tes);
            pipe.SetTessellation(&tsci);
            pipe.SetInputAssembly(&iasci);
        }
        pipe.AddDefaultColorAttachment();
        err = pipe.CreateVKPipeline(iter.pipeline_layout->handle(), renderPass());
        ASSERT_VK_SUCCESS(err);
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, iter.pipeline_layout->handle(), 0, 1,
                                  &iter.descriptor_set->set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
        uint32_t *data = (uint32_t *)buffer0.memory().map();
        data[0] = iter.index;
        buffer0.memory().unmap();

        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyFound();
        if (gs) {
            delete gs;
        }
        if (tcs && tes) {
            delete tcs;
            delete tes;
        }
    }
    auto c_queue = m_device->GetDefaultComputeQueue();
    if (c_queue && descriptor_indexing) {
        char const *csSource =
            "#version 450\n"
            "#extension GL_EXT_nonuniform_qualifier : enable\n "
            "layout(set = 0, binding = 0) uniform ufoo { uint index; } u_index;"
            "layout(set = 0, binding = 1) buffer StorageBuffer {\n"
            "    uint data;\n"
            "} Data[];\n"
            "void main() {\n"
            "   Data[(u_index.index - 1)].data = Data[u_index.index].data;\n"
            "}\n";

        VkShaderObj shader_module(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

        VkPipelineShaderStageCreateInfo stage = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage.flags = 0;
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = shader_module.handle();
        stage.pName = "main";
        stage.pSpecializationInfo = nullptr;

        // CreateComputePipelines
        VkComputePipelineCreateInfo pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
        pipeline_info.flags = 0;
        pipeline_info.layout = pipeline_layout_buffer.handle();
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;
        pipeline_info.stage = stage;

        VkPipeline c_pipeline;
        vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Stage = Compute");
        vk::QueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyFound();
        // Out of Bounds
        data = (uint32_t *)buffer0.memory().map();
        data[0] = 25;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Stage = Compute");
        vk::QueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyFound();
        vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);
    }
    return;
}

TEST_F(VkGpuAssistedLayerTest, GpuBufferOOB) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    InitGpuAssistedFramework(false);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

    auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
    auto multi_draw_features = LvlInitStruct<VkPhysicalDeviceMultiDrawFeaturesEXT>();

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_MULTI_DRAW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
        robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>(&multi_draw_features);
    }

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&robustness2_features);

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!robustness2_features.nullDescriptor) {
        printf("%s nullDescriptor feature not supported, skipping test\n", kSkipPrefix);
        return;
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBufferObj offset_buffer;
    VkBufferObj write_buffer;
    VkBufferObj uniform_texel_buffer;
    VkBufferObj storage_texel_buffer;
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    offset_buffer.init(*m_device, 4, reqs);
    write_buffer.init_as_storage(*m_device, 16, reqs);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 16;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    uniform_texel_buffer.init(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    storage_texel_buffer.init(*m_device, buffer_create_info, reqs);
    VkBufferView uniform_buffer_view;
    VkBufferView storage_buffer_view;
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vk::CreateBufferView(m_device->device(), &bvci, NULL, &uniform_buffer_view);
    bvci.buffer = storage_texel_buffer.handle();
    vk::CreateBufferView(m_device->device(), &bvci, NULL, &storage_buffer_view);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, uniform_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferView(4, storage_buffer_view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] =
        "#version 450\n"
        "layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;\n"      // index[1]
        "layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;\n"  // data[4]
        "layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;\n"     // VK_NULL_HANDLE
        "layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;\n"              // texel_buffer[4]
        "layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;\n"          // texel_buffer[4]
        "void main() {\n"
        "    vec4 x;\n"
        "    if (u_index.index[0] == 8)\n"
        "        Data.data[u_index.index[0]] = 0xdeadca71;\n"
        "    else if (u_index.index[0] == 0)\n"
        "        Data.data[0] = u_index.index[4];\n"
        "    else if (u_index.index[0] == 1)\n"
        "        Data.data[0] = Null.data[40];\n"  // No error
        "    else if (u_index.index[0] == 2)\n"
        "        x = texelFetch(u_buffer, 5);\n"
        "    else if (u_index.index[0] == 3)\n"
        "        x = imageLoad(s_buffer, 5);\n"
        "    else if (u_index.index[0] == 4)\n"
        "        imageStore(s_buffer, 5, x);\n"
        "    else if (u_index.index[0] == 5)\n"  // No Error
        "        imageStore(s_buffer, 0, x);\n"
        "    else if (u_index.index[0] == 6)\n"  // No Error
        "        x = imageLoad(s_buffer, 0);\n"
        "}\n";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-02706" Storage
    tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-02705" Uniform
    tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-02705" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-02706" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-02706" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
            m_errorMonitor->ExpectSuccess();
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
            m_errorMonitor->VerifyNotFound();
        } else {
            m_errorMonitor->VerifyFound();
        }
        vk::QueueWaitIdle(m_device->m_queue);
    }

    if (multi_draw_features.multiDraw) {
        auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
        auto vkCmdDrawMultiIndexedEXT =
            (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
        assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);

        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;

        VkBufferObj buffer;
        buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
            &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = 8;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-02706");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();

        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
            &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        data = (uint32_t *)offset_buffer.memory().map();
        *data = 0;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-02705");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyBufferView(m_device->handle(), uniform_buffer_view, nullptr);
    vk::DestroyBufferView(m_device->handle(), storage_buffer_view, nullptr);
}

void VkGpuAssistedLayerTest::ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                                                  VkDescriptorType descriptor_type, const char *fragment_shader,
                                                  const char *expected_error) {
    SetTargetApiVersion(VK_API_VERSION_1_1);

    InitGpuAssistedFramework(false);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFeatures features = {};  // Make sure robust buffer access is not enabled
    ASSERT_NO_FATAL_FAILURE(InitState(&features));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, expected_error);

    OneOffDescriptorSet ds(m_device, {{0, descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                                     : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = buffer_size;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, bci);
    VkPipelineObj pipe(m_device);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = binding_offset;
    buffer_info.range = binding_range;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    char const *vsSource =
        "#version 450\n"
        "vec2 vertices[3];\n"
        "void main(){\n"
        "      vertices[0] = vec2(-1.0, -1.0);\n"
        "      vertices[1] = vec2( 1.0, -1.0);\n"
        "      vertices[2] = vec2( 0.0,  1.0);\n"
        "      gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);\n"
        "}\n";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkResult err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_,
                              0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(true);
    m_errorMonitor->VerifyFound();
    DestroyRenderTarget();
}

TEST_F(VkGpuAssistedLayerTest, DrawTimeShaderUniformBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access uniform buffer outside the bound region.");
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0, binding=0) uniform readonly foo { int x; int y; } bar;\n"
        "void main(){\n"
        "   x = vec4(bar.x, bar.y, 0, 1);\n"
        "}\n";

    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource,
                         "Descriptor size is 4 and highest byte accessed was 7");
}
TEST_F(VkGpuAssistedLayerTest, DrawTimeShaderStorageBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access storage buffer outside the bound region.");

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0, binding=0) buffer readonly foo { int x; int y; } bar;\n"
        "void main(){\n"
        "   x = vec4(bar.x, bar.y, 0, 1);\n"
        "}\n";

    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, fsSource,
                         "Descriptor size is 4 and highest byte accessed was 7");
}

TEST_F(VkGpuAssistedLayerTest, DrawTimeShaderUniformBufferTooSmallArray) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses array in block "
        "definition.");

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0, binding=0) uniform readonly foo { int x[17]; } bar;\n"
        "void main(){\n"
        "   int y = 0;\n"
        "   for (int i = 0; i < 17; i++)\n"
        "       y += bar.x[i];\n"
        "   x = vec4(y, 0, 0, 1);\n"
        "}\n";

    ShaderBufferSizeTest(64,  // buffer size
                         0,   // binding offset
                         64,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource,
                         "Descriptor size is 64 and highest byte accessed was 67");
}

TEST_F(VkGpuAssistedLayerTest, DrawTimeShaderUniformBufferTooSmallNestedStruct) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses nested struct in block "
        "definition.");

    char const *fsSource =
        "#version 450\n"
        "\n"
        "struct S {\n"
        "    int x;\n"
        "    int y;\n"
        "};\n"
        "layout(location=0) out vec4 x;\n"
        "layout(set=0, binding=0) uniform readonly foo { int a; S b; } bar;\n"
        "void main(){\n"
        "   x = vec4(bar.a, bar.b.x, bar.b.y, 1);\n"
        "}\n";

    ShaderBufferSizeTest(8,  // buffer size
                         0,  // binding offset
                         8,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource,
                         "Descriptor size is 8 and highest byte accessed was 19");
}

TEST_F(VkGpuAssistedLayerTest, GpuBufferDeviceAddressOOB) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    bool supported = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    InitGpuAssistedFramework(false);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        printf("%s This test should not be run on the RADV driver.\n", kSkipPrefix);
        return;
    }
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        printf("%s This test should not be run on the AMD proprietary driver.\n", kSkipPrefix);
        return;
    }

    supported = supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto bda_features = DeviceExtensionSupported(gpu(), nullptr, VK_NV_MESH_SHADER_EXTENSION_NAME)
                            ? LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&mesh_shader_features)
                            : LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    supported = supported && bda_features.bufferDeviceAddress;

    if (!supported) {
        printf("%s Buffer Device Address feature not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkBool32 mesh_shader_supported = mesh_shader_features.meshShader;
    mesh_shader_supported &= DeviceExtensionSupported(gpu(), nullptr, VK_NV_MESH_SHADER_EXTENSION_NAME) ? VK_TRUE : VK_FALSE;
    if (mesh_shader_supported) {
        m_device_extension_names.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 12;  // 64 bit pointer + int
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    // Make another buffer to write to
    bci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    bci.size = 64;  // Buffer should be 16*4 = 64 bytes
    VkBuffer buffer1;
    vk::CreateBuffer(device(), &bci, NULL, &buffer1);
    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer1, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkMemoryAllocateFlagsInfo alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;
    VkDeviceMemory buffer_mem;
    VkResult err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(m_device->device(), buffer1, buffer_mem, 0);

    // Get device address of buffer to write to
    VkBufferDeviceAddressInfoKHR bda_info = LvlInitStruct<VkBufferDeviceAddressInfoKHR>();
    bda_info.buffer = buffer1;
    auto vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferDeviceAddressKHR");
    ASSERT_TRUE(vkGetBufferDeviceAddressKHR != nullptr);
    auto pBuffer = vkGetBufferDeviceAddressKHR(m_device->device(), &bda_info);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    VkDescriptorBufferInfo buffer_test_buffer_info[2] = {};
    buffer_test_buffer_info[0].buffer = buffer0.handle();
    buffer_test_buffer_info[0].offset = 0;
    buffer_test_buffer_info[0].range = sizeof(uint32_t);

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_test_buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

    char const *shader_source =
        "#version 450\n"
        "#extension GL_EXT_buffer_reference : enable\n "
        "layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;\n"
        "layout(set = 0, binding = 0) uniform ufoo {\n"
        "    bufStruct data;\n"
        "    int nWrites;\n"
        "} u_info;\n"
        "layout(buffer_reference, std140) buffer bufStruct {\n"
        "    int a[4];\n"
        "};\n"
        "void main() {\n"
        "    for (int i=0; i < u_info.nWrites; ++i) {\n"
        "        u_info.data.a[i] = 0xdeadca71;\n"
        "    }\n"
        "}\n";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

    VkViewport viewport = m_viewports[0];
    VkRect2D scissors = m_scissors[0];

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    begin_info.pInheritanceInfo = &hinfo;

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    // Starting address too low
    VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
    data[0] = pBuffer - 16;
    data[1] = 4;
    buffer0.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    // Run past the end
    data = (VkDeviceAddress *)buffer0.memory().map();
    data[0] = pBuffer;
    data[1] = 5;
    buffer0.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    // Positive test - stay inside buffer
    m_errorMonitor->ExpectSuccess();
    data = (VkDeviceAddress *)buffer0.memory().map();
    data[0] = pBuffer;
    data[1] = 4;
    buffer0.memory().unmap();
    err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyNotFound();

    if (mesh_shader_supported) {
        const unsigned push_constant_range_count = 1;
        VkPushConstantRange push_constant_ranges[push_constant_range_count] = {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = 2 * sizeof(VkDeviceAddress);

        VkPipelineLayout mesh_pipeline_layout;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo[1] = {};
        pipelineLayoutCreateInfo[0] = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipelineLayoutCreateInfo[0].pushConstantRangeCount = push_constant_range_count;
        pipelineLayoutCreateInfo[0].pPushConstantRanges = push_constant_ranges;
        pipelineLayoutCreateInfo[0].setLayoutCount = 0;
        pipelineLayoutCreateInfo[0].pSetLayouts = nullptr;
        vk::CreatePipelineLayout(m_device->handle(), pipelineLayoutCreateInfo, NULL, &mesh_pipeline_layout);

        char const *mesh_shader_source =
            "#version 460\n"
            "#extension GL_NV_mesh_shader : require\n"
            "#extension GL_EXT_buffer_reference : enable\n"
            "layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;\n"
            "layout(push_constant) uniform ufoo {\n"
            "    bufStruct data;\n"
            "    int nWrites;\n"
            "} u_info;\n"
            "layout(buffer_reference, std140) buffer bufStruct {\n"
            "    int a[4];\n"
            "};\n"

            "layout(local_size_x = 32) in;\n"
            "layout(max_vertices = 64, max_primitives = 126) out;\n"
            "layout(triangles) out;\n"

            "uint invocationID = gl_LocalInvocationID.x;\n"
            "void main() {\n"
            "    if (invocationID == 0) {\n"
            "        for (int i=0; i < u_info.nWrites; ++i) {\n"
            "            u_info.data.a[i] = 0xdeadca71;\n"
            "        }\n"
            "    }\n"
            "}\n";
        VkShaderObj ms(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       true);
        VkPipelineObj mesh_pipe(m_device);
        mesh_pipe.AddShader(&ms);
        mesh_pipe.AddDefaultColorAttachment();
        err = mesh_pipe.CreateVKPipeline(mesh_pipeline_layout, renderPass());
        ASSERT_VK_SUCCESS(err);
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.handle());
        VkDeviceAddress pushConstants[2] = {};
        pushConstants[0] = pBuffer;
        pushConstants[1] = 5;
        vk::CmdPushConstants(m_commandBuffer->handle(), mesh_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_NV, 0, sizeof(pushConstants),
                             pushConstants);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
        vk::DestroyPipelineLayout(m_device->handle(), mesh_pipeline_layout, nullptr);
    }

    vk::DestroyBuffer(m_device->handle(), buffer1, NULL);
    vk::FreeMemory(m_device->handle(), buffer_mem, NULL);
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders using gpu assited validation.");
    OOBRayTracingShadersTestBody(true);
}

TEST_F(VkGpuAssistedLayerTest, GpuBuildAccelerationStructureValidationInvalidHandle) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with an invalid handle for a bottom level acceleration structure.");

    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor,
                                        /*need_gpu_validation=*/true)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkCommandPoolObj command_pool(m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        1234567890,  // invalid
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkBufferObj top_level_as_scratch;
    top_level_as.create_scratch_buffer(*m_device, &top_level_as_scratch);

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vkCmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                      VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuBuildAccelerationStructureValidationBottomLevelNotYetBuilt) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with a handle for a bottom level acceleration structure that has not yet been built.");

    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor,
                                        /*need_gpu_validation=*/true)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkCommandPoolObj command_pool(m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkAccelerationStructureObj bot_level_as_never_built(*m_device, bot_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        bot_level_as_never_built.opaque_handle(),
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkBufferObj top_level_as_scratch;
    top_level_as.create_scratch_buffer(*m_device, &top_level_as_scratch);

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vkCmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                      VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuBuildAccelerationStructureValidationBottomLevelDestroyed) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with a handle for a destroyed bottom level acceleration structure.");

    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor,
                                        /*need_gpu_validation=*/true)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkCommandPoolObj command_pool(m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    uint64_t destroyed_bot_level_as_handle = 0;
    {
        VkAccelerationStructureObj destroyed_bot_level_as(*m_device, bot_level_as_create_info);
        m_errorMonitor->VerifyNotFound();

        destroyed_bot_level_as_handle = destroyed_bot_level_as.opaque_handle();

        VkBufferObj bot_level_as_scratch;
        destroyed_bot_level_as.create_scratch_buffer(*m_device, &bot_level_as_scratch);

        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();
        vkCmdBuildAccelerationStructureNV(command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                          destroyed_bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
        command_buffer.end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer.handle();
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyNotFound();

        // vk::DestroyAccelerationStructureNV called on destroyed_bot_level_as during destruction.
    }

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        destroyed_bot_level_as_handle,
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkBufferObj top_level_as_scratch;
    top_level_as.create_scratch_buffer(*m_device, &top_level_as_scratch);

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vkCmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                      VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuBuildAccelerationStructureValidationRestoresState) {
    TEST_DESCRIPTION("Validate that acceleration structure gpu validation correctly restores compute state.");

    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor,
                                        /*need_gpu_validation=*/true, /*need_push_descriptors=*/true)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdPushDescriptorSetKHR");
    assert(vkCmdPushDescriptorSetKHR != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkCommandPoolObj command_pool(m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        1234567,
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkBufferObj top_level_as_scratch;
    top_level_as.create_scratch_buffer(*m_device, &top_level_as_scratch);

    struct ComputeOutput {
        uint32_t push_constant_value;
        uint32_t push_descriptor_value;
        uint32_t normal_descriptor_value;
    };

    VkBufferObj push_descriptor_buffer;
    push_descriptor_buffer.init(*m_device, 4, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkBufferObj normal_descriptor_buffer;
    normal_descriptor_buffer.init(*m_device, 4, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDeviceSize output_descriptor_buffer_size = static_cast<VkDeviceSize>(sizeof(ComputeOutput));
    VkBufferObj output_descriptor_buffer;
    output_descriptor_buffer.init(*m_device, output_descriptor_buffer_size,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    const std::string cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        layout(push_constant) uniform PushConstants { uint value; } push_constant;
        layout(set = 0, binding = 0, std430) buffer PushDescriptorBuffer { uint value; } push_descriptor;
        layout(set = 1, binding = 0, std430) buffer NormalDescriptorBuffer { uint value; } normal_descriptor;

        layout(set = 2, binding = 0, std430) buffer ComputeOutputBuffer {
            uint push_constant_value;
            uint push_descriptor_value;
            uint normal_descriptor_value;
        } compute_output;

        void main() {
            compute_output.push_constant_value = push_constant.value;
            compute_output.push_descriptor_value = push_descriptor.value;
            compute_output.normal_descriptor_value = normal_descriptor.value;
        }
    )glsl";
    VkShaderObj cs(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet push_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                            },
                                            VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              });
    OneOffDescriptorSet output_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              });

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.size = 4;
    push_constant_range.offset = 0;

    const VkPipelineLayoutObj compute_pipeline_layout(m_device,
                                                      {
                                                          &push_descriptor_set.layout_,
                                                          &normal_descriptor_set.layout_,
                                                          &output_descriptor_set.layout_,
                                                      },
                                                      {push_constant_range});

    VkComputePipelineCreateInfo compute_pipeline_ci = LvlInitStruct<VkComputePipelineCreateInfo>();
    compute_pipeline_ci.layout = compute_pipeline_layout.handle();
    compute_pipeline_ci.stage = cs.GetStageCreateInfo();

    VkPipeline compute_pipeline;
    ASSERT_VK_SUCCESS(
        vk::CreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 1, &compute_pipeline_ci, nullptr, &compute_pipeline));

    normal_descriptor_set.WriteDescriptorBufferInfo(0, normal_descriptor_buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    normal_descriptor_set.UpdateDescriptorSets();

    output_descriptor_set.WriteDescriptorBufferInfo(0, output_descriptor_buffer.handle(), 0, output_descriptor_buffer_size,
                                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    output_descriptor_set.UpdateDescriptorSets();

    // Set input data
    const uint32_t push_constant_value = 1234567890;
    const uint32_t push_descriptor_value = 98765432;
    const uint32_t normal_descriptor_value = 1111111;

    uint32_t *mapped_push_descriptor_buffer_data = (uint32_t *)push_descriptor_buffer.memory().map();
    *mapped_push_descriptor_buffer_data = push_descriptor_value;
    push_descriptor_buffer.memory().unmap();

    uint32_t *mapped_normal_descriptor_buffer_data = (uint32_t *)normal_descriptor_buffer.memory().map();
    *mapped_normal_descriptor_buffer_data = normal_descriptor_value;
    normal_descriptor_buffer.memory().unmap();

    ComputeOutput *mapped_output_buffer_data = (ComputeOutput *)output_descriptor_buffer.memory().map();
    mapped_output_buffer_data->push_constant_value = 0;
    mapped_output_buffer_data->push_descriptor_value = 0;
    mapped_output_buffer_data->normal_descriptor_value = 0;
    output_descriptor_buffer.memory().unmap();

    VkDescriptorBufferInfo push_descriptor_buffer_info = {};
    push_descriptor_buffer_info.buffer = push_descriptor_buffer.handle();
    push_descriptor_buffer_info.offset = 0;
    push_descriptor_buffer_info.range = 4;
    VkWriteDescriptorSet push_descriptor_set_write = LvlInitStruct<VkWriteDescriptorSet>();
    push_descriptor_set_write.descriptorCount = 1;
    push_descriptor_set_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    push_descriptor_set_write.dstBinding = 0;
    push_descriptor_set_write.pBufferInfo = &push_descriptor_buffer_info;

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
    vk::CmdPushConstants(command_buffer.handle(), compute_pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, 4,
                         &push_constant_value);
    vkCmdPushDescriptorSetKHR(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 0, 1,
                              &push_descriptor_set_write);
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 1, 1,
                              &normal_descriptor_set.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 2, 1,
                              &output_descriptor_set.set_, 0, nullptr);

    vkCmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                      VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);

    vk::CmdDispatch(command_buffer.handle(), 1, 1, 1);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    mapped_output_buffer_data = (ComputeOutput *)output_descriptor_buffer.memory().map();
    EXPECT_EQ(mapped_output_buffer_data->push_constant_value, push_constant_value);
    EXPECT_EQ(mapped_output_buffer_data->push_descriptor_value, push_descriptor_value);
    EXPECT_EQ(mapped_output_buffer_data->normal_descriptor_value, normal_descriptor_value);
    output_descriptor_buffer.memory().unmap();

    // Clean up
    vk::DestroyPipeline(m_device->device(), compute_pipeline, nullptr);
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectCountDeviceLimit) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    InitGpuAssistedFramework(false);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    } else {
        printf("%s VK_KHR_draw_indirect_count extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceVulkan13Features features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        features13.dynamicRendering = true;
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, features13.dynamicRendering ? (void *)&features13 : nullptr, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdDrawIndirectCountKHR =
        (PFN_vkCmdDrawIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCountKHR");
    if (vkCmdDrawIndirectCountKHR == nullptr) {
        printf("%s did not find vkCmdDrawIndirectCountKHR function pointer;  Skipping.\n", kSkipPrefix);
        return;
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 2 * sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj draw_buffer;
    draw_buffer.init(*m_device, buffer_create_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndirectCommand));
    draw_buffer.memory().unmap();

    VkBufferCreateInfo count_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    count_buffer_create_info.size = sizeof(uint32_t);
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj count_buffer;
    count_buffer.init(*m_device, count_buffer_create_info,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.memory().unmap();

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkResult err = vk::CreatePipelineLayout(m_device->handle(), &pipelineLayoutCreateInfo, NULL, &pipeline_layout);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    err = pipe.CreateVKPipeline(pipeline_layout, renderPass());
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                              sizeof(VkDrawIndirectCommand));

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    if (features13.dynamicRendering) {
        VkPipelineObj dr_pipe(m_device);
        dr_pipe.AddShader(&vs);
        dr_pipe.AddDefaultColorAttachment();
        err = dr_pipe.CreateVKPipeline(pipeline_layout, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

        vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                  sizeof(VkDrawIndirectCommand));

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectCount) {
    TEST_DESCRIPTION("GPU validation: Validate Draw*IndirectCount countBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitGpuAssistedFramework(false);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    } else {
        printf("%s VK_KHR_draw_indirect_count extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, NULL, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    auto vkCmdDrawIndirectCountKHR =
        (PFN_vkCmdDrawIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCountKHR");
    if (vkCmdDrawIndirectCountKHR == nullptr) {
        printf("%s did not find vkCmdDrawIndirectCountKHR function pointer;  Skipping.\n", kSkipPrefix);
        return;
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj draw_buffer;
    draw_buffer.init(*m_device, buffer_create_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    draw_ptr->firstInstance = 0;
    draw_ptr->firstVertex = 0;
    draw_ptr->instanceCount = 1;
    draw_ptr->vertexCount = 3;
    draw_buffer.memory().unmap();

    VkBufferCreateInfo count_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    count_buffer_create_info.size = sizeof(uint32_t);
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj count_buffer;
    count_buffer.init(*m_device, count_buffer_create_info,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkResult err = vk::CreatePipelineLayout(m_device->handle(), &pipelineLayoutCreateInfo, NULL, &pipeline_layout);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    err = pipe.CreateVKPipeline(pipeline_layout, renderPass());
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03122");
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;
    count_buffer.memory().unmap();
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03121");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    // Offset of 4 should error
    vkCmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                              sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    auto vkCmdDrawIndexedIndirectCountKHR =
        (PFN_vkCmdDrawIndexedIndirectCountKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndexedIndirectCountKHR");
    if (vkCmdDrawIndexedIndirectCountKHR == nullptr) {
        printf("%s did not find vkCmdDrawIndexedIndirectCountKHR function pointer;  Skipping.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154");
    VkBufferObj indexed_draw_buffer;
    buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand);
    indexed_draw_buffer.init(*m_device, buffer_create_info,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.memory().map();
    indexed_draw_ptr->indexCount = 3;
    indexed_draw_ptr->firstIndex = 0;
    indexed_draw_ptr->instanceCount = 1;
    indexed_draw_ptr->firstInstance = 0;
    indexed_draw_ptr->vertexOffset = 0;
    indexed_draw_buffer.memory().unmap();

    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;
    count_buffer.memory().unmap();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    VkBufferCreateInfo index_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    index_buffer_create_info.size = 3 * sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferObj index_buffer;
    index_buffer.init(*m_device, index_buffer_create_info);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153");
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    // Offset of 4 should error
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectFirstInstance) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    InitGpuAssistedFramework(false);
    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    features2.features.drawIndirectFirstInstance = VK_FALSE;

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 4 * sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBufferObj draw_buffer;
    draw_buffer.init(*m_device, buffer_create_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    for (uint32_t i = 0; i < 4; i++) {
        draw_ptr->vertexCount = 3;
        draw_ptr->instanceCount = 1;
        draw_ptr->firstVertex = 0;
        draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        draw_ptr++;
    }
    draw_buffer.memory().unmap();

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkResult err = vk::CreatePipelineLayout(m_device->handle(), &pipelineLayoutCreateInfo, NULL, &pipeline_layout);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    err = pipe.CreateVKPipeline(pipeline_layout, renderPass());
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-firstInstance-00478");
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    // Now with an offset and indexed draw
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-firstInstance-00530");
    VkBufferObj indexed_draw_buffer;
    buffer_create_info.size = 4 * sizeof(VkDrawIndexedIndirectCommand);
    indexed_draw_buffer.init(*m_device, buffer_create_info,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.memory().map();
    for (uint32_t i = 0; i < 4; i++) {
        indexed_draw_ptr->indexCount = 3;
        indexed_draw_ptr->instanceCount = 1;
        indexed_draw_ptr->firstIndex = 0;
        indexed_draw_ptr->vertexOffset = 0;
        indexed_draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        indexed_draw_ptr++;
    }
    indexed_draw_buffer.memory().unmap();

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    VkBufferCreateInfo index_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    index_buffer_create_info.size = 3 * sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferObj index_buffer;
    index_buffer.init(*m_device, index_buffer_create_info);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_draw_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, nullptr);
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationInlineUniformBlockAndMiscGpu) {
    TEST_DESCRIPTION(
        "GPU validation: Make sure inline uniform blocks don't generate false validation errors, verify reserved descriptor slot "
        "and verify pipeline recovery");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    m_errorMonitor->ExpectSuccess();
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                              VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeaturesEXT features = LvlInitStruct<VkValidationFeaturesEXT>();
    features.enabledValidationFeatureCount = 2;
    features.pEnabledValidationFeatures = enables;
    bool descriptor_indexing = CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names,
                                                                              m_device_extension_names, &features, m_errorMonitor);
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto inline_uniform_block_features = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(&indexing_features);
    bool inline_uniform_block = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    if (!(descriptor_indexing && inline_uniform_block)) {
        printf("Descriptor indexing and/or inline uniform block not supported Skipping test\n");
        return;
    }
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&inline_uniform_block_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!indexing_features.descriptorBindingPartiallyBound || !inline_uniform_block_features.inlineUniformBlock) {
        printf("Not all features supported, skipping test\n");
        return;
    }
    auto inline_uniform_props = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&inline_uniform_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &prop2);

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    auto c_queue = m_device->GetDefaultComputeQueue();
    if (nullptr == c_queue) {
        printf("Compute not supported, skipping test\n");
        return;
    }

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 4;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    layout_createinfo_binding_flags[0] = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, 32, VK_SHADER_STAGE_ALL,
                                            nullptr},
                                       },
                                       0, layout_createinfo_binding_flags, 0);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    const uint32_t test_data = 0xdeadca7;
    VkWriteDescriptorSetInlineUniformBlockEXT write_inline_uniform = LvlInitStruct<VkWriteDescriptorSetInlineUniformBlockEXT>();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &test_data;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;

    descriptor_writes[1] = LvlInitStruct<VkWriteDescriptorSet>(&write_inline_uniform);
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 16;  // Skip first 16 bytes (dummy)
    descriptor_writes[1].descriptorCount = 4;   // Write 4 bytes to val
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    char const *csSource =
        "#version 450\n"
        "#extension GL_EXT_nonuniform_qualifier : enable\n "
        "layout(set = 0, binding = 0) buffer StorageBuffer { uint index; } u_index;"
        "layout(set = 0, binding = 1) uniform inlineubodef { ivec4 dummy; int val; } inlineubo;\n"

        "void main() {\n"
        "    u_index.index = inlineubo.val;\n"
        "}\n";

    VkShaderObj shader_module(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo stage = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = stage;

    VkPipeline c_pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyNotFound();
    vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    ASSERT_TRUE(*data = test_data);
    *data = 0;
    buffer0.memory().unmap();

    // Also verify that binding slot reservation is working
    auto ici = GetInstanceCreateInfo();
    VkInstance test_inst;
    vk::CreateInstance(&ici, nullptr, &test_inst);
    uint32_t gpu_count;
    vk::EnumeratePhysicalDevices(test_inst, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> phys_devices(gpu_count);
    vk::EnumeratePhysicalDevices(test_inst, &gpu_count, phys_devices.data());

    VkPhysicalDeviceProperties properties;
    vk::GetPhysicalDeviceProperties(phys_devices[m_gpu_index], &properties);
    if (m_device->props.limits.maxBoundDescriptorSets != properties.limits.maxBoundDescriptorSets - 1)
        m_errorMonitor->SetError("VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT not functioning as expected");
    vk::DestroyInstance(test_inst, NULL);

    m_errorMonitor->ExpectSuccess();
    auto set_count = properties.limits.maxBoundDescriptorSets;
    if (inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks < set_count) {
        printf("Max per stage inline uniform block limit too small - skipping recovery portion of this test\n");
        return;
    }
    // Now be sure that recovery from an unavailable descriptor set works and that uninstrumented shaders are used
    VkDescriptorSetLayoutBinding dsl_binding[2] = {};
    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dsl_binding[0].descriptorCount = 1;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[1].binding = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dsl_binding[1].descriptorCount = set_count;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_ALL;
    VkDescriptorSetLayout *layouts{new VkDescriptorSetLayout[set_count]{}};
    VkDescriptorSetLayoutCreateInfo dsl_create_info =
        LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(layout_createinfo_binding_flags);
    dsl_create_info.pBindings = dsl_binding;
    dsl_create_info.bindingCount = 2;
    for (uint32_t i = 0; i < set_count; i++) {
        vk::CreateDescriptorSetLayout(m_device->handle(), &dsl_create_info, NULL, &layouts[i]);
    }
    VkPipelineLayoutCreateInfo pl_create_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    VkPipelineLayout pl_layout;
    pl_create_info.setLayoutCount = set_count;
    pl_create_info.pSetLayouts = layouts;
    m_errorMonitor->VerifyNotFound();
    // Expect error since GPU-AV cannot add debug descriptor to layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GPU-Assisted-Validation");
    vk::CreatePipelineLayout(m_device->handle(), &pl_create_info, NULL, &pl_layout);
    m_errorMonitor->VerifyFound();

    // We should still be able to use the layout and create a temporary uninstrumented shader module
    m_errorMonitor->ExpectSuccess();
    pipeline_info.layout = pl_layout;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pl_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
    vk::QueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyPipelineLayout(m_device->handle(), pl_layout, NULL);
    vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);
    for (uint32_t i = 0; i < set_count; i++) {
        vk::DestroyDescriptorSetLayout(m_device->handle(), layouts[i], NULL);
    }
    data = (uint32_t *)buffer0.memory().map();
    if (*data != test_data) m_errorMonitor->SetError("Pipeline recovery when resources unavailable not functioning as expected");
    *data = 0;
    buffer0.memory().unmap();
    m_errorMonitor->VerifyNotFound();
    delete[] layouts;

    // Now make sure we can still use the shader with instrumentation
    m_errorMonitor->ExpectSuccess();
    VkPipeline c_pipeline2;
    // Use the sane pipeline layout
    pipeline_info.layout = pipeline_layout.handle();
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline2);
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline2);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
    vk::QueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyPipeline(m_device->handle(), c_pipeline2, nullptr);
    data = (uint32_t *)buffer0.memory().map();
    if (*data != test_data) m_errorMonitor->SetError("Using shader after pipeline recovery not functioning as expected");
    *data = 0;
    buffer0.memory().unmap();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationAbort) {
    TEST_DESCRIPTION("GPU validation: Verify that aborting GPU-AV is safe.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitGpuAssistedFramework(false);
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    PFN_vkSetPhysicalDeviceFeaturesEXT fpvkSetPhysicalDeviceFeaturesEXT =
        (PFN_vkSetPhysicalDeviceFeaturesEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFeaturesEXT");
    PFN_vkGetOriginalPhysicalDeviceFeaturesEXT fpvkGetOriginalPhysicalDeviceFeaturesEXT =
        (PFN_vkGetOriginalPhysicalDeviceFeaturesEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceFeaturesEXT");

    if (!(fpvkSetPhysicalDeviceFeaturesEXT) || !(fpvkGetOriginalPhysicalDeviceFeaturesEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceFeatures features = {};
    fpvkGetOriginalPhysicalDeviceFeaturesEXT(gpu(), &features);

    // Disable features necessary for GPU-AV so initialization aborts
    features.vertexPipelineStoresAndAtomics = false;
    features.fragmentStoresAndAtomics = false;
    fpvkSetPhysicalDeviceFeaturesEXT(gpu(), features);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "GPU-Assisted Validation disabled");
    ASSERT_NO_FATAL_FAILURE(InitState());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, ValidationFeatures) {
    TEST_DESCRIPTION("Validate Validation Features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeaturesEXT features = LvlInitStruct<VkValidationFeaturesEXT>();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;

    auto ici = GetInstanceCreateInfo();
    features.pNext = ici.pNext;
    ici.pNext = &features;
    VkInstance instance;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02967");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();

    VkValidationFeatureEnableEXT printf_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                     VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    features.pEnabledValidationFeatures = printf_enables;
    features.enabledValidationFeatureCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02968");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();
}

void VkDebugPrintfTest::InitDebugPrintfFramework() {
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = LvlInitStruct<VkValidationFeaturesEXT>();
    features.enabledValidationFeatureCount = 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;

    InitFramework(m_errorMonitor, &features);
}

TEST_F(VkDebugPrintfTest, GpuDebugPrintf) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    m_device_extension_names.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    InitDebugPrintfFramework();
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_MULTI_DRAW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_MULTI_DRAW_EXTENSION_NAME);
        return;
    }
    auto multi_draw_features = LvlInitStruct<VkPhysicalDeviceMultiDrawFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multi_draw_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto features = m_device->phy().features();
    if (!features.vertexPipelineStoresAndAtomics || !features.fragmentStoresAndAtomics) {
        printf("%s GPU-Assisted printf test requires vertexPipelineStoresAndAtomics and fragmentStoresAndAtomics.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s GPU-Assisted printf test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }
    // Make a uniform buffer to be passed to the shader that contains the test number
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

    char const *shader_source =
        "#version 450\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(set = 0, binding = 0) uniform ufoo {\n"
        "    int whichtest;\n"
        "} u_info;\n"
        "void main() {\n"
        "    float myfloat = 3.1415f;\n"
        "    int foo = -135;\n"
        "    if (gl_VertexIndex == 0) {\n"
        "        switch(u_info.whichtest) {\n"
        "            case 0:\n"
        "                debugPrintfEXT(\"Here are two float values %f, %f\", 1.0, myfloat);\n"
        "                break;\n"
        "            case 1:\n"
        "                debugPrintfEXT(\"Here's a smaller float value %1.2f\", myfloat);\n"
        "                break;\n"
        "            case 2:\n"
        "                debugPrintfEXT(\"Here's an integer %i with text before and after it\", foo);\n"
        "                break;\n"
        "            case 3:\n"
        "                foo = 256;\n"
        "                debugPrintfEXT(\"Here's an integer in octal %o and hex 0x%x\", foo, foo);\n"
        "                break;\n"
        "            case 4:\n"
        "                debugPrintfEXT(\"%d is a negative integer\", foo);\n"
        "                break;\n"
        "            case 5:\n"
        "                vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);\n"
        "                debugPrintfEXT(\"Here's a vector of floats %1.2v4f\", floatvec);\n"
        "                break;\n"
        "            case 6:\n"
        "                debugPrintfEXT(\"Here's a float in sn %e\", myfloat);\n"
        "                break;\n"
        "            case 7:\n"
        "                debugPrintfEXT(\"Here's a float in sn %1.2e\", myfloat);\n"
        "                break;\n"
        "            case 8:\n"
        "                debugPrintfEXT(\"Here's a float in shortest %g\", myfloat);\n"
        "                break;\n"
        "            case 9:\n"
        "                debugPrintfEXT(\"Here's a float in hex %1.9a\", myfloat);\n"
        "                break;\n"
        "            case 10:\n"
        "                debugPrintfEXT(\"First printf with a %% and no value\");\n"
        "                debugPrintfEXT(\"Second printf with a value %i\", foo);\n"
        "                break;\n"
        "        }\n"
        "    }\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "}\n";
    std::vector<char const *> messages;
    messages.push_back("Here are two float values 1.000000, 3.141500");
    messages.push_back("Here's a smaller float value 3.14");
    messages.push_back("Here's an integer -135 with text before and after it");
    messages.push_back("Here's an integer in octal 400 and hex 0x100");
    messages.push_back("-135 is a negative integer");
    messages.push_back("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    messages.push_back("Here's a float in sn 3.141500e+00");
    messages.push_back("Here's a float in sn 3.14e+00");
    messages.push_back("Here's a float in shortest 3.1415");
    messages.push_back("Here's a float in hex 0x1.921cac000p+1");
    // Two error messages have to be last in the vector
    messages.push_back("First printf with a % and no value");
    messages.push_back("Second printf with a value -135");
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

    VkViewport viewport = m_viewports[0];
    VkRect2D scissors = m_scissors[0];

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    VkResult err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    begin_info.pInheritanceInfo = &hinfo;

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = i;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }

    if (multi_draw_features.multiDraw) {
        auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
        auto vkCmdDrawMultiIndexedEXT =
            (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
        assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
            &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[0]);
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        VkBufferObj buffer;
        buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
        ptr[0] = 0;
        ptr[1] = 1;
        ptr[2] = 2;
        buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
            &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[1]);
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }

    if (features.shaderInt64) {
        char const *shader_source_int64 =
            "#version 450\n"
            "#extension GL_EXT_debug_printf : enable\n"
            "#extension GL_ARB_gpu_shader_int64 : enable\n"
            "layout(set = 0, binding = 0) uniform ufoo {\n"
            "    int whichtest;\n"
            "} u_info;\n"
            "void main() {\n"
            "    uint64_t bigvar = 0x2000000000000001ul;\n"
            "    if (gl_VertexIndex == 0) {\n"
            "        switch(u_info.whichtest) {\n"
            "            case 0:\n"
            "                debugPrintfEXT(\"Here's an unsigned long 0x%ul\", bigvar);\n"
            "                break;\n"
            "            case 1:\n"
            "                u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);"
            "                debugPrintfEXT(\"Here's a vector of ul %v4ul\", vecul);\n"
            "                break;\n"
            "            case 2:\n"
            "                debugPrintfEXT(\"Unsigned long as decimal %lu and as hex 0x%lx\", bigvar, bigvar);\n"
            "                break;\n"
            "        }\n"
            "    }\n"
            "    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
            "}\n";
        VkShaderObj vs_int64(this, shader_source_int64, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                             "main", true);
        VkPipelineObj pipe2(m_device);
        pipe2.AddShader(&vs_int64);
        pipe2.AddDefaultColorAttachment();
        err = pipe2.CreateVKPipeline(pipeline_layout.handle(), renderPass());
        ASSERT_VK_SUCCESS(err);

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(
            kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 2;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                             "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }
}
TEST_F(VkDebugPrintfTest, MeshTaskShadersPrintf) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    InitDebugPrintfFramework();
    std::vector<const char *> required_device_extensions = {VK_NV_MESH_SHADER_EXTENSION_NAME,
                                                            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%sNot suppored by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char taskShaderText[] =
        "#version 460\n"
        "#extension GL_NV_mesh_shader : enable\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(local_size_x = 32) in;\n"
        "uint invocationID = gl_LocalInvocationID.x;\n"
        "void main() {\n"
        "    if (invocationID == 0) {\n"
        "        gl_TaskCountNV = 1;\n"
        "        debugPrintfEXT(\"hello from task shader\");\n"
        "    }\n"
        "}\n";

    static const char meshShaderText[] =
        "#version 450\n"
        "#extension GL_NV_mesh_shader : require\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(local_size_x = 1) in;\n"
        "layout(max_vertices = 3) out;\n"
        "layout(max_primitives = 1) out;\n"
        "layout(triangles) out;\n"
        "uint invocationID = gl_LocalInvocationID.x;\n"
        "void main() {\n"
        "    if (invocationID == 0) {\n"
        "        debugPrintfEXT(\"hello from mesh shader\");\n"
        "    }\n"
        "}\n";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkPipelineLayoutObj pipeline_layout(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&ts);
    pipe.AddShader(&ms);
    pipe.AddDefaultColorAttachment();
    VkResult err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV =
        (PFN_vkCmdDrawMeshTasksNV)vk::GetInstanceProcAddr(instance(), "vkCmdDrawMeshTasksNV");
    ASSERT_TRUE(vkCmdDrawMeshTasksNV != nullptr);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from task shader");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from mesh shader");
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitGpuAssistedFramework(false));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests requires Vulkan 1.1 exactly";
    }

    char const *fs_source = R"glsl(
        #version 450
        layout (set = 1, binding = 0) uniform sampler2D samplerColor;
        layout(location = 0) out vec4 color;
        void main() {
           color = texture(samplerColor, gl_FragCoord.xy);
           color += texture(samplerColor, gl_FragCoord.wz);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    auto vkCmdDrawIndexedIndirectCountKHR =
        reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCountKHR>(vk::GetDeviceProcAddr(device(), "vkCmdDrawIndexedIndirectCountKHR"));
    ASSERT_TRUE(vkCmdDrawIndexedIndirectCountKHR != nullptr);

    m_errorMonitor->ExpectSuccess();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler);
    descriptor_set.UpdateDescriptorSets();

    VkBufferObj indirect_buffer;
    indirect_buffer.init(*m_device, sizeof(VkDrawIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj indexed_indirect_buffer;
    indexed_indirect_buffer.init(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj count_buffer;
    count_buffer.init(*m_device, sizeof(uint32_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    VkBufferObj index_buffer;
    index_buffer.init(*m_device, sizeof(uint32_t), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                     sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroySampler(device(), sampler, nullptr);
    m_errorMonitor->VerifyNotFound();
}
