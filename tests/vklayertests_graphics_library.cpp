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
    m_errorMonitor->ExpectSuccess();

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
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Vulkan >= 1.2 required", kSkipPrefix);
        return;
    }

    std::vector<const char *> failed_exts;
    if (!AreRequestedExtensionsEnabled(failed_exts)) {
        printf("%s The following device extensions are not supported: ", kSkipPrefix);
        for (const auto &ext : failed_exts) {
            printf("%s ", ext);
        }
        printf("\n");
        return;
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
    m_errorMonitor->VerifyNotFound();

    std::vector<const vk_testing::DescriptorSetLayout *> dsls = {&dsl, nullptr};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-flags-06562");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidIndependentSetsLinkOnly) {
    TEST_DESCRIPTION("Link pre-raster and FS subsets with invalid VkPipelineLayout create flags");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    std::vector<const char *> failed_exts;
    if (!AreRequestedExtensionsEnabled(failed_exts)) {
        printf("%s The following device extensions are not supported: ", kSkipPrefix);
        for (const auto &ext : failed_exts) {
            printf("%s ", ext);
        }
        printf("\n");
        return;
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
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    std::vector<const char *> failed_exts;
    if (!AreRequestedExtensionsEnabled(failed_exts)) {
        printf("%s The following device extensions are not supported: ", kSkipPrefix);
        for (const auto &ext : failed_exts) {
            printf("%s ", ext);
        }
        printf("\n");
        return;
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
        m_errorMonitor->VerifyNotFound();
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
    m_errorMonitor->ExpectSuccess();

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
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06563");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, InvalidDescriptorSetsGPL) {
    TEST_DESCRIPTION("Attempt to bind invalid descriptor sets with and with VK_EXT_graphics_pipeline_library");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Vulkan >= 1.2 required", kSkipPrefix);
        return;
    }

    std::vector<const char *> failed_exts;
    if (!AreRequestedExtensionsEnabled(failed_exts)) {
        printf("%s The following device extensions are not supported: ", kSkipPrefix);
        for (const auto &ext : failed_exts) {
            printf("%s ", ext);
        }
        printf("\n");
        return;
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
    VkPipelineLayoutObj pipeline_layout_is(m_device, {&ds.layout_, &ds2.layout_}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    m_commandBuffer->begin();
    // First bind using a layout created with independent sets, which should not trigger any error
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_is.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyNotFound();

    // Now bind with a layout that was _not_ created with independent sets, which should trigger 06564
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06564");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}
