/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, GpuValidationArrayOOBGraphicsShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");
    if (!VkRenderFramework::DeviceCanDraw()) {
        printf("%s GPU-Assisted validation test requires a driver that can draw.\n", kSkipPrefix);
        return;
    }

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;
    bool descriptor_indexing = CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names,
                                                                              m_device_extension_names, &features, m_errorMonitor);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    if (descriptor_indexing) {
        PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
            (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

        features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
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
    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s GPU-Assisted validation test requires Vulkan 1.1+.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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

        layout_createinfo_binding_flags[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        layout_createinfo_binding_flags[0].pNext = NULL;
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
        variable_count.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
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
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    if (descriptor_indexing)
        descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    else
        descriptor_writes[1].descriptorCount = 6;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vkUpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = descriptor_set_variable.set_;
        descriptor_writes[1].dstSet = descriptor_set_variable.set_;
        vkUpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);
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
        buffer_descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        buffer_descriptor_writes[0].dstSet = descriptor_set_buffer.set_;  // descriptor_set;
        buffer_descriptor_writes[0].dstBinding = 0;
        buffer_descriptor_writes[0].descriptorCount = 1;
        buffer_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        buffer_descriptor_writes[0].pBufferInfo = buffer_test_buffer_info;
        buffer_descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        buffer_descriptor_writes[1].dstSet = descriptor_set_buffer.set_;  // descriptor_set;
        buffer_descriptor_writes[1].dstBinding = 1;
        buffer_descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
        buffer_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        buffer_descriptor_writes[1].pBufferInfo = &buffer_test_buffer_info[1];
        vkUpdateDescriptorSets(m_device->device(), 2, buffer_descriptor_writes, 0, NULL);
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
            tests.push_back({bindStateVertShaderText, bindStateFragShaderText, gsSource, nullptr, nullptr, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 25, "Stage = Geometry"});
            // Uninitialized Geometry
            tests.push_back({bindStateVertShaderText, bindStateFragShaderText, gsSource, nullptr, nullptr, false,
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

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        VkResult err;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, iter.expected_error);
        VkShaderObj vs(m_device, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, this, "main", iter.debug);
        VkShaderObj fs(m_device, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, this, "main", iter.debug);
        VkShaderObj *gs = nullptr;
        VkShaderObj *tcs = nullptr;
        VkShaderObj *tes = nullptr;
        VkPipelineObj pipe(m_device);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        if (iter.geometry_source) {
            gs = new VkShaderObj(m_device, iter.geometry_source, VK_SHADER_STAGE_GEOMETRY_BIT, this, "main", iter.debug);
            pipe.AddShader(gs);
        }
        if (iter.tess_ctrl_source && iter.tess_eval_source) {
            tcs = new VkShaderObj(m_device, iter.tess_ctrl_source, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this, "main",
                                  iter.debug);
            tes = new VkShaderObj(m_device, iter.tess_eval_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this, "main",
                                  iter.debug);
            pipe.AddShader(tcs);
            pipe.AddShader(tes);
            VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                         VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
            VkPipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, VK_NULL_HANDLE,
                VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT};

            VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                                                       &tessellationDomainOriginStateInfo, 0, 3};
            pipe.SetTessellation(&tsci);
            pipe.SetInputAssembly(&iasci);
        }
        pipe.AddDefaultColorAttachment();
        err = pipe.CreateVKPipeline(iter.pipeline_layout->handle(), renderPass());
        ASSERT_VK_SUCCESS(err);
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, iter.pipeline_layout->handle(), 0, 1,
                                &iter.descriptor_set->set_, 0, nullptr);
        vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vkCmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
        uint32_t *data = (uint32_t *)buffer0.memory().map();
        data[0] = iter.index;
        buffer0.memory().unmap();
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->m_queue);
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

        auto shader_module = new VkShaderObj(m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT, this);

        VkPipelineShaderStageCreateInfo stage;
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.pNext = nullptr;
        stage.flags = 0;
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = shader_module->handle();
        stage.pName = "main";
        stage.pSpecializationInfo = nullptr;

        // CreateComputePipelines
        VkComputePipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_info.pNext = nullptr;
        pipeline_info.flags = 0;
        pipeline_info.layout = pipeline_layout_buffer.handle();
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;
        pipeline_info.stage = stage;

        VkPipeline c_pipeline;
        vkCreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);
        VkCommandBufferBeginInfo begin_info = {};
        VkCommandBufferInheritanceInfo hinfo = {};
        hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pInheritanceInfo = &hinfo;

        m_commandBuffer->begin(&begin_info);
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_buffer.handle(), 0, 1,
                                &descriptor_set_buffer.set_, 0, nullptr);
        vkCmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        // Uninitialized
        uint32_t *data = (uint32_t *)buffer0.memory().map();
        data[0] = 5;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Stage = Compute");
        vkQueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyFound();
        // Out of Bounds
        data = (uint32_t *)buffer0.memory().map();
        data[0] = 25;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Stage = Compute");
        vkQueueSubmit(c_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->m_queue);
        m_errorMonitor->VerifyFound();
        vkDestroyPipeline(m_device->handle(), c_pipeline, NULL);
        vkDestroyShaderModule(m_device->handle(), shader_module->handle(), NULL);
    }
    return;
}

TEST_F(VkLayerTest, GpuValidationArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");

    std::array<const char *, 1> required_instance_extensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
    for (auto instance_extension : required_instance_extensions) {
        if (InstanceExtensionSupported(instance_extension)) {
            m_instance_extension_names.push_back(instance_extension);
        } else {
            printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix, instance_extension);
            return;
        }
    }

    VkValidationFeatureEnableEXT validation_feature_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT validation_features = {};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures = validation_feature_enables;
    bool descriptor_indexing = CheckDescriptorIndexingSupportAndInitFramework(
        this, m_instance_extension_names, m_device_extension_names, &validation_features, m_errorMonitor);

    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    std::array<const char *, 2> required_device_extensions = {VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
                                                              VK_NV_RAY_TRACING_EXTENSION_NAME};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    if (descriptor_indexing) {
        PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
            (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

        features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingPartiallyBound ||
            !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingVariableDescriptorCount) {
            printf("Not all descriptor indexing features supported, skipping descriptor indexing tests\n");
            descriptor_indexing = false;
        }
    }
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesNV>();
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    if (ray_tracing_properties.maxTriangleCount == 0) {
        printf("%s Did not find required ray tracing properties; skipped.\n", kSkipPrefix);
        return;
    }

    VkQueue ray_tracing_queue = m_device->m_queue;
    uint32_t ray_tracing_queue_family_index = 0;

    // If supported, run on the compute only queue.
    uint32_t compute_only_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (compute_only_queue_family_index != UINT32_MAX) {
        const auto &compute_only_queues = m_device->queue_family_queues(compute_only_queue_family_index);
        if (!compute_only_queues.empty()) {
            ray_tracing_queue = compute_only_queues[0]->handle();
            ray_tracing_queue_family_index = compute_only_queue_family_index;
        }
    }

    VkCommandPoolObj ray_tracing_command_pool(m_device, ray_tracing_queue_family_index,
                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj ray_tracing_command_buffer(m_device, &ray_tracing_command_pool);

    struct AABB {
        float min_x;
        float min_y;
        float min_z;
        float max_x;
        float max_y;
        float max_z;
    };

    const std::vector<AABB> aabbs = {{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}};

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkDeviceSize aabb_buffer_size = sizeof(AABB) * aabbs.size();
    VkBufferObj aabb_buffer;
    aabb_buffer.init(*m_device, aabb_buffer_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    uint8_t *mapped_aabb_buffer_data = (uint8_t *)aabb_buffer.memory().map();
    std::memcpy(mapped_aabb_buffer_data, (uint8_t *)aabbs.data(), static_cast<std::size_t>(aabb_buffer_size));
    aabb_buffer.memory().unmap();

    VkGeometryNV geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    geometry.geometry.triangles = {};
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry.geometry.aabbs = {};
    geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    geometry.geometry.aabbs.aabbData = aabb_buffer.handle();
    geometry.geometry.aabbs.numAABBs = static_cast<uint32_t>(aabbs.size());
    geometry.geometry.aabbs.offset = 0;
    geometry.geometry.aabbs.stride = static_cast<VkDeviceSize>(sizeof(AABB));
    geometry.flags = 0;

    VkAccelerationStructureInfoNV bot_level_as_info = {};
    bot_level_as_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    bot_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_info.instanceCount = 0;
    bot_level_as_info.geometryCount = 1;
    bot_level_as_info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = {};
    bot_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    bot_level_as_create_info.info = bot_level_as_info;

    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

    const std::vector<VkGeometryInstanceNV> instances = {
        VkGeometryInstanceNV{
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
            bot_level_as.opaque_handle(),
        },
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV) * instances.size();
    VkBufferObj instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)instances.data(), static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureInfoNV top_level_as_info = {};
    top_level_as_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    top_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_info.instanceCount = 1;
    top_level_as_info.geometryCount = 0;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = {};
    top_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    top_level_as_create_info.info = top_level_as_info;

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);

    VkDeviceSize scratch_buffer_size = std::max(bot_level_as.build_scratch_memory_requirements().memoryRequirements.size,
                                                top_level_as.build_scratch_memory_requirements().memoryRequirements.size);
    VkBufferObj scratch_buffer;
    scratch_buffer.init(*m_device, scratch_buffer_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    ray_tracing_command_buffer.begin();

    // Build bot level acceleration structure
    ray_tracing_command_buffer.BuildAccelerationStructure(&bot_level_as, scratch_buffer.handle());

    // Barrier to prevent using scratch buffer for top level build before bottom level build finishes
    VkMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    ray_tracing_command_buffer.PipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
                                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memory_barrier, 0,
                                               nullptr, 0, nullptr);

    // Build top level acceleration structure
    ray_tracing_command_buffer.BuildAccelerationStructure(&top_level_as, scratch_buffer.handle(), instance_buffer.handle());

    ray_tracing_command_buffer.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &ray_tracing_command_buffer.handle();
    vkQueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(ray_tracing_queue);
    m_errorMonitor->VerifyNotFound();

    VkTextureObj texture(m_device, nullptr);
    VkSamplerObj sampler(m_device);

    VkDeviceSize storage_buffer_size = 1024;
    VkBufferObj storage_buffer;
    storage_buffer.init(*m_device, storage_buffer_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, {ray_tracing_queue_family_index});

    VkDeviceSize shader_binding_table_buffer_size = ray_tracing_properties.shaderGroupHandleSize * 4ull;
    VkBufferObj shader_binding_table_buffer;
    shader_binding_table_buffer.init(*m_device, shader_binding_table_buffer_size,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, {ray_tracing_queue_family_index});

    // Setup descriptors!
    const VkShaderStageFlags kAllRayTracingStages = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV |
                                                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV |
                                                    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV;

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    VkDescriptorPoolCreateFlags pool_create_flags = 0;
    VkDescriptorSetLayoutCreateFlags layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[3] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = 0;
        ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        layout_createinfo_binding_flags[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        layout_createinfo_binding_flags[0].pNext = NULL;
        layout_createinfo_binding_flags[0].bindingCount = 3;
        layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
        layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        layout_pnext = layout_createinfo_binding_flags;
    }

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, kAllRayTracingStages, nullptr},
                           },
                           layout_create_flags, layout_pnext, pool_create_flags);

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count = {};
    uint32_t desc_counts;
    if (descriptor_indexing) {
        layout_create_flags = 0;
        pool_create_flags = 0;
        ds_binding_flags[2] =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
        variable_count.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        variable_count.descriptorSetCount = 1;
        variable_count.pDescriptorCounts = &desc_counts;
        allocate_pnext = &variable_count;
    }

    OneOffDescriptorSet ds_variable(m_device,
                                    {
                                        {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                                        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, kAllRayTracingStages, nullptr},
                                    },
                                    layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    VkAccelerationStructureNV top_level_as_handle = top_level_as.handle();
    VkWriteDescriptorSetAccelerationStructureNV write_descript_set_as = {};
    write_descript_set_as.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    write_descript_set_as.accelerationStructureCount = 1;
    write_descript_set_as.pAccelerationStructures = &top_level_as_handle;

    VkDescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.buffer = storage_buffer.handle();
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = storage_buffer_size;

    VkDescriptorImageInfo descriptor_image_infos[6] = {};
    for (int i = 0; i < 6; i++) {
        descriptor_image_infos[i] = texture.DescriptorImageInfo();
        descriptor_image_infos[i].sampler = sampler.handle();
        descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = ds.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    descriptor_writes[0].pNext = &write_descript_set_as;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = ds.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &descriptor_buffer_info;

    descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[2].dstSet = ds.set_;
    descriptor_writes[2].dstBinding = 2;
    if (descriptor_indexing) {
        descriptor_writes[2].descriptorCount = 5;  // Intentionally don't write index 5
    } else {
        descriptor_writes[2].descriptorCount = 6;
    }
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[2].pImageInfo = descriptor_image_infos;
    vkUpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = ds_variable.set_;
        descriptor_writes[1].dstSet = ds_variable.set_;
        descriptor_writes[2].dstSet = ds_variable.set_;
        vkUpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    }

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});
    const VkPipelineLayoutObj pipeline_layout_variable(m_device, {&ds_variable.layout_});

    const auto SetImagesArrayLength = [](const std::string &shader_template, const std::string &length_str) {
        const std::string to_replace = "IMAGES_ARRAY_LENGTH";

        std::string result = shader_template;
        auto position = result.find(to_replace);
        assert(position != std::string::npos);
        result.replace(position, to_replace.length(), length_str);
        return result;
    };

    const std::string rgen_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 0) uniform accelerationStructureNV topLevelAS;
        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadNV vec3 payload;
        layout(location = 3) callableDataNV vec3 callableData;

        void main() {
            sbo.rgen_ran = 1;

	        executeCallableNV(0, 3);
	        sbo.result1 = callableData.x;

	        vec3 origin = vec3(0.0f, 0.0f, -2.0f);
	        vec3 direction = vec3(0.0f, 0.0f, 1.0f);

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, direction, 10000.0, 0);
	        sbo.result2 = payload.x;

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, -direction, 10000.0, 0);
	        sbo.result3 = payload.x;

            if (sbo.rgen_index > 0) {
                // OOB here:
                sbo.result3 = texelFetch(textures[sbo.rgen_index], ivec2(0, 0), 0).x;
            }
        }
        )";

    const std::string rgen_source = SetImagesArrayLength(rgen_source_template, "6");
    const std::string rgen_source_runtime = SetImagesArrayLength(rgen_source_template, "");

    const std::string ahit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
	        sbo.ahit_ran = 2;

	        payload = vec3(0.1234f);

            if (sbo.ahit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.ahit_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string ahit_source = SetImagesArrayLength(ahit_source_template, "6");
    const std::string ahit_source_runtime = SetImagesArrayLength(ahit_source_template, "");

    const std::string chit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        hitAttributeNV vec3 attribs;

        void main() {
            sbo.chit_ran = 3;

            payload = attribs;
            if (sbo.chit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.chit_index], ivec2(0, 0), 0).x;
            }
        }
        )";
    const std::string chit_source = SetImagesArrayLength(chit_source_template, "6");
    const std::string chit_source_runtime = SetImagesArrayLength(chit_source_template, "");

    const std::string miss_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
            sbo.miss_ran = 4;

            payload = vec3(1.0, 0.0, 0.0);

            if (sbo.miss_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.miss_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string miss_source = SetImagesArrayLength(miss_source_template, "6");
    const std::string miss_source_runtime = SetImagesArrayLength(miss_source_template, "");

    const std::string intr_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        void main() {
	        sbo.intr_ran = 5;

	        hitValue = vec3(0.0f, 0.5f, 0.0f);

	        reportIntersectionNV(1.0f, 0);

            if (sbo.intr_index > 0) {
                // OOB here:
                hitValue.x = texelFetch(textures[sbo.intr_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string intr_source = SetImagesArrayLength(intr_source_template, "6");
    const std::string intr_source_runtime = SetImagesArrayLength(intr_source_template, "");

    const std::string call_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 3) callableDataInNV vec3 callableData;

        void main() {
	        sbo.call_ran = 6;

	        callableData = vec3(0.1234f);

            if (sbo.call_index > 0) {
                // OOB here:
                callableData.x = texelFetch(textures[sbo.call_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string call_source = SetImagesArrayLength(call_source_template, "6");
    const std::string call_source_runtime = SetImagesArrayLength(call_source_template, "");

    struct TestCase {
        const std::string &rgen_shader_source;
        const std::string &ahit_shader_source;
        const std::string &chit_shader_source;
        const std::string &miss_shader_source;
        const std::string &intr_shader_source;
        const std::string &call_shader_source;
        bool variable_length;
        uint32_t rgen_index;
        uint32_t ahit_index;
        uint32_t chit_index;
        uint32_t miss_index;
        uint32_t intr_index;
        uint32_t call_index;
        const char *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 25, 0, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 25, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 25, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 25, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 25, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 0, 25,
                     "Index of 25 used to index descriptor array of length 6."});

    if (descriptor_indexing) {
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 25, 0, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 25, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 25, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 25, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 25, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 25, "Index of 25 used to index descriptor array of length 6."});

        // For this group, 6 is less than max specified (max specified is 8) but more than actual specified (actual specified is 5)
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 6, 0, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 6, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 6, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 6, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 6, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 6, "Index of 6 used to index descriptor array of length 6."});

        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 5, 0, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 5, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 5, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 5, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 5, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 5, "Descriptor index 5 is uninitialized."});
    }

    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(
        vkGetDeviceProcAddr(m_device->handle(), "vkCreateRayTracingPipelinesNV"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesNV != nullptr);

    PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(
            vkGetDeviceProcAddr(m_device->handle(), "vkGetRayTracingShaderGroupHandlesNV"));
    ASSERT_TRUE(vkGetRayTracingShaderGroupHandlesNV != nullptr);

    PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV =
        reinterpret_cast<PFN_vkCmdTraceRaysNV>(vkGetDeviceProcAddr(m_device->handle(), "vkCmdTraceRaysNV"));
    ASSERT_TRUE(vkCmdTraceRaysNV != nullptr);

    for (const auto &test : tests) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, test.expected_error);

        VkShaderObj rgen_shader(m_device, test.rgen_shader_source.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV, this, "main");
        VkShaderObj ahit_shader(m_device, test.ahit_shader_source.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV, this, "main");
        VkShaderObj chit_shader(m_device, test.chit_shader_source.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, this, "main");
        VkShaderObj miss_shader(m_device, test.miss_shader_source.c_str(), VK_SHADER_STAGE_MISS_BIT_NV, this, "main");
        VkShaderObj intr_shader(m_device, test.intr_shader_source.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV, this, "main");
        VkShaderObj call_shader(m_device, test.call_shader_source.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV, this, "main");

        VkPipelineShaderStageCreateInfo stage_create_infos[6] = {};
        stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        stage_create_infos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[2].module = chit_shader.handle();
        stage_create_infos[2].pName = "main";

        stage_create_infos[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[3].stage = VK_SHADER_STAGE_MISS_BIT_NV;
        stage_create_infos[3].module = miss_shader.handle();
        stage_create_infos[3].pName = "main";

        stage_create_infos[4].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[4].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[4].module = intr_shader.handle();
        stage_create_infos[4].pName = "main";

        stage_create_infos[5].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[5].stage = VK_SHADER_STAGE_CALLABLE_BIT_NV;
        stage_create_infos[5].module = call_shader.handle();
        stage_create_infos[5].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[4] = {};
        group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;  // rgen
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 3;  // miss
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[2].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[2].closestHitShader = 2;
        group_create_infos[2].anyHitShader = 1;
        group_create_infos[2].intersectionShader = 4;

        group_create_infos[3].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        group_create_infos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[3].generalShader = 5;  // call
        group_create_infos[3].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        pipeline_ci.stageCount = 6;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 4;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.maxRecursionDepth = 2;
        pipeline_ci.layout = test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle();

        VkPipeline pipeline = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline));

        std::vector<uint8_t> shader_binding_table_data;
        shader_binding_table_data.resize(static_cast<std::size_t>(shader_binding_table_buffer_size), 0);
        ASSERT_VK_SUCCESS(vkGetRayTracingShaderGroupHandlesNV(m_device->handle(), pipeline, 0, 4,
                                                              static_cast<std::size_t>(shader_binding_table_buffer_size),
                                                              shader_binding_table_data.data()));

        uint8_t *mapped_shader_binding_table_data = (uint8_t *)shader_binding_table_buffer.memory().map();
        std::memcpy(mapped_shader_binding_table_data, shader_binding_table_data.data(), shader_binding_table_data.size());
        shader_binding_table_buffer.memory().unmap();

        ray_tracing_command_buffer.begin();

        vkCmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);
        vkCmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);

        vkCmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                         ray_tracing_properties.shaderGroupHandleSize * 0ull, shader_binding_table_buffer.handle(),
                         ray_tracing_properties.shaderGroupHandleSize * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                         shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupHandleSize * 2ull,
                         ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                         ray_tracing_properties.shaderGroupHandleSize * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                         /*width=*/1, /*height=*/1, /*depth=*/1);

        ray_tracing_command_buffer.end();

        // Update the index of the texture that the shaders should read
        uint32_t *mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
        mapped_storage_buffer_data[0] = test.rgen_index;
        mapped_storage_buffer_data[1] = test.ahit_index;
        mapped_storage_buffer_data[2] = test.chit_index;
        mapped_storage_buffer_data[3] = test.miss_index;
        mapped_storage_buffer_data[4] = test.intr_index;
        mapped_storage_buffer_data[5] = test.call_index;
        mapped_storage_buffer_data[6] = 0;
        mapped_storage_buffer_data[7] = 0;
        mapped_storage_buffer_data[8] = 0;
        mapped_storage_buffer_data[9] = 0;
        mapped_storage_buffer_data[10] = 0;
        mapped_storage_buffer_data[11] = 0;
        storage_buffer.memory().unmap();

        vkQueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(ray_tracing_queue);
        m_errorMonitor->VerifyFound();

        mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
        ASSERT_TRUE(mapped_storage_buffer_data[6] == 1);
        ASSERT_TRUE(mapped_storage_buffer_data[7] == 2);
        ASSERT_TRUE(mapped_storage_buffer_data[8] == 3);
        ASSERT_TRUE(mapped_storage_buffer_data[9] == 4);
        ASSERT_TRUE(mapped_storage_buffer_data[10] == 5);
        ASSERT_TRUE(mapped_storage_buffer_data[11] == 6);
        storage_buffer.memory().unmap();

        vkDestroyPipeline(m_device->handle(), pipeline, nullptr);
    }
}

