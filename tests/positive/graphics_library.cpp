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
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>

class VkPositiveGraphicsLibraryLayerTest : public VkLayerTest {};

TEST_F(VkPositiveGraphicsLibraryLayerTest, VertexInputGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a vertex input graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, PreRasterGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a pre-raster graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, FragmentShaderGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a fragment shader graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, FragmentOutputGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a fragment output graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, FragmentMixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Create a fragment graphics library with mixed attachement sample support");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();

    // Ensure validation runs with pRasterizationState being nullptr.
    // It's legal for this fragment library to not have a raster state defined.
    ASSERT_TRUE(pipe.gp_ci_.pRasterizationState == nullptr);

    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, ExeLibrary) {
    TEST_DESCRIPTION("Create an executable library by linking one or more graphics libraries");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
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
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        // frag_shader_lib.InitState();
        // // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline(true, false));
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    // frag_out_lib.InitState();
    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, DrawWithNullDSLs) {
    TEST_DESCRIPTION("Make a draw with a pipeline layout derived from null DSLs");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

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

    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
        auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
        fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
        fs_ci.pCode = fs_spv.data();

        auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = "main";

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci);
        frag_shader_lib.InitState();

        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    // frag_out_lib.InitState();
    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, VertexInputAttributeDescriptionOffset) {
    TEST_DESCRIPTION("Test VUID-VkVertexInputAttributeDescription-offset-00622: is not trigged with graphics library");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    if (device_props.limits.maxVertexInputAttributeOffset == 0xFFFFFFFF) {
        GTEST_SKIP() << "maxVertexInputAttributeOffset is max<uint32_t> already";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));
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
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, VertexAttributeDivisorInstanceRateZero) {
    TEST_DESCRIPTION("VK_EXT_vertex_attribute_divisor is not checked with VK_EXT_graphics_pipeline_library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto vertex_attr_divisor_features = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&vertex_attr_divisor_features);
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    vertex_attr_divisor_features.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, NotAttachmentDynamicBlendEnable) {
    TEST_DESCRIPTION("make sure using an empty pAttachments doesn't crash a GPL pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto extended_dynamic_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(&gpl_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState dynamic_states[3] = {VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                        VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 3;

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    pipe.cb_ci_.pAttachments = nullptr;
    pipe.gp_ci_.pDynamicState = &dynamic_create_info;
    pipe.CreateGraphicsPipeline(true, false);
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, DynamicPrimitiveTopolgy) {
    TEST_DESCRIPTION("VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY works when GPL is used");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));
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
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
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
    render_pass = pre_raster_lib.gp_ci_.renderPass;
    subpass = pre_raster_lib.gp_ci_.subpass;

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
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
        frag_shader_lib.gp_ci_.pInputAssemblyState = &ia_state;
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.gp_ci_.renderPass = render_pass;
    frag_out_lib.gp_ci_.subpass = subpass;
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

TEST_F(VkPositiveGraphicsLibraryLayerTest, LinkingInputAttachment) {
    TEST_DESCRIPTION("Make sure OpCapability InputAttachment is not detected at linking time");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineLayout layout = VK_NULL_HANDLE;

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.InitState();
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
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
        // bindStateFragShaderText with manually added OpCapability
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
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}