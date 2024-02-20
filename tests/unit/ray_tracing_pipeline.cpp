/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/ray_tracing_objects.h"

TEST_F(NegativeRayTracingPipeline, BasicUsage) {
    TEST_DESCRIPTION("Validate CreateInfo parameters during ray-tracing pipeline creation");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj ahit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj miss_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj intr_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj call_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CALLABLE_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 1;  // Bad index here
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;
    VkPipelineLibraryCreateInfoKHR library_count_zero = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};
    VkPipelineLibraryCreateInfoKHR library_count_one = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 1};
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-07999");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.stageCount = 1;
        pipeline_ci.groupCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-08700");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.groupCount = 1;
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_one;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetUnexpectedError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03590");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-None-09497");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-02904");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        pipeline_ci.basePipelineIndex = -1;
        constexpr uint64_t fake_pipeline_id = 0xCADECADE;
        VkPipeline fake_pipeline_handle = CastFromUint64<VkPipeline>(fake_pipeline_id);
        pipeline_ci.basePipelineHandle = fake_pipeline_handle;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07984");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_ci.basePipelineIndex = 10;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateRayTracingPipelinesKHR-flags-03415");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07985");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
        pipeline_ci.pLibraryInterface = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03465");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDynamicState dynamic_state = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        VkPipelineDynamicStateCreateInfo dynamic_states = vku::InitStructHelper();
        dynamic_states.dynamicStateCount = 1;
        dynamic_states.pDynamicStates = &dynamic_state;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.stageCount = 1;
        pipeline_ci.pDynamicState = &dynamic_states;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-pDynamicStates-03602");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_count_zero;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03470");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03471");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        pipeline_ci.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateRayTracingPipelinesKHR-flags-03816");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingPipeline, ShaderGroupsKHR) {
    TEST_DESCRIPTION("Validate shader groups during ray-tracing pipeline creation");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj ahit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj miss_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj intr_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj call_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CALLABLE_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj frag_shader(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLibraryCreateInfoKHR library_info = {VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR, NULL, 0};

    // No raygen stage
    {
        VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
        stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_info.module = chit_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
        group_create_info.closestHitShader = 0;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't exist
    {
        VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
        stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_info.module = rgen_shader.handle();
        stage_create_info.pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
        group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_info.generalShader = 1;  // Bad index here
        group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 1;
        pipeline_ci.pStages = &stage_create_info;
        pipeline_ci.groupCount = 1;
        pipeline_ci.pGroups = &group_create_info;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader index doesn't correspond to a raygen/miss/callable shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // General shader group should not specify non general shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[1].generalShader = 0;
        group_create_infos[1].closestHitShader = 0;  // This should not be set for a general shader group
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader invalid index
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 5;  // invalid index

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader index does not correspond to intersection shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 0;  // Index 0 corresponds to a raygen shader

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Intersection shader must not be specified for triangle hit group
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        stage_create_infos[1].module = intr_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = 1;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 5;  // IKHRalid index
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Any hit shader index does not correspond to an any hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = 1;  // Index 1 corresponds to a closest hit shader
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index invalid
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stage_create_infos[1].module = chit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 5;  // invalid index
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Closest hit shader index does not correspond to an closest hit shader
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to an any hit shader
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    // Fragment stage among shader list
    {
        VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";
        // put a fragment shader in the list
        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_create_infos[1].module = frag_shader.handle();
        stage_create_infos[1].pName = "main";

        VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group_create_infos[0].generalShader = 0;
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
        pipeline_ci.pLibraryInfo = &library_info;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 2;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.layout = empty_pipeline_layout.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-stage-06899");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingPipeline, LibraryFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags match library flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    const char *ray_generation_shader = R"glsl(
        #version 460 core
        #extension GL_KHR_ray_tracing : enable
        void main() {
        }
    )glsl";

    VkShaderObj rgen_shader(this, ray_generation_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;

    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    VkPipeline invalid_library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &library);

    pipeline_ci.flags = 0;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
                                     &invalid_library);

    VkPipelineLibraryCreateInfoKHR library_ci = vku::InitStructHelper();
    library_ci.libraryCount = 1;
    library_ci.pLibraries = &library;

    pipeline_ci.pLibraryInfo = &library_ci;
    VkPipeline pipeline = VK_NULL_HANDLE;

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723");
        pipeline_ci.flags = VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR;
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }
    {
        pipeline_ci.flags = 0;
        library_ci.pLibraries = &invalid_library;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381");
        vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyPipeline(m_device->handle(), library, nullptr);
    vk::DestroyPipeline(m_device->handle(), invalid_library, nullptr);
}

TEST_F(NegativeRayTracingPipeline, GetCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION("Validate vkGetRayTracingCaptureReplayShaderGroupHandlesKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineShaderGroupHandleCaptureReplay);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe(*this, m_device);
    rt_pipe.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe.SetRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe.AddMissShader(kRayTracingMinimalGlsl);
    rt_pipe.AddMissShader(kRayTracingMinimalGlsl);
    rt_pipe.Build();

    uint32_t fake_buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-arraylength");
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.Handle(), 1, 1, 0, &fake_buffer);
    m_errorMonitor->VerifyFound();

    // dataSize must be at least VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    // Check only when the reported size is
    if (ray_tracing_properties.shaderGroupHandleCaptureReplaySize > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
        vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.Handle(), 1, 1,
                                                            (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1),
                                                            &fake_buffer);
        m_errorMonitor->VerifyFound();
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    // In nv::rt::CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.Handle(), 2, rt_pipe.GetShaderGroupsCount(),
                                                        (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1),
                                                        &fake_buffer);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051");
    // In nv::rt::CreateNVRayTracingPipelineHelper::InitKHRRayTracingPipelineInfo rp_ci_KHR_.groupCount = groups_KHR_.size();
    uint32_t invalid_firstgroup = rt_pipe.GetShaderGroupsCount() + 1;
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(m_device->handle(), rt_pipe.Handle(), invalid_firstgroup, 0,
                                                        (ray_tracing_properties.shaderGroupHandleCaptureReplaySize - 1),
                                                        &fake_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, DeferredOp) {
    TEST_DESCRIPTION(
        "Test that objects created with deferred operations are recorded once the operation has successfully completed.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkGetDeferredOperationResultKHR not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    static const char *chit_src = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        layout(location = 0) rayPayloadEXT uvec4 hitValue;
        layout(r32ui, set = 0, binding = 0) uniform uimage2D result;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

        void main()
        {
          float tmin     = 0.0;
          float tmax     = 1.0;
          vec3  origin   = vec3(float(gl_LaunchIDEXT.x) + 0.5f, float(gl_LaunchIDEXT.y) + 0.5f, float(gl_LaunchIDEXT.z + 0.5f));
          vec3  direct   = vec3(0.0, 0.0, -1.0);
          hitValue       = uvec4(1,0,0,0);
          traceRayEXT(topLevelAS, 0, 0xFF, 0, 0, 0, origin, tmin, direct, tmax, 0);
          imageStore(result, ivec2(gl_LaunchIDEXT.xy), hitValue);
        }
    )glsl";

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, chit_src, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr}};

    const vkt::DescriptorSetLayout ds_layout(*m_device, layout_bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
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

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkDeferredOperationKHR deferredOperation = VK_NULL_HANDLE;
    vk::CreateDeferredOperationKHR(m_device->handle(), 0, &deferredOperation);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), deferredOperation, VK_NULL_HANDLE, 1, &pipeline_ci,
                                                       nullptr, &pipeline);

    m_commandBuffer->begin();
    if (result == VK_OPERATION_DEFERRED_KHR) {
        result = vk::DeferredOperationJoinKHR(this->m_device->handle(), deferredOperation);
        ASSERT_EQ(result, VK_SUCCESS);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
        m_errorMonitor->VerifyFound();
    }

    result = vk::GetDeferredOperationResultKHR(m_device->handle(), deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_commandBuffer->end();

    vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    vk::DestroyDeferredOperationKHR(m_device->handle(), deferredOperation, nullptr);
    vk::DestroyPipeline(m_device->handle(), library, nullptr);
}

TEST_F(NegativeRayTracingPipeline, MaxResources) {
    TEST_DESCRIPTION("Create ray tracing pipeline with too many resources.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (!ray_tracing_features.rayTraversalPrimitiveCulling) {
        GTEST_SKIP() << "Feature rayTraversalPrimitiveCulling is not supported.";
    }

    const uint32_t maxPerStageResources = 4;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPerStageResources = maxPerStageResources;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_features));

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_device->phy().limits_.maxPerStageResources, VK_SHADER_STAGE_RAYGEN_BIT_KHR,
         nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr}};

    const vkt::DescriptorSetLayout ds_layout(*m_device, layout_bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR shader_group = vku::InitStructHelper();
    shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shader_group.generalShader = 0;
    shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
    shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
    shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR create_info = vku::InitStructHelper();
    create_info.layout = pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;
    create_info.groupCount = 1;
    create_info.pGroups = &shader_group;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03428");
    VkPipeline pipeline;
    vk::CreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, PipelineFlags) {
    TEST_DESCRIPTION("Validate ray tracing pipeline flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    RETURN_IF_SKIP(Init());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kMinimalShaderGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR shader_group = vku::InitStructHelper();
    shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    shader_group.generalShader = 0;
    shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
    shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
    shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR create_info = vku::InitStructHelper();
    create_info.flags = VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;
    create_info.layout = empty_pipeline_layout.handle();
    create_info.stageCount = 1;
    create_info.pStages = &stage_create_info;
    create_info.groupCount = 1;
    create_info.pGroups = &shader_group;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546");
    VkPipeline pipeline;
    vk::CreateRayTracingPipelinesKHR(device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, PipelineTypeGroupStackSize) {
    TEST_DESCRIPTION("Use a compute pipeline in GetRayTracingShaderGroupStackSizeKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateComputePipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622");
    vk::GetRayTracingShaderGroupStackSizeKHR(device(), pipe.pipeline_, 0, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, GetRayTracingShaderGroupStackSizeUnusedGroup) {
    TEST_DESCRIPTION("Call vkGetRayTracingShaderGroupStackSizeKHR on an unused shader in a shader group");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);
    pipeline.SetRayGenShader(kRayTracingMinimalGlsl);
    pipeline.Build();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-group-03608");
    vk::GetRayTracingShaderGroupStackSizeKHR(*m_device, pipeline.Handle(), 42, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupStackSizeKHR-groupShader-03609");
    vk::GetRayTracingShaderGroupStackSizeKHR(*m_device, pipeline.Handle(), 0, VK_SHADER_GROUP_SHADER_ANY_HIT_KHR);
    m_errorMonitor->VerifyFound();

    m_device->wait();
}

TEST_F(NegativeRayTracingPipeline, PipelineTypeGroupHandles) {
    TEST_DESCRIPTION("Use a compute pipeline in GetRayTracingShaderGroupHandlesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateComputePipeline();

    int data = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-04619");
    vk::GetRayTracingShaderGroupHandlesKHR(device(), pipe.pipeline_, 0, 0, 4, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, PipelineTypeCaptureReplay) {
    TEST_DESCRIPTION("Use a compute pipeline in GetRayTracingCaptureReplayShaderGroupHandlesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineShaderGroupHandleCaptureReplay);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateComputePipeline();

    int data = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-04620");
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(device(), pipe.pipeline_, 0, 0, 4, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingPipeline, LibraryGroupHandlesEXT) {
    TEST_DESCRIPTION("Validate VK_EXT_pipeline_library_group_handles");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineShaderGroupHandleCaptureReplay);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddDisabledFeature(vkt::Feature::pipelineLibraryGroupHandles);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe_lib(*this, m_device);
    rt_pipe_lib.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe_lib.InitLibraryInfo();
    rt_pipe_lib.SetRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.BuildPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-07828");
    rt_pipe_lib.GetRayTracingShaderGroupHandles();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-07829");
    rt_pipe_lib.GetRayTracingCaptureReplayShaderGroupHandles();
    m_errorMonitor->VerifyFound();
}