TEST_F(VkLayerTest, InvalidDescriptorPoolConsistency) {
    VkResult err;

    TEST_DESCRIPTION("Allocate descriptor sets from one DS pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkFreeDescriptorSets-pDescriptorSets-parent");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &bad_pool);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    err = vkFreeDescriptorSets(m_device->device(), bad_pool, 1, &descriptor_set.set_);

    m_errorMonitor->VerifyFound();

    vkDestroyDescriptorPool(m_device->device(), bad_pool, NULL);
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithSubpass) {
    TEST_DESCRIPTION("Use a pipeline for the wrong subpass in a render pass instance");

    ASSERT_NO_FATAL_FAILURE(Init());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 2, subpasses, 1, &dep};
    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    const VkPipelineLayoutObj pl(m_device);
    pipe.CreateVKPipeline(pl.handle(), rp);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    // subtest 1: bind in the wrong subpass
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "built for subpass 0 but used in subpass 1");
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vkCmdEndRenderPass(m_commandBuffer->handle());

    // subtest 2: bind in correct subpass, then transition to next subpass
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "built for subpass 0 but used in subpass 1");
    vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vkCmdEndRenderPass(m_commandBuffer->handle());

    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, ImageBarrierSubpassConflict) {
    TEST_DESCRIPTION("Check case where subpass index references different image from image barrier");
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create RP/FB combo where subpass has incorrect index attachment, this is 2nd half of "VUID-vkCmdPipelineBarrier-image-02635"
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    // ref attachment points to wrong attachment index compared to img_barrier below
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attach, 1, subpasses, 1, &dep};
    VkRenderPass rp;

    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    VkImageObj image2(m_device);
    image2.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView2 = image2.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    // re-use imageView from start of test
    VkImageView iv_array[2] = {imageView, imageView2};

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, iv_array, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle(); /* barrier references image from attachment index 0 */
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-image-02635");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentIndexOutOfRange) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // There are no attachments, but refer to attachment 0.
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    // "... must be less than the total number of attachments ..."
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-attachment-00834",
                         "VUID-VkRenderPassCreateInfo2KHR-attachment-03051");
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentReadOnlyButCleared) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool maintenance2Supported = rp2Supported;

    // Check for VK_KHR_maintenance2
    if (!rp2Supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        maintenance2Supported = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        maintenance2Supported = true;
    }

    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D32_SFLOAT_S8_UINT,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};

    // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL but depth cleared
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                         "VUID-VkRenderPassCreateInfo2KHR-pAttachments-02522");

    if (maintenance2Supported) {
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL but depth cleared
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                             "VUID-VkRenderPassCreateInfo-pAttachments-01566", nullptr);

        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL but depth cleared
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                             "VUID-VkRenderPassCreateInfo-pAttachments-01567", nullptr);
    }
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentMismatchingLayoutsColor) {
    TEST_DESCRIPTION("Attachment is used simultaneously as two color attachments with different layouts.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 2, refs, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "subpass 0 already uses attachment 0 with a different image layout",
                         "subpass 0 already uses attachment 0 with a different image layout");
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentDescriptionInvalidFinalLayout) {
    TEST_DESCRIPTION("VkAttachmentDescription's finalLayout must not be UNDEFINED or PREINITIALIZED");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2KHR-finalLayout-03061");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2KHR-finalLayout-03061");
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentsMisc) {
    TEST_DESCRIPTION(
        "Ensure that CreateRenderPass produces the expected validation errors when a subpass's attachments violate the valid usage "
        "conditions.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {0, VK_FORMAT_D24_UNORM_S8_UINT, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // preserve attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    std::vector<VkAttachmentReference> resolve = {
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    std::vector<uint32_t> preserve = {5};

    VkSubpassDescription subpass = {0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    (uint32_t)input.size(),
                                    input.data(),
                                    (uint32_t)color.size(),
                                    color.data(),
                                    resolve.data(),
                                    &depth,
                                    (uint32_t)preserve.size(),
                                    preserve.data()};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    // Test too many color attachments
    {
        std::vector<VkAttachmentReference> too_many_colors(m_device->props.limits.maxColorAttachments + 1, color[0]);
        subpass.colorAttachmentCount = (uint32_t)too_many_colors.size();
        subpass.pColorAttachments = too_many_colors.data();
        subpass.pResolveAttachments = NULL;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                             "VUID-VkSubpassDescription-colorAttachmentCount-00845",
                             "VUID-VkSubpassDescription2KHR-colorAttachmentCount-03063");

        subpass.colorAttachmentCount = (uint32_t)color.size();
        subpass.pColorAttachments = color.data();
        subpass.pResolveAttachments = resolve.data();
    }

    // Test sample count mismatch between color buffers
    attachments[subpass.pColorAttachments[1].attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    depth.attachment = VK_ATTACHMENT_UNUSED;  // Avoids triggering 01418

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pColorAttachments-01417",
                         "VUID-VkSubpassDescription2KHR-pColorAttachments-03069");

    depth.attachment = 3;
    attachments[subpass.pColorAttachments[1].attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;

    // Test sample count mismatch between color buffers and depth buffer
    attachments[subpass.pDepthStencilAttachment->attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    subpass.colorAttachmentCount = 1;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pDepthStencilAttachment-01418",
                         "VUID-VkSubpassDescription2KHR-pDepthStencilAttachment-03071");

    attachments[subpass.pDepthStencilAttachment->attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;
    subpass.colorAttachmentCount = (uint32_t)color.size();

    // Test resolve attachment with UNUSED color attachment
    color[0].attachment = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00847",
                         "VUID-VkSubpassDescription2KHR-pResolveAttachments-03065");

    color[0].attachment = 1;

    // Test resolve from a single-sampled color attachment
    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;
    subpass.colorAttachmentCount = 1;           // avoid mismatch (00337), and avoid double report
    subpass.pDepthStencilAttachment = nullptr;  // avoid mismatch (01418)

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00848",
                         "VUID-VkSubpassDescription2KHR-pResolveAttachments-03066");

    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;
    subpass.colorAttachmentCount = (uint32_t)color.size();
    subpass.pDepthStencilAttachment = &depth;

    // Test resolve to a multi-sampled resolve attachment
    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00849",
                         "VUID-VkSubpassDescription2KHR-pResolveAttachments-03067");

    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;

    // Test with color/resolve format mismatch
    attachments[subpass.pColorAttachments[0].attachment].format = VK_FORMAT_R8G8B8A8_SRGB;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00850",
                         "VUID-VkSubpassDescription2KHR-pResolveAttachments-03068");

    attachments[subpass.pColorAttachments[0].attachment].format = attachments[subpass.pResolveAttachments[0].attachment].format;

    // Test for UNUSED preserve attachments
    preserve[0] = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-attachment-00853",
                         "VUID-VkSubpassDescription2KHR-attachment-03073");

    preserve[0] = 5;
    // Test for preserve attachments used elsewhere in the subpass
    color[0].attachment = preserve[0];

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pPreserveAttachments-00854",
                         "VUID-VkSubpassDescription2KHR-pPreserveAttachments-03074");

    color[0].attachment = 1;
    input[0].attachment = 0;
    input[0].layout = VK_IMAGE_LAYOUT_GENERAL;

    // Test for attachment used first as input with loadOp=CLEAR
    {
        std::vector<VkSubpassDescription> subpasses = {subpass, subpass, subpass};
        subpasses[0].inputAttachmentCount = 0;
        subpasses[1].inputAttachmentCount = 0;
        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkRenderPassCreateInfo rpci_multipass = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                 nullptr,
                                                 0,
                                                 (uint32_t)attachments.size(),
                                                 attachments.data(),
                                                 (uint32_t)subpasses.size(),
                                                 subpasses.data(),
                                                 0,
                                                 nullptr};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci_multipass, rp2Supported,
                             "VUID-VkSubpassDescription-loadOp-00846", "VUID-VkSubpassDescription2KHR-loadOp-03064");

        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentReferenceInvalidLayout) {
    TEST_DESCRIPTION("Attachment reference uses PREINITIALIZED or UNDEFINED layouts");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_UNDEFINED},
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, refs, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};

    // Use UNDEFINED layout
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentReference-layout-00857",
                         "VUID-VkAttachmentReference2KHR-layout-03077");

    // Use PREINITIALIZED layout
    refs[0].layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentReference-layout-00857",
                         "VUID-VkAttachmentReference2KHR-layout-03077");
}

TEST_F(VkLayerTest, RenderPassCreateOverlappingCorrelationMasks) {
    TEST_DESCRIPTION("Create a subpass with overlapping correlation masks");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u, 0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, viewMasks, 0, nullptr, 2, correlationMasks};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 1, &subpass, 0, nullptr};

    // Correlation masks must not overlap
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841",
                         "VUID-VkRenderPassCreateInfo2KHR-pCorrelatedViewMasks-03056");

    // Check for more specific "don't set any correlation masks when multiview is not enabled"
    if (rp2Supported) {
        viewMasks[0] = 0;
        correlationMasks[0] = 0;
        correlationMasks[1] = 0;
        safe_VkRenderPassCreateInfo2KHR safe_rpci2;
        ConvertVkRenderPassCreateInfoToV2KHR(&rpci, &safe_rpci2);

        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), safe_rpci2.ptr(),
                                 "VUID-VkRenderPassCreateInfo2KHR-viewMask-03057");
    }
}

TEST_F(VkLayerTest, RenderPassCreateInvalidViewMasks) {
    TEST_DESCRIPTION("Create a subpass with the wrong number of view masks, or inconsistent setting of view masks");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };
    uint32_t viewMasks[] = {0x3u, 0u};
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, viewMasks, 0, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 2, subpasses, 0, nullptr};

    // Not enough view masks
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-01928",
                         "VUID-VkRenderPassCreateInfo2KHR-viewMask-03058");
}

