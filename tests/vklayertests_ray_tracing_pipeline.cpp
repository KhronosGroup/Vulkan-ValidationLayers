/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "layer_validation_tests.h"

TEST_F(VkLayerTest, RayTracingPipelineCreateInfoKHR) {
    TEST_DESCRIPTION("Validate CreateInfo parameters during ray-tracing pipeline creation");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_KHR);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_KHR);
    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 1;  // Bad index here
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;
    VkPipelineLibraryCreateInfoKHR library_count_zero = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};
    VkPipelineLibraryCreateInfoKHR library_count_one = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 1};
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03600");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.stageCount = 1;
        pipeline_ci.groupCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03601");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.groupCount = 1;
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_one;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetUnexpectedError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = reinterpret_cast<VkPipeline &>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03421");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03422");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDynamicState dynamic_state = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        VkPipelineDynamicStateCreateInfo dynamic_states = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dynamic_states.dynamicStateCount = 1;
        dynamic_states.pDynamicStates = &dynamic_state;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pDynamicState = &dynamic_states;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesKHR-flags-03816");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingPipelineShaderGroupsKHR) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);

    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    VkShaderObj rgen_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj ahit_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj miss_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj intr_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj call_shader(this, bindStateRTShaderText, VK_SHADER_STAGE_CALLABLE_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj frag_shader(this, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLibraryCreateInfoKHR library_info = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 5;  // IKHRalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 5;  // invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Fragment stage among shader list
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";
        // put a fragment shader in the list
        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_create_infos[1].module = frag_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingLibraryFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags match library flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!InitFrameworkForRayTracingTest(this, true)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    const char *ray_generation_shader = R"glsl(
        #version 460 core
        #extension GL_KHR_ray_tracing : enable
        void main() {
        }
    )glsl";

    VkShaderObj rgen_shader(this, ray_generation_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    VkPipeline invalid_library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &library);

    pipeline_ci.flags = 0;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &invalid_library);

    VkPipelineLibraryCreateInfoKHR library_ci = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_ci.libraryCount = 1;
    library_ci.pLibraries = &library;

    pipeline_ci.pLibraryInfo = &library_ci;
    VkPipeline pipeline = VK_NULL_HANDLE;

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07403");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT;
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    {
        pipeline_ci.flags = 0;
        library_ci.pLibraries = &invalid_library;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381");
        vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyPipeline(m_device->handle(), library, nullptr);
    vk::DestroyPipeline(m_device->handle(), invalid_library, nullptr);
}

