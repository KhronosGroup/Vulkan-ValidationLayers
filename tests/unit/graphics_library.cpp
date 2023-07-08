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

TEST_F(NegativeGraphicsLibrary, DSLs) {
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, GPLDSLs) {
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

TEST_F(NegativeGraphicsLibrary, IndependentSetsLinkOnly) {
    TEST_DESCRIPTION("Link pre-raster and FS subsets with invalid VkPipelineLayout create flags");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
        pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
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

TEST_F(NegativeGraphicsLibrary, IndependentSetsLinkCreate) {
    TEST_DESCRIPTION("Create pre-raster subset while linking FS subset with invalid VkPipelineLayout create flags");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
        pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06614");
        frag_shader_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, DescriptorSets) {
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
        VK_NULL_HANDLE,  // Triggers 06754
    };

    VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_, &ds2.layout_});

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-graphicsPipelineLibrary-06754");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, DescriptorSetsGPL) {
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

TEST_F(NegativeGraphicsLibrary, MissingDSState) {
    TEST_DESCRIPTION("Create a library with fragment shader state, but no fragment output state, and invalid DS state");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    frag_shader_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    frag_shader_lib.gp_ci_.pDepthStencilState = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06590");
    frag_shader_lib.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, MissingDSStateWithFragOutputState) {
    TEST_DESCRIPTION("Create a library with both fragment shader state and fragment output state, and invalid DS state");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfo>();

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

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

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci, &pipeline_rendering_info);
        pre_raster_lib.InitState();

        pre_raster_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        pre_raster_lib.gp_ci_.pDepthStencilState = nullptr;
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;

        // Should be fine even though pDepthStencilState is NULL
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    // Create a fragment output pipeline first. It's because we can't create a valid fragment shader pipeline
    // without pDepthStencilState since it hits by "VUID-VkGraphicsPipelineCreateInfo-renderPass-06590"
    CreatePipelineHelper frag_output_lib(*this);
    {
        frag_output_lib.InitFragmentOutputLibInfo();
        frag_output_lib.InitState();

        frag_output_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        frag_output_lib.gp_ci_.pDepthStencilState = nullptr;

        // Should be fine even though pDepthStencilState is NULL
        ASSERT_VK_SUCCESS(frag_output_lib.CreateGraphicsPipeline());
    }

    VkPipeline libraries[2] = {
        pre_raster_lib.pipeline_,
        frag_output_lib.pipeline_,
    };

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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.pNext = &pipeline_rendering_info;
        link_info.libraryCount = 2;
        link_info.pLibraries = libraries;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();

        frag_shader_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
        frag_shader_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        frag_shader_lib.gp_ci_.pDepthStencilState = nullptr;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06053");
        frag_shader_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, MissingColorBlendState) {
    TEST_DESCRIPTION("Create a library with fragment output state and invalid ColorBlendState state");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    InitBasicGraphicsLibrary(&dynamic_rendering_features);
    if (::testing::Test::IsSkipped()) return;

    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfo>();
    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();

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

        pre_raster_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06054");
        frag_output_lib.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, ImplicitVUIDs) {
    TEST_DESCRIPTION("Test various VUIDs that were previously implicit, but now explicit due to VK_EXT_graphics_pipeline_library");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.LateBindPipelineInfo();

    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-None-07826");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.layout = pipe.pipeline_layout_.handle();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06575");
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