TEST_F(VkLayerTest, RenderPassCreateInvalidInputAttachmentReferences) {
    TEST_DESCRIPTION("Create a subpass with the meta data aspect mask set for an input attachment");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};

    // Invalid meta data aspect
    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRenderPassCreateInfo-pNext-01963");  // Cannot/should not avoid getting this one too
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964",
                         nullptr);

    // Aspect not present
    iaar.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01963", nullptr);

    // Invalid subpass index
    iaar.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iaar.subpass = 1;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01926", nullptr);
    iaar.subpass = 0;

    // Invalid input attachment index
    iaar.inputAttachmentIndex = 1;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01927", nullptr);
}

TEST_F(VkLayerTest, RenderPassCreateInvalidFragmentDensityMapReferences) {
    TEST_DESCRIPTION("Create a subpass with the wrong attachment information for a fragment density map ");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_LOAD,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    // Set 1 instead of 0
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
                                                          nullptr, ref};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02547", nullptr);

    // Set wrong VkImageLayout
    ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549", nullptr);

    // Set wrong load operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_CLEAR,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_UNDEFINED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550", nullptr);

    // Set wrong store operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_LOAD,
              VK_ATTACHMENT_STORE_OP_STORE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_UNDEFINED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551", nullptr);
}

TEST_F(VkLayerTest, RenderPassCreateSubpassNonGraphicsPipeline) {
    TEST_DESCRIPTION("Create a subpass with the compute pipeline bind point");
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_COMPUTE, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pipelineBindPoint-00844",
                         "VUID-VkSubpassDescription2KHR-pipelineBindPoint-03062");
}

TEST_F(VkLayerTest, RenderPassCreateSubpassMissingAttributesBitMultiviewNVX) {
    TEST_DESCRIPTION("Create a subpass with the VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX flag missing");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        return;
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr,
         nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-flags-00856",
                         "VUID-VkSubpassDescription2KHR-flags-03076");
}

TEST_F(VkLayerTest, RenderPassCreate2SubpassInvalidInputAttachmentParameters) {
    TEST_DESCRIPTION("Create a subpass with parameters in the input attachment ref which are invalid");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentReference2KHR reference = {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR, nullptr, VK_ATTACHMENT_UNUSED,
                                           VK_IMAGE_LAYOUT_UNDEFINED, 0};
    VkSubpassDescription2KHR subpass = {VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR,
                                        nullptr,
                                        0,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        0,
                                        1,
                                        &reference,
                                        0,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        0,
                                        nullptr};

    VkRenderPassCreateInfo2KHR rpci2 = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR, nullptr, 0, 0, nullptr, 1, &subpass, 0, nullptr, 0, nullptr};

    // Test for aspect mask of 0
    TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), &rpci2, "VUID-VkSubpassDescription2KHR-aspectMask-03176");

    // Test for invalid aspect mask bits
    reference.aspectMask |= VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), &rpci2, "VUID-VkSubpassDescription2KHR-aspectMask-03175");
}

TEST_F(VkLayerTest, RenderPassCreateInvalidSubpassDependencies) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    bool rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool multiviewSupported = rp2_supported;

    if (!rp2_supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        multiviewSupported = true;
    }

    // Add a device features struct enabling NO features
    VkPhysicalDeviceFeatures features = {0};
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    if (m_device->props.apiVersion >= VK_API_VERSION_1_1) {
        multiviewSupported = true;
    }

    // Create two dummy subpasses
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependency;
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 2, subpasses, 1, &dependency};

    // Non graphics stages in subpass dependency
    dependency = {0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03055");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03055");

    dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054");

    dependency = {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03055");

    dependency = {0, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03054");

    // Geometry shaders not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-00860",
                         "VUID-VkSubpassDependency2KHR-srcStageMask-03080");

    // Geometry shaders not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-00861",
                         "VUID-VkSubpassDependency2KHR-dstStageMask-03081");

    // Tessellation not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-00862",
                         "VUID-VkSubpassDependency2KHR-srcStageMask-03082");

    // Tessellation not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-00863",
                         "VUID-VkSubpassDependency2KHR-dstStageMask-03083");

    // Potential cyclical dependency
    dependency = {1, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00864",
                         "VUID-VkSubpassDependency2KHR-srcSubpass-03084");

    // EXTERNAL to EXTERNAL dependency
    dependency = {
        VK_SUBPASS_EXTERNAL, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00865",
                         "VUID-VkSubpassDependency2KHR-srcSubpass-03085");

    // Logically later source stages in self dependency
    dependency = {0, 0, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00867",
                         "VUID-VkSubpassDependency2KHR-srcSubpass-03087");

    // Source access mask mismatch with source stage mask
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_UNIFORM_READ_BIT, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcAccessMask-00868",
                         "VUID-VkSubpassDependency2KHR-srcAccessMask-03088");

    // Destination access mask mismatch with destination stage mask
    dependency = {
        0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstAccessMask-00869",
                         "VUID-VkSubpassDependency2KHR-dstAccessMask-03089");

    if (multiviewSupported) {
        // VIEW_LOCAL_BIT but multiview is not enabled
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, nullptr,
                             "VUID-VkRenderPassCreateInfo2KHR-viewMask-03059");

        // Enable multiview
        uint32_t pViewMasks[2] = {0x3u, 0x3u};
        int32_t pViewOffsets[2] = {0, 0};
        VkRenderPassMultiviewCreateInfo rpmvci = {
            VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 2, pViewMasks, 0, nullptr, 0, nullptr};
        rpci.pNext = &rpmvci;

        // Excessive view offsets
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        rpmvci.dependencyCount = 2;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01929", nullptr);

        rpmvci.dependencyCount = 0;

        // View offset with subpass self dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        pViewOffsets[0] = 1;
        rpmvci.dependencyCount = 1;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01930", nullptr);

        rpmvci.dependencyCount = 0;

        // View offset with no view local bit
        if (rp2_supported) {
            dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
            rpmvci.pViewOffsets = pViewOffsets;
            pViewOffsets[0] = 1;
            rpmvci.dependencyCount = 1;

            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, nullptr,
                                 "VUID-VkSubpassDependency2KHR-dependencyFlags-03092");

            rpmvci.dependencyCount = 0;
        }

        // EXTERNAL subpass with VIEW_LOCAL_BIT - source subpass
        dependency = {VK_SUBPASS_EXTERNAL,         1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                      VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                             "VUID-VkSubpassDependency-dependencyFlags-02520",
                             "VUID-VkSubpassDependency2KHR-dependencyFlags-03090");

        // EXTERNAL subpass with VIEW_LOCAL_BIT - destination subpass
        dependency = {0, VK_SUBPASS_EXTERNAL,         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                      0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                             "VUID-VkSubpassDependency-dependencyFlags-02521",
                             "VUID-VkSubpassDependency2KHR-dependencyFlags-03091");

        // Multiple views but no view local bit in self-dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00872",
                             "VUID-VkRenderPassCreateInfo2KHR-pDependencies-03060");
    }
}

TEST_F(VkLayerTest, RenderPassCreateInvalidMixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Verify error messages for supported and unsupported sample counts in render pass attachments.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        return;
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    ASSERT_NO_FATAL_FAILURE(InitState());

    std::vector<VkAttachmentDescription> attachments;

    {
        VkAttachmentDescription att = {};
        att.format = VK_FORMAT_R8G8B8A8_UNORM;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);

        att.format = VK_FORMAT_D16_UNORM;
        att.samples = VK_SAMPLE_COUNT_4_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);
    }

    VkAttachmentReference color_ref = {};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_ref = {};
    depth_ref.attachment = 1;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = attachments.size();
    rpci.pAttachments = attachments.data();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    m_errorMonitor->ExpectSuccess();

    VkRenderPass rp;
    VkResult err;

    err = vkCreateRenderPass(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyNotFound();
    if (err == VK_SUCCESS) vkDestroyRenderPass(m_device->device(), rp, nullptr);

    // Expect an error message for invalid sample counts
    attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pColorAttachments-01506",
                         "VUID-VkSubpassDescription2KHR-pColorAttachments-03070");
}

TEST_F(VkLayerTest, RenderPassBeginInvalidRenderArea) {
    TEST_DESCRIPTION("Generate INVALID_RENDER_AREA error by beginning renderpass with extent outside of framebuffer");
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Framebuffer for render target is 256x256, exceed that for INVALID_RENDER_AREA
    m_renderPassBeginInfo.renderArea.extent.width = 257;
    m_renderPassBeginInfo.renderArea.extent.height = 257;

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &m_renderPassBeginInfo, rp2Supported,
                        "Cannot execute a render pass with renderArea not within the bound of the framebuffer.",
                        "Cannot execute a render pass with renderArea not within the bound of the framebuffer.");
}

TEST_F(VkLayerTest, RenderPassBeginWithinRenderPass) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (rp2Supported) {
        vkCmdBeginRenderPass2KHR =
            (PFN_vkCmdBeginRenderPass2KHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdBeginRenderPass2KHR");
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Bind a BeginRenderPass within an active RenderPass
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Just use a dummy Renderpass
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginRenderPass-renderpass");
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassBeginInfoKHR subpassBeginInfo = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginRenderPass2KHR-renderpass");
        vkCmdBeginRenderPass2KHR(m_commandBuffer->handle(), &m_renderPassBeginInfo, &subpassBeginInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RenderPassBeginIncompatibleFramebufferRenderPass) {
    TEST_DESCRIPTION("Test that renderpass begin is compatible with the framebuffer renderpass ");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Create a depth stencil image view
    VkImageObj image(m_device);

    image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    VkImageView dsv;
    VkImageViewCreateInfo dsvci = {};
    dsvci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    dsvci.pNext = nullptr;
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = VK_FORMAT_D16_UNORM;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vkCreateImageView(m_device->device(), &dsvci, NULL, &dsv);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};
    VkRenderPass rp1, rp2;

    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp1);
    subpass.pDepthStencilAttachment = nullptr;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp2);

    // Create a framebuffer

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp1, 1, &dsv, 128, 128, 1};
    VkFramebuffer fb;

    vkCreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);

    VkRenderPassBeginInfo rp_begin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp2, fb, {{0, 0}, {128, 128}}, 0, nullptr};

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkRenderPassBeginInfo-renderPass-00904", nullptr);

    vkDestroyRenderPass(m_device->device(), rp1, nullptr);
    vkDestroyRenderPass(m_device->device(), rp2, nullptr);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyImageView(m_device->device(), dsv, nullptr);
}

TEST_F(VkLayerTest, RenderPassBeginLayoutsFramebufferImageUsageMismatches) {
    TEST_DESCRIPTION(
        "Test that renderpass initial/final layouts match up with the usage bits set for each attachment of the framebuffer");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool maintenance2Supported = rp2Supported;

    // Check for VK_KHR_maintenance2
    if (!rp2Supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        maintenance2Supported = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (m_device->props.apiVersion >= VK_API_VERSION_1_1) {
        maintenance2Supported = true;
    }

    // Create an input attachment view
    VkImageObj iai(m_device);

    iai.InitNoLayout(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(iai.initialized());

    VkImageView iav;
    VkImageViewCreateInfo iavci = {};
    iavci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    iavci.pNext = nullptr;
    iavci.image = iai.handle();
    iavci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iavci.format = VK_FORMAT_R8G8B8A8_UNORM;
    iavci.subresourceRange.layerCount = 1;
    iavci.subresourceRange.baseMipLevel = 0;
    iavci.subresourceRange.levelCount = 1;
    iavci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCreateImageView(m_device->device(), &iavci, NULL, &iav);

    // Create a color attachment view
    VkImageObj cai(m_device);

    cai.InitNoLayout(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(cai.initialized());

    VkImageView cav;
    VkImageViewCreateInfo cavci = {};
    cavci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    cavci.pNext = nullptr;
    cavci.image = cai.handle();
    cavci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    cavci.format = VK_FORMAT_R8G8B8A8_UNORM;
    cavci.subresourceRange.layerCount = 1;
    cavci.subresourceRange.baseMipLevel = 0;
    cavci.subresourceRange.levelCount = 1;
    cavci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCreateImageView(m_device->device(), &cavci, NULL, &cav);

    // Create a renderPass with those attachments
    VkAttachmentDescription descriptions[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        {1, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference input_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input_ref, 1, &color_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, descriptions, 1, &subpass, 0, nullptr};

    VkRenderPass rp;

    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    // Create a framebuffer

    VkImageView views[] = {iav, cav};

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, views, 128, 128, 1};
    VkFramebuffer fb;

    vkCreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);

    VkRenderPassBeginInfo rp_begin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {128, 128}}, 0, nullptr};

    VkRenderPass rp_invalid;

    // Initial layout is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but attachment doesn't support IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-vkCmdBeginRenderPass-initialLayout-00895", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03094");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

    // Initial layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    // / VK_IMAGE_USAGE_SAMPLED_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-vkCmdBeginRenderPass-initialLayout-00897", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03097");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-vkCmdBeginRenderPass-initialLayout-00898", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03098");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-vkCmdBeginRenderPass-initialLayout-00899", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03099");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;
    const char *initial_layout_vuid_rp1 =
        maintenance2Supported ? "VUID-vkCmdBeginRenderPass-initialLayout-01758" : "VUID-vkCmdBeginRenderPass-initialLayout-00896";

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        initial_layout_vuid_rp1, "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03096");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
    rp_begin.renderPass = rp_invalid;

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        initial_layout_vuid_rp1, "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03096");

    vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

    if (maintenance2Supported || rp2Supported) {
        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
        rp_begin.renderPass = rp_invalid;

        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                            "VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03096");

        vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);

        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_invalid);
        rp_begin.renderPass = rp_invalid;

        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                            "VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03096");

        vkDestroyRenderPass(m_device->handle(), rp_invalid, nullptr);
    }

    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyImageView(m_device->device(), iav, nullptr);
    vkDestroyImageView(m_device->device(), cav, nullptr);
}

TEST_F(VkLayerTest, RenderPassBeginClearOpMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where clearValueCount is less than the number of renderPass attachments that use "
        "loadOp VK_ATTACHMENT_LOAD_OP_CLEAR.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    // Set loadOp to CLEAR
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    VkRenderPassBeginInfo rp_begin = {};
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext = NULL;
    rp_begin.renderPass = renderPass();
    rp_begin.framebuffer = framebuffer();
    rp_begin.clearValueCount = 0;  // Should be 1

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-clearValueCount-00902", "VUID-VkRenderPassBeginInfo-clearValueCount-00902");

    vkDestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, RenderPassBeginSampleLocationsInvalidIndicesEXT) {
    TEST_DESCRIPTION("Test that attachment indices and subpass indices specifed by sample locations structures are valid");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Create a depth stencil image view
    VkImageObj image(m_device);

    image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    VkImageView dsv;
    VkImageViewCreateInfo dsvci = {};
    dsvci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    dsvci.pNext = nullptr;
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = VK_FORMAT_D16_UNORM;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vkCreateImageView(m_device->device(), &dsvci, NULL, &dsv);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};
    VkRenderPass rp;

    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    // Create a framebuffer

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &dsv, 128, 128, 1};
    VkFramebuffer fb;

    vkCreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);

    VkSampleLocationEXT sample_location = {0.5, 0.5};

    VkSampleLocationsInfoEXT sample_locations_info = {
        VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT, nullptr, VK_SAMPLE_COUNT_1_BIT, {1, 1}, 1, &sample_location};

    VkAttachmentSampleLocationsEXT attachment_sample_locations = {0, sample_locations_info};
    VkSubpassSampleLocationsEXT subpass_sample_locations = {0, sample_locations_info};

    VkRenderPassSampleLocationsBeginInfoEXT rp_sl_begin = {VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT,
                                                           nullptr,
                                                           1,
                                                           &attachment_sample_locations,
                                                           1,
                                                           &subpass_sample_locations};

    VkRenderPassBeginInfo rp_begin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, &rp_sl_begin, rp, fb, {{0, 0}, {128, 128}}, 0, nullptr};

    attachment_sample_locations.attachmentIndex = 1;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkAttachmentSampleLocationsEXT-attachmentIndex-01531", nullptr);
    attachment_sample_locations.attachmentIndex = 0;

    subpass_sample_locations.subpassIndex = 1;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkSubpassSampleLocationsEXT-subpassIndex-01532", nullptr);
    subpass_sample_locations.subpassIndex = 0;

    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyImageView(m_device->device(), dsv, nullptr);
}

