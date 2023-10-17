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

static std::array gpu_av_enables = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
static std::array gpu_av_disables = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                     VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

// All VkGpuAssistedLayerTest should use this for setup as a single access point to more easily toggle which validation features are
// enabled/disabled
VkValidationFeaturesEXT VkGpuAssistedLayerTest::GetValidationFeatures() {
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = size32(gpu_av_enables);
    // TODO - Add command line flag or env var or another system for setting this to 'zero' to allow for someone writting a new
    // GPU-AV test to easily check the test is valid
    features.disabledValidationFeatureCount = size32(gpu_av_disables);
    features.pEnabledValidationFeatures = gpu_av_enables.data();
    features.pDisabledValidationFeatures = gpu_av_disables.data();
    return features;
}

// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
void VkGpuAssistedLayerTest::InitGpuAvFramework() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics || !features2.features.vertexPipelineStoresAndAtomics) {
        GTEST_SKIP() << "fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics are required for GPU-AV";
    } else if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }
}

// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
bool VkGpuAssistedLayerTest::CanEnableGpuAV() {
    // Check version first before trying to call GetPhysicalDeviceFeatures2
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("At least Vulkan version 1.1 is required for GPU-AV\n");
        return false;
    }
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics || !features2.features.vertexPipelineStoresAndAtomics) {
        printf("fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics are required for GPU-AV\n");
        return false;
    } else if (IsPlatformMockICD()) {
        printf("Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw\n");
        return false;
    }
    return true;
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationArrayOOBGraphicsShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())
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
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1;
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
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
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
                         &descriptor_set_variable, 6, "(set = 0, binding = 1) Index of 6 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_frag_runtime, nullptr, nullptr, nullptr, false, &pipeline_layout_variable,
                         &descriptor_set_variable, 5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 25, "(set = 0, binding = 1) Index of 25 used to index descriptor array of length 6."});
        tests.push_back({vsSource_frag, fsSource_buffer, nullptr, nullptr, nullptr, false, &pipeline_layout_buffer,
                         &descriptor_set_buffer, 5, "(set = 0, binding = 1) Descriptor index 5 is uninitialized"});
        if (m_device->phy().features().geometryShader) {
            // OOB Geometry
            tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, false, &pipeline_layout_buffer,
                             &descriptor_set_buffer, 25, "Stage = Geometry"});
            // Uninitialized Geometry
            tests.push_back({vsSourceForGS, kFragmentMinimalGlsl, gsSource, nullptr, nullptr, false, &pipeline_layout_buffer,
                             &descriptor_set_buffer, 5, "Stage = Geometry"});
        }
        if (m_device->phy().features().tessellationShader) {
            tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 25, "Stage = Tessellation Eval"});
            tests.push_back({kVertexMinimalGlsl, kFragmentMinimalGlsl, nullptr, kTessellationControlMinimalGlsl, tesSource, false,
                             &pipeline_layout_buffer, &descriptor_set_buffer, 5, "Stage = Tessellation Eval"});
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Stage = Compute");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
        // Out of Bounds
        data = (uint32_t *)buffer0.memory().map();
        data[0] = 25;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Stage = Compute");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
        vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);
    }
    return;
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationArrayEarlyDelete) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection descriptors where resources have been deleted while in use.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

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
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1;
    buffer1.init(*m_device, bci, mem_props);

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
    ds_binding_flags[1] =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
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
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
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

    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable,
                    &descriptor_set_variable, 1, "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed."});

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

TEST_F(VkGpuAssistedLayerTest, GpuValidationArrayEarlySamplerDelete) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection descriptors where resources have been deleted while in use.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

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
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 1024;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1;
    buffer1.init(*m_device, bci, mem_props);

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
    ds_binding_flags[1] =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
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
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
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

    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable,
                    &descriptor_set_variable, 1, "(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed."});

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