TEST_F(NegativeGraphicsLibrary, CreateStateGPL) {
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
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.InitState();
        pipe.gp_ci_.layout = VK_NULL_HANDLE;
        pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
        pipe.gp_ci_.pStages = pipe.shader_stages_.data();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-None-07826");
        pipe.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    {
        // Test creating a pipeline with incomplete state, but feature is not enabled
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06846");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06607");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, LinkOptimization) {
    TEST_DESCRIPTION("Create graphics pipeline libraries with mismatching link-time optimization flags");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

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

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
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

TEST_F(NegativeGraphicsLibrary, DSLShaderBindingsNullInCreate) {
    TEST_DESCRIPTION("Link pre-raster state while creating FS state with invalid null DSL + shader stage bindings");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();

        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06756");
        frag_shader_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, DSLShaderBindingsNullInLink) {
    TEST_DESCRIPTION("Link pre-raster state with invalid null DSL + shader stage bindings while creating FS state");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &pre_raster_lib.pipeline_;

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci, &link_info);
        frag_shader_lib.InitState();
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06757");
        frag_shader_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, DSLShaderBindingsLinkOnly) {
    TEST_DESCRIPTION("Link pre-raster and FS subsets with invalid null DSL + shader stage bindings");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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

TEST_F(NegativeGraphicsLibrary, PreRasterStateNoLayout) {
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

    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06642");
    pipe.CreateGraphicsPipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, ImmutableSamplersIncompatibleDSL) {
    TEST_DESCRIPTION("Link pipelines with DSLs that only differ by immutable samplers");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_null.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, PreRasterWithFS) {
    TEST_DESCRIPTION("Create a library with no FS state, but an FS");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    // Create and add a vertex shader to silence 06896
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    auto vs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    vs_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs_stage_ci.module = VK_NULL_HANDLE;
    vs_stage_ci.pName = "main";
    stages.emplace_back(vs_stage_ci);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
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

TEST_F(NegativeGraphicsLibrary, FragmentStateWithPreRaster) {
    TEST_DESCRIPTION("Create a library with no pre-raster state, but that contains a pre-raster shader.");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    auto fs_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    fs_ci.codeSize = fs_spv.size() * sizeof(decltype(fs_spv)::value_type);
    fs_ci.pCode = fs_spv.data();

    auto fs_stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&fs_ci);
    fs_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs_stage_ci.module = VK_NULL_HANDLE;
    fs_stage_ci.pName = "main";
    stages.emplace_back(fs_stage_ci);

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
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

TEST_F(NegativeGraphicsLibrary, MissingShaderStages) {
    TEST_DESCRIPTION("Create a library with pre-raster state, but no pre-raster shader");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitPreRasterLibInfo(0, nullptr);
        pipe.InitState();

        // 02096 is effectively unrelated, but gets triggered due to lack of mesh shader extension
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-stage-02096");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-06896");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, DescriptorBufferLibrary) {
    TEST_DESCRIPTION("Descriptor buffer and graphics library");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    auto db_features = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>();
    InitBasicGraphicsLibrary(&db_features);
    if (::testing::Test::IsSkipped()) return;

    if (!db_features.descriptorBuffer) {
        GTEST_SKIP() << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBuffer not supported";
    }

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
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
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
        // Layout, renderPass, and subpass all need to be shared across libraries in the same executable pipeline
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
    exe_pipe_ci.layout = layout;
    VkPipeline pipeline;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096");
    vk::CreateGraphicsPipelines(m_device->handle(), VK_NULL_HANDLE, 1, &exe_pipe_ci, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, DSLShaderStageMask) {
    TEST_DESCRIPTION(
        "Attempt to bind invalid descriptor sets with and without VK_EXT_graphics_pipeline_library and independent sets");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07988");
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07988");
        fs_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, Tessellation) {
    TEST_DESCRIPTION("Test various errors when creating a graphics pipeline with tessellation stages active.");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vk_testing::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
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

    {
        // Pass patch topology without tessellation shaders
        CreatePipelineHelper vi_patch_lib(*this);
        vi_patch_lib.InitVertexInputLibInfo();
        vi_patch_lib.InitState();
        vi_patch_lib.gp_ci_.pInputAssemblyState = &iasci;
        vi_patch_lib.CreateGraphicsPipeline(true, false);

        CreatePipelineHelper pre_raster_lib(*this);
        pre_raster_lib.InitPreRasterLibInfo(1, &vs_stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = fs_lib.gp_ci_.layout;
        pre_raster_lib.CreateGraphicsPipeline();

        libs[0] = vi_patch_lib.pipeline_;
        libs[1] = pre_raster_lib.pipeline_;
        auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
        exe_pipe_ci.layout = fs_lib.gp_ci_.layout;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08889");
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
        exe_pipe_ci.layout = fs_lib.gp_ci_.layout;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-08888");
        vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, PipelineExecutableProperties) {
    TEST_DESCRIPTION("VK_KHR_pipeline_executable_properties with GPL");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    auto executable_features = LvlInitStruct<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>();
    InitBasicGraphicsLibrary(&executable_features);
    if (::testing::Test::IsSkipped()) return;

    if (!executable_features.pipelineExecutableInfo) {
        GTEST_SKIP() << "pipelineExecutableInfo not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        CreatePipelineHelper vertex_input_lib(*this);
        vertex_input_lib.InitVertexInputLibInfo();
        vertex_input_lib.InitState();
        vertex_input_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
        ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

        CreatePipelineHelper frag_out_lib(*this);
        frag_out_lib.InitFragmentOutputLibInfo();
        ASSERT_VK_SUCCESS(frag_out_lib.CreateGraphicsPipeline(true, false));

        VkPipeline libraries[2] = {
            vertex_input_lib.pipeline_,
            frag_out_lib.pipeline_,
        };
        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = size(libraries);
        link_info.pLibraries = libraries;

        auto exe_pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06646");
        vk_testing::Pipeline exe_pipe(*m_device, exe_pipe_ci);
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper vertex_input_lib(*this);
        vertex_input_lib.InitVertexInputLibInfo();
        vertex_input_lib.InitState();
        ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &vertex_input_lib.pipeline_;

        CreatePipelineHelper frag_out_lib(*this);
        frag_out_lib.InitFragmentOutputLibInfo(&link_info);
        frag_out_lib.gp_ci_.flags =
            VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06645");
        frag_out_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper vertex_input_lib(*this);
        vertex_input_lib.InitVertexInputLibInfo();
        vertex_input_lib.InitState();
        vertex_input_lib.gp_ci_.flags =
            VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
        ASSERT_VK_SUCCESS(vertex_input_lib.CreateGraphicsPipeline(true, false));

        auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
        link_info.libraryCount = 1;
        link_info.pLibraries = &vertex_input_lib.pipeline_;

        CreatePipelineHelper frag_out_lib(*this);
        frag_out_lib.InitFragmentOutputLibInfo(&link_info);
        frag_out_lib.gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06647");
        frag_out_lib.CreateGraphicsPipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGraphicsLibrary, BindEmptyDS) {
    TEST_DESCRIPTION("Bind an empty descriptor set");

    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
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

TEST_F(NegativeGraphicsLibrary, BindLibraryPipeline) {
    TEST_DESCRIPTION("Test binding a pipeline that was created with library flag");

    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;

    // Required for graphics pipeline libraries
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipeline(*this);
    pipeline.InitVertexInputLibInfo();
    pipeline.InitState();
    ASSERT_VK_SUCCESS(pipeline.CreateGraphicsPipeline(true, false));
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-03382");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeGraphicsLibrary, ShaderModuleIdentifier) {
    TEST_DESCRIPTION("Test for VK_EXT_shader_module_identifier extension.");
    TEST_DESCRIPTION("Create a pipeline using a shader module identifier");

    SetTargetApiVersion(VK_API_VERSION_1_3);  // Pipeline cache control needed
    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    auto pipeline_cache_control_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeatures>();
    auto shader_module_id_features =
        LvlInitStruct<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(&pipeline_cache_control_features);
    InitBasicGraphicsLibrary(&shader_module_id_features);
    if (::testing::Test::IsSkipped()) return;

    if (!pipeline_cache_control_features.pipelineCreationCacheControl) {
        GTEST_SKIP() << "pipelineCreationCacheControl not supported";
    } else if (!shader_module_id_features.shaderModuleIdentifier) {
        GTEST_SKIP() << "shaderModuleIdentifier not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto sm_id_create_info = LvlInitStruct<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>();
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    auto vs_ci = LvlInitStruct<VkShaderModuleCreateInfo>(&sm_id_create_info);
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&vs_ci);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe.InitState();

    // Both a shader module ci and shader module id ci in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06844");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    stage_ci.pNext = nullptr;
    // No shader module ci and no shader module id ci in pNext and invalid module
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06845");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // shader module id ci and module not VK_NULL_HANDLE
    stage_ci.pNext = &sm_id_create_info;
    stage_ci.module = vs.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06848");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    auto get_identifier = LvlInitStruct<VkShaderModuleIdentifierEXT>();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    sm_id_create_info.pIdentifier = get_identifier.identifier;
    stage_ci.module = VK_NULL_HANDLE;
    pipe.gp_ci_.flags = 0;
    // shader module id ci and no VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06851");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    sm_id_create_info.identifierSize = VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT + 1;
    // indentifierSize too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-identifierSize-06852");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    // Trying to create a pipeline with a shader module identifier at this point can result in a VK_PIPELINE_COMPILE_REQUIRED from
    // some drivers, and no pipeline creation. Create a pipeline using the module itself presumably getting the driver to compile
    // the shader so we can create a pipeline using the identifier
    pipe.gp_ci_.flags = 0;
    stage_ci.pNext = nullptr;
    stage_ci.module = vs.handle();
    pipe.CreateGraphicsPipeline();

    // Now really create a pipeline with a smid
    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.stageCount = 1;
    pipe2.gp_ci_.pStages = &stage_ci;
    pipe2.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe2.InitState();
    stage_ci.pNext = &sm_id_create_info;
    stage_ci.module = VK_NULL_HANDLE;
    VkResult result = pipe2.CreateGraphicsPipeline();
    if (result != VK_SUCCESS) {
        printf("Cannot create a pipeline with a shader module identifier, skipping gpl part of the test\n");
        return;
    }
    // Now use it in a gpl
    VkPipeline libraries[1] = {pipe2.pipeline_};
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto pipe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    // no VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-06855");
    vk_testing::Pipeline exe_pipe(*m_device, pipe_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, ShaderModuleIdentifierFeatures) {
    TEST_DESCRIPTION("Test for VK_EXT_shader_module_identifier extension with missing features.");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto pipeline_cache_control_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_cache_control_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-06846");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    auto sm_id_create_info = LvlInitStruct<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>();
    sm_id_create_info.identifierSize = 4;
    stage_ci.pNext = &sm_id_create_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06850");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    auto get_identifier = LvlInitStruct<VkShaderModuleIdentifierEXT>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetShaderModuleIdentifierEXT-shaderModuleIdentifier-06884");
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    m_errorMonitor->VerifyFound();

    auto sm_ci = LvlInitStruct<VkShaderModuleCreateInfo>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetShaderModuleCreateInfoIdentifierEXT-shaderModuleIdentifier-06885");
    uint32_t code = 0;
    sm_ci.codeSize = 4;
    sm_ci.pCode = &code;
    vk::GetShaderModuleCreateInfoIdentifierEXT(device(), &sm_ci, &get_identifier);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, Layouts) {
    TEST_DESCRIPTION("Various invalid layouts");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    ASSERT_VK_SUCCESS(vi_lib.CreateGraphicsPipeline());

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    ASSERT_VK_SUCCESS(fo_lib.CreateGraphicsPipeline());

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage stage_ci(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

        pre_raster_lib.InitPreRasterLibInfo(1, &stage_ci.stage_ci);
        pre_raster_lib.InitState();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline());
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage stage_ci(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        frag_shader_lib.InitFragmentLibInfo(1, &stage_ci.stage_ci);
        frag_shader_lib.InitState();
        ASSERT_VK_SUCCESS(frag_shader_lib.CreateGraphicsPipeline());
    }

    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        fo_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto lib_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-None-07826");
    vk_testing::Pipeline lib(*m_device, lib_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGraphicsLibrary, IncompatibleLayouts) {
    TEST_DESCRIPTION("Link pre-raster state while creating FS state with invalid null DSL + shader stage bindings");
    InitBasicGraphicsLibrary();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                     });
    OneOffDescriptorSet ds2(m_device, {
                                          {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                      });

    // pipeline_layout_lib is used for library creation, pipeline_layout_exe is used at link time for the executable pipeline, and
    // these layouts are incompatible
    VkPipelineLayoutObj pipeline_layout_lib(m_device, {&ds.layout_}, {});
    VkPipelineLayoutObj pipeline_layout_exe(m_device, {&ds2.layout_}, {});

    CreatePipelineHelper vi_lib(*this);
    vi_lib.InitVertexInputLibInfo();
    vi_lib.InitState();
    ASSERT_VK_SUCCESS(vi_lib.CreateGraphicsPipeline());

    CreatePipelineHelper fo_lib(*this);
    fo_lib.InitFragmentOutputLibInfo();
    fo_lib.InitState();
    ASSERT_VK_SUCCESS(fo_lib.CreateGraphicsPipeline());

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

        pre_raster_lib.InitPreRasterLibInfo(1, &stage.stage_ci);
        pre_raster_lib.InitState();
        pre_raster_lib.gp_ci_.layout = pipeline_layout_lib.handle();
        ASSERT_VK_SUCCESS(pre_raster_lib.CreateGraphicsPipeline(true, false));
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vk_testing::GraphicsPipelineLibraryStage stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

        frag_shader_lib.InitFragmentLibInfo(1, &stage.stage_ci);
        frag_shader_lib.InitState();
        frag_shader_lib.gp_ci_.layout = pipeline_layout_lib.handle();
        frag_shader_lib.CreateGraphicsPipeline(true, false);
    }

    VkPipeline libraries[4] = {
        vi_lib.pipeline_,
        pre_raster_lib.pipeline_,
        frag_shader_lib.pipeline_,
        fo_lib.pipeline_,
    };
    auto link_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    auto exe_ci = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&link_info);
    exe_ci.layout = pipeline_layout_exe.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkGraphicsPipelineCreateInfo-layout-07827");  // incompatible with pre-raster state
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07827");  // incompatible with fragment shader state
    vk_testing::Pipeline exe_pipe(*m_device, exe_ci);
    m_errorMonitor->VerifyFound();
}