TEST_F(VkLayerTest, RenderPassNextSubpassExcessive) {
    TEST_DESCRIPTION("Test that an error is produced when CmdNextSubpass is called too many times in a renderpass instance");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    PFN_vkCmdNextSubpass2KHR vkCmdNextSubpass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (rp2Supported) {
        vkCmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdNextSubpass2KHR");
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdNextSubpass-None-00909");
    vkCmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassBeginInfoKHR subpassBeginInfo = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};
        VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdNextSubpass2KHR-None-03102");

        vkCmdNextSubpass2KHR(m_commandBuffer->handle(), &subpassBeginInfo, &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RenderPassEndBeforeFinalSubpass) {
    TEST_DESCRIPTION("Test that an error is produced when CmdEndRenderPass is called before the final subpass has been reached");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (rp2Supported) {
        vkCmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdEndRenderPass2KHR");
    }

    VkSubpassDescription sd[2] = {{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
                                  {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr}};

    VkRenderPassCreateInfo rcpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 2, sd, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rcpi, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 0, nullptr, 16, 16, 1};

    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {16, 16}}, 0, nullptr};

    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdEndRenderPass-None-00910");
    vkCmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdEndRenderPass2KHR-None-03103");
        vkCmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    // Clean up.
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, RenderPassDestroyWhileInUse) {
    TEST_DESCRIPTION("Delete in-use renderPass.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create simple renderpass
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->ExpectSuccess();

    m_commandBuffer->begin();
    VkRenderPassBeginInfo rpbi = {};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.framebuffer = m_framebuffer;
    rpbi.renderPass = rp;
    m_commandBuffer->BeginRenderPass(rpbi);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyRenderPass-renderPass-00873");
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyFound();

    // Wait for queue to complete so we can safely destroy rp
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If renderPass is not VK_NULL_HANDLE, renderPass must be a valid VkRenderPass handle");
    m_errorMonitor->SetUnexpectedError("Was it created? Has it already been destroyed?");
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, FramebufferCreateErrors) {
    TEST_DESCRIPTION(
        "Hit errors when attempting to create a framebuffer :\n"
        " 1. Mismatch between framebuffer & renderPass attachmentCount\n"
        " 2. Use a color image as depthStencil attachment\n"
        " 3. Mismatch framebuffer & renderPass attachment formats\n"
        " 4. Mismatch framebuffer & renderPass attachment #samples\n"
        " 5. Framebuffer attachment w/ non-1 mip-levels\n"
        " 6. Framebuffer attachment where dimensions don't match\n"
        " 7. Framebuffer attachment where dimensions don't match\n"
        " 8. Framebuffer attachment w/o identity swizzle\n"
        " 9. framebuffer dimensions exceed physical device limits\n"
        "10. null pAttachments\n");

    // Check for VK_KHR_get_physical_device_properties2
    bool push_physical_device_properties_2_support =
        InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (push_physical_device_properties_2_support) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    bool push_fragment_density_support = false;

    if (push_physical_device_properties_2_support) {
        push_fragment_density_support = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        if (push_fragment_density_support) m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, 0));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-attachmentCount-00876");

    // Create a renderPass with a single color attachment
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageView ivs[2];
    ivs[0] = m_renderTargets[0]->targetView(VK_FORMAT_B8G8R8A8_UNORM);
    ivs[1] = m_renderTargets[0]->targetView(VK_FORMAT_B8G8R8A8_UNORM);
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = rp;
    // Set mis-matching attachmentCount
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = ivs;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;

    VkFramebuffer fb;
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vkDestroyRenderPass(m_device->device(), rp, NULL);

    // Create a renderPass with a depth-stencil attachment created with
    // IMAGE_USAGE_COLOR_ATTACHMENT
    // Add our color attachment to pDepthStencilAttachment
    subpass.pDepthStencilAttachment = &attach;
    subpass.pColorAttachments = NULL;
    VkRenderPass rp_ds;
    err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp_ds);
    ASSERT_VK_SUCCESS(err);
    // Set correct attachment count, but attachment has COLOR usage bit set
    fb_info.attachmentCount = 1;
    fb_info.renderPass = rp_ds;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-02633");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vkDestroyRenderPass(m_device->device(), rp_ds, NULL);

    // Create new renderpass with alternate attachment format from fb
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    subpass.pDepthStencilAttachment = NULL;
    subpass.pColorAttachments = &attach;
    err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    // Cause error due to mis-matched formats between rp & fb
    //  rp attachment 0 now has RGBA8 but corresponding fb attach is BGRA8
    fb_info.renderPass = rp;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00880");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vkDestroyRenderPass(m_device->device(), rp, NULL);

    // Create new renderpass with alternate sample count from fb
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_4_BIT;
    err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    // Cause error due to mis-matched sample count between rp & fb
    fb_info.renderPass = rp;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00881");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    vkDestroyRenderPass(m_device->device(), rp, NULL);

    {
        // Create an image with 2 mip levels.
        VkImageObj image(m_device);
        image.Init(128, 128, 2, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image.initialized());

        // Create a image view with two mip levels.
        VkImageView view;
        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        // Set level count to 2 (only 1 is allowed for FB attachment)
        ivci.subresourceRange.levelCount = 2;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        err = vkCreateImageView(m_device->device(), &ivci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        // Re-create renderpass to have matching sample count
        attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
        ASSERT_VK_SUCCESS(err);

        fb_info.renderPass = rp;
        fb_info.pAttachments = &view;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00883");
        err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vkDestroyFramebuffer(m_device->device(), fb, NULL);
        }
        vkDestroyImageView(m_device->device(), view, NULL);
    }

    // Update view to original color buffer and grow FB dimensions too big
    fb_info.pAttachments = ivs;
    fb_info.height = 1024;
    fb_info.width = 1024;
    fb_info.layers = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00882");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    {
        if (!push_fragment_density_support) {
            printf("%s VK_EXT_fragment_density_map Extension not supported, skipping tests\n", kSkipPrefix);
        } else {
            uint32_t attachment_width = 512;
            uint32_t attachment_height = 512;
            VkFormat attachment_format = VK_FORMAT_R8G8_UNORM;
            uint32_t frame_width = 512;
            uint32_t frame_height = 512;

            // Create a renderPass with a single color attachment for fragment density map
            VkAttachmentReference attach_fragment_density_map = {};
            attach_fragment_density_map.layout = VK_IMAGE_LAYOUT_GENERAL;
            VkSubpassDescription subpass_fragment_density_map = {};
            subpass_fragment_density_map.pColorAttachments = &attach_fragment_density_map;
            VkRenderPassCreateInfo rpci_fragment_density_map = {};
            rpci_fragment_density_map.subpassCount = 1;
            rpci_fragment_density_map.pSubpasses = &subpass_fragment_density_map;
            rpci_fragment_density_map.attachmentCount = 1;
            VkAttachmentDescription attach_desc_fragment_density_map = {};
            attach_desc_fragment_density_map.format = attachment_format;
            attach_desc_fragment_density_map.samples = VK_SAMPLE_COUNT_1_BIT;
            attach_desc_fragment_density_map.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
            rpci_fragment_density_map.pAttachments = &attach_desc_fragment_density_map;
            rpci_fragment_density_map.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            VkRenderPass rp_fragment_density_map;

            err = vkCreateRenderPass(m_device->device(), &rpci_fragment_density_map, NULL, &rp_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            // Create view attachment
            VkImageView view_fragment_density_map;
            VkImageViewCreateInfo ivci = {};
            ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.format = attachment_format;
            ivci.flags = 0;
            ivci.subresourceRange.layerCount = 1;
            ivci.subresourceRange.baseMipLevel = 0;
            ivci.subresourceRange.levelCount = 1;
            ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            VkFramebufferAttachmentImageInfoKHR fb_fdm = {};
            fb_fdm.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR;
            fb_fdm.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
            fb_fdm.width = frame_width;
            fb_fdm.height = frame_height;
            fb_fdm.layerCount = 1;
            fb_fdm.viewFormatCount = 1;
            fb_fdm.pViewFormats = &attachment_format;
            VkFramebufferAttachmentsCreateInfoKHR fb_aci_fdm = {};
            fb_aci_fdm.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
            fb_aci_fdm.attachmentImageInfoCount = 1;
            fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

            VkFramebufferCreateInfo fbci = {};
            fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbci.pNext = &fb_aci_fdm;
            fbci.flags = 0;
            fbci.width = frame_width;
            fbci.height = frame_height;
            fbci.layers = 1;
            fbci.renderPass = rp_fragment_density_map;
            fbci.attachmentCount = 1;
            fbci.pAttachments = &view_fragment_density_map;

            // Set small width
            VkImageObj image2(m_device);
            image2.Init(16, attachment_height, 1, attachment_format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
                        VK_IMAGE_TILING_LINEAR, 0);
            ASSERT_TRUE(image2.initialized());

            ivci.image = image2.handle();
            err = vkCreateImageView(m_device->device(), &ivci, NULL, &view_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            fbci.pAttachments = &view_fragment_density_map;

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-02555");
            err = vkCreateFramebuffer(device(), &fbci, NULL, &fb);

            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vkDestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vkDestroyImageView(m_device->device(), view_fragment_density_map, NULL);

            // Set small height
            VkImageObj image3(m_device);
            image3.Init(attachment_width, 16, 1, attachment_format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
                        VK_IMAGE_TILING_LINEAR, 0);
            ASSERT_TRUE(image3.initialized());

            ivci.image = image3.handle();
            err = vkCreateImageView(m_device->device(), &ivci, NULL, &view_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            fbci.pAttachments = &view_fragment_density_map;

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-02556");
            err = vkCreateFramebuffer(device(), &fbci, NULL, &fb);

            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vkDestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vkDestroyImageView(m_device->device(), view_fragment_density_map, NULL);

            vkDestroyRenderPass(m_device->device(), rp_fragment_density_map, NULL);
        }
    }

    {
        // Create an image with one mip level.
        VkImageObj image(m_device);
        image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image.initialized());

        // Create view attachment with non-identity swizzle
        VkImageView view;
        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.components.r = VK_COMPONENT_SWIZZLE_G;
        ivci.components.g = VK_COMPONENT_SWIZZLE_R;
        ivci.components.b = VK_COMPONENT_SWIZZLE_A;
        ivci.components.a = VK_COMPONENT_SWIZZLE_B;
        err = vkCreateImageView(m_device->device(), &ivci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        fb_info.pAttachments = &view;
        fb_info.height = 100;
        fb_info.width = 100;
        fb_info.layers = 1;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00884");
        err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);

        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vkDestroyFramebuffer(m_device->device(), fb, NULL);
        }
        vkDestroyImageView(m_device->device(), view, NULL);
    }

    // reset attachment to color attachment
    fb_info.pAttachments = ivs;

    // Request fb that exceeds max width
    fb_info.width = m_device->props.limits.maxFramebufferWidth + 1;
    fb_info.height = 100;
    fb_info.layers = 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-width-00886");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00882");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and width=0
    fb_info.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-width-00885");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Request fb that exceeds max height
    fb_info.width = 100;
    fb_info.height = m_device->props.limits.maxFramebufferHeight + 1;
    fb_info.layers = 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-height-00888");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00882");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and height=0
    fb_info.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-height-00887");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Request fb that exceeds max layers
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = m_device->props.limits.maxFramebufferLayers + 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-layers-00890");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-pAttachments-00882");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and layers=0
    fb_info.layers = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkFramebufferCreateInfo-layers-00889");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Try to create with pAttachments = NULL
    fb_info.layers = 1;
    fb_info.pAttachments = NULL;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID_Undefined");
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vkDestroyFramebuffer(m_device->device(), fb, NULL);
    }

    vkDestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, AllocDescriptorFromEmptyPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // This test is valid for Vulkan 1.0 only -- skip if device has an API version greater than 1.0.
    if (m_device->props.apiVersion >= VK_API_VERSION_1_1) {
        printf("%s Device has apiVersion greater than 1.0 -- skipping Descriptor Set checks.\n", kSkipPrefix);
        return;
    }

    // Create Pool w/ 1 Sampler descriptor, but try to alloc Uniform Buffer
    // descriptor from it
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_samp(m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-00306");
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();

    alloc_info.descriptorSetCount = 1;
    // Create layout w/ descriptor type not available in pool
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_ub(m_device, {dsl_binding});

    VkDescriptorSet descriptor_set;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_ub.handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetAllocateInfo-descriptorPool-00307");
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_set);

    m_errorMonitor->VerifyFound();

    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, FreeDescriptorFromOneShotPool) {
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkFreeDescriptorSets-descriptorPool-00312");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    // Not specifying VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT means
    // app can only call vkResetDescriptorPool on this pool.;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
    ASSERT_VK_SUCCESS(err);

    err = vkFreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);
    m_errorMonitor->VerifyFound();

    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, InvalidDescriptorPool) {
    // Attempt to clear Descriptor Pool with bad object.
    // ObjectTracker should catch this.

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetDescriptorPool-descriptorPool-parameter");
    uint64_t fake_pool_handle = 0xbaad6001;
    VkDescriptorPool bad_pool = reinterpret_cast<VkDescriptorPool &>(fake_pool_handle);
    vkResetDescriptorPool(device(), bad_pool, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDescriptorSet) {
    // Attempt to bind an invalid Descriptor Set to a valid Command Buffer
    // ObjectTracker should catch this.
    // Create a valid cmd buffer
    // call vkCmdBindDescriptorSets w/ false Descriptor Set

    uint64_t fake_set_handle = 0xbaad6001;
    VkDescriptorSet bad_set = reinterpret_cast<VkDescriptorSet &>(fake_set_handle);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-parameter");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});

    m_commandBuffer->begin();
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &bad_set, 0,
                            NULL);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidDescriptorSetLayout) {
    // Attempt to create a Pipeline Layout with an invalid Descriptor Set Layout.
    // ObjectTracker should catch this.
    uint64_t fake_layout_handle = 0xbaad6001;
    VkDescriptorSetLayout bad_layout = reinterpret_cast<VkDescriptorSetLayout &>(fake_layout_handle);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo plci = {};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.pNext = NULL;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &bad_layout;
    vkCreatePipelineLayout(device(), &plci, NULL, &pipeline_layout);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, WriteDescriptorSetIntegrityCheck) {
    TEST_DESCRIPTION(
        "This test verifies some requirements of chapter 13.2.3 of the Vulkan Spec "
        "1) A uniform buffer update must have a valid buffer index. "
        "2) When using an array of descriptors in a single WriteDescriptor, the descriptor types and stageflags "
        "must all be the same. "
        "3) Immutable Sampler state must match across descriptors. "
        "4) That sampled image descriptors have required layouts. ");

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-00324");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet::Bindings bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, NULL},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<VkSampler *>(&sampler)},
        {3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL}};
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    ASSERT_TRUE(descriptor_set.Initialized());

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    // 1) The uniform buffer is intentionally invalid here
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer;
    dynamic_uniform_buffer.init(*m_device, buffCI);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer.handle();
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;
    buffInfo[1].buffer = dynamic_uniform_buffer.handle();
    buffInfo[1].offset = 0;
    buffInfo[1].range = 1024;
    descriptor_write.pBufferInfo = buffInfo;
    descriptor_write.descriptorCount = 2;

    // 2) The stateFlags don't match between the first and second descriptor
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // 3) The second descriptor has a null_ptr pImmutableSamplers and
    // the third descriptor contains an immutable sampler
    descriptor_write.dstBinding = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

    // Make pImageInfo index non-null to avoid complaints of it missing
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_write.pImageInfo = &imageInfo;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // 4) That sampled image descriptors have required layouts
    // Create images to update the descriptor with
    VkImageObj image(m_device);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    // Attmept write with incorrect layout for sampled descriptor
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = image.targetView(tex_format);
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    descriptor_write.dstBinding = 3;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-01403");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, WriteDescriptorSetConsecutiveUpdates) {
    TEST_DESCRIPTION(
        "Verifies that updates rolling over to next descriptor work correctly by destroying buffer from consecutive update known "
        "to be used in descriptor set and verifying that error is flagged.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 2048;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    buffer0.init(*m_device, bci);
    CreatePipelineHelper pipe(*this);
    {  // Scope 2nd buffer to cause early destruction
        VkBufferObj buffer1;
        bci.size = 1024;
        buffer1.init(*m_device, bci);

        VkDescriptorBufferInfo buffer_info[3] = {};
        buffer_info[0].buffer = buffer0.handle();
        buffer_info[0].offset = 0;
        buffer_info[0].range = 1024;
        buffer_info[1].buffer = buffer0.handle();
        buffer_info[1].offset = 1024;
        buffer_info[1].range = 1024;
        buffer_info[2].buffer = buffer1.handle();
        buffer_info[2].offset = 0;
        buffer_info[2].range = 1024;

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set.set_;  // descriptor_set;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 3;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = buffer_info;

        // Update descriptor
        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        // Create PSO that uses the uniform buffers
        char const *fsSource =
            "#version 450\n"
            "\n"
            "layout(location=0) out vec4 x;\n"
            "layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;\n"
            "layout(set=0) layout(binding=1) uniform blah { int x; } duh;\n"
            "void main(){\n"
            "   x = vec4(duh.x, bar.y, bar.x, 1);\n"
            "}\n";
        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
        dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                &descriptor_set.set_, 0, nullptr);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        vkCmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vkCmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
    }
    // buffer2 just went out of scope and was destroyed along with its memory
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferDescriptorSetBufferDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor set with a buffer dependency being "
        "destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    {
        // Create a buffer to update the descriptor with
        uint32_t qfi = 0;
        VkBufferCreateInfo buffCI = {};
        buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffCI.size = 1024;
        buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        VkBufferObj buffer;
        buffer.init(*m_device, buffCI);

        // Create PSO to be used for draw-time errors below
        char const *fsSource =
            "#version 450\n"
            "\n"
            "layout(location=0) out vec4 x;\n"
            "layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;\n"
            "void main(){\n"
            "   x = vec4(bar.y);\n"
            "}\n";
        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
        dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Correctly update descriptor to avoid "NOT_UPDATED" error
        pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 1024);
        pipe.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                &pipe.descriptor_set_->set_, 0, NULL);

        vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &m_viewports[0]);
        vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &m_scissors[0]);

        m_commandBuffer->Draw(1, 0, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
    // Destroy buffer should invalidate the cmd buffer, causing error on submit

    // Attempt to submit cmd buffer
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Invalid VkBuffe
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffe");
    // Invalid VkDeviceMemory
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " that is invalid because bound ");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferDescriptorSetImageSamplerDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor sets with a combined image sampler having "
        "their image, sampler, and descriptor set each respectively destroyed and then attempting to submit associated cmd "
        "buffers. Attempt to destroy a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    VkResult err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});

    // Create images to update the descriptor with
    VkImage image;
    VkImage image2;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;
    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);
    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image2);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = NULL;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Allocate enough memory for both images
    VkDeviceSize align_mod = memory_reqs.size % memory_reqs.alignment;
    VkDeviceSize aligned_size = ((align_mod == 0) ? memory_reqs.size : (memory_reqs.size + memory_reqs.alignment - align_mod));
    memory_info.allocationSize = aligned_size * 2;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(err);
    err = vkBindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(err);
    // Bind second image to memory right after first image
    err = vkBindImageMemory(m_device->device(), image2, image_memory, aligned_size);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView tmp_view;  // First test deletes this view
    VkImageView view;
    VkImageView view2;
    err = vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &tmp_view);
    ASSERT_VK_SUCCESS(err);
    err = vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    image_view_create_info.image = image2;
    err = vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &view2);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkSampler sampler2;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler2);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = tmp_view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptorSet;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    // First error case is destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();

    // Transit image layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptorSet, 0, NULL);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // This first submit should be successful
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->m_queue);

    // Now destroy imageview and reset cmdBuffer
    vkDestroyImageView(m_device->device(), tmp_view, NULL);
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptorSet, 0, NULL);
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " that has been destroyed.");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Re-update descriptor with new view
    img_info.imageView = view;
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    // Now test destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptorSet, 0, NULL);
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Destroy sampler invalidates the cmd buffer, causing error on submit
    vkDestroySampler(m_device->device(), sampler, NULL);
    // Attempt to submit cmd buffer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkSampler");
    submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Now re-update descriptor with valid sampler and delete image
    img_info.sampler = sampler2;
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_commandBuffer->begin(&info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptorSet, 0, NULL);
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Destroy image invalidates the cmd buffer, causing error on submit
    vkDestroyImage(m_device->device(), image, NULL);
    // Attempt to submit cmd buffer
    submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    // Now update descriptor to be valid, but then free descriptor
    img_info.imageView = view2;
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_commandBuffer->begin(&info);

    // Transit image2 layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    barrier.image = image2;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptorSet, 0, NULL);
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Immediately try to destroy the descriptor set in the active command buffer - failure expected
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkFreeDescriptorSets-pDescriptorSets-00309");
    vkFreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);
    m_errorMonitor->VerifyFound();

    // Try again once the queue is idle - should succeed w/o error
    // TODO - though the particular error above doesn't re-occur, there are other 'unexpecteds' still to clean up
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError(
        "pDescriptorSets must be a valid pointer to an array of descriptorSetCount VkDescriptorSet handles, each element of which "
        "must either be a valid handle or VK_NULL_HANDLE");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorSet obj");
    vkFreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);

    // Attempt to submit cmd buffer containing the freed descriptor set
    submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDescriptorSet");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vkFreeMemory(m_device->device(), image_memory, NULL);
    vkDestroySampler(m_device->device(), sampler2, NULL);
    vkDestroyImage(m_device->device(), image2, NULL);
    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroyImageView(m_device->device(), view2, NULL);
    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, InvalidDescriptorSetSamplerDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a bound descriptor sets with a combined image sampler where sampler has been deleted.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    // Create images to update the descriptor with
    VkImageObj image(m_device);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView view;
    VkResult err = vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    VkSampler sampler1;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler1);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo img_info1 = img_info;
    img_info1.sampler = sampler1;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    std::array<VkWriteDescriptorSet, 2> descriptor_writes = {descriptor_write, descriptor_write};
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].pImageInfo = &img_info1;

    vkUpdateDescriptorSets(m_device->device(), 2, descriptor_writes.data(), 0, NULL);

    // Destroy the sampler before it's bound to the cmd buffer
    vkDestroySampler(m_device->device(), sampler1, NULL);

    // Create PSO to be used for draw-time errors below
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(set=0, binding=1) uniform sampler2D s1;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "   x = texture(s1, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    // First error case is destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, NULL);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " Descriptor in binding #1 index 0 is using sampler ");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vkDestroySampler(m_device->device(), sampler, NULL);
    vkDestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, ImageDescriptorLayoutMismatch) {
    TEST_DESCRIPTION("Create an image sampler layout->image layout mismatch within/without a command buffer");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool maint2_support = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    if (maint2_support) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    } else {
        printf("%s Relaxed layout matching subtest requires API >= 1.1 or KHR_MAINTENANCE2 extension, unavailable - skipped.\n",
               kSkipPrefix);
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    VkDescriptorSet descriptorSet = descriptor_set.set_;

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_TILING_OPTIMAL,
               0);
    ASSERT_TRUE(image.initialized());

    vk_testing::ImageView view;
    auto image_view_create_info = SafeSaneImageViewCreateInfo(image, format, VK_IMAGE_ASPECT_COLOR_BIT);
    view.init(*m_device, image_view_create_info);
    ASSERT_TRUE(view.initialized());

    // Create Sampler
    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    // Setup structure for descriptor update with sampler, for update in do_test below
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptorSet;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    VkCommandBufferObj cmd_buf(m_device, m_commandPool);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buf.handle();

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {kInternal, kExternal};
    const std::vector<std::string> internal_errors = {"VUID-VkDescriptorImageInfo-imageLayout-00344",
                                                      "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotUpdated"};
    const std::vector<std::string> external_errors = {"UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout"};

    // Common steps to create the two classes of errors (or two classes of positives)
    auto do_test = [&](VkImageObj *image, vk_testing::ImageView *view, VkImageAspectFlags aspect_mask, VkImageLayout image_layout,
                       VkImageLayout descriptor_layout, const bool positive_test) {
        // Set up the descriptor
        img_info.imageView = view->handle();
        img_info.imageLayout = descriptor_layout;
        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        for (TestType test_type : test_list) {
            cmd_buf.begin();
            // record layout different than actual descriptor layout.
            const VkFlags read_write = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            auto image_barrier = image->image_memory_barrier(read_write, read_write, VK_IMAGE_LAYOUT_UNDEFINED, image_layout,
                                                             image->subresource_range(aspect_mask));
            cmd_buf.PipelineBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 0,
                                    nullptr, 1, &image_barrier);

            if (test_type == kExternal) {
                // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
                cmd_buf.end();
                vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
                vkQueueWaitIdle(m_device->m_queue);
                cmd_buf.begin();
            }

            cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
            vkCmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
            vkCmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                    &descriptorSet, 0, NULL);
            vkCmdSetViewport(cmd_buf.handle(), 0, 1, &viewport);
            vkCmdSetScissor(cmd_buf.handle(), 0, 1, &scissor);

            // At draw time the update layout will mis-match the actual layout
            if (positive_test || (test_type == kExternal)) {
                m_errorMonitor->ExpectSuccess();
            } else {
                for (const auto &err : internal_errors) {
                    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, err.c_str());
                }
            }
            cmd_buf.Draw(1, 0, 0, 0);
            if (positive_test || (test_type == kExternal)) {
                m_errorMonitor->VerifyNotFound();
            } else {
                m_errorMonitor->VerifyFound();
            }

            m_errorMonitor->ExpectSuccess();
            cmd_buf.EndRenderPass();
            cmd_buf.end();
            m_errorMonitor->VerifyNotFound();

            // Submit cmd buffer
            if (positive_test || (test_type == kInternal)) {
                m_errorMonitor->ExpectSuccess();
            } else {
                for (const auto &err : external_errors) {
                    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, err.c_str());
                }
            }
            vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_device->m_queue);
            if (positive_test || (test_type == kInternal)) {
                m_errorMonitor->VerifyNotFound();
            } else {
                m_errorMonitor->VerifyFound();
            }
        }
    };
    do_test(&image, &view, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, /* positive */ false);

    // Create depth stencil image and views
    const VkFormat format_ds = m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    bool ds_test_support = maint2_support && (format_ds != VK_FORMAT_UNDEFINED);
    VkImageObj image_ds(m_device);
    vk_testing::ImageView stencil_view;
    vk_testing::ImageView depth_view;
    const VkImageLayout ds_image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageLayout depth_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    const VkImageLayout stencil_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageAspectFlags depth_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    if (ds_test_support) {
        image_ds.Init(32, 32, 1, format_ds, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image_ds.initialized());
        auto ds_view_ci = SafeSaneImageViewCreateInfo(image_ds, format_ds, VK_IMAGE_ASPECT_DEPTH_BIT);
        depth_view.init(*m_device, ds_view_ci);
        ds_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencil_view.init(*m_device, ds_view_ci);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, depth_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, stencil_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
    }
}