TEST_F(VkGpuAssistedLayerTest, GpuRobustBufferOOB) {
    TEST_DESCRIPTION("Check buffer oob validation when per pipeline robustness is enabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

    VkPhysicalDevicePipelineRobustnessFeaturesEXT pipeline_robustness_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_robustness_features);
    features2.features.robustBufferAccess = VK_FALSE;
    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    OneOffDescriptorSet descriptor_set(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *vertshader = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo { uint index[]; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;

        void main() {
            vec4 x;
            if (u_index.index[0] == 0)
                x[0] = u_index.index[1]; // Uniform read OOB
            else
                Data.data[8] = 0xdeadca71; // Storage write OOB
        }
    )glsl";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_ci = vku::InitStructHelper();
    pipeline_robustness_ci.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
    pipeline_robustness_ci.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;

    CreatePipelineHelper robust_pipe(*this);
    robust_pipe.InitState();
    robust_pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    robust_pipe.gp_ci_.layout = pipeline_layout.handle();
    robust_pipe.gp_ci_.pNext = &pipeline_robustness_ci;
    robust_pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, robust_pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)uniform_buffer.memory().map();
    *data = 0;
    uniform_buffer.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "Descriptor index 0 access out of bounds. Descriptor size is 4 and highest byte accessed was 19");
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyFound();
    data = (uint32_t *)uniform_buffer.memory().map();
    *data = 1;
    uniform_buffer.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "Descriptor index 0 access out of bounds. Descriptor size is 16 and highest byte accessed was 35");
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuBufferOOB) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper();
    const bool multi_draw = IsExtensionsEnabled(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    auto robustness2_features = vku::InitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>(multi_draw ? &multi_draw_features : nullptr);
    auto features2 = GetPhysicalDeviceFeatures2(robustness2_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 16;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    vkt::Buffer uniform_texel_buffer(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    vkt::Buffer storage_texel_buffer(*m_device, buffer_create_info, reqs);
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
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        vk::QueueWaitIdle(m_default_queue);
    }

    if (multi_draw && multi_draw_features.multiDraw) {
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;

        vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
        vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = 8;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-08613");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();

        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        data = (uint32_t *)offset_buffer.memory().map();
        *data = 0;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-08612");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();
    }
}

void VkGpuAssistedLayerTest::ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                                                  VkDescriptorType descriptor_type, const char *fragment_shader,
                                                  const char *expected_error, bool shader_objects) {
    if (shader_objects) {
        AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    }
    RETURN_IF_SKIP(InitGpuAvFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    dynamic_rendering_features.dynamicRendering = VK_TRUE;
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    shader_object_features.shaderObject = VK_TRUE;
    VkPhysicalDeviceFeatures2 features = vku::InitStructHelper();  // Make sure robust buffer access is not enabled
    if (shader_objects) {
        features.pNext = &shader_object_features;
    }
    RETURN_IF_SKIP(InitState(nullptr, &features));
    if (shader_objects) {
        InitDynamicRenderTarget();
    } else {
        InitRenderTarget();
    }

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, expected_error);

    OneOffDescriptorSet ds(m_device, {{0, descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                                     : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = buffer_size;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer(*m_device, bci);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = binding_offset;
    buffer_info.range = binding_range;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

    vkt::Shader *vso = nullptr;
    vkt::Shader *fso = nullptr;
    if (shader_objects) {
        vso = new vkt::Shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource),
                              &ds.layout_.handle());
        fso = new vkt::Shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
                              &ds.layout_.handle());
    }

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    if (shader_objects) {
        m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    } else {
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    }

    if (shader_objects) {
        const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                VK_SHADER_STAGE_FRAGMENT_BIT};
        const VkShaderEXT shaders[] = {vso->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fso->handle()};
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
        SetDefaultDynamicStates(m_commandBuffer->handle());
    } else {
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    }
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_,
                              0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    if (shader_objects) {
        m_commandBuffer->EndRendering();
    } else {
        m_commandBuffer->EndRenderPass();
    }
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(true);
    m_errorMonitor->VerifyFound();
    DestroyRenderTarget();
    if (shader_objects) {
        delete vso;
        delete fso;
    }
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
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        GTEST_SKIP() << "This test should not be run on the RADV driver.";
    }
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "This test should not be run on the AMD proprietary driver.";
    }
    const bool mesh_shader_supported = IsExtensionsEnabled(VK_NV_MESH_SHADER_EXTENSION_NAME);

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto bda_features =
        vku::InitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(mesh_shader_supported ? &mesh_shader_features : nullptr);

    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (!features2.features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 is not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 12;  // 64 bit pointer + int
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    // Make another buffer to write to
    bci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    bci.size = 64;  // Buffer should be 16*4 = 64 bytes
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer1(*m_device, bci, mem_props, &allocate_flag_info);

    // Get device address of buffer to write to
    auto pBuffer = buffer1.address();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &hinfo;

    struct TestCase {
        std::string name;
        std::string error;
        VkDeviceAddress push_constants[2] = {};
    };
    std::array<TestCase, 3> testcases{{{"starting address too low", "access out of bounds", {pBuffer - 16, 4}},
                                       {"run past the end", "access out of bounds", {pBuffer, 5}},
                                       {"positive", "", {pBuffer, 4}}}};

    // push constant version
    {
        VkPushConstantRange push_constant_ranges = {};
        push_constant_ranges.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_ranges.offset = 0;
        push_constant_ranges.size = 2 * sizeof(VkDeviceAddress);

        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci = vku::InitStructHelper();
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges = &push_constant_ranges;
        plci.setLayoutCount = 0;
        plci.pSetLayouts = nullptr;

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
                int a[4];
            };
            void main() {
                for (int i=0; i < u_info.nWrites; ++i) {
                    u_info.data.a[i] = 0xdeadca71;
                }
            }
        )glsl";
        VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_[0] = vs.GetStageCreateInfo();
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        for (const auto &test : testcases) {
            m_commandBuffer->begin(&begin_info);
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                                 sizeof(test.push_constants), test.push_constants);
            vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();

            if (!test.error.empty()) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
            }
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            vk::QueueWaitIdle(m_default_queue);
            if (!test.error.empty()) {
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->reset();
        }
    }
    // descriptor set version
    {
        OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

        const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
        VkDescriptorBufferInfo buffer_test_buffer_info = {};
        buffer_test_buffer_info.buffer = buffer0.handle();
        buffer_test_buffer_info.offset = 0;
        buffer_test_buffer_info.range = sizeof(uint32_t);

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_test_buffer_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

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
        VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_[0] = vs.GetStageCreateInfo();
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        for (const auto &test : testcases) {
            auto *data = static_cast<VkDeviceAddress *>(buffer0.memory().map());
            data[0] = test.push_constants[0];
            data[1] = test.push_constants[1];
            buffer0.memory().unmap();

            if (!test.error.empty()) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
            }
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            vk::QueueWaitIdle(m_default_queue);
            if (!test.error.empty()) {
                m_errorMonitor->VerifyFound();
            }
        }
    }

    if (mesh_shader_supported) {
        const unsigned push_constant_range_count = 1;
        VkPushConstantRange push_constant_ranges[push_constant_range_count] = {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = 2 * sizeof(VkDeviceAddress);

        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci.pushConstantRangeCount = push_constant_range_count;
        plci.pPushConstantRanges = push_constant_ranges;
        plci.setLayoutCount = 0;
        plci.pSetLayouts = nullptr;
        vkt::PipelineLayout mesh_pipeline_layout(*m_device, plci);

        char const *mesh_shader_source = R"glsl(
            #version 460
            #extension GL_NV_mesh_shader : require
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(push_constant) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4];
            };

            layout(local_size_x = 32) in;
            layout(max_vertices = 64, max_primitives = 126) out;
            layout(triangles) out;

            uint invocationID = gl_LocalInvocationID.x;
            void main() {
                if (invocationID == 0) {
                    for (int i=0; i < u_info.nWrites; ++i) {
                        u_info.data.a[i] = 0xdeadca71;
                    }
                }
            }
        )glsl";
        VkShaderObj ms(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       true);

        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.InitState();
        mesh_pipe.shader_stages_ = {ms.GetStageCreateInfo()};
        mesh_pipe.gp_ci_.layout = mesh_pipeline_layout.handle();
        mesh_pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        VkDeviceAddress push_constants[2] = {pBuffer, 5};
        vk::CmdPushConstants(m_commandBuffer->handle(), mesh_pipeline_layout.handle(), VK_SHADER_STAGE_MESH_BIT_NV, 0,
                             sizeof(push_constants), push_constants);
        vk::CmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectCountDeviceLimit) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);  // instead of enabling feature
    RETURN_IF_SKIP(InitGpuAvFramework())

    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper();
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        GetPhysicalDeviceFeatures2(features13);
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState(nullptr, features13.dynamicRendering ? (void *)&features13 : nullptr));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 2 * sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndirectCommand));
    draw_buffer.memory().unmap();

    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = sizeof(uint32_t);
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.memory().unmap();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);
    ASSERT_TRUE(pipeline_layout.initialized());

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                sizeof(VkDrawIndirectCommand));

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    m_errorMonitor->VerifyFound();

    if (!IsDriver(VK_DRIVER_ID_MESA_RADV) && features13.dynamicRendering) {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
        vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndirectCommand));

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndexedIndirectCountDeviceLimitSubmit2) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit using vkQueueSubmit2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework())

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    VkPhysicalDeviceVulkan13Features features_13 = vku::InitStructHelper();
    VkPhysicalDeviceVulkan12Features features_12 = vku::InitStructHelper(&features_13);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&features_12);
    GetPhysicalDeviceFeatures2(features2);
    if (!features_12.drawIndirectCount) {
        GTEST_SKIP() << "drawIndirectCount not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 2 * sizeof(VkDrawIndexedIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *draw_ptr = static_cast<VkDrawIndexedIndirectCommand *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndexedIndirectCommand));
    draw_buffer.memory().unmap();

    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = sizeof(uint32_t);
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.memory().unmap();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);
    ASSERT_TRUE(pipeline_layout.initialized());

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717");
    vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndexedIndirectCommand));

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vkt::Fence null_fence;
    // use vkQueueSumit2
    m_commandBuffer->QueueCommandBuffer(null_fence, true, true);
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectCount) {
    TEST_DESCRIPTION("GPU validation: Validate Draw*IndirectCount countBuffer contents");
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    draw_ptr->firstInstance = 0;
    draw_ptr->firstVertex = 0;
    draw_ptr->instanceCount = 1;
    draw_ptr->vertexCount = 3;
    draw_buffer.memory().unmap();

    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = sizeof(uint32_t);
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03122");
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;
    count_buffer.memory().unmap();
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03121");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Offset of 4 should error
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154");
    buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand);
    vkt::Buffer indexed_draw_buffer(*m_device, buffer_create_info,
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
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = 3 * sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153");
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    // Offset of 4 should error
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuDrawIndirectFirstInstance) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    features2.features.drawIndirectFirstInstance = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 4 * sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    for (uint32_t i = 0; i < 4; i++) {
        draw_ptr->vertexCount = 3;
        draw_ptr->instanceCount = 1;
        draw_ptr->firstVertex = 0;
        draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        draw_ptr++;
    }
    draw_buffer.memory().unmap();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawIndirectCommand-firstInstance-00501");
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();

    // Now with an offset and indexed draw
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554");
    buffer_create_info.size = 4 * sizeof(VkDrawIndexedIndirectCommand);
    vkt::Buffer indexed_draw_buffer(*m_device, buffer_create_info,
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
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = 3 * sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_draw_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationInlineUniformBlockAndMiscGpu) {
    TEST_DESCRIPTION(
        "GPU validation: Make sure inline uniform blocks don't generate false validation errors, verify reserved descriptor slot "
        "and verify pipeline recovery");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                              VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    validation_features.pEnabledValidationFeatures = enables;
    validation_features.enabledValidationFeatureCount = 2;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT inline_uniform_block_features = vku::InitStructHelper(&indexing_features);
    auto features2 = GetPhysicalDeviceFeatures2(inline_uniform_block_features);
    if (!indexing_features.descriptorBindingPartiallyBound || !inline_uniform_block_features.inlineUniformBlock) {
        GTEST_SKIP() << "Not all features supported";
    }
    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(inline_uniform_props);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();
    if (m_device->compute_queues().empty()) {
        GTEST_SKIP() << "Compute not supported";
    }

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 4;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vkt::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    layout_createinfo_binding_flags[0] = vku::InitStructHelper();
    layout_createinfo_binding_flags[0].bindingCount = 2;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, 32, VK_SHADER_STAGE_ALL,
                                            nullptr},
                                       },
                                       0, layout_createinfo_binding_flags, 0);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    const uint32_t test_data = 0xdeadca7;
    VkWriteDescriptorSetInlineUniformBlockEXT write_inline_uniform = vku::InitStructHelper();
    write_inline_uniform.dataSize = 4;
    write_inline_uniform.pData = &test_data;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;

    descriptor_writes[1] = vku::InitStructHelper(&write_inline_uniform);
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

    VkPipelineShaderStageCreateInfo stage = vku::InitStructHelper();
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shader_module.handle();
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    // CreateComputePipelines
    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
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
    if (m_device->phy().limits_.maxBoundDescriptorSets != properties.limits.maxBoundDescriptorSets - 1)
        m_errorMonitor->SetError("VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT not functioning as expected");
    vk::DestroyInstance(test_inst, NULL);

    auto set_count = properties.limits.maxBoundDescriptorSets;
    if (inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks < set_count) {
        GTEST_SKIP() << "Max per stage inline uniform block limit too small - skipping recovery portion of this test";
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
        vku::InitStructHelper(layout_createinfo_binding_flags);
    dsl_create_info.pBindings = dsl_binding;
    dsl_create_info.bindingCount = 2;
    for (uint32_t i = 0; i < set_count; i++) {
        vk::CreateDescriptorSetLayout(m_device->handle(), &dsl_create_info, NULL, &layouts[i]);
    }
    VkPipelineLayoutCreateInfo pl_create_info = vku::InitStructHelper();
    VkPipelineLayout pl_layout;
    pl_create_info.setLayoutCount = set_count;
    pl_create_info.pSetLayouts = layouts;
    // Expect error since GPU-AV cannot add debug descriptor to layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GPU-Assisted-Validation");
    vk::CreatePipelineLayout(m_device->handle(), &pl_create_info, NULL, &pl_layout);
    m_errorMonitor->VerifyFound();

    // We should still be able to use the layout and create a temporary uninstrumented shader module
    pipeline_info.layout = pl_layout;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &c_pipeline);
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pl_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    vk::DestroyPipelineLayout(m_device->handle(), pl_layout, NULL);
    vk::DestroyPipeline(m_device->handle(), c_pipeline, NULL);
    for (uint32_t i = 0; i < set_count; i++) {
        vk::DestroyDescriptorSetLayout(m_device->handle(), layouts[i], NULL);
    }
    data = (uint32_t *)buffer0.memory().map();
    if (*data != test_data) m_errorMonitor->SetError("Pipeline recovery when resources unavailable not functioning as expected");
    *data = 0;
    buffer0.memory().unmap();
    delete[] layouts;

    // Now make sure we can still use the shader with instrumentation
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
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    vk::DestroyPipeline(m_device->handle(), c_pipeline2, nullptr);
    data = (uint32_t *)buffer0.memory().map();
    if (*data != test_data) m_errorMonitor->SetError("Using shader after pipeline recovery not functioning as expected");
    *data = 0;
    buffer0.memory().unmap();

    // Destroy pipeline layout after creating pipeline
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    {
        const vkt::PipelineLayout doomed_pipeline_layout(*m_device);
        pipe.gp_ci_.layout = doomed_pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();
    }

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Unable to find pipeline layout to bind debug descriptor set");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkGpuAssistedLayerTest, GpuValidationAbort) {
    TEST_DESCRIPTION("GPU validation: Verify that aborting GPU-AV is safe.");
    RETURN_IF_SKIP(InitGpuAvFramework())

    PFN_vkSetPhysicalDeviceFeaturesEXT fpvkSetPhysicalDeviceFeaturesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFeaturesEXT fpvkGetOriginalPhysicalDeviceFeaturesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFeaturesEXT, fpvkGetOriginalPhysicalDeviceFeaturesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceFeatures features = {};
    fpvkGetOriginalPhysicalDeviceFeaturesEXT(gpu(), &features);

    // Disable features necessary for GPU-AV so initialization aborts
    features.vertexPipelineStoresAndAtomics = false;
    features.fragmentStoresAndAtomics = false;
    fpvkSetPhysicalDeviceFeaturesEXT(gpu(), features);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "GPU-Assisted Validation disabled");
    RETURN_IF_SKIP(InitState())
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, ValidationFeatures) {
    TEST_DESCRIPTION("Validate Validation Features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
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

TEST_F(VkGpuAssistedLayerTest, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound.");
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
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

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkGpuAssistedLayerTest, DispatchIndirectWorkgroupSize) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "GPU-Assisted validation test requires a driver that can draw.";
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 5 * sizeof(VkDispatchIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    vkt::Buffer indirect_buffer(*m_device, buffer_create_info,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.memory().map());
    // VkDispatchIndirectCommand[0]
    ptr->x = 4;  // over
    ptr->y = 2;
    ptr->z = 1;
    // VkDispatchIndirectCommand[1]
    ptr++;
    ptr->x = 2;
    ptr->y = 3;  // over
    ptr->z = 1;
    // VkDispatchIndirectCommand[2] - valid inbetween
    ptr++;
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;
    // VkDispatchIndirectCommand[3]
    ptr++;
    ptr->x = 0;  // allowed
    ptr->y = 2;
    ptr->z = 3;  // over
    // VkDispatchIndirectCommand[4]
    ptr++;
    ptr->x = 3;  // over
    ptr->y = 2;
    ptr->z = 3;  // over
    indirect_buffer.memory().unmap();

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));

    // Check again in a 2nd submitted command buffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, GpuBufferOOBGPL) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    auto validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(&robustness2_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 16;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    vkt::Buffer uniform_texel_buffer(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    vkt::Buffer storage_texel_buffer(*m_device, buffer_create_info, reqs);
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
    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 8)
                Data.data[u_index.index[0]] = 0xdeadca71;
            else if (u_index.index[0] == 0)
                Data.data[0] = u_index.index[4];
            else if (u_index.index[0] == 1)
                Data.data[0] = Null.data[40];  // No error
            else if (u_index.index[0] == 2)
                x = texelFetch(u_buffer, 5);
            else if (u_index.index[0] == 3)
                x = imageLoad(s_buffer, 5);
            else if (u_index.index[0] == 4)
                imageStore(s_buffer, 5, x);
            else if (u_index.index[0] == 5)  // No Error
                imageStore(s_buffer, 0, x);
            else if (u_index.index[0] == 6)  // No Error
                x = imageLoad(s_buffer, 0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    ASSERT_EQ(VK_SUCCESS, vi.CreateGraphicsPipeline(false));

    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(&pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = pipeline_layout.handle();
    pre_raster.CreateGraphicsPipeline(false);

    const auto render_pass = pre_raster.gp_ci_.renderPass;
    const auto subpass = pre_raster.gp_ci_.subpass;

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(nullptr);
    fragment.gp_ci_.stageCount = 0;
    fragment.shader_stages_.clear();
    fragment.gp_ci_.layout = pipeline_layout.handle();
    fragment.gp_ci_.renderPass = render_pass;
    fragment.gp_ci_.subpass = subpass;
    fragment.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    frag_out.gp_ci_.renderPass = render_pass;
    frag_out.gp_ci_.subpass = subpass;
    ASSERT_EQ(VK_SUCCESS, frag_out.CreateGraphicsPipeline(false));

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vkt::GraphicsPipelineFromLibraries pipe(*m_device, libraries, pipeline_layout.handle());
    ASSERT_TRUE(pipe);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        vk::QueueWaitIdle(m_default_queue);
    }
}

