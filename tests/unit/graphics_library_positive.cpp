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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "generated/vk_extension_helper.h"

void GraphicsLibraryTest::InitBasicGraphicsLibrary(void *pNextFeatures) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(pNextFeatures);
    GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &gpl_features));
}

TEST_F(PositiveGraphicsLibrary, VertexInput) {
    TEST_DESCRIPTION("Create a vertex input graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline(false);
}

TEST_F(PositiveGraphicsLibrary, PreRaster) {
    TEST_DESCRIPTION("Create a pre-raster graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())

    InitRenderTarget();

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, FragmentShader) {
    TEST_DESCRIPTION("Create a fragment shader graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe.InitFragmentLibInfo(&fs_stage.stage_ci);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, FragmentOutput) {
    TEST_DESCRIPTION("Create a fragment output graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline(false);
}

TEST_F(PositiveGraphicsLibrary, FragmentMixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Create a fragment graphics library with mixed attachement sample support");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.gp_ci_.pRasterizationState = nullptr;

    pipe.gp_ci_.pRasterizationState = nullptr;

    // Ensure validation runs with pRasterizationState being nullptr.
    // It's legal for this fragment library to not have a raster state defined.
    ASSERT_TRUE(pipe.gp_ci_.pRasterizationState == nullptr);

    pipe.CreateGraphicsPipeline(false);
}

TEST_F(PositiveGraphicsLibrary, ExeLibrary) {
    TEST_DESCRIPTION("Create an executable library by linking one or more graphics libraries");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.CreateGraphicsPipeline(false);

    VkPipelineLayout layout = VK_NULL_HANDLE;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, DrawWithNullDSLs) {
    TEST_DESCRIPTION("Make a draw with a pipeline layout derived from null DSLs");

    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      });

    // We _vs and _fs layouts are identical, but we want them to be separate handles handles for the sake layout merging
    vkt::PipelineLayout pipeline_layout_vs(*m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_fs(*m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_null(*m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                             VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    const std::array<VkDescriptorSet, 3> desc_sets = {ds.set_, VK_NULL_HANDLE, ds2.set_};

    VkBufferCreateInfo ub_ci = vku::InitStructHelper();
    ub_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ub_ci.size = 1024;
    vkt::Buffer uniform_buffer(*m_device, ub_ci);
    ds.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds.UpdateDescriptorSets();
    ds2.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds2.UpdateDescriptorSets();

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.CreateGraphicsPipeline(false);

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
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = pipeline_layout_vs.handle();
        pre_raster_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.InitState();
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout_null.handle();
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    if (device_props.limits.maxVertexInputAttributeOffset == 0xFFFFFFFF) {
        GTEST_SKIP() << "maxVertexInputAttributeOffset is max<uint32_t> already";
    }

    InitRenderTarget();

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = m_device->phy().limits_.maxVertexInputBindingStride;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // Test when offset is greater than maximum.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.format = VK_FORMAT_R8_UNORM;
    vertex_input_attribute_description.offset = device_props.limits.maxVertexInputAttributeOffset + 1;

    CreatePipelineHelper frag_shader_lib(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    // override vertex input
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
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
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())

    InitRenderTarget();

    VkVertexInputBindingDivisorDescriptionEXT divisor_description = {};
    divisor_description.binding = 0;
    divisor_description.divisor = 0;
    VkPipelineVertexInputDivisorStateCreateInfoEXT divisor_state_create_info = vku::InitStructHelper();
    divisor_state_create_info.vertexBindingDivisorCount = 1;
    divisor_state_create_info.pVertexBindingDivisors = &divisor_description;
    VkVertexInputBindingDescription vertex_input_binding_description = {divisor_description.binding, 12,
                                                                        VK_VERTEX_INPUT_RATE_INSTANCE};
    VkVertexInputAttributeDescription vertex_input_attribute_description = {0, 0, VK_FORMAT_R8_UNORM, 0};

    CreatePipelineHelper frag_shader_lib(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
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
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&extended_dynamic_state3_features))

    if (!m_device->phy().features().dualSrcBlend) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.cb_ci_.pAttachments = nullptr;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.CreateGraphicsPipeline(false);
}

TEST_F(PositiveGraphicsLibrary, DynamicPrimitiveTopolgyAllState) {
    TEST_DESCRIPTION("VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY works when GPL sets it in every state");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = vku::InitStructHelper();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    VkPipelineLayout layout = VK_NULL_HANDLE;

    VkPipelineInputAssemblyStateCreateInfo ia_state = vku::InitStructHelper();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    vertex_input_lib.CreateGraphicsPipeline(false);

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        pre_raster_lib.gp_ci_.pInputAssemblyState = &ia_state;
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.pDynamicState = &dynamic_create_info;
        frag_shader_lib.gp_ci_.pInputAssemblyState = &ia_state;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    frag_out_lib.gp_ci_.pInputAssemblyState = &ia_state;
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.pInputAssemblyState = &ia_state;
    exe_pipe_ci.pDynamicState = &dynamic_create_info;
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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

    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    VkDynamicState dynamic_states[1] = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = vku::InitStructHelper();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 1;

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    VkPipelineInputAssemblyStateCreateInfo ia_state = vku::InitStructHelper();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.gp_ci_.pDynamicState = &dynamic_create_info;
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    vertex_input_lib.CreateGraphicsPipeline(false);

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.pInputAssemblyState = &ia_state;
    exe_pipe_ci.pDynamicState = &dynamic_create_info;
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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

    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    VkPipelineInputAssemblyStateCreateInfo ia_state = vku::InitStructHelper();
    ia_state.primitiveRestartEnable = false;
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    vertex_input_lib.gp_ci_.pInputAssemblyState = &ia_state;
    vertex_input_lib.CreateGraphicsPipeline(false);

    // change here and make sure other libraries don't consume this
    ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dyn_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dyn_state3_features))
    InitRenderTarget();

    if (!dyn_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToOneEnable";
    }

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    frag_out_lib.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dyn_state3_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dyn_state3_features))
    InitRenderTarget();

    if (!dyn_state3_features.extendedDynamicState3AlphaToOneEnable) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState3AlphaToOneEnable";
    }

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    layout = pre_raster_lib.gp_ci_.layout;
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
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
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())
    InitRenderTarget();

    VkPipelineLayout layout = VK_NULL_HANDLE;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.CreateGraphicsPipeline();
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
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, TessellationWithoutPreRasterization) {
    TEST_DESCRIPTION("have Tessellation stages with null pTessellationState but not Pre-Rasterization");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary())

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();

    VkPipelineShaderStageCreateInfo stages[2];

    const auto tcs_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    VkShaderModuleCreateInfo tcs_ci = vku::InitStructHelper();
    tcs_ci.codeSize = tcs_spv.size() * sizeof(decltype(tcs_spv)::value_type);
    tcs_ci.pCode = tcs_spv.data();
    stages[0] = vku::InitStructHelper(&tcs_ci);
    stages[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    stages[0].module = VK_NULL_HANDLE;
    stages[0].pName = "main";

    const auto tes_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    VkShaderModuleCreateInfo tes_ci = vku::InitStructHelper();
    tes_ci.codeSize = tes_spv.size() * sizeof(decltype(tes_spv)::value_type);
    tes_ci.pCode = tes_spv.data();
    stages[1] = vku::InitStructHelper(&tes_ci);
    stages[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    stages[1].module = VK_NULL_HANDLE;
    stages[1].pName = "main";

    pipe.gp_ci_.stageCount = 2;
    pipe.gp_ci_.pStages = stages;
    pipe.CreateGraphicsPipeline(false);
}

TEST_F(PositiveGraphicsLibrary, FSIgnoredPointerGPLDynamicRendering) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and GPL");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dynamic_rendering_features))

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    // Create a full pipeline with the same bad rendering info, but enable rasterizer discard to ignore the bad data
    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    vi_lib.CreateGraphicsPipeline();

    // Create an executable pipeline with rasterization disabled
    // Pass rendering info with null pointers that should be ignored
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 2;  // <- bad data that should be ignored

    CreatePipelineHelper fs_lib(*this);
    {
        VkStencilOpState stencil = {};
        stencil.failOp = VK_STENCIL_OP_KEEP;
        stencil.passOp = VK_STENCIL_OP_KEEP;
        stencil.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil.compareOp = VK_COMPARE_OP_NEVER;

        VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_TRUE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(&fs_stage.stage_ci, &pipeline_rendering_info);
        fs_lib.InitState();
        fs_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        fs_lib.CreateGraphicsPipeline();
    }

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pr_lib(*this);
    pr_lib.InitPreRasterLibInfo(&vs_stage.stage_ci, &pipeline_rendering_info);
    pr_lib.rs_state_ci_.rasterizerDiscardEnable =
        VK_TRUE;  // This should cause the bad info in pipeline_rendering_info to be ignored
    pr_lib.InitState();
    pr_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pr_lib.CreateGraphicsPipeline();

    VkPipeline libraries[3] = {
        vi_lib.pipeline_, pr_lib.pipeline_, fs_lib.pipeline_,
        // fragment output not needed due to rasterization being disabled
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, GPLDynamicRenderingWithDepthDraw) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and GPL");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dynamic_rendering_features))

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

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

        VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_TRUE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        fs_lib.CreateGraphicsPipeline();
    }

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pr_lib(*this);
    pr_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pr_lib.InitState();
    pr_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pr_lib.CreateGraphicsPipeline();

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = m_depth_stencil_fmt;

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo(&pipeline_rendering_info);
    fo_lib.InitState();
    fo_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    fo_lib.CreateGraphicsPipeline(false);

    // Create an executable pipeline with rasterization disabled
    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pr_lib.pipeline_,
        fs_lib.pipeline_,
        fo_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

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
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dyn_state2_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dyn_state2_features))

    if (!dyn_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    CreatePipelineHelper fs_lib(*this);
    {
        VkStencilOpState stencil = {};
        stencil.failOp = VK_STENCIL_OP_KEEP;
        stencil.passOp = VK_STENCIL_OP_KEEP;
        stencil.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil.compareOp = VK_COMPARE_OP_NEVER;

        VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
        ds_ci.depthTestEnable = VK_FALSE;
        ds_ci.depthWriteEnable = VK_TRUE;
        ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
        ds_ci.depthBoundsTestEnable = VK_FALSE;
        ds_ci.stencilTestEnable = VK_FALSE;
        ds_ci.front = stencil;
        ds_ci.back = stencil;

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        fs_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.pDepthStencilState = &ds_ci;
        fs_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    vi_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    fo_lib.CreateGraphicsPipeline(false);

    // Create a GPL and subpass that utilizes depth
    {
        CreatePipelineHelper pr_lib(*this);
        {
            const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
            vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

            pr_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
            pr_lib.InitState();
            pr_lib.CreateGraphicsPipeline();
        }

        VkPipeline libraries[4] = {
            vi_lib.pipeline_,
            pr_lib.pipeline_,
            fs_lib.pipeline_,
            fo_lib.pipeline_,
        };
        VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
        link_info.libraryCount = size32(libraries);
        link_info.pLibraries = libraries;

        VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
        exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
        vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        ASSERT_TRUE(exe_pipe.initialized());
    }

    // Create a GPL and subpass that utilizes depth, but specifies rasterizerDiscardEnabled dynamically
    CreatePipelineHelper pr_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

        pr_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pr_lib.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;  // This should get ignored due to its state being set as dynamic
        pr_lib.InitState();
        pr_lib.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT);
        pr_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        pr_lib.CreateGraphicsPipeline(false);
    }

    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pr_lib.pipeline_,
        fs_lib.pipeline_,
        fo_lib.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pr_lib.gp_ci_.layout;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGraphicsLibrary, FOIgnoredDynamicRendering) {
    TEST_DESCRIPTION("Check ignored pointers with dynamics rendering and no fragment output state");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dynamic_rendering_features))

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    // Create an executable pipeline with rasterization disabled
    // Pass rendering info with null pointers that should be ignored
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 2;  // <- bad data that should be ignored

    VkGraphicsPipelineLibraryCreateInfoEXT lib_info = vku::InitStructHelper(&pipeline_rendering_info);
    lib_info.flags =
        VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.depthBoundsTestEnable = VK_FALSE;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::array stages = {vs_stage.stage_ci, fs_stage.stage_ci};

    CreatePipelineHelper lib(*this);
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