TEST_F(VkLayerTest, DescriptorPoolInUseResetSignaled) {
    TEST_DESCRIPTION("Reset a DescriptorPool with a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create image to update the descriptor with
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vkCreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put pool in-flight
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Reset pool while in-flight, causing error
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetDescriptorPool-descriptorPool-00313");
    vkResetDescriptorPool(m_device->device(), descriptor_set.pool_, 0);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
    // Cleanup
    vkDestroySampler(m_device->device(), sampler, nullptr);
    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
}

TEST_F(VkLayerTest, DescriptorImageUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt an image descriptor set update where image's bound memory has been freed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create images to update the descriptor with
    VkImage image;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;
    VkResult err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);
    // Initially bind memory to avoid error at bind view time. We'll break binding before update.
    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = NULL;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Allocate enough memory for image
    memory_info.allocationSize = memory_reqs.size;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(err);
    err = vkBindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView view;
    err = vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    // Break memory binding and attempt update
    vkFreeMemory(m_device->device(), image_memory, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " previously bound memory was freed. Memory must not be freed prior to this operation.");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "vkUpdateDescriptorSets() failed write update validation for ");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    // Cleanup
    vkDestroyImage(m_device->device(), image, NULL);
    vkDestroySampler(m_device->device(), sampler, NULL);
    vkDestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, InvalidDynamicOffsetCases) {
    // Create a descriptorSet w/ dynamic descriptor and then hit 3 offset error
    // cases:
    // 1. No dynamicOffset supplied
    // 2. Too many dynamicOffsets supplied
    // 3. Dynamic offset oversteps buffer being updated
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " requires 1 dynamicOffsets, but only 0 dynamicOffsets are left in pDynamicOffsets ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer;
    dynamic_uniform_buffer.init(*m_device, buffCI);

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 1024, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 0, NULL);
    m_errorMonitor->VerifyFound();
    uint32_t pDynOff[2] = {512, 756};
    // Now cause error b/c too many dynOffsets in array for # of dyn descriptors
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "Attempting to bind 1 descriptorSets with 1 dynamic descriptors, but ");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 2, pDynOff);
    m_errorMonitor->VerifyFound();
    // Finally cause error due to dynamicOffset being too big
    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT,
        " dynamic offset 512 combined with offset 0 and range 1024 that oversteps the buffer size of 1024");
    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // This update should succeed, but offset size of 512 will overstep buffer
    // /w range 1024 & size 1024
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set.set_, 1, pDynOff);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DescriptorBufferUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt to update a descriptor with a non-sparse buffer that doesn't have memory bound");
    VkResult err;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " used with no memory bound. Memory should be bound by calling vkBindBufferMemory().");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "vkUpdateDescriptorSets() failed write update validation for ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBuffer dynamic_uniform_buffer;
    err = vkCreateBuffer(m_device->device(), &buffCI, NULL, &dynamic_uniform_buffer);
    ASSERT_VK_SUCCESS(err);

    // Attempt to update descriptor without binding memory to it
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer, 1024, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    vkDestroyBuffer(m_device->device(), dynamic_uniform_buffer, NULL);
}

