/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

class VkPositiveGraphicsLibraryLayerTest : public VkLayerTest {};

TEST_F(VkPositiveGraphicsLibraryLayerTest, VertexInputGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a vertex input graphics library");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        printf("%s VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    CreatePipelineHelper pipe(*this);
    pipe.InitVertexInputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, PreRasterGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a pre-raster graphics library");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        printf("%s VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported", kSkipPrefix);
        return;
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

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, FragmentShaderGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a fragment shader graphics library");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        printf("%s VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, bindStateFragShaderText);
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

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, FragmentOutputGraphicsPipelineLibrary) {
    TEST_DESCRIPTION("Create a fragment output graphics library");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        printf("%s VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitFragmentOutputLibInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline(true, false));

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveGraphicsLibraryLayerTest, ExeLibrary) {
    TEST_DESCRIPTION("Create an executable library by linking one or more graphics libraries");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&gpl_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!gpl_features.graphicsPipelineLibrary) {
        printf("%s VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported", kSkipPrefix);
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
        {
            std::ofstream out("/tmp/frag.spv", std::ios::binary | std::ios::out);
            out.write(reinterpret_cast<const char *>(fs_spv.data()), fs_spv.size() * sizeof(decltype(fs_spv)::value_type));
        }

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

    m_errorMonitor->VerifyNotFound();
}
