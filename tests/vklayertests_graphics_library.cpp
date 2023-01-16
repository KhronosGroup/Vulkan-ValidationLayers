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
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

class VkGraphicsLibraryLayerTest : public VkLayerTest {};

TEST_F(VkGraphicsLibraryLayerTest, InvalidDSLs) {
    TEST_DESCRIPTION("Create a pipeline layout with invalid descriptor set layouts");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    auto dsl_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dsl_ci);

    std::vector<const vk_testing::DescriptorSetLayout*> dsls = {&dsl, nullptr};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-06561");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, GPLInvalidDSLs) {
    TEST_DESCRIPTION("Create a pipeline layout with invalid descriptor set layouts with VK_EXT_grahpics_pipeline_library enabled");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    auto dsl_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dsl_ci);

    std::vector<const vk_testing::DescriptorSetLayout *> dsls = {&dsl, nullptr};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidIndependentSetsLinkOnly) {
    TEST_DESCRIPTION("Link pre-raster and FS subsets with invalid VkPipelineLayout create flags");

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

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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
        pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
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
        // frag_shader_lib's layout will not be created with VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT, which will trigger
        // the desired error
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline());
    }

    VkPipeline libraries[2] = {
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06615");
    auto lib_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    vk_testing::Pipeline lib(*m_device, lib_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidIndependentSetsLinkCreate) {
    TEST_DESCRIPTION("Create pre-raster subset while linking FS subset with invalid VkPipelineLayout create flags");

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

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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
        pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();
        // frag_shader_lib's layout will not be created with VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT, which will trigger
        // the desired error
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06614");
        frag_shader_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDescriptorSets) {
    TEST_DESCRIPTION(
        "Attempt to bind invalid descriptor sets with and without VK_EXT_graphics_pipeline_library and independent sets");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                      });
    std::array<VkDescriptorSet, 2> sets = {
        ds.set_,
        VK_NULL_HANDLE,  // Triggers 06563
    };

    VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_, &ds2.layout_});

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06563");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDescriptorSetsGPL) {
    TEST_DESCRIPTION("Attempt to bind invalid descriptor sets with and with VK_EXT_graphics_pipeline_library");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                      });
    std::array<VkDescriptorSet, 2> sets = {
        ds.set_,
        VK_NULL_HANDLE,
    };

    VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_, &ds2.layout_});

    m_commandBuffer->begin();

    // Now bind with a layout that was _not_ created with independent sets, which should trigger 06754
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-graphicsPipelineLibrary-06754");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, MissingDSState) {
    TEST_DESCRIPTION("Create a library with fragment shader state, but no fragment output state, and invalid DS state");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    frag_shader_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    frag_shader_lib.gp_ci_.pDepthStencilState = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06590");
    frag_shader_lib.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, ImplicitVUIDs) {
    TEST_DESCRIPTION("Test various VUIDs that were previously implicit, but now explicit due to VK_EXT_graphics_pipeline_library");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.LateBindPipelineInfo();

    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-06602");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.layout = pipe.pipeline_layout_.handle();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06574");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.renderPass = renderPass();
    pipe.gp_ci_.stageCount = 0;
    pipe.gp_ci_.pStages = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06600");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stageCount-06604");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidCreateStateGPL) {
    TEST_DESCRIPTION("Create invalid graphics pipeline state with VK_EXT_graphics_pipeline_library enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Do _not_ enable VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        // Test creating a pipeline with incorrect create flags
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.InitState();
        pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06608");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        // Test creating a pipeline with incomplete state, but feature is not enabled
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06846");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06607");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidLinkOptimization) {
    TEST_DESCRIPTION("Create graphics pipeline libraries with mismatching link-time optimization flags");

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
    // Ensure this library is created _without_ VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT
    vertex_input_lib.gp_ci_.flags &= ~VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

    VkPipeline libraries[1] = {
        vertex_input_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();

        // Creating with VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT while linking against a library without
        // VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT is invalid
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT;
        pre_raster_lib.gpl_info->pNext = &link_info;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06609");
        pre_raster_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci);
        pre_raster_lib.InitState();

        // Creating with VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT while linking against a library without
        // VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT is invalid
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
        pre_raster_lib.gpl_info->pNext = &link_info;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06610");
        pre_raster_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDSLShaderBindingsNullInCreate) {
    TEST_DESCRIPTION("Link pre-raster state while creating FS state with invalid null DSL + shader stage bindings");

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
    OneOffDescriptorSet ds2(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    VkPipelineLayoutObj pipeline_layout_vs(m_device, {&ds.layout_, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_fs(m_device, {&ds.layout_, nullptr}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();

        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06756");
        frag_shader_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDSLShaderBindingsNullInLink) {
    TEST_DESCRIPTION("Link pre-raster state with invalid null DSL + shader stage bindings while creating FS state");

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
    OneOffDescriptorSet ds2(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    VkPipelineLayoutObj pipeline_layout_vs(m_device, {&ds.layout_, nullptr}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_fs(m_device, {&ds.layout_, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();

        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06757");
        frag_shader_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDSLShaderBindingsLinkOnly) {
    TEST_DESCRIPTION("Link pre-raster and FS subsets with invalid null DSL + shader stage bindings");

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
    OneOffDescriptorSet ds2(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    VkPipelineLayoutObj pipeline_layout_vs(m_device, {&ds.layout_, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_fs(m_device, {&ds.layout_, nullptr}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline(true, false));
    }

    VkPipeline libraries[2] = {
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto lib_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06758");
    vk_testing::Pipeline lib(*m_device, lib_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, CreateGraphicsPipelineWithMissingMultisampleState) {
    TEST_DESCRIPTION("Create pipeline with fragment shader that uses samples, but multisample state not begin set");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-06630");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, PreRasterStateNoLayout) {
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

    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06642");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, ImmutableSamplersIncompatibleDSL) {
    TEST_DESCRIPTION("Link pipelines with DSLs that only differ by immutable samplers");

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

    auto sampler_info = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler(*m_device, sampler_info);
    ASSERT_TRUE(sampler.initialized());
    const auto sampler_handle = sampler.handle();

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      });
    OneOffDescriptorSet ds_immutable_sampler(m_device,
                                             {
                                                 {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler_handle},
                                             });

    // We _vs and _fs layouts are identical, but we want them to be separate handles handles for the sake layout merging
    VkPipelineLayoutObj pipeline_layout_vs(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_fs(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    VkPipelineLayoutObj pipeline_layout_null(m_device, {&ds.layout_, nullptr, &ds_immutable_sampler.layout_}, {},
                                             VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    const std::array<VkDescriptorSet, 3> desc_sets = {ds.set_, VK_NULL_HANDLE, ds2.set_};

    auto ub_ci = LvlInitStruct<VkBufferCreateInfo>();
    ub_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ub_ci.size = 1024;
    VkBufferObj uniform_buffer(*m_device, ub_ci);
    ds.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds.UpdateDescriptorSets();

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_null.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, PreRasterWithFS) {
    TEST_DESCRIPTION("Create a library with no FS state, but an FS");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    // Create and add a vertex shader to silence 06896
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    auto vs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    vs_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs_stage_ci.module = VK_NULL_HANDLE;
    vs_stage_ci.pName = "main";
    stages.emplace_back(vs_stage_ci);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto fs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    // The library is not created with fragment shader state, and therefore cannot have a fragment shader
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";
    stages.emplace_back(fs_stage_ci);

    CreatePipelineHelper pipe(*this);
    pipe.InitPreRasterLibInfo(static_cast<uint32_t>(stages.size()), stages.data());
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06894");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, FragmentStateWithPreRaster) {
    TEST_DESCRIPTION("Create a library with no pre-raster state, but that contains a pre-raster shader.");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto fs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";
    stages.emplace_back(fs_stage_ci);

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    auto vs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    // VK_SHADER_STAGE_VERTEX_BIT is a pre-raster shader stage, but the library will be created with only fragment shader state
    vs_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs_stage_ci.module = VK_NULL_HANDLE;
    vs_stage_ci.pName = "main";
    stages.emplace_back(vs_stage_ci);

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentLibInfo(static_cast<uint32_t>(stages.size()), stages.data());
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06895");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, MissingShaderStages) {
    TEST_DESCRIPTION("Create a library with pre-raster state, but no pre-raster shader");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitPreRasterLibInfo(0, nullptr);
        pipe.InitState();

        // 00727 is effectively unrelated, but gets triggered due to lack of mesh shader extension
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-00727");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06896");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DescriptorBufferLibrary) {
    TEST_DESCRIPTION("Descriptor buffer and graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto db_features = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(&gpl_features);
    auto features2 = GetPhysicalDeviceFeatures2(db_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
        return;
    }
    if (!db_features.descriptorBuffer) {
        GTEST_SKIP() << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBuffer not supported";
        return;
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
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
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
        // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
        frag_shader_lib.gp_ci_.layout = layout;
        frag_shader_lib.gp_ci_.renderPass = render_pass;
        frag_shader_lib.gp_ci_.subpass = subpass;
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline(true, false));
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
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
    exe_pipe_ci.layout = layout;
    VkPipeline pipeline;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &exe_pipe_ci, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, BadDSLShaderStageMask) {
    TEST_DESCRIPTION(
        "Attempt to bind invalid descriptor sets with and without VK_EXT_graphics_pipeline_library and independent sets");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char vs_src[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } bar;
        void main() {
            gl_Position = vec4(bar.x);
        }
    )glsl";

    const char fs_src[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } bar;
        layout(location = 0) out vec4 c;
        void main() {
            c = vec4(bar.x);
        }
    )glsl";

    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    ASSERT_VK_SUCCESS(vi_lib.CreateGraphicsPipeline(true, false));

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    ASSERT_VK_SUCCESS(fo_lib.CreateGraphicsPipeline(true, false));

    // Check pre-raster library with shader accessing FS-only descriptor
    {
        OneOffDescriptorSet fs_ds(m_device, {
                                                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                            });

        VkPipelineLayoutObj pipeline_layout(m_device, {&fs_ds.layout_});
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src);
        vk_testing::GraphicsPipelineLibraryStage stage(vs_spv);
        CreatePipelineHelper vs_lib(*this);
        vs_lib.InitPreRasterLibInfo(1, &stage.stage_ci);
        vs_lib.InitState();
        vs_lib.gp_ci_.layout = pipeline_layout.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
        vs_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_src);
        vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        CreatePipelineHelper fs_lib(*this);
        fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.layout = pipeline_layout.handle();
        ASSERT_VK_SUCCESS(fs_lib.CreateGraphicsPipeline(true, false));
    }

    // Check FS library with shader accessing FS-only descriptor
    {
        OneOffDescriptorSet vs_ds(m_device, {
                                                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });

        VkPipelineLayoutObj pipeline_layout(m_device, {&vs_ds.layout_});
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src);
        vk_testing::GraphicsPipelineLibraryStage stage(vs_spv);
        CreatePipelineHelper vs_lib(*this);
        vs_lib.InitPreRasterLibInfo(1, &stage.stage_ci);
        vs_lib.InitState();
        vs_lib.gp_ci_.layout = pipeline_layout.handle();
        ASSERT_VK_SUCCESS(vs_lib.CreateGraphicsPipeline(true, false));

        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_src);
        vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        CreatePipelineHelper fs_lib(*this);
        fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci);
        fs_lib.InitState();
        fs_lib.gp_ci_.layout = pipeline_layout.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
        fs_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, CreatePipelineTessErrors) {
    TEST_DESCRIPTION("Test various errors when creating a graphics pipeline with tessellation stages active.");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(gpl_features);

    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &gpl_features));

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateVertShaderText);
    vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, bindStateFragShaderText);
    vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    char const *tcs_src = R"glsl(
        #version 450
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
        }
    )glsl";
    const auto tcs_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tcs_src);
    vk_testing::GraphicsPipelineLibraryStage tcs_stage(tcs_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);

    char const *tes_src = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = 0;
        }
    )glsl";
    const auto tes_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tes_src);
    vk_testing::GraphicsPipelineLibraryStage tes_stage(tes_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    auto iasci = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    iasci.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    VkPipelineInputAssemblyStateCreateInfo iasci_bad = iasci;
    VkPipelineTessellationStateCreateInfo tsci_bad = tsci;

    CreatePipelineHelper vi_bad_lib(*this);
    vi_bad_lib.InitVertexInputLibInfo();
    vi_bad_lib.InitState();
    iasci_bad.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // otherwise we get a failure about invalid topology
    vi_bad_lib.gp_ci_.pInputAssemblyState = &iasci_bad;
    vi_bad_lib.CreateGraphicsPipeline(true, false);

    CreatePipelineHelper fs_lib(*this);
    fs_lib.InitFragmentLibInfo(1, &fs_stage.stage_ci);
    fs_lib.InitState();
    fs_lib.CreateGraphicsPipeline();

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    fo_lib.CreateGraphicsPipeline(true, false);

    // libs[0] == vertex input lib
    // libs[1] == pre-raster lib
    // libs[2] == FS lib
    // libs[3] == FO lib
    std::array libs = {
        vi_bad_lib.pipeline_,
        static_cast<VkPipeline>(VK_NULL_HANDLE),  // Filled out for each VUID check below
        fs_lib.pipeline_,
        fo_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = static_cast<uint32_t>(libs.size());
    link_info.pLibraries = libs.data();

    // Pass a tess control shader without a tess eval shader
    {
        CreatePipelineHelper pre_raster_lib(*this);
        const std::array shaders = {vs_stage.stage_ci, tcs_stage.stage_ci};
        pre_raster_lib.InitPreRasterLibInfoFromContainer(shaders);
        pre_raster_lib.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00729");
        pre_raster_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Pass a tess eval shader without a tess control shader
    {
        CreatePipelineHelper pre_raster_lib(*this);
        const std::array shaders = {vs_stage.stage_ci, tes_stage.stage_ci};
        pre_raster_lib.InitPreRasterLibInfoFromContainer(shaders);
        pre_raster_lib.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00730");
        pre_raster_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Pass patch topology without tessellation shaders
    CreatePipelineHelper vi_patch_lib(*this);
    vi_patch_lib.InitVertexInputLibInfo();
    vi_patch_lib.InitState();
    vi_patch_lib.gp_ci_.pInputAssemblyState = &iasci;
    vi_patch_lib.CreateGraphicsPipeline(true, false);
    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        pre_raster_lib.CreateGraphicsPipeline();

        libs[0] = vi_patch_lib.pipeline_;
        libs[1] = pre_raster_lib.pipeline_;
        auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-00737");
        vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        m_errorMonitor->VerifyFound();
    }

    // Pass a NULL pTessellationState (with active tessellation shader stages)
    const std::array tess_shaders = {vs_stage.stage_ci, tcs_stage.stage_ci, tes_stage.stage_ci};
    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfoFromContainer(tess_shaders);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00731");
        pre_raster_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Pass an invalid pTessellationState (bad sType)
    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfoFromContainer(tess_shaders);
        pre_raster_lib.InitState();
        // stype-check off
        tsci_bad.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // stype-check on
        pre_raster_lib.gp_ci_.pTessellationState = &tsci_bad;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");
        pre_raster_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    // Pass out-of-range patchControlPoints
    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfoFromContainer(tess_shaders);
        pre_raster_lib.InitState();
        tsci_bad = tsci;
        tsci_bad.patchControlPoints = 0;
        pre_raster_lib.gp_ci_.pTessellationState = &tsci_bad;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");
        pre_raster_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfoFromContainer(tess_shaders);
        pre_raster_lib.InitState();
        tsci_bad = tsci;
        tsci_bad.patchControlPoints = m_device->props.limits.maxTessellationPatchSize + 1;
        pre_raster_lib.gp_ci_.pTessellationState = &tsci_bad;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");
        pre_raster_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    // Pass an invalid primitive topology
    {
        CreatePipelineHelper vi_lib(*this);
        vi_lib.InitVertexInputLibInfo();
        vi_lib.InitState();
        iasci_bad = iasci;
        iasci_bad.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        vi_lib.gp_ci_.pInputAssemblyState = &iasci_bad;
        vi_lib.CreateGraphicsPipeline(true, false);

        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfoFromContainer(tess_shaders);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.pTessellationState = &tsci;
        pre_raster_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        pre_raster_lib.CreateGraphicsPipeline(true, false);

        libs[0] = vi_lib.pipeline_;
        libs[1] = pre_raster_lib.pipeline_;
        auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00736");
        vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkGraphicsLibraryLayerTest, BindEmptyDS) {
    TEST_DESCRIPTION("Bind an empty descriptor set");

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
    OneOffDescriptorSet ds1(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      });
    OneOffDescriptorSet ds_empty(m_device, {});  // empty set

    // vs and fs "do not use" set 1, so set it to null
    VkPipelineLayoutObj pipeline_layout_lib(m_device, {&ds.layout_, nullptr, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    // The final, linked layout will define something for set 1
    VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_, &ds1.layout_, &ds2.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    const std::array<VkDescriptorSet, 3> desc_sets_null = {ds.set_, VK_NULL_HANDLE, ds2.set_};
    const std::array<VkDescriptorSet, 3> desc_sets_empty = {ds.set_, ds_empty.set_, ds2.set_};

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
        pre_raster_lib.gp_ci_.layout = pipeline_layout_lib.handle();
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
        frag_shader_lib.gp_ci_.layout = pipeline_layout_lib.handle();
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.InitState();
    ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

    std::array libraries = {
        vertex_input_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        frag_out_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = libraries.size();
    link_info.pLibraries = libraries.data();

    auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_pipe_ci.layout = pipeline_layout.handle();
    vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Using an "empty" descriptor set is not legal
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets_empty.size()), desc_sets_empty.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Using a VK_NULL_HANDLE descriptor set _is_ legal
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets_null.size()), desc_sets_null.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