TEST_F(VkLayerTest, DescriptorSetCompatibility) {
    // Test various desriptorSet errors with bad binding combinations
    using std::vector;
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const uint32_t NUM_DESCRIPTOR_TYPES = 5;
    VkDescriptorPoolSize ds_type_count[NUM_DESCRIPTOR_TYPES] = {};
    ds_type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count[0].descriptorCount = 10;
    ds_type_count[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    ds_type_count[1].descriptorCount = 2;
    ds_type_count[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    ds_type_count[2].descriptorCount = 2;
    ds_type_count[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count[3].descriptorCount = 5;
    // TODO : LunarG ILO driver currently asserts in desc.c w/ INPUT_ATTACHMENT
    // type
    // ds_type_count[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    ds_type_count[4].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    ds_type_count[4].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 5;
    ds_pool_ci.poolSizeCount = NUM_DESCRIPTOR_TYPES;
    ds_pool_ci.pPoolSizes = ds_type_count;

    VkDescriptorPool ds_pool;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    static const uint32_t MAX_DS_TYPES_IN_LAYOUT = 2;
    VkDescriptorSetLayoutBinding dsl_binding[MAX_DS_TYPES_IN_LAYOUT] = {};
    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 5;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[0].pImmutableSamplers = NULL;

    // Create layout identical to set0 layout but w/ different stageFlags
    VkDescriptorSetLayoutBinding dsl_fs_stage_only = {};
    dsl_fs_stage_only.binding = 0;
    dsl_fs_stage_only.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_fs_stage_only.descriptorCount = 5;
    dsl_fs_stage_only.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // Different stageFlags to cause error at
                                                                  // bind time
    dsl_fs_stage_only.pImmutableSamplers = NULL;

    vector<VkDescriptorSetLayoutObj> ds_layouts;
    // Create 4 unique layouts for full pipelineLayout, and 1 special fs-only
    // layout for error case
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const VkDescriptorSetLayoutObj ds_layout_fs_only(m_device, {dsl_fs_stage_only});

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dsl_binding[0].descriptorCount = 2;
    dsl_binding[1].binding = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dsl_binding[1].descriptorCount = 2;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[1].pImmutableSamplers = NULL;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>({dsl_binding[0], dsl_binding[1]}));

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding[0].descriptorCount = 5;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dsl_binding[0].descriptorCount = 2;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const auto &ds_vk_layouts = MakeVkHandles<VkDescriptorSetLayout>(ds_layouts);

    static const uint32_t NUM_SETS = 4;
    VkDescriptorSet descriptorSet[NUM_SETS] = {};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.descriptorSetCount = ds_vk_layouts.size();
    alloc_info.pSetLayouts = ds_vk_layouts.data();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, descriptorSet);
    ASSERT_VK_SUCCESS(err);
    VkDescriptorSet ds0_fs_only = {};
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_fs_only.handle();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &ds0_fs_only);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layouts[0], &ds_layouts[1]});
    // Create pipelineLayout with only one setLayout
    const VkPipelineLayoutObj single_pipe_layout(m_device, {&ds_layouts[0]});
    // Create pipelineLayout with 2 descriptor setLayout at index 0
    const VkPipelineLayoutObj pipe_layout_one_desc(m_device, {&ds_layouts[3]});
    // Create pipelineLayout with 5 SAMPLER descriptor setLayout at index 0
    const VkPipelineLayoutObj pipe_layout_five_samp(m_device, {&ds_layouts[2]});
    // Create pipelineLayout with UB type, but stageFlags for FS only
    VkPipelineLayoutObj pipe_layout_fs_only(m_device, {&ds_layout_fs_only});
    // Create pipelineLayout w/ incompatible set0 layout, but set1 is fine
    const VkPipelineLayoutObj pipe_layout_bad_set0(m_device, {&ds_layout_fs_only, &ds_layouts[1]});

    // Add buffer binding for UBO
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, bci);
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptorSet[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipe_layout_fs_only.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // TODO : Want to cause various binding incompatibility issues here to test
    // DrawState
    //  First cause various verify_layout_compatibility() fails
    //  Second disturb early and late sets and verify INFO msgs
    // VerifySetLayoutCompatibility fail cases:
    // 1. invalid VkPipelineLayout (layout) passed into vkCmdBindDescriptorSets
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindDescriptorSets-layout-parameter");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            CastToHandle<VkPipelineLayout, uintptr_t>(0xbaadb1be), 0, 1, &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 2. layoutIndex exceeds # of layouts in layout
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " attempting to bind set to index 1");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, single_pipe_layout.handle(), 0, 2,
                            &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 3. Pipeline setLayout[0] has 2 descriptors, but set being bound has 5
    // descriptors
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " has 2 descriptors, but DescriptorSetLayout ");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_one_desc.handle(), 0, 1,
                            &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 4. same # of descriptors but mismatch in type
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " is type 'VK_DESCRIPTOR_TYPE_SAMPLER' but binding ");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_five_samp.handle(), 0, 1,
                            &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 5. same # of descriptors but mismatch in stageFlags
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " has stageFlags 16 but binding 0 for DescriptorSetLayout ");
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_fs_only.handle(), 0, 1,
                            &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // Now that we're done actively using the pipelineLayout that gfx pipeline
    //  was created with, we should be able to delete it. Do that now to verify
    //  that validation obeys pipelineLayout lifetime
    pipe_layout_fs_only.Reset();

    // Cause draw-time errors due to PSO incompatibilities
    // 1. Error due to not binding required set (we actually use same code as
    // above to disturb set0)
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                            &descriptorSet[0], 0, NULL);
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_bad_set0.handle(), 1, 1,
                            &descriptorSet[1], 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " uses set #0 but that set is not bound.");

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // 2. Error due to bound set not being compatible with PSO's
    // VkPipelineLayout (diff stageFlags in this case)
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                            &descriptorSet[0], 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " bound as set #0 is not compatible with ");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Remaining clean-up
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, NullRenderPass) {
    // Bind a NULL RenderPass
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "vkCmdBeginRenderPass: required parameter pRenderPassBegin specified as NULL");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    // Don't care about RenderPass handle b/c error should be flagged before
    // that
    vkCmdBeginRenderPass(m_commandBuffer->handle(), NULL, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, EndCommandBufferWithinRenderPass) {
    TEST_DESCRIPTION("End a command buffer with an active render pass");

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkEndCommandBuffer-commandBuffer-00060");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkEndCommandBuffer(m_commandBuffer->handle());

    m_errorMonitor->VerifyFound();

    // End command buffer properly to avoid driver issues. This is safe -- the
    // previous vkEndCommandBuffer should not have reached the driver.
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // TODO: Add test for VK_COMMAND_BUFFER_LEVEL_SECONDARY
    // TODO: Add test for VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
}

TEST_F(VkLayerTest, DSUsageBitsErrors) {
    TEST_DESCRIPTION("Attempt to update descriptor sets for images and buffers that do not have correct usage bits sets.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat buffer_format = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), buffer_format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        printf("%s Device does not support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT for this format; skipped.\n", kSkipPrefix);
        return;
    }

    std::array<VkDescriptorPoolSize, VK_DESCRIPTOR_TYPE_RANGE_SIZE> ds_type_count;
    for (uint32_t i = 0; i < ds_type_count.size(); ++i) {
        ds_type_count[i].type = VkDescriptorType(i);
        ds_type_count[i].descriptorCount = 1;
    }

    vk_testing::DescriptorPool ds_pool;
    ds_pool.init(*m_device, vk_testing::DescriptorPool::create_info(0, VK_DESCRIPTOR_TYPE_RANGE_SIZE, ds_type_count));
    ASSERT_TRUE(ds_pool.initialized());

    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings(1);
    dsl_bindings[0].binding = 0;
    dsl_bindings[0].descriptorType = VkDescriptorType(0);
    dsl_bindings[0].descriptorCount = 1;
    dsl_bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_bindings[0].pImmutableSamplers = NULL;

    // Create arrays of layout and descriptor objects
    using UpDescriptorSet = std::unique_ptr<vk_testing::DescriptorSet>;
    std::vector<UpDescriptorSet> descriptor_sets;
    using UpDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayoutObj>;
    std::vector<UpDescriptorSetLayout> ds_layouts;
    descriptor_sets.reserve(VK_DESCRIPTOR_TYPE_RANGE_SIZE);
    ds_layouts.reserve(VK_DESCRIPTOR_TYPE_RANGE_SIZE);
    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i) {
        dsl_bindings[0].descriptorType = VkDescriptorType(i);
        ds_layouts.push_back(UpDescriptorSetLayout(new VkDescriptorSetLayoutObj(m_device, dsl_bindings)));
        descriptor_sets.push_back(UpDescriptorSet(ds_pool.alloc_sets(*m_device, *ds_layouts.back())));
        ASSERT_TRUE(descriptor_sets.back()->initialized());
    }

    // Create a buffer & bufferView to be used for invalid updates
    const VkDeviceSize buffer_size = 256;
    uint8_t data[buffer_size];
    VkConstantBufferObj buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkConstantBufferObj storage_texel_buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized() && storage_texel_buffer.initialized());

    auto buff_view_ci = vk_testing::BufferView::createInfo(buffer.handle(), VK_FORMAT_R8_UNORM);
    vk_testing::BufferView buffer_view_obj, storage_texel_buffer_view_obj;
    buffer_view_obj.init(*m_device, buff_view_ci);
    buff_view_ci.buffer = storage_texel_buffer.handle();
    storage_texel_buffer_view_obj.init(*m_device, buff_view_ci);
    ASSERT_TRUE(buffer_view_obj.initialized() && storage_texel_buffer_view_obj.initialized());
    VkBufferView buffer_view = buffer_view_obj.handle();
    VkBufferView storage_texel_buffer_view = storage_texel_buffer_view_obj.handle();

    // Create an image to be used for invalid updates
    VkImageObj image_obj(m_device);
    image_obj.InitNoLayout(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image_obj.initialized());
    VkImageView image_view = image_obj.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    VkDescriptorImageInfo img_info = {};
    img_info.imageView = image_view;
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = &buffer_view;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = &img_info;

    // These error messages align with VkDescriptorType struct
    std::string error_codes[] = {
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView",  // placeholder, no error for SAMPLER descriptor
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView",  // COMBINED_IMAGE_SAMPLER
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView",  // SAMPLED_IMAGE
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView",  // STORAGE_IMAGE
        "VUID-VkWriteDescriptorSet-descriptorType-00334",        // UNIFORM_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00335",        // STORAGE_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",        // UNIFORM_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00331",        // STORAGE_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",        // UNIFORM_BUFFER_DYNAMIC
        "VUID-VkWriteDescriptorSet-descriptorType-00331",        // STORAGE_BUFFER_DYNAMIC
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView"   // INPUT_ATTACHMENT
    };
    // Start loop at 1 as SAMPLER desc type has no usage bit error
    for (uint32_t i = 1; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i) {
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            // Now check for UNIFORM_TEXEL_BUFFER using storage_texel_buffer_view
            descriptor_write.pTexelBufferView = &storage_texel_buffer_view;
        }
        descriptor_write.descriptorType = VkDescriptorType(i);
        descriptor_write.dstSet = descriptor_sets[i]->handle();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, error_codes[i]);

        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyFound();
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            descriptor_write.pTexelBufferView = &buffer_view;
        }
    }
}

TEST_F(VkLayerTest, DSBufferInfoErrors) {
    TEST_DESCRIPTION(
        "Attempt to update buffer descriptor set that has incorrect parameters in VkDescriptorBufferInfo struct. This includes:\n"
        "1. offset value greater than or equal to buffer size\n"
        "2. range value of 0\n"
        "3. range value greater than buffer (size - offset)");

    // GPDDP2 needed for push descriptors support below
    bool gpdp2_support = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (gpdp2_support) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    bool update_template_support = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    if (update_template_support) {
        m_device_extension_names.push_back(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    } else {
        printf("%s Descriptor Update Template Extensions not supported, template cases skipped.\n", kSkipPrefix);
    }

    // Note: Includes workaround for some implementations which incorrectly return 0 maxPushDescriptors
    bool push_descriptor_support = gpdp2_support &&
                                   DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) &&
                                   (GetPushDescriptorProperties(instance(), gpu()).maxPushDescriptors > 0);
    if (push_descriptor_support) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptor Extension not supported, push descriptor cases skipped.\n", kSkipPrefix);
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    // Create a buffer to be used for invalid updates
    VkBufferCreateInfo buff_ci = {};
    buff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = m_device->props.limits.minUniformBufferOffsetAlignment;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_ci);

    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;

    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = descriptor_set.set_;

    // Relying on the "return nullptr for non-enabled extensions
    auto vkCreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkCreateDescriptorUpdateTemplateKHR");
    auto vkDestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkDestroyDescriptorUpdateTemplateKHR");
    auto vkUpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkUpdateDescriptorSetWithTemplateKHR");

    if (update_template_support) {
        ASSERT_NE(vkCreateDescriptorUpdateTemplateKHR, nullptr);
        ASSERT_NE(vkDestroyDescriptorUpdateTemplateKHR, nullptr);
        ASSERT_NE(vkUpdateDescriptorSetWithTemplateKHR, nullptr);
    }

    // Setup for update w/ template tests
    // Create a template of descriptor set updates
    struct SimpleTemplateData {
        uint8_t padding[7];
        VkDescriptorBufferInfo buff_info;
        uint32_t other_padding[4];
    };
    SimpleTemplateData update_template_data = {};

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    auto update_template_ci = lvl_init_struct<VkDescriptorUpdateTemplateCreateInfoKHR>();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    if (update_template_support) {
        auto result = vkCreateDescriptorUpdateTemplateKHR(m_device->device(), &update_template_ci, nullptr, &update_template);
        ASSERT_VK_SUCCESS(result);
    }

    // VK_KHR_push_descriptor support
    auto vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    auto vkCmdPushDescriptorSetWithTemplateKHR =
        (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetWithTemplateKHR");

    std::unique_ptr<VkDescriptorSetLayoutObj> push_dsl = nullptr;
    std::unique_ptr<VkPipelineLayoutObj> pipeline_layout = nullptr;
    VkDescriptorUpdateTemplate push_template = VK_NULL_HANDLE;
    if (push_descriptor_support) {
        ASSERT_NE(vkCmdPushDescriptorSetKHR, nullptr);
        push_dsl.reset(
            new VkDescriptorSetLayoutObj(m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));
        pipeline_layout.reset(new VkPipelineLayoutObj(m_device, {push_dsl.get()}));
        ASSERT_TRUE(push_dsl->initialized());

        if (update_template_support) {
            ASSERT_NE(vkCmdPushDescriptorSetWithTemplateKHR, nullptr);
            auto push_template_ci = lvl_init_struct<VkDescriptorUpdateTemplateCreateInfoKHR>();
            push_template_ci.descriptorUpdateEntryCount = 1;
            push_template_ci.pDescriptorUpdateEntries = &update_template_entry;
            push_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
            push_template_ci.descriptorSetLayout = VK_NULL_HANDLE;
            push_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            push_template_ci.pipelineLayout = pipeline_layout->handle();
            push_template_ci.set = 0;
            auto result = vkCreateDescriptorUpdateTemplateKHR(m_device->device(), &push_template_ci, nullptr, &push_template);
            ASSERT_VK_SUCCESS(result);
        }
    }

    auto do_test = [&](const char *desired_failure) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, desired_failure);
        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
        m_errorMonitor->VerifyFound();

        if (push_descriptor_support) {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, desired_failure);
            m_commandBuffer->begin();
            vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->handle(), 0, 1,
                                      &descriptor_write);
            m_commandBuffer->end();
            m_errorMonitor->VerifyFound();
        }

        if (update_template_support) {
            update_template_data.buff_info = buff_info;  // copy the test case information into our "pData"
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, desired_failure);
            vkUpdateDescriptorSetWithTemplateKHR(m_device->device(), descriptor_set.set_, update_template, &update_template_data);
            m_errorMonitor->VerifyFound();
            if (push_descriptor_support) {
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, desired_failure);
                m_commandBuffer->begin();
                vkCmdPushDescriptorSetWithTemplateKHR(m_commandBuffer->handle(), push_template, pipeline_layout->handle(), 0,
                                                      &update_template_data);
                m_commandBuffer->end();
                m_errorMonitor->VerifyFound();
            }
        }
    };

    // Cause error due to offset out of range
    buff_info.offset = buff_ci.size;
    buff_info.range = VK_WHOLE_SIZE;
    do_test("VUID-VkDescriptorBufferInfo-offset-00340");

    // Now cause error due to range of 0
    buff_info.offset = 0;
    buff_info.range = 0;
    do_test("VUID-VkDescriptorBufferInfo-range-00341");

    // Now cause error due to range exceeding buffer size - offset
    buff_info.offset = 0;
    buff_info.range = buff_ci.size + 1;
    do_test("VUID-VkDescriptorBufferInfo-range-00342");

    if (update_template_support) {
        vkDestroyDescriptorUpdateTemplateKHR(m_device->device(), update_template, nullptr);
        if (push_descriptor_support) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->device(), push_template, nullptr);
        }
    }
}