TEST_F(VkGpuAssistedLayerTest, GpuBufferOOBGPLIndependentSets) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    auto validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(&robustness2_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 16;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    vkt::Buffer uniform_texel_buffer(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    vkt::Buffer storage_texel_buffer(*m_device, buffer_create_info, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet common_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet fragment_set(m_device,
                                     {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, &common_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &common_set.layout_, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &common_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vertex_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    common_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    common_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.WriteDescriptorBufferView(1, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    fragment_set.WriteDescriptorBufferView(2, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    fragment_set.UpdateDescriptorSets();

    const std::array<VkDescriptorSet, 3> desc_sets = {vertex_set.set_, common_set.set_, fragment_set.set_};

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (u_index.index[0] == 8) {
                Data.data[u_index.index[0]] = 0xdeadca71;
            } else if (u_index.index[0] == 0) {
                Data.data[0] = u_index.index[4];
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    ASSERT_EQ(VK_SUCCESS, vi.CreateGraphicsPipeline(false));

    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(&pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = pipeline_layout_vs.handle();
    pre_raster.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 2, binding = 0) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 2, binding = 1) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 2, binding = 2, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            vec4 x;
            if (u_index.index[0] == 2) {
                x = texelFetch(u_buffer, 5);
            } else if (u_index.index[0] == 3) {
                x = imageLoad(s_buffer, 5);
            } else if (u_index.index[0] == 4) {
                imageStore(s_buffer, 5, x);
            } else if (u_index.index[0] == 5) { // No Error
                imageStore(s_buffer, 0, x);
            } else if (u_index.index[0] == 6) { // No Error
                x = imageLoad(s_buffer, 0);
            }
            c_out = x;
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fragment_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(&fragment_stage.stage_ci);
    fragment.gp_ci_.layout = pipeline_layout_fs.handle();
    fragment.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    ASSERT_EQ(VK_SUCCESS, frag_out.CreateGraphicsPipeline(false));

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vkt::GraphicsPipelineFromLibraries pipe(*m_device, libraries, pipeline_layout.handle());
    ASSERT_TRUE(pipe);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    // tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    // tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    // tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        vk::QueueWaitIdle(m_default_queue);
    }
}

TEST_F(VkGpuAssistedLayerTest, DrawTimeShaderObjectUniformBufferTooSmall) {
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
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, "Descriptor size is 4 and highest byte accessed was 7",
                         true);
}

TEST_F(VkGpuAssistedLayerTest, DispatchIndirectWorkgroupSizeShaderObjects) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "GPU-Assisted validation test requires a driver that can draw.";
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 5 * sizeof(VkDispatchIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    vkt::Buffer indirect_buffer(*m_device, buffer_create_info,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.memory().map());
    // VkDispatchIndirectCommand[0]
    ptr->x = 4;  // over
    ptr->y = 2;
    ptr->z = 1;
    // VkDispatchIndirectCommand[1]
    ptr++;
    ptr->x = 2;
    ptr->y = 3;  // over
    ptr->z = 1;
    // VkDispatchIndirectCommand[2] - valid inbetween
    ptr++;
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;
    // VkDispatchIndirectCommand[3]
    ptr++;
    ptr->x = 0;  // allowed
    ptr->y = 2;
    ptr->z = 3;  // over
    // VkDispatchIndirectCommand[4]
    ptr++;
    ptr->x = 3;  // over
    ptr->y = 2;
    ptr->z = 3;  // over
    indirect_buffer.memory().unmap();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
    vkt::Shader shader(*m_device, stage, GLSLToSPV(stage, kMinimalShaderGlsl));

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));

    // Check again in a 2nd submitted command buffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGpuAssistedLayerTest, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    validation_features.pNext = &layer_settings_create_info;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.robustBufferAccess) {
        GTEST_SKIP() << "Not safe to write outside of buffer memory";
    }
    // Robust buffer access will be on by default
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    InitState(nullptr, nullptr, pool_flags);
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] =
        "#version 450\n"
        "layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;\n"
        "void main() {\n"
        "        Data.data[4] = 0xdeadca71;\n"
        "}\n";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Should not get a warning since shader wasn't instrumented
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    VkShaderObj instrumented_vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                                false, &features);
    CreatePipelineHelper pipe2(*this);
    pipe2.InitState();
    pipe2.shader_stages_[0] = instrumented_vs.GetStageCreateInfo();
    pipe2.gp_ci_.layout = pipeline_layout.handle();
    pipe2.CreateGraphicsPipeline();

    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Should get a warning since shader was instrumented
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "VUID-vkCmdDraw-None-08613");
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}