TEST_F(PositiveGraphicsLibrary, ShaderModuleIdentifier) {
    TEST_DESCRIPTION("Create pipeline sub-state that references shader module identifiers");
    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper();
    VkPhysicalDevicePipelineCreationCacheControlFeatures pipeline_cache_control_features = vku::InitStructHelper(&gpl_features);
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT shader_module_id_features =
        vku::InitStructHelper(&pipeline_cache_control_features);
    GetPhysicalDeviceFeatures2(shader_module_id_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "graphicsPipelineLibrary feature not supported";
    }
    if (!shader_module_id_features.shaderModuleIdentifier) {
        GTEST_SKIP() << "shaderModuleIdentifier feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &shader_module_id_features));
    InitRenderTarget();

    // Create a pre-raster pipeline referencing a VS via identifier, with the VS identifier queried from a shader module
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    ASSERT_TRUE(vs.initialized());

    VkShaderModuleIdentifierEXT vs_identifier = vku::InitStructHelper();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &vs_identifier);

    VkPipelineShaderStageModuleIdentifierCreateInfoEXT sm_id_create_info = vku::InitStructHelper();
    sm_id_create_info.identifierSize = vs_identifier.identifierSize;
    sm_id_create_info.pIdentifier = vs_identifier.identifier;

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&sm_id_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitPreRasterLibInfo(&stage_ci);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Create a fragment shader library with FS referencing an identifier queried from VkShaderModuleCreateInfo
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderModuleCreateInfo fs_ci = vku::InitStructHelper();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    VkShaderModuleIdentifierEXT fs_identifier = vku::InitStructHelper();
    vk::GetShaderModuleCreateInfoIdentifierEXT(device(), &fs_ci, &fs_identifier);

    sm_id_create_info.identifierSize = fs_identifier.identifierSize;
    sm_id_create_info.pIdentifier = fs_identifier.identifier;

    VkPipelineShaderStageCreateInfo fs_stage_ci = vku::InitStructHelper(&sm_id_create_info);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";

    CreatePipelineHelper fs_pipe(*this);
    fs_pipe.InitFragmentLibInfo(&fs_stage_ci);
    fs_pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    fs_pipe.gp_ci_.layout = pipe.gp_ci_.layout;
    fs_pipe.InitState();
    fs_pipe.CreateGraphicsPipeline(false);

    // Create a complete pipeline with the above pre-raster fs libraries
    CreatePipelineHelper vi_pipe(*this);
    vi_pipe.InitVertexInputLibInfo();
    vi_pipe.CreateGraphicsPipeline();

    CreatePipelineHelper fo_pipe(*this);
    fo_pipe.InitFragmentOutputLibInfo();
    fo_pipe.CreateGraphicsPipeline();

    VkPipeline libraries[4] = {
        vi_pipe.pipeline_,
        pipe.pipeline_,
        fs_pipe.pipeline_,
        fo_pipe.pipeline_,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo pipe_ci = vku::InitStructHelper(&link_info);
    pipe_ci.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe_ci.layout = pipe.gp_ci_.layout;
    vkt::Pipeline exe_pipe(*m_device, pipe_ci);
}

TEST_F(PositiveGraphicsLibrary, DepthStencilStateIgnored) {
    TEST_DESCRIPTION("Create a library with fragment shader state, but no fragment output state, and no DS state, but ignored");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features =
        vku::InitStructHelper(&extended_dynamic_state_features);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dynamic_rendering_features))

    InitRenderTarget();

    CreatePipelineHelper frag_shader_lib(*this);
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib.InitState();

    frag_shader_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    frag_shader_lib.gp_ci_.pDepthStencilState = nullptr;
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_OP);
    frag_shader_lib.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    frag_shader_lib.CreateGraphicsPipeline();
}

TEST_F(PositiveGraphicsLibrary, ColorBlendStateIgnored) {
    TEST_DESCRIPTION("Create a library with fragment output state and invalid ColorBlendState state");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features = vku::InitStructHelper();
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features =
        vku::InitStructHelper(&extended_dynamic_state2_features);
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features =
        vku::InitStructHelper(&extended_dynamic_state3_features);
    RETURN_IF_SKIP(InitBasicGraphicsLibrary(&dynamic_rendering_features))
    InitRenderTarget();

    if (!extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable) {
        GTEST_SKIP() << "extendedDynamicState3LogicOpEnable not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEnable not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation) {
        GTEST_SKIP() << "extendedDynamicState3ColorBlendEquation not supported";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "extendedDynamicState3ColorWriteMask not supported";
    }

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.InitState();

        pre_raster_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
        pre_raster_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper frag_output_lib(*this);
    {
        link_info.pNext = &pipeline_rendering_info;
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_output_lib.InitFragmentOutputLibInfo(&link_info);
        frag_output_lib.InitState();

        frag_output_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        frag_output_lib.gp_ci_.pColorBlendState = nullptr;
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
        frag_output_lib.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        frag_output_lib.CreateGraphicsPipeline();
    }
}