TEST_F(VkLayerTest, DSBufferLimitErrors) {
    TEST_DESCRIPTION(
        "Attempt to update buffer descriptor set that has VkDescriptorBufferInfo values that violate device limits.\n"
        "Test cases include:\n"
        "1. range of uniform buffer update exceeds maxUniformBufferRange\n"
        "2. offset of uniform buffer update is not multiple of minUniformBufferOffsetAlignment\n"
        "3. using VK_WHOLE_SIZE with uniform buffer size exceeding maxUniformBufferRange\n"
        "4. range of storage buffer update exceeds maxStorageBufferRange\n"
        "5. offset of storage buffer update is not multiple of minStorageBufferOffsetAlignment\n"
        "6. using VK_WHOLE_SIZE with storage buffer size exceeding maxStorageBufferRange");

    ASSERT_NO_FATAL_FAILURE(Init());

    struct TestCase {
        VkDescriptorType descriptor_type;
        VkBufferUsageFlagBits buffer_usage;
        VkDeviceSize max_range;
        std::string max_range_vu;
        VkDeviceSize min_align;
        std::string min_align_vu;
    };

    for (const auto &test_case : {
             TestCase({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       m_device->props.limits.maxUniformBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00332",
                       m_device->props.limits.minUniformBufferOffsetAlignment, "VUID-VkWriteDescriptorSet-descriptorType-00327"}),
             TestCase({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       m_device->props.limits.maxStorageBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00333",
                       m_device->props.limits.minStorageBufferOffsetAlignment, "VUID-VkWriteDescriptorSet-descriptorType-00328"}),
         }) {
        // Create layout with single buffer
        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, test_case.descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        // Create a buffer to be used for invalid updates
        VkBufferCreateInfo bci = {};
        bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bci.usage = test_case.buffer_usage;
        bci.size = test_case.max_range + test_case.min_align;  // Make buffer bigger than range limit
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vkCreateBuffer(m_device->device(), &bci, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        // Have to bind memory to buffer before descriptor update
        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo mem_alloc = {};
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = mem_reqs.size;
        bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
        if (!pass) {
            printf("%s Failed to allocate memory in DSBufferLimitErrors; skipped.\n", kSkipPrefix);
            vkDestroyBuffer(m_device->device(), buffer, NULL);
            continue;
        }

        VkDeviceMemory mem;
        err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
        if (VK_SUCCESS != err) {
            printf("%s Failed to allocate memory in DSBufferLimitErrors; skipped.\n", kSkipPrefix);
            vkDestroyBuffer(m_device->device(), buffer, NULL);
            continue;
        }
        err = vkBindBufferMemory(m_device->device(), buffer, mem, 0);
        ASSERT_VK_SUCCESS(err);

        VkDescriptorBufferInfo buff_info = {};
        buff_info.buffer = buffer;
        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pTexelBufferView = nullptr;
        descriptor_write.pBufferInfo = &buff_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.descriptorType = test_case.descriptor_type;
        descriptor_write.dstSet = descriptor_set.set_;

        // Exceed range limit
        if (test_case.max_range != UINT32_MAX) {
            buff_info.range = test_case.max_range + 1;
            buff_info.offset = 0;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, test_case.max_range_vu);
            vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Reduce size of range to acceptable limit and cause offset error
        if (test_case.min_align > 1) {
            buff_info.range = test_case.max_range;
            buff_info.offset = test_case.min_align - 1;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, test_case.min_align_vu);
            vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Exceed effective range limit by using VK_WHOLE_SIZE
        buff_info.range = VK_WHOLE_SIZE;
        buff_info.offset = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, test_case.max_range_vu);
        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
        m_errorMonitor->VerifyFound();

        // Cleanup
        vkFreeMemory(m_device->device(), mem, NULL);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
    }
}

TEST_F(VkLayerTest, DSAspectBitsErrors) {
    // TODO : Initially only catching case where DEPTH & STENCIL aspect bits
    //  are set, but could expand this test to hit more cases.
    TEST_DESCRIPTION("Attempt to update descriptor sets for images that do not have correct aspect bits sets.");
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    // Create an image to be used for invalid updates
    VkImageObj image_obj(m_device);
    VkFormatProperties fmt_props;
    vkGetPhysicalDeviceFormatProperties(m_device->phy().handle(), depth_format, &fmt_props);
    if (!image_obj.IsCompatible(VK_IMAGE_USAGE_SAMPLED_BIT, fmt_props.linearTilingFeatures) &&
        !image_obj.IsCompatible(VK_IMAGE_USAGE_SAMPLED_BIT, fmt_props.optimalTilingFeatures)) {
        printf("%s Depth + Stencil format cannot be sampled. Skipped.\n", kSkipPrefix);
        return;
    }
    image_obj.Init(64, 64, 1, depth_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    ASSERT_TRUE(image_obj.initialized());
    VkImage image = image_obj.image();

    // Now create view for image
    VkImageViewCreateInfo image_view_ci = {};
    image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.image = image;
    image_view_ci.format = depth_format;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    // Setting both depth & stencil aspect bits is illegal for an imageView used
    // to populate a descriptor set.
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    VkImageView image_view;
    err = vkCreateImageView(m_device->device(), &image_view_ci, NULL, &image_view);
    ASSERT_VK_SUCCESS(err);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

    const char *error_msg = "VUID-VkDescriptorImageInfo-imageView-01976";
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, error_msg);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    vkDestroyImageView(m_device->device(), image_view, NULL);
}

TEST_F(VkLayerTest, DSTypeMismatch) {
    // Create DS w/ layout of one type and attempt Update w/ mis-matched type
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT,
        " binding #0 with type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER but update type is VK_DESCRIPTOR_TYPE_SAMPLER");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, DSUpdateOutOfBounds) {
    // For overlapping Update, have arrayIndex exceed that of layout
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    if (!buffer_test.GetBufferCurrent()) {
        // Something prevented creation of buffer so abort
        printf("%s Buffer creation failed, skipping test\n", kSkipPrefix);
        return;
    }

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer_test.GetBuffer();
    buff_info.offset = 0;
    buff_info.range = 1024;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstArrayElement = 1; /* This index out of bounds for the update */
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buff_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDSUpdateIndex) {
    // Create layout w/ count of 1 and attempt update to that layout w/ binding index 2
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-dstBinding-00315");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // This is the wrong type, but out of bounds will be flagged first
    descriptor_set.WriteDescriptorImageInfo(2, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, DSUpdateEmptyBinding) {
    // Create layout w/ empty binding and attempt to update it
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 0 /* !! */, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // descriptor_write.descriptorCount = 1, Lie here to avoid parameter_validation error
    // This is the wrong type, but empty binding error will be flagged first
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-dstBinding-00316");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, InvalidDSUpdateStruct) {
    // Call UpdateDS w/ struct type other than valid VK_STRUCTUR_TYPE_UPDATE_*
    // types
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, ".sType must be VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET");

    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorImageInfo info = {};
    info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; /* Intentionally broken struct type */
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.descriptorCount = 1;
    // This is the wrong type, but out of bounds will be flagged first
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_write.pImageInfo = &info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, SampleDescriptorUpdateError) {
    // Create a single Sampler descriptor and send it an invalid Sampler
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-00325");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSampler sampler = CastToHandle<VkSampler, uintptr_t>(0xbaadbeef);  // Sampler with invalid handle

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageViewDescriptorUpdateError) {
    // Create a single combined Image/Sampler descriptor and send it an invalid
    // imageView
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-00326");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageView view = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);  // invalid imageView object

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, CopyDescriptorUpdateErrors) {
    // Create DS w/ layout of 2 types, write update 1 and attempt to copy-update
    // into the other
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " binding #1 with type VK_DESCRIPTOR_TYPE_SAMPLER. Types do not match.");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // SAMPLER binding from layout above
    // This write update should succeed
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    // Now perform a copy update that fails due to type mismatch
    VkCopyDescriptorSet copy_ds_update;
    memset(&copy_ds_update, 0, sizeof(VkCopyDescriptorSet));
    copy_ds_update.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;  // Copy from SAMPLER binding
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;       // ERROR : copy to UNIFORM binding
    copy_ds_update.descriptorCount = 1;  // copy 1 descriptor
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();
    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " does not have copy update src binding of 3.");
    memset(&copy_ds_update, 0, sizeof(VkCopyDescriptorSet));
    copy_ds_update.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 3;  // ERROR : Invalid binding for matching layout
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;  // Copy 1 descriptor
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " binding#1 with offset index of 1 plus update array offset of 0 and update of 5 "
                                         "descriptors oversteps total number of descriptors in set: 2.");

    memset(&copy_ds_update, 0, sizeof(VkCopyDescriptorSet));
    copy_ds_update.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 5;  // ERROR copy 5 descriptors (out of bounds for layout)
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    vkDestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithRenderPass) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases. Initial case is drawing with an active renderpass that's not compatible with the bound "
        "pipeline state object's creation renderpass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);  // We shouldn't need a fragment shader
    // but add it to be able to run on more devices
    // Create a renderpass that will be incompatible with default renderpass
    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    // Format incompatible with PSO RP color attach format B8G8R8A8_UNORM
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), rp);

    VkCommandBufferInheritanceInfo cbii = {};
    cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cbii.renderPass = rp;
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = {};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.pInheritanceInfo = &cbii;
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (the error should trigger on the attempt to draw).
    m_commandBuffer->Draw(3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();

    vkDestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, Maint1BindingSliceOf3DImage) {
    TEST_DESCRIPTION(
        "Attempt to bind a slice of a 3D texture in a descriptor set. This is explicitly disallowed by KHR_maintenance1 to keep "
        "things simple for drivers.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    } else {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkResult err;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageCreateInfo ici = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                             nullptr,
                             VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
                             VK_IMAGE_TYPE_3D,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             {32, 32, 32},
                             1,
                             1,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_SHARING_MODE_EXCLUSIVE,
                             0,
                             nullptr,
                             VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&ici);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkImageView view;
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    // Meat of the test.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorImageInfo-imageView-00343");

    VkDescriptorImageInfo dii = {VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL};
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                  nullptr,
                                  descriptor_set.set_,
                                  0,
                                  0,
                                  1,
                                  VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                  &dii,
                                  nullptr,
                                  nullptr};
    vkUpdateDescriptorSets(m_device->device(), 1, &write, 0, nullptr);

    m_errorMonitor->VerifyFound();

    vkDestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkLayerTest, UpdateDestroyDescriptorSetLayout) {
    TEST_DESCRIPTION("Attempt updates to descriptor sets with destroyed descriptor set layouts");
    // TODO: Update to match the descriptor set layout specific VUIDs/VALIDATION_ERROR_* when present
    const auto kWriteDestroyedLayout = "VUID-VkWriteDescriptorSet-dstSet-00320";
    const auto kCopyDstDestroyedLayout = "VUID-VkCopyDescriptorSet-dstSet-parameter";
    const auto kCopySrcDestroyedLayout = "VUID-VkCopyDescriptorSet-srcSet-parameter";

    ASSERT_NO_FATAL_FAILURE(Init());

    // Set up the descriptor (resource) and write/copy operations to use.
    float data[16] = {};
    VkConstantBufferObj buffer(m_device, sizeof(data), data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized());

    VkDescriptorBufferInfo info = {};
    info.buffer = buffer.handle();
    info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write_descriptor = {};
    write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor.dstSet = VK_NULL_HANDLE;  // must update this
    write_descriptor.dstBinding = 0;
    write_descriptor.descriptorCount = 1;
    write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor.pBufferInfo = &info;

    VkCopyDescriptorSet copy_descriptor = {};
    copy_descriptor.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_descriptor.srcSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.srcBinding = 0;
    copy_descriptor.dstSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.dstBinding = 0;
    copy_descriptor.descriptorCount = 1;

    // Create valid and invalid source and destination descriptor sets
    std::vector<VkDescriptorSetLayoutBinding> one_uniform_buffer = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet good_dst(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_dst.Initialized());

    OneOffDescriptorSet bad_dst(m_device, one_uniform_buffer);
    // Must assert before invalidating it below
    ASSERT_TRUE(bad_dst.Initialized());
    bad_dst.layout_ = VkDescriptorSetLayoutObj();

    OneOffDescriptorSet good_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_src.Initialized());

    // Put valid data in the good and bad sources, simultaneously doing a positive test on write and copy operations
    m_errorMonitor->ExpectSuccess();
    write_descriptor.dstSet = good_src.set_;
    vkUpdateDescriptorSets(m_device->device(), 1, &write_descriptor, 0, NULL);
    m_errorMonitor->VerifyNotFound();

    OneOffDescriptorSet bad_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(bad_src.Initialized());

    // to complete our positive testing use copy, where above we used write.
    copy_descriptor.srcSet = good_src.set_;
    copy_descriptor.dstSet = bad_src.set_;
    vkUpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    bad_src.layout_ = VkDescriptorSetLayoutObj();
    m_errorMonitor->VerifyNotFound();

    // Trigger the three invalid use errors
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, kWriteDestroyedLayout);
    write_descriptor.dstSet = bad_dst.set_;
    vkUpdateDescriptorSets(m_device->device(), 1, &write_descriptor, 0, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, kCopyDstDestroyedLayout);
    copy_descriptor.dstSet = bad_dst.set_;
    vkUpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, kCopySrcDestroyedLayout);
    copy_descriptor.srcSet = bad_src.set_;
    copy_descriptor.dstSet = good_dst.set_;
    vkUpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, FramebufferIncompatible) {
    TEST_DESCRIPTION(
        "Bind a secondary command buffer with a framebuffer that does not match the framebuffer for the active renderpass.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          VK_FORMAT_B8G8R8A8_UNORM,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkImageView view;
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferAllocateInfo cbai = {};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = m_commandPool->handle();
    cbai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cbai.commandBufferCount = 1;

    VkCommandBuffer sec_cb;
    err = vkAllocateCommandBuffers(m_device->device(), &cbai, &sec_cb);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferBeginInfo cbbi = {};
    VkCommandBufferInheritanceInfo cbii = {};
    cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    cbii.renderPass = renderPass();
    cbii.framebuffer = fb;
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.pNext = NULL;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;
    vkBeginCommandBuffer(sec_cb, &cbbi);
    vkEndCommandBuffer(sec_cb);

    VkCommandBufferBeginInfo cbbi2 = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cbbi2);
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdExecuteCommands-pCommandBuffers-00099");
    vkCmdExecuteCommands(m_commandBuffer->handle(), 1, &sec_cb);
    m_errorMonitor->VerifyFound();
    // Cleanup

    vkCmdEndRenderPass(m_commandBuffer->handle());
    vkEndCommandBuffer(m_commandBuffer->handle());

    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroyRenderPass(m_device->device(), rp, NULL);
    vkDestroyFramebuffer(m_device->device(), fb, NULL);
}

TEST_F(VkLayerTest, RenderPassMissingAttachment) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create a renderPass with a single color attachment
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    auto createView = lvl_init_struct<VkImageViewCreateInfo>();
    createView.image = m_renderTargets[0]->handle();
    createView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createView.format = VK_FORMAT_B8G8R8A8_UNORM;
    createView.components.r = VK_COMPONENT_SWIZZLE_R;
    createView.components.g = VK_COMPONENT_SWIZZLE_G;
    createView.components.b = VK_COMPONENT_SWIZZLE_B;
    createView.components.a = VK_COMPONENT_SWIZZLE_A;
    createView.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    createView.flags = 0;

    VkImageView iv;
    vkCreateImageView(m_device->handle(), &createView, nullptr, &iv);

    auto fb_info = lvl_init_struct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &iv;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;

    // Create the framebuffer then destory the view it uses.
    VkFramebuffer fb;
    err = vkCreateFramebuffer(device(), &fb_info, NULL, &fb);
    vkDestroyImageView(device(), iv, NULL);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRenderPassBeginInfo-framebuffer-parameter");

    auto rpbi = lvl_init_struct<VkRenderPassBeginInfo>();
    rpbi.renderPass = rp;
    rpbi.framebuffer = fb;
    rpbi.renderArea = {{0, 0}, {32, 32}};

    m_commandBuffer->begin();
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // Don't call vkCmdEndRenderPass; as the begin has been "skipped" based on the error condition
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vkDestroyFramebuffer(m_device->device(), fb, NULL);
    vkDestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, AttachmentDescriptionUndefinedFormat) {
    TEST_DESCRIPTION("Create a render pass with an attachment description format set to VK_FORMAT_UNDEFINED");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT, "format is VK_FORMAT_UNDEFINED");

    VkAttachmentReference color_attach = {};
    color_attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attach.attachment = 0;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attach;

    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    VkResult result = vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    m_errorMonitor->VerifyFound();

    if (result == VK_SUCCESS) {
        vkDestroyRenderPass(m_device->device(), rp, NULL);
    }
}

