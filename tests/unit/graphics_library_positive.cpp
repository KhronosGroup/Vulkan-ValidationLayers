/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

void GraphicsLibraryTest::InitBasicGraphicsLibrary(void *pNextFeatures) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(pNextFeatures);
    GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));
    if (DeviceValidationVersion() < m_attempted_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_attempted_api_version.minor() << " is required";
    }
}

TEST_F(PositiveGraphicsLibrary, VertexInput) {
    TEST_DESCRIPTION("Create a vertex input graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(PositiveGraphicsLibrary, PreRaster) {
    TEST_DESCRIPTION("Create a pre-raster graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitPreRasterLibInfo(1, &stage_ci);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, FragmentShader) {
    TEST_DESCRIPTION("Create a fragment shader graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentLibInfo(1, &stage_ci);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, FragmentOutput) {
    TEST_DESCRIPTION("Create a fragment output graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(PositiveGraphicsLibrary, FragmentMixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Create a fragment graphics library with mixed attachement sample support");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.gp_ci_.pRasterizationState = nullptr;

    pipe.gp_ci_.pRasterizationState = nullptr;

    // Ensure validation runs with pRasterizationState being nullptr.
    // It's legal for this fragment library to not have a raster state defined.
    ASSERT_TRUE(pipe.gp_ci_.pRasterizationState == nullptr);

    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(PositiveGraphicsLibrary, ExeLibrary) {
    TEST_DESCRIPTION("Create an executable library by linking one or more graphics libraries");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    VkPipelineLayout layout = VK_NULL_HANDLE;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline(true, false));
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, DrawWithNullDSLs) {
    TEST_DESCRIPTION("Make a draw with a pipeline layout derived from null DSLs");

    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      });

    // We _vs and _fs layouts are identical, but we want them to be separate handles handles for the sake layout merging
    VkPipelineLayoutObj pipeline_layout_vs(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_fs(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_null(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                             VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    const std::array<VkDescriptorSet, 3> desc_sets = {ds.set_, VK_NULL_HANDLE, ds2.set_};

    auto ub_ci = LvlInitStruct<VkBufferCreateInfo>();
    ub_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ub_ci.size = 1024;
    VkBufferObj uniform_buffer(*m_device, ub_ci);
    ds.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds.UpdateDescriptorSets();
    ds2.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds2.UpdateDescriptorSets();

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const char vs_src[] = R"glsl(
            #version 450
            layout(set=0, binding=0) uniform foo { float x; } bar;
            void main() {
            gl_Position = vec4(bar.x);
            }
        )glsl";
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = pipeline_layout_vs.handle();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline(true, false));
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.InitState();
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pipeline_layout_null.handle();
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Draw with pipeline created with null set
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_null.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, VertexInputAttributeDescriptionOffset) {
    TEST_DESCRIPTION("Test VUID-VkVertexInputAttributeDescription-offset-00622: is not trigged with graphics library");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    if (device_props.limits.maxVertexInputAttributeOffset == 0xFFFFFFFF) {
        GTEST_SKIP() << "maxVertexInputAttributeOffset is max<uint32_t> already";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = m_device->props.limits.maxVertexInputBindingStride;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // Test when offset is greater than maximum.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.format = VK_FORMAT_R8_UNORM;
    vertex_input_attribute_description.offset = device_props.limits.maxVertexInputAttributeOffset + 1;

    CreatePipelineHelper frag_shader_lib(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    // override vertex input
    frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
    frag_shader_lib.InitState();
    frag_shader_lib.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
    frag_shader_lib.vi_ci_.vertexBindingDescriptionCount = 1;
    frag_shader_lib.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
    frag_shader_lib.vi_ci_.vertexAttributeDescriptionCount = 1;
    frag_shader_lib.gp_ci_.pVertexInputState = &frag_shader_lib.vi_ci_;

    // VUID-VkVertexInputAttributeDescription-offset-00622 shouldn't be trigged
    frag_shader_lib.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, VertexAttributeDivisorInstanceRateZero) {
    TEST_DESCRIPTION("VK_EXT_vertex_attribute_divisor is not checked with VK_EXT_graphics_pipeline_library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDivisorDescriptionEXT divisor_description = {};
    divisor_description.binding = 0;
    divisor_description.divisor = 0;
    auto divisor_state_create_info = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    divisor_state_create_info.vertexBindingDivisorCount = 1;
    divisor_state_create_info.pVertexBindingDivisors = &divisor_description;
    VkVertexInputBindingDescription vertex_input_binding_description = {divisor_description.binding, 12,
                                                                        VK_VERTEX_INPUT_RATE_INSTANCE};
    VkVertexInputAttributeDescription vertex_input_attribute_description = {0, 0, VK_FORMAT_R8_UNORM, 0};

    CreatePipelineHelper frag_shader_lib(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
    frag_shader_lib.InitState();

    // override vertex input
    frag_shader_lib.vi_ci_.pNext = &divisor_state_create_info;
    frag_shader_lib.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
    frag_shader_lib.vi_ci_.vertexBindingDescriptionCount = 1;
    frag_shader_lib.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
    frag_shader_lib.vi_ci_.vertexAttributeDescriptionCount = 1;
    frag_shader_lib.gp_ci_.pVertexInputState = &frag_shader_lib.vi_ci_;

    // VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateZeroDivisor-02228 shouldn't be trigged
    frag_shader_lib.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, NotAttachmentDynamicBlendEnable) {
    TEST_DESCRIPTION("make sure using an empty pAttachments doesn't crash a GPL pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    auto extended_dynamic_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicGraphicsLibrary(&extended_dynamic_state3_features);
    if (::testing::Test::IsSkipped()) return;

    if (!m_device->phy().features().dualSrcBlend) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                        VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 4;

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.cb_ci_.pAttachments = nullptr;
    pipe.gp_ci_.pDynamicState = &dynamic_create_info;
    pipe.CreateGraphicsPipeline(true, false);
}

TEST_F(PositiveGraphicsLibrary, DynamicPrimitiveTopolgyAllState) {
    TEST_DESCRIPTION("VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY works when GPL sets it in every state");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    VkPipelineLayout layout = VK_NULL_HANDLE;

    auto ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        pre_raster_lib.gp_ci_.pInputAssemblyState = &ia_state;
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        frag_shader_lib.gp_ci_.pInputAssemblyState = &ia_state;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    frag_out_lib.gp_ci_.pInputAssemblyState = &ia_state;
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.pInputAssemblyState = &ia_state;
    exe_pipe_ci.pDynamicState = &dynamic_create_info;
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetPrimitiveTopology(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, DynamicPrimitiveTopolgyVertexStateAndLinked) {
    TEST_DESCRIPTION("set VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY in Vertex State only and at Link time");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    auto ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.pInputAssemblyState = &ia_state;
    exe_pipe_ci.pDynamicState = &dynamic_create_info;
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetPrimitiveTopology(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, DynamicPrimitiveTopolgyVertexStateOnly) {
    TEST_DESCRIPTION("set VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY in Vertex State only, but not at Link time");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    auto ia_state = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetPrimitiveTopology(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, DynamicAlphaToOneEnableFragmentOutput) {
    TEST_DESCRIPTION("set VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT in Fragment Output");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

    auto dyn_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicGraphicsLibrary(&dyn_state3_features);
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!dyn_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToOneEnable";
    }

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    frag_out_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetAlphaToOneEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, DynamicAlphaToOneEnableFragmentShader) {
    TEST_DESCRIPTION("set VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT in Fragment Shader");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

    auto dyn_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    InitBasicGraphicsLibrary(&dyn_state3_features);
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!dyn_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToOneEnable";
    }

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdSetAlphaToOneEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, LinkingInputAttachment) {
    TEST_DESCRIPTION("Make sure OpCapability InputAttachment is not detected at linking time");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineLayout layout = VK_NULL_HANDLE;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
        vs_ci.pCode = vs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    layout = pre_raster_lib.gp_ci_.layout;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        // kFragmentMinimalGlsl with manually added OpCapability
        const char fs_src[] = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %12 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %uFragColor %12
               OpReturn
               OpFunctionEnd
        )";
        vector<uint32_t> fs_spv;
        ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, fs_src, fs_spv);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, TessellationWithoutPreRasterization) {
    TEST_DESCRIPTION("have Tessellation stages with null pTessellationState but not Pre-Rasterization");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();

    VkPipelineShaderStageCreateInfo stages[2];

    const auto tcs_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    auto tcs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    tcs_ci.codeSize = tcs_spv.size() * sizeof(decltype(tcs_spv)::value_type);
    tcs_ci.pCode = tcs_spv.data();
    stages[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&tcs_ci);
    stages[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    stages[0].module = VK_NULL_HANDLE;
    stages[0].pName = "main";

    const auto tes_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    auto tes_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    tes_ci.codeSize = tes_spv.size() * sizeof(decltype(tes_spv)::value_type);
    tes_ci.pCode = tes_spv.data();
    stages[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&tes_ci);
    stages[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    stages[1].module = VK_NULL_HANDLE;
    stages[1].pName = "main";

    pipe.gp_ci_.stageCount = 2;
    pipe.gp_ci_.pStages = stages;
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(PositiveGraphicsLibrary, FSIgnoredPointerGPLDynamicRendering) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and GPL");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    // Create a full pipeline with the same bad rendering info, but enable rasterizer discard to ignore the bad data
    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    vi_lib.CreateGraphicsPipeline();

    // Create an executable pipeline with rasterization disabled
    // Pass rendering info with null pointers that should be ignored
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfo>();
    pipeline_rendering_info.colorAttachmentCount = 2;  // <- bad data that should be ignored

    CreatePipelineHelper fs_lib(*this);
    {
        VkStencilOpState stencil = {};
        stencil.failOp = VK_STENCIL_OP_KEEP;
        stencil.passOp = VK_STENCIL_OP_KEEP;
        stencil.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil.compareOp = VK_COMPARE_OP_NEVER;

        auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_TRUE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci, &pipeline_rendering_info);
        fs_lib.InitState();
        fs_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        ASSERT_VK_SUCCESS(fs_lib.CreateGraphicsPipeline());
    }

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pr_lib(*this);
    pr_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci, &pipeline_rendering_info);
    pr_lib.rs_state_ci_.rasterizerDiscardEnable =
        VK_TRUE;  // This should cause the bad info in pipeline_rendering_info to be ignored
    pr_lib.InitState();
    pr_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pr_lib.CreateGraphicsPipeline();

    VkPipeline libraries[3] = {
        vi_lib.pipeline_, pr_lib.pipeline_, fs_lib.pipeline_,
        // fragment output not needed due to rasterization being disabled
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, GPLDynamicRenderingWithDepthDraw) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and GPL");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    // Create a full pipeline with the same bad rendering info, but enable rasterizer discard to ignore the bad data
    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    vi_lib.CreateGraphicsPipeline();

    // Create an executable pipeline with rasterization enabled and make a draw call using dynamic rendering
    CreatePipelineHelper fs_lib(*this);
    {
        VkStencilOpState stencil = {};
        stencil.failOp = VK_STENCIL_OP_KEEP;
        stencil.passOp = VK_STENCIL_OP_KEEP;
        stencil.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil.compareOp = VK_COMPARE_OP_NEVER;

        auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_TRUE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        ASSERT_VK_SUCCESS(fs_lib.CreateGraphicsPipeline());
    }

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pr_lib(*this);
    pr_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci);
    pr_lib.InitState();
    pr_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pr_lib.CreateGraphicsPipeline();

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfo>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = m_depth_stencil_fmt;

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    fo_lib.InitState();
    fo_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(fo_lib.CreateGraphicsPipeline(true, false));

    // Create an executable pipeline with rasterization disabled
    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pr_lib.pipeline_,
        fs_lib.pipeline_,
        fo_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto depth_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    depth_attachment.imageView = *m_depthStencil->BindInfo();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveGraphicsLibrary, DepthState) {
    TEST_DESCRIPTION("Create a GPL with depth state");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    auto dyn_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    InitBasicGraphicsLibrary(&dyn_state2_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dyn_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper fs_lib(*this);
    {
        VkStencilOpState stencil = {};
        stencil.failOp = VK_STENCIL_OP_KEEP;
        stencil.passOp = VK_STENCIL_OP_KEEP;
        stencil.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil.compareOp = VK_COMPARE_OP_NEVER;

        auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_FALSE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        ASSERT_VK_SUCCESS(fs_lib.CreateGraphicsPipeline());
    }

    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    ASSERT_VK_SUCCESS(vi_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    ASSERT_VK_SUCCESS(fo_lib.CreateGraphicsPipeline(true, false));

    // Create a GPL and subpass that utilizes depth
    {
        CreatePipelineHelper pr_lib(*this);
        {
            const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
            vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

            pr_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci);
            pr_lib.InitState();
            ASSERT_VK_SUCCESS(pr_lib.CreateGraphicsPipeline());
        }

        VkPipeline libraries[4] = {
            vi_lib.pipeline_,
            pr_lib.pipeline_,
            fs_lib.pipeline_,
            fo_lib.pipeline_,
        };
        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = size32(libraries);
        link_info.pLibraries = libraries;

        auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
        exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
        vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        ASSERT_TRUE(exe_pipe.initialized());
    }

    // Create a GPL and subpass that utilizes depth, but specifies rasterizerDiscardEnabled dynamically
    CreatePipelineHelper pr_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

        pr_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci);
        pr_lib.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;  // This should get ignored due to its state being set as dynamic
        pr_lib.InitState();
        VkDynamicState dynamic_state = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT;
        VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dynamic_create_info.pDynamicStates = &dynamic_state;
        dynamic_create_info.dynamicStateCount = 1;
        pr_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        pr_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        ASSERT_VK_SUCCESS(pr_lib.CreateGraphicsPipeline(true, false));
    }

    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pr_lib.pipeline_,
        fs_lib.pipeline_,
        fo_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, FOIgnoredDynamicRendering) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and no fragment output state");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_device, m_width, m_height, m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    // Create an executable pipeline with rasterization disabled
    // Pass rendering info with null pointers that should be ignored
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfo>();
    pipeline_rendering_info.colorAttachmentCount = 2;  // <- bad data that should be ignored

    auto lib_info = LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&pipeline_rendering_info);
    lib_info.flags =
        VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.depthBoundsTestEnable = VK_FALSE;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::array stages = {vs_stage.stage_ci, fs_stage.stage_ci};

    CreatePipelineHelper lib(*this);
    lib.InitInfo();
    lib.InitState();
    lib.gp_ci_.pNext = &lib_info;
    lib.gp_ci_.flags |= VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    lib.gp_ci_.pDepthStencilState = &ds_ci;
    lib.gp_ci_.stageCount = size32(stages);
    lib.gp_ci_.pStages = stages.data();

    // Remove VI and FO state-related pointers
    lib.gp_ci_.pVertexInputState = nullptr;
    lib.gp_ci_.pVertexInputState = nullptr;
    lib.gp_ci_.pColorBlendState = nullptr;
    lib.gp_ci_.pMultisampleState = nullptr;
    lib.gp_ci_.renderPass = VK_NULL_HANDLE;

    lib.CreateGraphicsPipeline();
}