TEST_F(VkLayerTest, RayTracingValidateGetCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION("Validate vkGetRayTracingCaptureReplayShaderGroupHandlesKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    auto rt_pipeline_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    rt_pipeline_features.rayTracingPipelineShaderGroupHandleCaptureReplay = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&rt_pipeline_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkGetRayTracingCaptureReplayShaderGroupHandlesKHR =
        GetInstanceProcAddr<PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR>(
            "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    const auto vkGetPhysicalDeviceProperties2KHR =
        GetInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2KHR>("vkGetPhysicalDeviceProperties2KHR");

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (ray_tracing_features.rayTracingPipelineShaderGroupHandleCaptureReplay == VK_FALSE) {
        GTEST_SKIP() << "rayTracingShaderGroupHandleCaptureReplay not enabled";
    }
    CreateNVRayTracingPipelineHelper rt_pipe(*this);
    rt_pipe.InitInfo(true /*isKHR*/);
    rt_pipe.rp_ci_KHR_.flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
    rt_pipe.InitState();
    VkResult err = rt_pipe.CreateKHRRayTracingPipeline();
    ASSERT_VK_SUCCESS(err);

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vk_testing::Buffer buffer(*m_device, buf_info, vk_testing::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 4096;
    vk_testing::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-arraylength");
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 1, 1, 0, &buffer);
    m_errorMonitor->VerifyFound();

    // dataSize must be at least VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    // Check only when the reported size is
    if (ray_tracing_properties.shaderGroupHandleCaptureReplaySize > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
        vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 1, 1,
                                                          (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
        m_errorMonitor->VerifyFound();
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    // In CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, 2, rt_pipe.groups_KHR_.size(),
                                                      (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051");
    // In CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    uint32_t invalid_firstgroup = rt_pipe.groups_KHR_.size() + 1;
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.pipeline_, invalid_firstgroup, 0,
                                                      (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1), &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingPipelineDeferredOp) {
    TEST_DESCRIPTION(
        "Test that objects created with deferred operations are recorded once the operation has successfully completed.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    if (!InitFrameworkForRayTracingTest(this, true, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    // Needed for Ray Tracing
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");
    const auto vkCreateDeferredOperationKHR = GetInstanceProcAddr<PFN_vkCreateDeferredOperationKHR>("vkCreateDeferredOperationKHR");
    const auto vkDeferredOperationJoinKHR = GetInstanceProcAddr<PFN_vkDeferredOperationJoinKHR>("vkDeferredOperationJoinKHR");
    const auto vkGetDeferredOperationResultKHR =
        GetInstanceProcAddr<PFN_vkGetDeferredOperationResultKHR>("vkGetDeferredOperationResultKHR");

    PFN_vkDestroyDeferredOperationKHR vkDestroyDeferredOperationKHR =
        reinterpret_cast<PFN_vkDestroyDeferredOperationKHR>(vk::GetInstanceProcAddr(instance(), "vkDestroyDeferredOperationKHR"));
    ASSERT_TRUE(vkDestroyDeferredOperationKHR != nullptr);

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkDeferredOperationKHR deferredOperation = VK_NULL_HANDLE;
    vkCreateDeferredOperationKHR(m_device->handle(), 0, &deferredOperation);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result =
        vkCreateRayTracingPipelinesKHR(m_device->handle(), deferredOperation, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(result, VK_OPERATION_DEFERRED_KHR);

    result = vkDeferredOperationJoinKHR(this->m_device->handle(), deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_errorMonitor->VerifyFound();

    result = vkGetDeferredOperationResultKHR(m_device->handle(), deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_commandBuffer->end();

    vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    vkDestroyDeferredOperationKHR(m_device->handle(), deferredOperation, nullptr);
    vk::DestroyPipeline(m_device->handle(), library, nullptr);
}

TEST_F(VkLayerTest, RayTracingPipelineWrongBindPoint) {
    TEST_DESCRIPTION("Bind a graphics pipeline in the ray-tracing bind point");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipelineBindPoint-02392");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipe.pipeline_);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingPipelineMaxResources) {
    TEST_DESCRIPTION("Create ray tracing pipeline with too many resources.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }
    if (!ray_tracing_features.rayTraversalPrimitiveCulling) {
        GTEST_SKIP() << "Feature rayTraversalPrimitiveCulling is not supported.";
    }

    const uint32_t maxPerStageResources = 4;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPerStageResources = maxPerStageResources;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_device->phy().properties().limits.maxPerStageResources,
         VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr}};

    const VkDescriptorSetLayoutObj ds_layout(m_device, layout_bindings);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});
    const char *empty_shader = R"glsl(
        #version 460
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingPipelineCreateInfoKHR create_info = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    create_info.layout = pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03428");
    VkPipeline pipeline;
    vkCreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingInvalidPipelineFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTracingPipeline) {
        GTEST_SKIP() << "Feature rayTracing is not supported.";
    }
    if (!ray_tracing_features.rayTraversalPrimitiveCulling) {
        GTEST_SKIP() << "Feature rayTraversalPrimitiveCulling is not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCreateRayTracingPipelinesKHR =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR");

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        void main() {}
    )glsl";
    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingPipelineCreateInfoKHR create_info = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    create_info.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
    create_info.layout = empty_pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546");
    VkPipeline pipeline;
    vkCreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RayTracingTestWrongPipelineType) {
    TEST_DESCRIPTION("Use a compute pipeline in GetRayTracingShaderGroupStackSizeKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateComputePipeline();

    const auto vkGetRayTracingShaderGroupStackSizeKHR =
        GetInstanceProcAddr<PFN_vkGetRayTracingShaderGroupStackSizeKHR>("vkGetRayTracingShaderGroupStackSizeKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622");
    vkGetRayTracingShaderGroupStackSizeKHR(device(), pipe.pipeline_, 0, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NVRayTracingValidatePipeline) {
    TEST_DESCRIPTION("Validate vkCreateRayTracingPipelinesNV and CreateInfo parameters during ray-tracing pipeline creation");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto pipleline_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(pipleline_features);
    // Set this to true as it is a required feature
    pipleline_features.pipelineCreationCacheControl = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const auto vkCreateRayTracingPipelinesNV =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesNV>("vkCreateRayTracingPipelinesNV");

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});
    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_NV);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    ;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    group_create_info.generalShader = VK_SHADER_UNUSED_NV;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = reinterpret_cast<VkPipeline &>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03421");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateRayTracingPipelinesNV-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03422");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02957");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-02904");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03456");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03458");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03459");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03460");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03461");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03462");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03463");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-03588");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkRayTracingPipelineCreateInfoNV-flags-04948");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    // test for vkCreateRayTracingPipelinesNV
    {
        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        // appending twice as it is generated twice in auto-validation code
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCreateRayTracingPipelinesNV-createInfoCount-arraylength");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 0, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, NVRayTracingPipelineShaderGroups) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");

    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    const auto vkCreateRayTracingPipelinesNV =
        GetInstanceProcAddr<PFN_vkCreateRayTracingPipelinesNV>("vkCreateRayTracingPipelinesNV");

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_NV_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    VkShaderObj ahit_shader(this, empty_shader, VK_SHADER_STAGE_ANY_HIT_BIT_NV);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    VkShaderObj miss_shader(this, empty_shader, VK_SHADER_STAGE_MISS_BIT_NV);
    VkShaderObj intr_shader(this, empty_shader, VK_SHADER_STAGE_INTERSECTION_BIT_NV);
    VkShaderObj call_shader(this, empty_shader, VK_SHADER_STAGE_CALLABLE_BIT_NV);

    VkPipeline pipeline = VK_NULL_HANDLE;

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_info.generalShader = VK_SHADER_UNUSED_NV;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Two raygen stages
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[1].module = rgen_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = 1;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        vk::DestroyPipeline(m_device->device(), pipeline, NULL);
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 5;  // Invalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 5;  // Invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[2] = {};
        group_create_infos[0] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417");
        vkCreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, NVRayTracingPipelineStageCreationFeedbackCount) {
    TEST_DESCRIPTION("Test NV ray tracing pipeline feedback stage count check.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto rtnv_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
    GetPhysicalDeviceProperties2(rtnv_props);

    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "maxDescriptorSetAccelerationStructures < 1";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto feedback_info = LvlInitStruct<VkPipelineCreationFeedbackCreateInfoEXT>();
    VkPipelineCreationFeedbackEXT feedbacks[4] = {};

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 2;
    feedback_info.pPipelineStageCreationFeedbacks = &feedbacks[1];

    auto set_feedback = [&feedback_info](CreateNVRayTracingPipelineHelper &helper) { helper.rp_ci_.pNext = &feedback_info; };

    feedback_info.pipelineStageCreationFeedbackCount = 3;
    CreateNVRayTracingPipelineHelper::OneshotPositiveTest(*this, set_feedback);

    feedback_info.pipelineStageCreationFeedbackCount = 2;
    CreateNVRayTracingPipelineHelper::OneshotTest(*this, set_feedback,
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineStageCreationFeedbackCount-06651");
}