TEST_F(VkLayerTest, InvalidCreateDescriptorPool) {
    TEST_DESCRIPTION("Attempt to create descriptor pool with invalid parameters");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t default_descriptor_count = 1;
    const VkDescriptorPoolSize dp_size_template{VK_DESCRIPTOR_TYPE_SAMPLER, default_descriptor_count};

    const VkDescriptorPoolCreateInfo dp_ci_template{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                    nullptr,  // pNext
                                                    0,        // flags
                                                    1,        // maxSets
                                                    1,        // poolSizeCount
                                                    &dp_size_template};

    // try maxSets = 0
    {
        VkDescriptorPoolCreateInfo invalid_dp_ci = dp_ci_template;
        invalid_dp_ci.maxSets = 0;  // invalid maxSets value

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301");
        {
            VkDescriptorPool pool;
            vkCreateDescriptorPool(m_device->device(), &invalid_dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }

    // try descriptorCount = 0
    {
        VkDescriptorPoolSize invalid_dp_size = dp_size_template;
        invalid_dp_size.descriptorCount = 0;  // invalid descriptorCount value

        VkDescriptorPoolCreateInfo dp_ci = dp_ci_template;
        dp_ci.pPoolSizes = &invalid_dp_size;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorPoolSize-descriptorCount-00302");
        {
            VkDescriptorPool pool;
            vkCreateDescriptorPool(m_device->device(), &dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DuplicateDescriptorBinding) {
    TEST_DESCRIPTION("Create a descriptor set layout with a duplicate binding number.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Create layout where two binding #s are "1"
    static const uint32_t NUM_BINDINGS = 3;
    VkDescriptorSetLayoutBinding dsl_binding[NUM_BINDINGS] = {};
    dsl_binding[0].binding = 1;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 1;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[0].pImmutableSamplers = NULL;
    dsl_binding[1].binding = 0;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[1].pImmutableSamplers = NULL;
    dsl_binding[2].binding = 1;  // Duplicate binding should cause error
    dsl_binding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[2].descriptorCount = 1;
    dsl_binding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[2].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_ci.pNext = NULL;
    ds_layout_ci.bindingCount = NUM_BINDINGS;
    ds_layout_ci.pBindings = dsl_binding;
    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetLayoutCreateInfo-binding-00279");
    vkCreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPushDescriptorSetLayout) {
    TEST_DESCRIPTION("Create a push descriptor set layout with invalid bindings.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Get the push descriptor limits
    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    // Note that as binding is referenced in ds_layout_ci, it is effectively in the closure by reference as well.
    auto test_create_ds_layout = [&ds_layout_ci, this](std::string error) {
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, error);
        vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
        m_errorMonitor->VerifyFound();
        vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    };

    // Starting with the initial VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC type set above..
    test_create_ds_layout("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    test_create_ds_layout(
        "VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");  // This is the same VUID as above, just a second error condition.

    if (!(push_descriptor_prop.maxPushDescriptors == std::numeric_limits<uint32_t>::max())) {
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.descriptorCount = push_descriptor_prop.maxPushDescriptors + 1;
        test_create_ds_layout("VUID-VkDescriptorSetLayoutCreateInfo-flags-00281");
    } else {
        printf("%s maxPushDescriptors is set to maximum unit32_t value, skipping 'out of range test'.\n", kSkipPrefix);
    }
}

TEST_F(VkLayerTest, PushDescriptorSetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create a push descriptor set layout without loading the needed extension.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    std::string error = "Attempted to use VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR in ";
    error = error + "VkDescriptorSetLayoutCreateInfo::flags but its required extension ";
    error = error + VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME;
    error = error + " has not been enabled.";

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, error.c_str());
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetLayoutCreateInfo-flags-00281");
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, DescriptorIndexingSetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create an update_after_bind set layout without loading the needed extension.");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    std::string error = "Attemped to use VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT in ";
    error = error + "VkDescriptorSetLayoutCreateInfo::flags but its required extension ";
    error = error + VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
    error = error + " has not been enabled.";

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, error.c_str());
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, DescriptorIndexingSetLayout) {
    TEST_DESCRIPTION("Exercise various create/allocate-time errors related to VK_EXT_descriptor_indexing.");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("%s Descriptor indexing or one of its dependencies not supported, skipping tests\n.", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                                                        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT};
    auto flags_create_info = lvl_init_struct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = (uint32_t)flags.size();
    flags_create_info.pBindingFlags = flags.data();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    // VU for VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::bindingCount
    flags_create_info.bindingCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-bindingCount-03002");
    VkResult err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);

    flags_create_info.bindingCount = 1;

    // set is missing UPDATE_AFTER_BIND_POOL flag.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000");
    // binding uses a feature we disabled
    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT,
        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-descriptorBindingUniformBufferUpdateAfterBind-03005");
    err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);

    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 0;
    flags_create_info.bindingCount = 0;
    err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
    auto dspci = lvl_init_struct<VkDescriptorPoolCreateInfo>();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vkCreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = lvl_init_struct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    // mismatch between descriptor set and pool
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044");
    vkAllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    m_errorMonitor->VerifyFound();

    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    vkDestroyDescriptorPool(m_device->handle(), pool, nullptr);

    if (indexing_features.descriptorBindingVariableDescriptorCount) {
        ds_layout_ci.flags = 0;
        ds_layout_ci.bindingCount = 1;
        flags_create_info.bindingCount = 1;
        flags[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
        ASSERT_VK_SUCCESS(err);

        pool_size = {binding.descriptorType, binding.descriptorCount};
        dspci = lvl_init_struct<VkDescriptorPoolCreateInfo>();
        dspci.poolSizeCount = 1;
        dspci.pPoolSizes = &pool_size;
        dspci.maxSets = 1;
        err = vkCreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
        ASSERT_VK_SUCCESS(err);

        auto count_alloc_info = lvl_init_struct<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>();
        count_alloc_info.descriptorSetCount = 1;
        // Set variable count larger than what was in the descriptor binding
        uint32_t variable_count = 2;
        count_alloc_info.pDescriptorCounts = &variable_count;

        ds_alloc_info = lvl_init_struct<VkDescriptorSetAllocateInfo>(&count_alloc_info);
        ds_alloc_info.descriptorPool = pool;
        ds_alloc_info.descriptorSetCount = 1;
        ds_alloc_info.pSetLayouts = &ds_layout;

        ds = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfoEXT-pSetLayouts-03046");
        vkAllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
        m_errorMonitor->VerifyFound();

        vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
        vkDestroyDescriptorPool(m_device->handle(), pool, nullptr);
    }
}

TEST_F(VkLayerTest, DescriptorIndexingUpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE3_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    } else {
        printf("%s Descriptor Indexing or Maintenance3 Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
    }
    if (VK_FALSE == features2.features.fragmentStoresAndAtomics) {
        printf("%s Test requires (unsupported) fragmentStoresAndAtomics, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorBindingFlagsEXT flags[2] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT};
    auto flags_create_info = lvl_init_struct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = &flags[0];

    // Descriptor set has two bindings - only the second is update_after_bind
    VkDescriptorSetLayoutBinding binding[2] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 2;
    ds_layout_ci.pBindings = &binding[0];
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    VkResult err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);

    VkDescriptorPoolSize pool_sizes[2] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
    };
    auto dspci = lvl_init_struct<VkDescriptorPoolCreateInfo>();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    dspci.poolSizeCount = 2;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vkCreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = lvl_init_struct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    vkAllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    ASSERT_VK_SUCCESS(err);

    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer dynamic_uniform_buffer;
    err = vkCreateBuffer(m_device->device(), &buffCI, NULL, &dynamic_uniform_buffer);
    ASSERT_VK_SUCCESS(err);

    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device->device(), dynamic_uniform_buffer, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    err = vkAllocateMemory(m_device->device(), &mem_alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vkBindBufferMemory(m_device->device(), dynamic_uniform_buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer;
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;

    VkWriteDescriptorSet descriptor_write[2] = {};
    descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = buffInfo;
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 1;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;

    vkCreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);

    // Create a dummy pipeline, since VL inspects which bindings are actually used at draw time
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "layout(set=0, binding=0) uniform foo0 { float x0; } bar0;\n"
        "layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;\n"
        "void main(){\n"
        "   color = vec4(bar0.x0 + bar1.x1);\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipe(m_device);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.CreateVKPipeline(pipeline_layout, m_renderPass);

    // Make both bindings valid before binding to the command buffer
    vkUpdateDescriptorSets(m_device->device(), 2, &descriptor_write[0], 0, NULL);
    m_errorMonitor->VerifyNotFound();

    // Two subtests. First only updates the update_after_bind binding and expects
    // no error. Second updates the other binding and expects an error when the
    // command buffer is ended.
    for (uint32_t i = 0; i < 2; ++i) {
        m_commandBuffer->begin();

        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &ds, 0, NULL);

        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vkCmdDraw(m_commandBuffer->handle(), 0, 0, 0, 0);
        vkCmdEndRenderPass(m_commandBuffer->handle());

        m_errorMonitor->VerifyNotFound();
        // Valid to update binding 1 after being bound
        vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write[1], 0, NULL);
        m_errorMonitor->VerifyNotFound();

        if (i == 0) {
            // expect no errors
            m_commandBuffer->end();
            m_errorMonitor->VerifyNotFound();
        } else {
            // Invalid to update binding 0 after being bound. But the error is actually
            // generated during vkEndCommandBuffer
            vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write[0], 0, NULL);
            m_errorMonitor->VerifyNotFound();

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                 "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDescriptorSet");

            vkEndCommandBuffer(m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }
    }

    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    vkDestroyDescriptorPool(m_device->handle(), pool, nullptr);
    vkDestroyBuffer(m_device->handle(), dynamic_uniform_buffer, NULL);
    vkFreeMemory(m_device->handle(), mem, NULL);
    vkDestroyPipelineLayout(m_device->handle(), pipeline_layout, NULL);
}

TEST_F(VkLayerTest, AllocatePushDescriptorSet) {
    TEST_DESCRIPTION("Attempt to allocate a push descriptor set.");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    auto ds_layout_ci = lvl_init_struct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkResult err = vkCreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
    auto dspci = lvl_init_struct<VkDescriptorPoolCreateInfo>();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vkCreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = lvl_init_struct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308");
    vkAllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    m_errorMonitor->VerifyFound();

    vkDestroyDescriptorPool(m_device->handle(), pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, CreateDescriptorUpdateTemplate) {
    TEST_DESCRIPTION("Verify error messages for invalid vkCreateDescriptorUpdateTemplate calls.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    // Note: Includes workaround for some implementations which incorrectly return 0 maxPushDescriptors
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) &&
        (GetPushDescriptorProperties(instance(), gpu()).maxPushDescriptors > 0)) {
        m_device_extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    } else {
        printf("%s Push Descriptors and Descriptor Update Template Extensions not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_ub(m_device, {dsl_binding});
    const VkDescriptorSetLayoutObj ds_layout_ub1(m_device, {dsl_binding});
    const VkDescriptorSetLayoutObj ds_layout_ub_push(m_device, {dsl_binding},
                                                     VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    const VkPipelineLayoutObj pipeline_layout(m_device, {{&ds_layout_ub, &ds_layout_ub1, &ds_layout_ub_push}});
    PFN_vkCreateDescriptorUpdateTemplateKHR vkCreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkCreateDescriptorUpdateTemplateKHR");
    ASSERT_NE(vkCreateDescriptorUpdateTemplateKHR, nullptr);
    PFN_vkDestroyDescriptorUpdateTemplateKHR vkDestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(m_device->device(), "vkDestroyDescriptorUpdateTemplateKHR");
    ASSERT_NE(vkDestroyDescriptorUpdateTemplateKHR, nullptr);

    VkDescriptorUpdateTemplateEntry entries = {0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, sizeof(VkBuffer)};
    VkDescriptorUpdateTemplateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.descriptorUpdateEntryCount = 1;
    create_info.pDescriptorUpdateEntries = &entries;

    auto do_test = [&](std::string err) {
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, err);
        if (VK_SUCCESS == vkCreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
        m_errorMonitor->VerifyFound();
    };

    // Descriptor set type template
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    // descriptorSetLayout is NULL
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");

    // Push descriptor type template
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = pipeline_layout.handle();
    create_info.set = 2;

    // Bad bindpoint -- force fuzz the bind point
    memset(&create_info.pipelineBindPoint, 0xFE, sizeof(create_info.pipelineBindPoint));
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351");
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    // Bad pipeline layout
    create_info.pipelineLayout = VK_NULL_HANDLE;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");
    create_info.pipelineLayout = pipeline_layout.handle();

    // Wrong set #
    create_info.set = 0;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Invalid set #
    create_info.set = 42;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");
}

TEST_F(VkLayerTest, InlineUniformBlockEXT) {
    TEST_DESCRIPTION("Test VK_EXT_inline_uniform_block.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 2> required_device_extensions = {VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                                                              VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    // Enable descriptor indexing if supported, but don't require it.
    bool supportsDescriptorIndexing = true;
    required_device_extensions = {VK_KHR_MAINTENANCE3_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            supportsDescriptorIndexing = false;
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto descriptor_indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    void *pNext = supportsDescriptorIndexing ? &descriptor_indexing_features : nullptr;
    // Create a device that enables inline_uniform_block
    auto inline_uniform_block_features = lvl_init_struct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(pNext);
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&inline_uniform_block_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Get the inline uniform block limits
    auto inline_uniform_props = lvl_init_struct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&inline_uniform_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {};
    ds_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    VkDescriptorSetLayout ds_layout = {};

    // Test too many bindings
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dslb.descriptorCount = 4;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    if (inline_uniform_props.maxInlineUniformBlockSize < dslb.descriptorCount) {
        printf("%sDescriptorCount exceeds InlineUniformBlockSize limit, skipping tests\n", kSkipPrefix);
        return;
    }

    uint32_t maxBlocks = std::max(inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks,
                                  inline_uniform_props.maxDescriptorSetInlineUniformBlocks);
    for (uint32_t i = 0; i < 1 + maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    VkResult err = vkCreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02214");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02216");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02215");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02217");

    VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = NULL;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    err = vkCreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);
    pipeline_layout = VK_NULL_HANDLE;
    vkDestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
    ds_layout = VK_NULL_HANDLE;

    // Single binding that's too large and is not a multiple of 4
    dslb.binding = 0;
    dslb.descriptorCount = inline_uniform_props.maxInlineUniformBlockSize + 1;

    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dslb;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02209");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02210");
    err = vkCreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
    vkDestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
    ds_layout = VK_NULL_HANDLE;

    // Pool size must be a multiple of 4
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    ds_type_count.descriptorCount = 33;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDescriptorPoolSize-type-02218");
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    m_errorMonitor->VerifyFound();
    if (ds_pool) {
        vkDestroyDescriptorPool(m_device->handle(), ds_pool, nullptr);
        ds_pool = VK_NULL_HANDLE;
    }

    // Create a valid pool
    ds_type_count.descriptorCount = 32;
    err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    m_errorMonitor->VerifyNotFound();

    // Create two valid sets with 8 bytes each
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dslb.descriptorCount = 8;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = &dslb_vec[0];

    err = vkCreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyNotFound();

    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout, ds_layout};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = set_layouts;
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyNotFound();

    // Test invalid VkWriteDescriptorSet parameters (array element and size must be multiple of 4)
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 3;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;

    uint32_t dummyData[8] = {};
    VkWriteDescriptorSetInlineUniformBlockEXT write_inline_uniform = {};
    write_inline_uniform.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
    write_inline_uniform.dataSize = 3;
    write_inline_uniform.pData = &dummyData[0];
    descriptor_write.pNext = &write_inline_uniform;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-02220");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 4;
    write_inline_uniform.dataSize = 4;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-02219");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = nullptr;
    descriptor_write.dstArrayElement = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-02221");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = &write_inline_uniform;
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyNotFound();

    // Test invalid VkCopyDescriptorSet parameters (array element and size must be multiple of 4)
    VkCopyDescriptorSet copy_ds_update = {};
    copy_ds_update.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_ds_update.srcSet = descriptor_sets[0];
    copy_ds_update.srcBinding = 0;
    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstSet = descriptor_sets[1];
    copy_ds_update.dstBinding = 0;
    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 4;

    copy_ds_update.srcArrayElement = 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkCopyDescriptorSet-srcBinding-02223");
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstArrayElement = 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkCopyDescriptorSet-dstBinding-02224");
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 5;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkCopyDescriptorSet-srcBinding-02225");
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.descriptorCount = 4;
    vkUpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyNotFound();

    vkDestroyDescriptorPool(m_device->handle(), ds_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
}