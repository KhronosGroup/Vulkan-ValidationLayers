/*
 * Copyright (c) 2023 Nintendo
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeShaderObject, SpirvCodeSize) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-codeSize-08735");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]) - 2u;
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedComputeShader) {
    TEST_DESCRIPTION("Create compute shader with linked flag.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08412");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidFlags) {
    TEST_DESCRIPTION("Create shader with invalid flags.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08992");
    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08485");
    createInfo.flags = VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08486");
    createInfo.flags = VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08488");
    createInfo.flags = VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidMeshShaderExtFlags) {
    TEST_DESCRIPTION("Create mesh shader with invalid flags.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08414");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, false));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, VertexNextStage) {
    TEST_DESCRIPTION("Create vertex shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08427");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TessellationControlNextStage) {
    TEST_DESCRIPTION("Create tessellation control shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08430");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TessellationEvaluationNextStage) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08431");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryNextStage) {
    TEST_DESCRIPTION("Create geometry shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08433");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, FragmentNextStage) {
    TEST_DESCRIPTION("Create fragment shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08434");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TaskNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08435");
    
    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_2, true, false));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    createInfo.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08436");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_2, false, true));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfo.nextStage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TaskNVNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08435");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_2, true, false));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_NV, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TASK_BIT_NV;
    createInfo.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshNVNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08436");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2))
    if (meshShaderFeatures.meshShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_NV, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_NV;
    createInfo.nextStage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BinaryCodeAlignment) {
    TEST_DESCRIPTION("Create binary shader with invalid binary code alignment.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08492");

    RETURN_IF_SKIP(InitBasicShaderObject())

    std::vector<uint32_t> spv(256);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(uint8_t);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = reinterpret_cast<void*>(ptr);
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpirvCodeAlignment) {
    TEST_DESCRIPTION("Create shader with invalid binary code alignment.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08493");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(uint8_t);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = reinterpret_cast<void*>(ptr);
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidStage) {
    TEST_DESCRIPTION("Create shader with invalid stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08425");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08426");

    createInfo.stage = VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindVertexAndTaskShaders) {
    TEST_DESCRIPTION("Bind vertex and task shaders in the same call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08470");

    
    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_3, true, false));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto task_spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT vertCreateInfo = vku::InitStructHelper();
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vertCreateInfo.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vertCreateInfo.pCode = vert_spv.data();
    vertCreateInfo.pName = "main";

    VkShaderCreateInfoEXT taskCreateInfo = vku::InitStructHelper();
    taskCreateInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    taskCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    taskCreateInfo.codeSize = task_spv.size() * sizeof(task_spv[0]);
    taskCreateInfo.pCode = task_spv.data();
    taskCreateInfo.pName = "main";

    vkt::Shader vert_shader(*m_device, vertCreateInfo);
    vkt::Shader task_shader(*m_device, taskCreateInfo);
    VkShaderEXT shaderHandles[] = {
        vert_shader.handle(),
        task_shader.handle(),
    };

    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_TASK_BIT_EXT,
    };
    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, stages, shaderHandles);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindVertexAndMeshShaders) {
    TEST_DESCRIPTION("Bind vertex and mesh shaders in the same call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08471");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_3, false, true));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT vertCreateInfo = vku::InitStructHelper();
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vertCreateInfo.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vertCreateInfo.pCode = vert_spv.data();
    vertCreateInfo.pName = "main";

    VkShaderCreateInfoEXT meshCreateInfo = vku::InitStructHelper();
    meshCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    meshCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    meshCreateInfo.codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    meshCreateInfo.pCode = mesh_spv.data();
    meshCreateInfo.pName = "main";

    vkt::Shader vert_shader(*m_device, vertCreateInfo);
    vkt::Shader mesh_shader(*m_device, meshCreateInfo);
    VkShaderEXT shaderHandles[] = {
        vert_shader.handle(),
        mesh_shader.handle(),
    };

    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_MESH_BIT_EXT,
    };
    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, stages, shaderHandles);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, CreateShadersWithoutEnabledFeatures) {
    TEST_DESCRIPTION("Create tessellation shader without tessellationShader feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    features2.features.tessellationShader = VK_FALSE;
    features2.features.geometryShader = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08419");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08740");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vkt::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08420");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08740");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vkt::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, CreateMeshShadersWithoutEnabledFeatures) {
    TEST_DESCRIPTION("Create mesh and task shaders without features enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper();
    VkPhysicalDeviceMaintenance4Features maintenance4Features = vku::InitStructHelper(&shaderObjectFeatures);
    GetPhysicalDeviceFeatures2(maintenance4Features);
    if (shaderObjectFeatures.shaderObject == VK_FALSE) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (maintenance4Features.maintenance4 == VK_FALSE) {
        GTEST_SKIP() << "maintenance4 not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &maintenance4Features));

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08421");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vkt::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08422");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vkt::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, ComputeShaderNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use compute shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08476");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_COMPUTE_BIT);

    if (!graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GraphicsShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use graphics shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08477");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GraphicsMeshShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use mesh shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08478");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_3, false, true));

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, NonUniqueShadersBind) {
    TEST_DESCRIPTION("Bind multiple shaders with same stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08463");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader1(*m_device, createInfo);
    vkt::Shader shader2(*m_device, createInfo);

    VkShaderEXT shaders[] = {
        shader1.handle(),
        shader2.handle(),
    };
    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_VERTEX_BIT,
    };

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, stages, shaders);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidShaderStageBind) {
    TEST_DESCRIPTION("Bind shader with invalid stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08469");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08464");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL_GRAPHICS;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GetShaderBinaryDataInvalidPointer) {
    TEST_DESCRIPTION("Get shader binary data with invalid pointer.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetShaderBinaryDataEXT-None-08499");

    RETURN_IF_SKIP(InitBasicShaderObject())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vkt::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    size_t dataSize = 0;
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, nullptr);
    std::vector<uint8_t> data(dataSize + 1u);
    auto ptr = reinterpret_cast<std::uintptr_t>(data.data()) + sizeof(uint8_t);
    void* dataPtr = reinterpret_cast<void*>(ptr);
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, dataPtr);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithNoShadersBound) {
    TEST_DESCRIPTION("Call vkCmdDraw when there are no shaders or pipeline bound.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08684");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08688");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_1, false))

    InitDynamicRenderTarget();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithMissingShaders) {
    TEST_DESCRIPTION("Draw without setting all of the shader objects.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08687");

    RETURN_IF_SKIP(InitBasicShaderObject())

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);

    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithoutBindingMeshShadersWhenEnabled) {
    TEST_DESCRIPTION("Draw without binding all of the shader objects supported by graphics.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08689");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08690");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, false, true));

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidShaderCreateInfoFlags) {
    TEST_DESCRIPTION("Create shader with invalid flags.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08487");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08489");
    createInfo.flags = VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkSingleStage) {
    TEST_DESCRIPTION("Create a single linked shader object.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08401");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLinkStageBit) {
    TEST_DESCRIPTION("Create a linked and non-linked shader in the same call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08402");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLinkStageBitMesh) {
    TEST_DESCRIPTION("Create a linked and non-linked mesh shader in the same call.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08403");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, false, true));

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    createInfos[0].pCode = mesh_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedVertexAndMeshStages) {
    TEST_DESCRIPTION("Attempt to create linked vertex and mesh stages.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08404");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, false, true));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    createInfos[1].pCode = mesh_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedTaskAndMeshNoTaskShaders) {
    TEST_DESCRIPTION("Attempt to create linked task shader and linked mesh shader with no task shader flag.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08405");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));

    const auto task_spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    createInfos[0].nextStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = task_spv.size() * sizeof(task_spv[0]);
    createInfos[0].pCode = task_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    createInfos[1].pCode = mesh_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingNextStage) {
    TEST_DESCRIPTION("Attempt to linked vertex and fragment shaders with missing nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08409");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08409");
    createInfos[0].nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SameLinkedStage) {
    TEST_DESCRIPTION("Create multiple linked shaders with the same stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08410");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = spv.size() * sizeof(spv[0]);
    createInfos[0].pCode = spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = spv.size() * sizeof(spv[0]);
    createInfos[1].pCode = spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedStagesWithDifferentCodeType) {
    TEST_DESCRIPTION("Create linked shaders with different code types.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08411");

    RETURN_IF_SKIP(InitBasicShaderObject())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    size_t dataSize = 0u;
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaders[0], &dataSize, nullptr);
    std::vector<uint8_t> binaryData(dataSize);
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaders[0], &dataSize, binaryData.data());

    createInfos[0].codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfos[0].codeSize = dataSize;
    createInfos[0].pCode = binaryData.data();

    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, UnsupportedNextStage) {
    TEST_DESCRIPTION("Create shader with unsupported next stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08428");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (!shaderObjectFeatures.shaderObject) {
        GTEST_SKIP() << "Test requires (unsupported) shaderObject , skipping.";
    }
    features2.features.tessellationShader = VK_FALSE;
    features2.features.geometryShader = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08429");
    createInfo.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTessellationControlNextStage) {
    TEST_DESCRIPTION("Create tessellation control shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08430");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTessellationEvaluationNextStage) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08431");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidGeometryNextStage) {
    TEST_DESCRIPTION("Create geometry shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08433");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidFragmentNextStage) {
    TEST_DESCRIPTION("Create fragment shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08434");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.nextStage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTaskNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08435");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    createInfo.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidMeshNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid nextStage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-nextStage-08436");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, false, true));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfo.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindInvalidShaderStage) {
    TEST_DESCRIPTION("Bind shader with different stage than it was created with.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08469");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &vertShader.handle());
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithShadersOutsideRenderPass) {
    TEST_DESCRIPTION("Draw with shaders outside of a render pass.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderpass");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithShadersInNonDynamicRenderPass) {
    TEST_DESCRIPTION("Draw with shaders inside a non-dynamic render pass.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08876");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1u;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1u;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo framebuffer_info = vku::InitStructHelper();
    framebuffer_info.renderPass = render_pass.handle();
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &image_view;
    framebuffer_info.width = 32;
    framebuffer_info.height = 32;
    framebuffer_info.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_info);

    VkClearValue clear_value;
    clear_value.color.float32[0] = 0.25f;
    clear_value.color.float32[1] = 0.25f;
    clear_value.color.float32[2] = 0.25f;
    clear_value.color.float32[3] = 0.0f;

    VkRenderPassBeginInfo beginInfo = vku::InitStructHelper();
    beginInfo.renderPass = render_pass.handle();
    beginInfo.framebuffer = framebuffer.handle();
    beginInfo.renderArea.extent.width = 32;
    beginInfo.renderArea.extent.height = 32;
    beginInfo.renderArea.offset.x = 0;
    beginInfo.renderArea.offset.y = 0;
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clear_value;

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, IncompatibleDescriptorSet) {
    TEST_DESCRIPTION("Bind an incompatible descriptor set.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08600");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08600");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char vertSource[] = R"glsl(
        #version 460
        layout(set = 0, binding = 1) buffer foo {
            int x;
        } bar;
        void main() {
           gl_Position = vec4(bar.x);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                       });

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], vertSource), &descriptor_set.layout_.handle());
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentUniformGlsl),
                                 &descriptor_set.layout_.handle());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, NotSettingViewportAndScissor) {
    TEST_DESCRIPTION("Draw with shader object without setting viewport and scissor.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08635");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DifferentViewportAndScissorCount) {
    TEST_DESCRIPTION("Draw with shader object with different viewport and scissor count.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08635");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2u, viewports);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidViewportWScaling) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetViewportWScaling.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08636");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 2u, scissors);
    VkViewportWScalingNV viewportWScaling = {1.0f, 1.0f};
    vk::CmdSetViewportWScalingEnableNV(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdSetViewportWScalingNV(m_commandBuffer->handle(), 0u, 1u, &viewportWScaling);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidShadingRatePaletteViewportCount) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetViewportShadingRatePaletteNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08637");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&shadingRateImageFeatures))
    if (shadingRateImageFeatures.shadingRateImage == VK_FALSE) {
        GTEST_SKIP() << "shadingRateImage not supported.";
    }
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 2u, scissors);
    VkShadingRatePaletteEntryNV defaultShadingRatePaletteEntry = VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV;
    VkShadingRatePaletteNV shadingRatePalette;
    shadingRatePalette.shadingRatePaletteEntryCount = 1u;
    shadingRatePalette.pShadingRatePaletteEntries = &defaultShadingRatePaletteEntry;
    vk::CmdSetShadingRateImageEnableNV(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 0u, 1u, &shadingRatePalette);
    vk::CmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetExclusiveScissorEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetExclusiveScissorEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-exclusiveScissor-09235");

    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusiveScissorFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&exclusiveScissorFeatures))
    if (exclusiveScissorFeatures.exclusiveScissor == VK_FALSE) {
        GTEST_SKIP() << "exclusiveScissor not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidExclusiveScissorCount) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetExclusiveScissorNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08638");

    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusiveScissorFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&exclusiveScissorFeatures))
    if (exclusiveScissorFeatures.exclusiveScissor == VK_FALSE) {
        GTEST_SKIP() << "exclusiveScissor not supported.";
    }
    uint32_t count;
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> properties(count);
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, properties.data());
    for (const auto& p : properties) {
        if (std::strcmp(p.extensionName, VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME) == 0) {
            if (p.specVersion < 2) {
                GTEST_SKIP() << "exclusiveScissor specVersion 2 not supported.";
            }
        }
    }
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 2u, scissors);
    VkBool32 exclusiveScissorEnable = VK_TRUE;
    vk::CmdSetExclusiveScissorEnableNV(m_commandBuffer->handle(), 0u, 1u, &exclusiveScissorEnable);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizerDiscardEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetRasterizerDiscardEnable.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08639");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBiasEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetDepthBiasEnable.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08640");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetLogicOp) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetLogicOp.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08641");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (shaderObjectFeatures.shaderObject == VK_FALSE) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (features2.features.logicOp == VK_FALSE) {
        GTEST_SKIP() << "logicOp not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LOGIC_OP_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdSetLogicOpEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BlendEnabledWithNonBlendableFormat) {
    TEST_DESCRIPTION("Draw with shader objects with blend enabled for attachment format that does not support blending.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08643");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_R32_UINT, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0 ||
        (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) != 0) {
        GTEST_SKIP() << "color attachment format not suitable.";
    }

    InitDynamicRenderTarget(VK_FORMAT_R32_UINT);

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkBool32 enabled = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &enabled);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, RasterizationSamplesMismatch) {
    TEST_DESCRIPTION("Draw with shader objects with invalid rasterization samples in vkCmdSetRasterizationSamplesEXT().");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08644");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdSetRasterizationSamplesEXT(m_commandBuffer->handle(), VK_SAMPLE_COUNT_2_BIT);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingColorWriteEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorWriteEnableEXT().");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08646");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_enable_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&color_write_enable_features))
    if (!color_write_enable_features.colorWriteEnable) {
        GTEST_SKIP() << "colorWriteEnable not supported.";
    }

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ColorWriteEnableAttachmentCount) {
    TEST_DESCRIPTION("Draw with shader objects without setting color write enable for all attachments.");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08647");

    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_enable_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&color_write_enable_features))
    if (!color_write_enable_features.colorWriteEnable) {
        GTEST_SKIP() << "colorWriteEnable not supported.";
    }

    VkImageObj img1(m_device);
    img1.Init(m_width, m_height, 1, m_render_target_fmt,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);
    VkImageObj img2(m_device);
    img2.Init(m_width, m_height, 1, m_render_target_fmt,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);

    VkImageView view1 = img1.targetView(m_render_target_fmt);
    VkImageView view2 = img2.targetView(m_render_target_fmt);

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    VkRenderingAttachmentInfo attachments[2];
    attachments[0] = vku::InitStructHelper();
    attachments[0].imageView = view1;
    attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1] = vku::InitStructHelper();
    attachments[1].imageView = view2;
    attachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 2u;
    renderingInfo.pColorAttachments = attachments;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkBool32 colorWriteEnable = VK_TRUE;
    vk::CmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1u, &colorWriteEnable);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 1u, 1u, &colorBlendEnable);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleEnableEXT) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetDiscardRectangleEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08648");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDiscardRectangleModeEXT().");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08649");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    uint32_t count;
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> properties(count);
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, properties.data());
    for (const auto& p : properties) {
        if (std::strcmp(p.extensionName, VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME) == 0) {
            if (p.specVersion < 2) {
                GTEST_SKIP() << "discard rectangles specVersion 2 not supported.";
            }
        }
    }

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetDiscardRectangleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    VkRect2D discardRectangle;
    discardRectangle.offset = {};
    discardRectangle.extent = {100u, 100u};
    vk::CmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0u, 1u, &discardRectangle);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDiscardRectangleEXT().");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-rasterizerDiscardEnable-09236");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    uint32_t count;
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> properties(count);
    vk::EnumerateDeviceExtensionProperties(m_device->phy(), nullptr, &count, properties.data());
    for (const auto& p : properties) {
        if (std::strcmp(p.extensionName, VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME) == 0) {
            if (p.specVersion < 2) {
                GTEST_SKIP() << "discard rectangles specVersion 2 not supported.";
            }
        }
    }

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetDiscardRectangleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdSetDiscardRectangleModeEXT(m_commandBuffer->handle(), VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClampEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClampEnableEXT().");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08650");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetPolygonModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPolygonModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08651");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_POLYGON_MODE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizationSamplesEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRasterizationSamplesEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08652");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleMaskEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08653");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_SAMPLE_MASK_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetAlphaToCoverageEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAlphaToCoverageEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08654");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetAlphaToOneEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAlphaToOneEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08655");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetLogicOpEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLogicOpEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08656");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (shaderObjectFeatures.shaderObject == VK_FALSE) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (features2.features.logicOp == VK_FALSE) {
        GTEST_SKIP() << "logicOp not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08657");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEquationEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEquationEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08658");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    BindVertFragShader(vertShader, fragShader);
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendEnable);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendAdvancedEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendAdvancedEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08658");

    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendEnable);
    VkColorBlendAdvancedEXT colorBlendAdvanced;
    colorBlendAdvanced.advancedBlendOp = VK_BLEND_OP_ADD;
    colorBlendAdvanced.srcPremultiplied = VK_FALSE;
    colorBlendAdvanced.dstPremultiplied = VK_FALSE;
    colorBlendAdvanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
    colorBlendAdvanced.clampResults = VK_FALSE;
    vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendAdvanced);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetFragmentShadingRateKHR) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetFragmentShadingRateKHR.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pipelineFragmentShadingRate-09238");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&fsr_features))
    if (!fsr_features.pipelineFragmentShadingRate) {
        GTEST_SKIP() << "pipelineFragmentShadingRate not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorWriteMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorWriteMaskEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08659");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizationStreamEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRasterizationStreamEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08660");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transformFeedbackFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&transformFeedbackFeatures))
    if (transformFeedbackFeatures.geometryStreams == VK_FALSE) {
        GTEST_SKIP() << "geometryStreams not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, geomShader.handle(), fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetConservativeRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetConservativeRasterizationModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08661");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetExtraPrimitiveOverestimationSizeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetExtraPrimitiveOverestimationSizeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08662");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetConservativeRasterizationModeEXT(m_commandBuffer->handle(), VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClipEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClipEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08663");

    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = vku::InitStructHelper();
    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject(&depthClipEnableFeatures))
    if (depthClipEnableFeatures.depthClipEnable == VK_FALSE) {
        GTEST_SKIP() << "depthClipEnable not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleLocationsEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleLocationsEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08664");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetProvokingVertexModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetProvokingVertexModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08665");

    AddRequiredExtensions(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPolygonModeCmdSetLineRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineRasterizationModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08666");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_LINE);
    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyCmdSetLineRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineRasterizationModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08667");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPolygonModeCmdSetLineStippleEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08669");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_LINE);
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyCmdSetLineStippleEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08670");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetLineStippleEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08672");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClipNegativeOneToOneEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClipNegativeOneToOneEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08673");

    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME);
    VkPhysicalDeviceDepthClipControlFeaturesEXT depthClipControlFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&depthClipControlFeatures))
    if (depthClipControlFeatures.depthClipControl == VK_FALSE) {
        GTEST_SKIP() << "depthClipControl not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportWScalingEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportWScalingEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08674");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportWScalingNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportWScalingNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-09232");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetViewportWScalingEnableNV(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportSwizzleNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportSwizzleNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08675");

    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageToColorEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageToColorEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08676");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageToColorLocationNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageToColorLocationNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08677");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoverageToColorEnableNV(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationModeNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationModeNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08678");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoverageModulationTableEnableNV(m_commandBuffer->handle(), VK_FALSE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationTableEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationTableEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08679");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoverageModulationModeNV(m_commandBuffer->handle(), VK_COVERAGE_MODULATION_MODE_RGBA_NV);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationTableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationTableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08680");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoverageModulationModeNV(m_commandBuffer->handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
    vk::CmdSetCoverageModulationTableEnableNV(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetShadingRateImageEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetShadingRateImageEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08681");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&shadingRateImageFeatures))
    if (shadingRateImageFeatures.shadingRateImage == VK_FALSE) {
        GTEST_SKIP() << "shadingRateImage not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportShadingRatePaletteNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportShadingRatePaletteNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-shadingRateImage-09234");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&shadingRateImageFeatures))
    if (shadingRateImageFeatures.shadingRateImage == VK_FALSE) {
        GTEST_SKIP() << "shadingRateImage not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);
    vk::CmdSetShadingRateImageEnableNV(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoarseSampleOrderNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoarseSampleOrderNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-shadingRateImage-09233");

    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    VkPhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&shadingRateImageFeatures))
    if (shadingRateImageFeatures.shadingRateImage == VK_FALSE) {
        GTEST_SKIP() << "shadingRateImage not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetShadingRateImageEnableNV(m_commandBuffer->handle(), VK_FALSE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetRepresentativeFragmentTestEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRepresentativeFragmentTestEnableNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08682");

    AddRequiredExtensions(VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME);
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV representativeFragmentTestFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&representativeFragmentTestFeatures))
    if (representativeFragmentTestFeatures.representativeFragmentTest == VK_FALSE) {
        GTEST_SKIP() << "representativeFragmentTest not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageReductionModeNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageReductionModeNV.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08683");

    AddRequiredExtensions(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    VkPhysicalDeviceCoverageReductionModeFeaturesNV coverageReductionFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&coverageReductionFeatures))
    if (coverageReductionFeatures.coverageReductionMode == VK_FALSE) {
        GTEST_SKIP() << "coverageReductionMode not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    vk::CmdSetCoverageModulationModeNV(m_commandBuffer->handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
    vk::CmdSetCoverageModulationTableEnableNV(m_commandBuffer->handle(), VK_FALSE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingVertexShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding vertex shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08684");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingTessellationControlBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding tessellation control shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08685");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (!shaderObjectFeatures.shaderObject) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (features2.features.tessellationShader == VK_FALSE) {
        GTEST_SKIP() << "tessellationShader not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingTessellationEvaluationBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding tessellation evaluation shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08686");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (!shaderObjectFeatures.shaderObject) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (features2.features.tessellationShader == VK_FALSE) {
        GTEST_SKIP() << "tessellationShader not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingGeometryBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding geometry shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08687");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (!shaderObjectFeatures.shaderObject) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (features2.features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingFragmentShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding fragment shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08688");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 4u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingTaskShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding task shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08689");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    {
        VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &nullShader);
    }
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingMeshShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding mesh shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08690");
    
    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    {
        VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &nullShader);
    }
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, VertAndMeshShaderBothBound) {
    TEST_DESCRIPTION("Draw with both vertex and mesh shader objects bound.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08693");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08696");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08885");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const vkt::Shader meshShader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                                 GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3),
                                 VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    {
        VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &nullShader);
    }
    VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &meshShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshShaderWithMissingTaskShader) {
    TEST_DESCRIPTION("Draw with a mesh shader that was created without the no task shader flag, but no task shader bound.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08694");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08885");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const vkt::Shader meshShader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                                 GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);

    VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
    VkShaderEXT nullShader = VK_NULL_HANDLE;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &nullShader);
    VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &meshShader.handle());

    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TaskAndMeshShaderWithNoTaskFlag) {
    TEST_DESCRIPTION("Draw with a task and a mesh shader that was created with the no task shader flag.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08695");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08885");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const vkt::Shader taskShader(*m_device, VK_SHADER_STAGE_TASK_BIT_EXT,
                                 GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3));

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT meshCreateInfo = vku::InitStructHelper();
    meshCreateInfo.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
    meshCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    meshCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    meshCreateInfo.codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    meshCreateInfo.pCode = mesh_spv.data();
    meshCreateInfo.pName = "main";
    const vkt::Shader meshShader(*m_device, meshCreateInfo);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);

    VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &taskShader.handle());
    VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &meshShader.handle());

    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, VertAndTaskShadersBound) {
    TEST_DESCRIPTION("Draw with a vertex shader and task shaders bound as well.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08693");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08696");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08885");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT meshCreateInfo = vku::InitStructHelper();
    meshCreateInfo.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
    meshCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    meshCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    meshCreateInfo.codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    meshCreateInfo.pCode = mesh_spv.data();
    meshCreateInfo.pName = "main";
    const vkt::Shader meshShader(*m_device, meshCreateInfo);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);

    VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
    VkShaderEXT nullShader = VK_NULL_HANDLE;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &nullShader);
    VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    {
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &meshShader.handle());
    }

    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLinkedShaderBind) {
    TEST_DESCRIPTION("Draw with not all linked shaders bound.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08698");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";

    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(*m_device, 2u, createInfos, nullptr, shaders);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT bindShaders[] = {shaders[0], VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, bindShaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
    }

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindShaderBetweenLinkedShaders) {
    TEST_DESCRIPTION("Draw when a shader is bound between linked shaders.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08699");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";

    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(*m_device, 2u, createInfos, nullptr, shaders);

    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT bindShaders[] = {shaders[0], VK_NULL_HANDLE, VK_NULL_HANDLE, geomShader.handle(), shaders[1]};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, bindShaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
    }

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DifferentShaderPushConstantRanges) {
    TEST_DESCRIPTION("Draw with shaders that have different push constant ranges.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08878");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkPushConstantRange pushConstRange;
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstRange.offset = 0u;
    pushConstRange.size = sizeof(uint32_t);

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl),
                                 nullptr, &pushConstRange);
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DifferentShaderDescriptorLayouts) {
    TEST_DESCRIPTION("Draw with shaders that have different descriptor layouts.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08879");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    VkDescriptorSetLayout descriptorSetLayoutHandle = descriptorSet.GetDescriptorSetLayout();
    VkDescriptorSet descriptorSetHandle = descriptorSet.GetDescriptorSetHandle();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl),
                                 &descriptorSetLayoutHandle, nullptr);
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(), 0u, 1u,
                              &descriptorSetHandle, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetAttachmentFeedbackLoopEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAttachmentFeedbackLoopEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08880");

    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT attachmentFeedbackLoopDynamicStateFeatures =
        vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&attachmentFeedbackLoopDynamicStateFeatures))
    if (attachmentFeedbackLoopDynamicStateFeatures.attachmentFeedbackLoopDynamicState == VK_FALSE) {
        GTEST_SKIP() << "attachmentFeedbackLoopDynamicState not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetPrimitiveTopologyEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPrimitiveTopologyEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07842");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetVertexInputEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetVertexInputEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08882");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_VERTEX_INPUT_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetPatchControlPointsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPatchControlPointsEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04875");

    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.tessellationShader == VK_FALSE) {
        GTEST_SKIP() << "tessellationShader not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader tescShader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl));
    const vkt::Shader teseShader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT});
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), VK_NULL_HANDLE,
                                   fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetTessellationDomainOriginEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetTessellationDomainOriginEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-09237");

    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.tessellationShader == VK_FALSE) {
        GTEST_SKIP() << "tessellationShader not supported.";
    }
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader tescShader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl));
    const vkt::Shader teseShader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT});
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), VK_NULL_HANDLE,
                                   fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetPrimitiveRestartEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPrimitiveRestartEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04879");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithGraphicsShadersWhenMeshShaderIsBound) {
    TEST_DESCRIPTION("Draw with graphics shader objects when a mesh shader is bound.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08885");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, true, true));
    InitDynamicRenderTarget();

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const vkt::Shader meshShader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                                 GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3),
                                 VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
    {
        VkShaderStageFlagBits taskStage = VK_SHADER_STAGE_TASK_BIT_EXT;
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &taskStage, &nullShader);
    }
    VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &meshStage, &meshShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPolygonLineCmdSetLineWidthEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineWidthEXT when polygon mode is line.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08617");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_WIDTH});
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_LINE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyLineCmdSetLineWidthEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineWidthEXT when primitive topology is line.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08618");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_WIDTH});
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBiasEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBiasEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08620");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BIAS});
    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetBlendConstantsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetBlendConstantsEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08621");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_BLEND_CONSTANTS});
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_CONSTANT_COLOR,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendEquation);
    BindVertFragShader(vertShader, fragShader);
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendEnable);
    VkColorBlendAdvancedEXT colorBlendAdvanced;
    colorBlendAdvanced.advancedBlendOp = VK_BLEND_OP_ADD;
    colorBlendAdvanced.srcPremultiplied = VK_FALSE;
    colorBlendAdvanced.dstPremultiplied = VK_FALSE;
    colorBlendAdvanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
    colorBlendAdvanced.clampResults = VK_FALSE;
    vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendAdvanced);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBoundsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBoundsEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08622");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BOUNDS});
    vk::CmdSetDepthBoundsTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilCompareMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilCompareMaskEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08623");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK});
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilWriteMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilWriteMaskEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08624");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_STENCIL_WRITE_MASK});
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilReferenceEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilReferenceEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08625");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_STENCIL_REFERENCE});
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleLocationsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleLocationsEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08626");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT});
    vk::CmdSetSampleLocationsEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetCullModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCullModeEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08627");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_CULL_MODE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetFrontFaceEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetFrontFaceEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08628");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_FRONT_FACE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdSetCullModeEXT(m_commandBuffer->handle(), VK_CULL_MODE_BACK_BIT);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthTestEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthTestEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08629");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthWriteEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthWriteEnableEXT.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08630");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthCompareOp) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthCompareOp.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08631");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT});
    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBoundsTestEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBoundsTestEnable.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08632");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.depthBounds == VK_FALSE) {
        GTEST_SKIP() << "depthBounds not supported.";
    }

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilTestEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilTestEnable.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08633");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.depthBounds == VK_FALSE) {
        GTEST_SKIP() << "depthBounds not supported.";
    }

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilOp) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilOp.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08634");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.depthBounds == VK_FALSE) {
        GTEST_SKIP() << "depthBounds not supported.";
    }

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_STENCIL_OP_EXT});
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ComputeShaderGroupCount) {
    TEST_DESCRIPTION("Dispatch with group count higher than maxComputeWorkGroupCount.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    uint32_t x_count_limit = m_device->phy().limits_.maxComputeWorkGroupCount[0];
    uint32_t y_count_limit = m_device->phy().limits_.maxComputeWorkGroupCount[1];
    uint32_t z_count_limit = m_device->phy().limits_.maxComputeWorkGroupCount[2];

    const vkt::Shader compShader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl));

    m_commandBuffer->begin();

    BindCompShader(compShader);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountX-00386");
    vk::CmdDispatch(m_commandBuffer->handle(), x_count_limit + 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountY-00387");
    vk::CmdDispatch(m_commandBuffer->handle(), 1u, y_count_limit + 1u, 1u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-groupCountZ-00388");
    vk::CmdDispatch(m_commandBuffer->handle(), 1u, 1u, z_count_limit + 1u);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeShaderObject, ComputeShaderMissingPushConst) {
    TEST_DESCRIPTION("Dispatch with a shader object using push const, but not setting it.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-maintenance4-08602");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkPushConstantRange pushConstRange;
    pushConstRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstRange.offset = 0u;
    pushConstRange.size = sizeof(int);

    static const char kComputeShaderGlsl[] = R"glsl(
        #version 460
        layout (push_constant) uniform constants {
	        int value;
        } PushConstants;
        layout(set = 0, binding = 0) buffer foo {
            int x;
        } bar;
        void main() {
            bar.x = PushConstants.value;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pushConstRange});

    const vkt::Shader compShader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kComputeShaderGlsl),
                                 &descriptor_set.layout_.handle(), &pushConstRange);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    BindCompShader(compShader);
    vk::CmdDispatch(m_commandBuffer->handle(), 1u, 1u, 1u);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints + 16);
    csSource << R"glsl(];
        void main(){
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, csSource.str().c_str());

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidRequireFullSubgroupsFlag) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08992");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMapEntryOffset) {
    TEST_DESCRIPTION("Create shader with invalid specialization map entry offset.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationInfo-offset-00773");

    RETURN_IF_SKIP(InitBasicShaderObject())

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry mapEntry = {};
    mapEntry.constantID = 0u;
    mapEntry.offset = sizeof(int) * 2;
    mapEntry.size = sizeof(int);

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &mapEntry;
    specializationInfo.dataSize = sizeof(uint32_t);
    specializationInfo.pData = &data;

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = &specializationInfo;

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMapEntrySize) {
    TEST_DESCRIPTION("Create shader with specialization map entry out of bounds.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationInfo-pMapEntries-00774");

    RETURN_IF_SKIP(InitBasicShaderObject())

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry mapEntry = {};
    mapEntry.constantID = 0u;
    mapEntry.offset = sizeof(int) / 2;
    mapEntry.size = sizeof(int);

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &mapEntry;
    specializationInfo.dataSize = sizeof(uint32_t);
    specializationInfo.pData = &data;

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = &specializationInfo;

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMismatch) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");

    RETURN_IF_SKIP(InitBasicShaderObject())

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data[2] = {0u, 0u};

    VkSpecializationMapEntry mapEntry = {};
    mapEntry.constantID = 0u;
    mapEntry.offset = 0u;
    mapEntry.size = sizeof(int) * 2u;

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &mapEntry;
    specializationInfo.dataSize = sizeof(uint32_t) * 2;
    specializationInfo.pData = &data;

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = &specializationInfo;

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationSameConstantId) {
    TEST_DESCRIPTION("Create shader with non unique specialization map entries.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationInfo-constantID-04911");

    RETURN_IF_SKIP(InitBasicShaderObject())

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry mapEntries[2] = {};
    mapEntries[0].constantID = 0u;
    mapEntries[0].offset = 0u;
    mapEntries[0].size = sizeof(int);
    mapEntries[1].constantID = 0u;
    mapEntries[1].offset = 0u;
    mapEntries[1].size = sizeof(int);

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 2;
    specializationInfo.pMapEntries = mapEntries;
    specializationInfo.dataSize = sizeof(uint32_t);
    specializationInfo.pData = &data;

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = &specializationInfo;

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingEntrypoint) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pName-08440");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "invalid";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08460");

    RETURN_IF_SKIP(InitBasicShaderObject())

    // Size an array using a specialization constant of default value equal to 1.
    const char* fs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %size "size"
               OpName %array "array"
               OpDecorate %size SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
       %size = OpSpecConstant %int 1
%_arr_float_size = OpTypeArray %float %size
%_ptr_Function__arr_float_size = OpTypePointer Function %_arr_float_size
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
       %main = OpFunction %void None %3
          %5 = OpLabel
      %array = OpVariable %_ptr_Function__arr_float_size Function
         %15 = OpAccessChain %_ptr_Function_float %array %int_0
               OpStore %15 %float_0
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> fs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, fs_src, fs_spv);

    uint32_t data = 0u;

    VkSpecializationMapEntry mapEntry = {};
    mapEntry.constantID = 0u;
    mapEntry.offset = 0u;
    mapEntry.size = sizeof(uint32_t);

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &mapEntry;
    specializationInfo.dataSize = sizeof(uint32_t);
    specializationInfo.pData = &data;

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = fs_spv.size() * sizeof(fs_spv[0]);
    createInfo.pCode = fs_spv.data();
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = &specializationInfo;

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MinTexelGatherOffset) {
    TEST_DESCRIPTION("Create shader with texel gather offset lower than minimum.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImage-06377");

    RETURN_IF_SKIP(InitBasicShaderObject())

    // Size an array using a specialization constant of default value equal to 1.
    const char* cs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450

               ; Annotations
               OpDecorate %samp DescriptorSet 0
               OpDecorate %samp Binding 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %samp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %17 = OpConstantComposite %v2float %float_0_5 %float_0_5
              ; set up composite to be validated
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2uint %uint_0 %uint_n100

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
      %color = OpVariable %_ptr_Function_v4float Function
         %14 = OpLoad %11 %samp
               ; Should trigger min and max
         %24 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_100
               ; Should only trigger max since uint
         %25 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_n100
               OpStore %color %24
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> cs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, cs_src, cs_spv);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = cs_spv.size() * sizeof(cs_spv[0]);
    createInfo.pCode = cs_spv.data();
    createInfo.pName = "main";
    createInfo.setLayoutCount = 1u;
    createInfo.pSetLayouts = &descriptor_set.layout_.handle();

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, UnsupportedSpirvCapability) {
    TEST_DESCRIPTION("Create shader with unsupported spirv capability.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08740");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_0, false))

    const char* vs_src = R"(
               OpCapability Shader
               OpCapability ClipDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_2 = OpConstant %int 2
%_ptr_Output_float = OpTypePointer Output %float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
         %22 = OpAccessChain %_ptr_Output_float %_ %int_2 %int_0
               OpStore %22 %float_1
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> vs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, vs_src, vs_spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = vs_spv.size() * sizeof(vs_spv[0]);
    createInfo.pCode = vs_spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, UnsupportedSpirvExtension) {
    TEST_DESCRIPTION("Create shader with unsupported spirv extension.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08741");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_0, false))

    const char* vs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource GLSL 450
               OpExtension "GL_EXT_scalar_block_layout"
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> vs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, vs_src, vs_spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = vs_spv.size() * sizeof(vs_spv[0]);
    createInfo.pCode = vs_spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpirvExtensionRequirementsNotMet) {
    TEST_DESCRIPTION("Create shader with extension requirements not met.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08742");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_0, false))

    const char* cs_src = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd)";

    std::vector<uint32_t> cs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, cs_src, cs_spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = cs_spv.size() * sizeof(cs_spv[0]);
    createInfo.pCode = cs_spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MemoryModelNotEnabled) {
    TEST_DESCRIPTION("Create shader with unsupported spirv extension.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-vulkanMemoryModel-06265");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper();
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper(&shaderObjectFeatures);
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    features12.vulkanMemoryModelDeviceScope = VK_FALSE;
    if (features12.vulkanMemoryModel == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModel feature is not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    char const* cs_src = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
	   }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MaxTransformFeedbackStream) {
    TEST_DESCRIPTION("Test maxTransformFeedbackStream with shader objects.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpEmitStreamVertex-06310");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&transform_feedback_features))
    auto features2 = GetPhysicalDeviceFeatures2(transform_feedback_features);
    if (features2.features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported";
    }
    if (!transform_feedback_features.transformFeedback || !transform_feedback_features.geometryStreams) {
        GTEST_SKIP() << "transformFeedback or geometryStreams feature is not supported";
    }

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    // seen sometimes when using profiles and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "maxTransformFeedbackStreams is zero";
    }

    std::stringstream gsSource;
    gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %tf
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 1

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %10

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf Stream 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
     %int_17 = OpConstant %int )asm";
    gsSource << transform_feedback_props.maxTransformFeedbackStreams;
    gsSource << R"asm(
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_17
               OpReturn
               OpFunctionEnd
        )asm";

    std::vector<uint32_t> gs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, gsSource.str().c_str(), gs_spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = gs_spv.size() * sizeof(gs_spv[0]);
    createInfo.pCode = gs_spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TransformFeedbackStride) {
    TEST_DESCRIPTION("Test maxTransformFeedbackStream with shader objects.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-XfbStride-06313");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&transform_feedback_features))
    auto features2 = GetPhysicalDeviceFeatures2(transform_feedback_features);
    if (features2.features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported";
    }
    if (!transform_feedback_features.transformFeedback || !transform_feedback_features.geometryStreams) {
        GTEST_SKIP() << "transformFeedback or geometryStreams feature is not supported";
    }

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    // seen sometimes when using profiles and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "maxTransformFeedbackStreams is zero";
    }

    std::stringstream vsSource;
    vsSource << R"asm(
               OpCapability Shader
               OpCapability TransformFeedback
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %tf
               OpExecutionMode %main Xfb

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %8

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride )asm";
    vsSource << transform_feedback_props.maxTransformFeedbackBufferDataStride + 4;
    vsSource << R"asm(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )asm";

    std::vector<uint32_t> vs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, vsSource.str().c_str(), vs_spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = vs_spv.size() * sizeof(vs_spv[0]);
    createInfo.pCode = vs_spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshOutputVertices) {
    TEST_DESCRIPTION("Create mesh shader with output vertices higher than max.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-MeshEXT-07115");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_1, false, true));

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    std::string mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices )";
    mesh_src += std::to_string(mesh_shader_properties.maxMeshOutputVertices + 1);
    mesh_src += R"(
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, mesh_src.c_str(), spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfo.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, Atomics) {
    TEST_DESCRIPTION("Test atomics with shader objects.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-None-06278");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkPhysicalDeviceFeatures available_features = {};
    GetPhysicalDeviceFeatures(&available_features);
    if (!available_features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 is not supported";
    }

    std::string cs_src = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_atomic_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
           atomicAdd(y, 1);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src.c_str());

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ExtendedTypesDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-None-06275");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMaintenance4Features maintenance4 = vku::InitStructHelper();
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper(&maintenance4);
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR extended_types_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&extended_types_features);
    GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    extended_types_features.shaderSubgroupExtendedTypes = VK_FALSE;
    if (shaderObjectFeatures.shaderObject == VK_FALSE) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    VkPhysicalDeviceSubgroupProperties subgroup_prop = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_prop);
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) || !float16_features.shaderFloat16 ||
        !maintenance4.maintenance4) {
        GTEST_SKIP() << "Required features not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &shaderObjectFeatures));

    char const* cs_src = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-shaderSubgroupClock-06267");

    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())

    char const* vs_src = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, WriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, cs_src, spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LocalSizeIdExecutionMode) {
    TEST_DESCRIPTION("Test LocalSizeId spirv execution mode.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-LocalSizeId-06434");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* cs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, cs_src, spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ZeroInitializeWorkgroupMemory) {
    TEST_DESCRIPTION("Test initializing workgroup memory in compute shader.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* cs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
  %zero_uint = OpConstantNull %uint
    %counter = OpVariable %_ptr_Workgroup_uint Workgroup %zero_uint
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, cs_src, spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingNonReadableDecorationFormatRead) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format not marked as non readable.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-apiVersion-07954");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-apiVersion-07955");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_1, false))

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    const char* cs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "value"
               OpName %12 "img"
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %22 BuiltIn WorkgroupSize
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 2 Unknown
         %11 = OpTypePointer UniformConstant %10
         %12 = OpVariable %11 UniformConstant
         %14 = OpTypeInt 32 1
         %15 = OpTypeVector %14 2
         %16 = OpConstant %14 0
         %17 = OpConstantComposite %15 %16 %16
         %19 = OpTypeInt 32 0
         %20 = OpTypeVector %19 3
         %21 = OpConstant %19 1
         %22 = OpConstantComposite %20 %21 %21 %21
          %4 = OpFunction %2 None %3
          %l = OpLabel
          %9 = OpVariable %8 Function
               OpReturn
               OpFunctionEnd
        )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, cs_src, spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08451");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    if (!shaderObjectFeatures.shaderObject) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    // Set limit to match with hardcoded values in shaders
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxSampleMaskWords = 3;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    char const* fs_src = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           int x = gl_SampleMaskIn[2];
           int y = gl_SampleMaskIn[0];
           uFragColor = vec4(0,1,0,1) * x * y;
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_src, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ConservativeRasterizationPostDepthCoverage) {
    TEST_DESCRIPTION("Make sure conservativeRasterizationPostDepthCoverage is set if needed.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235");

    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_2))

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);
    if (conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        GTEST_SKIP() << "conservativeRasterizationPostDepthCoverage not supported";
    }

    const char* fs_src = R"(
               OpCapability Shader
               OpCapability SampleMaskPostDepthCoverage
               OpCapability FragmentFullyCoveredEXT
               OpExtension "SPV_EXT_fragment_fully_covered"
               OpExtension "SPV_KHR_post_depth_coverage"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12
               OpExecutionMode %4 OriginUpperLeft
               OpExecutionMode %4 EarlyFragmentTests
               OpExecutionMode %4 PostDepthCoverage
               OpDecorate %12 BuiltIn FullyCoveredEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
         %12 = OpVariable %_ptr_Input_bool Input
          %4 = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, fs_src, spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LocalSizeExceedLimits) {
    TEST_DESCRIPTION("Create shader where local size exceeds limits.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06429");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06432");

    RETURN_IF_SKIP(InitBasicShaderObject())

    uint32_t x_count_limit = m_device->phy().limits_.maxComputeWorkGroupCount[0];

    std::string cs_src = R"asm(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 44 1 1

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4

               ; Annotations
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
    %uint_44 = OpConstant %uint )asm";
    cs_src += std::to_string(x_count_limit);
    cs_src += R"asm(
               %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_44 %uint_1 %uint_1

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)asm";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, cs_src.c_str(), spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLineWidthSet) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line width dynamic state.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08619");

    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }
    InitDynamicRenderTarget();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader geomShader(*m_device, stages[1], GLSLToSPV(stages[1], geom_src));
    const vkt::Shader fragShader(*m_device, stages[2], GLSLToSPV(stages[2], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_WIDTH});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stages[1], &geomShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidViewportCount) {
    TEST_DESCRIPTION("Draw with a shader that uses PrimitiveShadingRateKHR with invalid viewport count.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-08642");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&fsr_features))
    if (!fsr_features.pipelineFragmentShadingRate) {
        GTEST_SKIP() << "pipelineFragmentShadingRate not supported.";
    }
    InitDynamicRenderTarget();

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);
    if (fsr_properties.primitiveFragmentShadingRateWithMultipleViewports) {
        GTEST_SKIP() << "required primitiveFragmentShadingRateWithMultipleViewports to be unsupported.";
    }

    char const* vsSource = R"glsl(
            #version 450
            #extension GL_EXT_fragment_shading_rate : enable
            void main() {
                gl_PrimitiveShadingRateEXT = gl_ShadingRateFlag4VerticalPixelsEXT | gl_ShadingRateFlag4HorizontalPixelsEXT;
            }
        )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], vsSource));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 2u, scissors);
    VkExtent2D fragmentSize = {1u, 1u};
    VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};
    vk::CmdSetFragmentShadingRateKHR(m_commandBuffer->handle(), &fragmentSize, combinerOps);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, AlphaToCoverage) {
    TEST_DESCRIPTION("Draw with fragment shader missing alpha to coverage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-alphaToCoverageEnable-08920");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 1) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0,1,0,1);
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], frag_src));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_WIDTH});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdSetAlphaToCoverageEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLineRasterizationMode) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line rasterization mode dynamic state.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08668");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }
    InitDynamicRenderTarget();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader geomShader(*m_device, stages[1], GLSLToSPV(stages[1], geom_src));
    const vkt::Shader fragShader(*m_device, stages[2], GLSLToSPV(stages[2], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stages[1], &geomShader.handle());
    vk::CmdSetLineStippleEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLineStippleEnable) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line stipple enable dynamic state.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08671");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }
    InitDynamicRenderTarget();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader geomShader(*m_device, stages[1], GLSLToSPV(stages[1], geom_src));
    const vkt::Shader fragShader(*m_device, stages[2], GLSLToSPV(stages[2], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT});
    BindVertFragShader(vertShader, fragShader);
    vk::CmdSetLineRasterizationModeEXT(m_commandBuffer->handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stages[1], &geomShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidColorWriteMask) {
    TEST_DESCRIPTION("Draw with invalid color write mask.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-09116");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkFormat format = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 not supported as color attachment.";
    }

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = vku::InitStructHelper();
    imageFormatInfo.format = format;
    imageFormatInfo.type = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageFormatInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageFormatProperties2 imageFormatProperties = vku::InitStructHelper();
    auto res = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy(), &imageFormatInfo, &imageFormatProperties);
    if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "image format not supported as color attachment.";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(format);

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(image_view);
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0u, 1u, &colorWriteMask);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, Mismatched64BitAttributeType) {
    TEST_DESCRIPTION("Draw with vertex format not matching vertex input format.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-format-08936");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const VkFormat format = VK_FORMAT_R64_SINT;

    VkFormatProperties2 formatProperties = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(m_device->phy(), format, &formatProperties);

    if ((formatProperties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "format not supported.";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        layout(location = 0) in int pos;
        void main() {
           gl_Position = vec4(pos);
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], vert_src));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);

    VkVertexInputBindingDescription2EXT vertexBindingDescription = vku::InitStructHelper();
    vertexBindingDescription.binding = 0u;
    vertexBindingDescription.stride = 16u;
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexBindingDescription.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertexAttributeDescription = vku::InitStructHelper();
    vertexAttributeDescription.location = 0u;
    vertexAttributeDescription.binding = 0u;
    vertexAttributeDescription.format = format;
    vertexAttributeDescription.offset = 0u;

    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1u, &vertexBindingDescription, 1u, &vertexAttributeDescription);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, Mismatched32BitAttributeType) {
    TEST_DESCRIPTION("Draw with vertex format not matching vertex input format.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-format-08937");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(location = 0) in int64_t pos;
        void main() {
           gl_Position = vec4(pos);
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], vert_src));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);

    VkVertexInputBindingDescription2EXT vertexBindingDescription = vku::InitStructHelper();
    vertexBindingDescription.binding = 0u;
    vertexBindingDescription.stride = 16u;
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexBindingDescription.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertexAttributeDescription = vku::InitStructHelper();
    vertexAttributeDescription.location = 0u;
    vertexAttributeDescription.binding = 0u;
    vertexAttributeDescription.format = VK_FORMAT_R32_SINT;
    vertexAttributeDescription.offset = 0u;

    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1u, &vertexBindingDescription, 1u, &vertexAttributeDescription);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedFormat64Components) {
    TEST_DESCRIPTION("Draw with vertex format components not matching vertex input format components.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-09203");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    const VkFormat format = VK_FORMAT_R64G64B64_SINT;

    VkFormatProperties2 formatProperties = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(m_device->phy(), format, &formatProperties);

    if ((formatProperties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "format not supported.";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(location = 0) in i64vec4 pos;
        void main() {
           gl_Position = vec4(pos.xy, pos.xy);
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], vert_src));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);

    VkVertexInputBindingDescription2EXT vertexBindingDescription = vku::InitStructHelper();
    vertexBindingDescription.binding = 0u;
    vertexBindingDescription.stride = 16u;
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexBindingDescription.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertexAttributeDescription = vku::InitStructHelper();
    vertexAttributeDescription.location = 0u;
    vertexAttributeDescription.binding = 0u;
    vertexAttributeDescription.format = format;
    vertexAttributeDescription.offset = 0u;

    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1u, &vertexBindingDescription, 1u, &vertexAttributeDescription);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorNotUpdated) {
    TEST_DESCRIPTION("Draw with shaders using a descriptor set that was never updated.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08114");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    OneOffDescriptorSet frag_descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_, &frag_descriptor_set.layout_});

    static const char vert_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec2 uv;
        layout(set = 0, binding = 0) buffer Buffer {
            vec4 pos;
        } buf;
        void main() {
            uv = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
            gl_Position = vec4(buf.pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(set = 1, binding = 0) uniform sampler2D s;
        layout(location = 0) in vec2 uv;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(s, uv);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_.handle(), frag_descriptor_set.layout_.handle()};

    VkShaderCreateInfoEXT vert_create_info = vku::InitStructHelper();
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vert_create_info.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vert_create_info.pCode = vert_spv.data();
    vert_create_info.pName = "main";
    vert_create_info.setLayoutCount = 2u;
    vert_create_info.pSetLayouts = descriptor_set_layouts;

    VkShaderCreateInfoEXT frag_create_info = vku::InitStructHelper();
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    frag_create_info.codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    frag_create_info.pCode = frag_spv.data();
    frag_create_info.pName = "main";
    frag_create_info.setLayoutCount = 2u;
    frag_create_info.pSetLayouts = descriptor_set_layouts;

    const vkt::Shader vertShader(*m_device, vert_create_info);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1u, 1u,
                              &frag_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ComputeVaryingAndFullSubgroups) {
    TEST_DESCRIPTION("Dispatch with compute shader using required full subgroups and allow varying subgroup size flags.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08416");

    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    VkPhysicalDeviceSubgroupSizeControlFeatures subgroup_size_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&subgroup_size_features));
    if (::testing::Test::IsSkipped()) return;
    if (!subgroup_size_features.subgroupSizeControl || !subgroup_size_features.computeFullSubgroups) {
        GTEST_SKIP() << "subgroupSizeControl or computeFullSubgroups not supported.";
    }

    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_size_control_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_size_control_properties);

    std::string comp_src = R"glsl(
        #version 460
        layout(local_size_x = )glsl";
    comp_src += std::to_string(subgroup_size_control_properties.maxSubgroupSize + 1u);
    comp_src += R"glsl() in;
        void main() {}
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src.c_str());

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT | VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ComputeVaryingSubgroups) {
    TEST_DESCRIPTION("Dispatch with compute shader using required full subgroups and allow varying subgroup size flags.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08417");

    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    VkPhysicalDeviceSubgroupSizeControlFeatures subgroup_size_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&subgroup_size_features, VK_API_VERSION_1_2));
    if (!subgroup_size_features.computeFullSubgroups) {
        GTEST_SKIP() << "computeFullSubgroups not supported.";
    }

    VkPhysicalDeviceVulkan11Properties properties11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(properties11);

    std::string comp_src = R"glsl(
        #version 460
        layout(local_size_x = )glsl";
    comp_src += std::to_string(properties11.subgroupSize + 1u);
    comp_src += R"glsl() in;
        void main() {}
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src.c_str());

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryShaderMaxOutputVertices) {
    TEST_DESCRIPTION("Create geometry shader with output vertices higher than maximum.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08454");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }

    std::string geom_src = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices )";
    geom_src += std::to_string(m_device->phy().limits_.maxGeometryOutputVertices + 1);
    geom_src += R"(
               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpEmitVertex
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, geom_src.c_str(), spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryShaderMaxInvocations) {
    TEST_DESCRIPTION("Create geometry shader with invocations higher than maximum.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-pCode-08455");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "geometryShader not supported.";
    }

    std::string geom_src = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations )";
    geom_src += std::to_string(m_device->phy().limits_.maxGeometryShaderInvocations + 1);
    geom_src += R"(
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 2
               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpEmitVertex
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, geom_src.c_str(), spv);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingImageFilterLinearBit) {
    TEST_DESCRIPTION("Draw with shaders sampling from an image which does not have required filter linear bit.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-magFilter-04553");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkFormat format = VK_FORMAT_R16_SINT;
    VkFormatProperties props = {};
    vk::GetPhysicalDeviceFormatProperties(gpu(), format, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) > 0 ||
        (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        GTEST_SKIP() << "Required image features not supported.";
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    static const char frag_src[] = R"glsl(
        #version 460
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl),
                                 &descriptor_set.layout_.handle());

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src),
                                 &descriptor_set.layout_.handle());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view = image.targetView(format);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.compareEnable = VK_FALSE;
    vkt::Sampler sampler(*m_device, sampler_info);

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MaxMultiviewInstanceIndex) {
    TEST_DESCRIPTION("Draw with a read only depth stencil attachment and invalid stencil op.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&features11, VK_API_VERSION_1_2))

    if (!features11.multiview) {
        GTEST_SKIP() << "multiview not supported.";
    }

    InitDynamicRenderTarget();

    VkPhysicalDeviceMultiviewProperties multiview_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_properties);

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    VkImageObj img(m_device);
    img.Init(m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView view = img.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 1u;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.viewMask = 0x1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, multiview_properties.maxMultiviewInstanceIndex);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5208
TEST_F(NegativeShaderObject, DISABLED_MaxFragmentDualSrcAttachmentsDynamicBlendEnable) {
    TEST_DESCRIPTION(
        "Test drawing with dual source blending with too many fragment output attachments, but using dynamic blending.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    InitDynamicRenderTarget();

    uint32_t count = m_device->phy().limits_.maxFragmentDualSrcAttachments + 1;

    std::stringstream fsSource;
    fsSource << "#version 450\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "layout(location = " << i << ") out vec4 c" << i << ";\n";
    }
    fsSource << " void main() {\n";
    for (uint32_t i = 0; i < count; ++i) {
        fsSource << "c" << i << " = vec4(0.0f);\n";
    }
    fsSource << "}";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fsSource.str().c_str()));

    VkImageObj img1(m_device);
    img1.Init(m_width, m_height, 1, m_render_target_fmt,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);
    VkImageObj img2(m_device);
    img2.Init(m_width, m_height, 1, m_render_target_fmt,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);

    VkImageView view1 = img1.targetView(m_render_target_fmt);
    VkImageView view2 = img2.targetView(m_render_target_fmt);

    VkRenderingAttachmentInfo attachments[2];
    attachments[0] = vku::InitStructHelper();
    attachments[0].imageView = view1;
    attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1] = vku::InitStructHelper();
    attachments[1].imageView = view2;
    attachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 2u;
    renderingInfo.pColorAttachments = attachments;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);

    SetDefaultDynamicStates();

    VkBool32 color_blend_enabled[2] = {VK_TRUE, VK_FALSE};
    VkColorBlendEquationEXT color_blend_equation = {
        VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    VkColorComponentFlags color_component_flags = VK_COLOR_COMPONENT_R_BIT;

    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &color_blend_enabled[0]);  // enables
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 1, 1, &color_blend_enabled[1]);
    vk::CmdSetColorBlendEquationEXT(m_commandBuffer->handle(), 0, 1, &color_blend_equation);
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0, 1, &color_component_flags);

    VkColorBlendAdvancedEXT colorBlendAdvanced;
    colorBlendAdvanced.advancedBlendOp = VK_BLEND_OP_ADD;
    colorBlendAdvanced.srcPremultiplied = VK_FALSE;
    colorBlendAdvanced.dstPremultiplied = VK_FALSE;
    colorBlendAdvanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
    colorBlendAdvanced.clampResults = VK_FALSE;
    vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendAdvanced);
    vk::CmdSetColorBlendAdvancedEXT(m_commandBuffer->handle(), 1u, 1u, &colorBlendAdvanced);

    BindVertFragShader(vertShader, fragShader);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-Fragment-06427");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // disables blending so no error should appear
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &color_blend_enabled[1]);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(NegativeShaderObject, PrimitivesGeneratedQuery) {
    TEST_DESCRIPTION("Draw with primitives generated query.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT pgq_features = vku::InitStructHelper();

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&pgq_features);
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_object_features);

    pgq_features.primitivesGeneratedQueryWithRasterizerDiscard = VK_FALSE;

    if (!shader_object_features.shaderObject) {
        GTEST_SKIP() << "shaderObject not supported.";
    }
    if (!pgq_features.primitivesGeneratedQuery) {
        GTEST_SKIP() << "primitivesGeneratedQuery not supported.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryCount = 1;
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, CooperativeMatrix) {
    TEST_DESCRIPTION("Test cooperative matrix with shader objects");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-08974");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);

    RETURN_IF_SKIP(InitBasicShaderObject(&memory_model_features))

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    // Tests are assume that Float16 3*5 is not available
    char const* comp_src = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator> badSize = coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator>(float16_t(0.0));
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationSubdivision) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different subdivision.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08867");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* tesc_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main Quads

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

    const char* tese_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCw

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> tesc_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src, tesc_spv);
    std::vector<uint32_t> tese_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, tese_spv);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[0].pCode = tesc_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[1].pCode = tese_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationOrientation) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different orientations.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08868");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* tesc_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main VertexOrderCcw

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

    const char* tese_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCw

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> tesc_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src, tesc_spv);
    std::vector<uint32_t> tese_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, tese_spv);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[0].pCode = tesc_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[1].pCode = tese_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationPointMode) {
    TEST_DESCRIPTION("Create linked tessellation control with point mode and evaluation shader without.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08869");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* tesc_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main PointMode

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

    const char* tese_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> tesc_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src, tesc_spv);
    std::vector<uint32_t> tese_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, tese_spv);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[0].pCode = tesc_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[1].pCode = tese_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationSpacing) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different spacing.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08870");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* tesc_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main SpacingFractionalEven

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

    const char* tese_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCw

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> tesc_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src, tesc_spv);
    std::vector<uint32_t> tese_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, tese_spv);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[0].pCode = tesc_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[1].pCode = tese_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationOutputPatchSize) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different output patch sizes.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateShadersEXT-pCreateInfos-08871");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const char* tesc_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 4

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

    const char* tese_src = R"(
			   OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCw
               OpExecutionMode %main OutputVertices 3

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> tesc_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src, tesc_spv);
    std::vector<uint32_t> tese_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, tese_spv);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[0].pCode = tesc_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[1].pCode = tese_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingSubgroupSizeControlFeature) {
    TEST_DESCRIPTION("Create shader with invalid flags when subgroupSizeControl is not enabled.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-AllowVaryingSubgroupSize");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingComputeFullSubgroups) {
    TEST_DESCRIPTION("Create shader with invalid flags when computeFullSubgroups is not enabled.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-Shader-RequireFullSubgroups");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    static const char comp_source[] = R"glsl(
        #version 460
        layout(local_size_x = 32) in;
        void main() {}
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_source);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}
