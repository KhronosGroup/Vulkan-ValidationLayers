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

TEST_F(NegativeShaderObject, SpirvCodeSize) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-codeSize-08735");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08992");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

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
    TEST_DESCRIPTION("Create shader with invalid flags.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-flags-08414");

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.taskShader == VK_FALSE) {
        GTEST_SKIP() << "taskShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.taskShader == VK_FALSE) {
        GTEST_SKIP() << "taskShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.meshShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.taskShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_NV, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.meshShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_NV, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    std::vector<uint32_t> spv(256);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(unsigned char);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(unsigned char);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
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

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.taskShader == VK_FALSE) {
        GTEST_SKIP() << "taskShader not supported.";
    }

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto task_spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT vertCreateInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vertCreateInfo.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vertCreateInfo.pCode = vert_spv.data();
    vertCreateInfo.pName = "main";

    VkShaderCreateInfoEXT taskCreateInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    taskCreateInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    taskCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    taskCreateInfo.codeSize = task_spv.size() * sizeof(task_spv[0]);
    taskCreateInfo.pCode = task_spv.data();
    taskCreateInfo.pName = "main";

    vk_testing::Shader vert_shader(*m_device, vertCreateInfo);
    vk_testing::Shader task_shader(*m_device, taskCreateInfo);
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

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.meshShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT vertCreateInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vertCreateInfo.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vertCreateInfo.pCode = vert_spv.data();
    vertCreateInfo.pName = "main";

    VkShaderCreateInfoEXT meshCreateInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    meshCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    meshCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    meshCreateInfo.codeSize = mesh_spv.size() * sizeof(mesh_spv[0]);
    meshCreateInfo.pCode = mesh_spv.data();
    meshCreateInfo.pName = "main";

    vk_testing::Shader vert_shader(*m_device, vertCreateInfo);
    vk_testing::Shader mesh_shader(*m_device, meshCreateInfo);
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

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    auto shaderObjectFeatures = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    features2.features.tessellationShader = VK_FALSE;
    features2.features.geometryShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08419");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

        VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vk_testing::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08420");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

        VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vk_testing::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, CreateMeshShadersWithoutEnabledFeatures) {
    TEST_DESCRIPTION("Create mesh and task shaders without features enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    auto shaderObjectFeatures = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>();
    GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &shaderObjectFeatures));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08421");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

        VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vk_testing::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderCreateInfoEXT-stage-08422");

        const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

        VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
        createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv.size() * sizeof(spv[0]);
        createInfo.pCode = spv.data();
        createInfo.pName = "main";

        vk_testing::Shader shader(*m_device, createInfo);

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, BindTessellationAndGeometryShadersWithoutEnabledFeature) {
    TEST_DESCRIPTION("Bind tessellation and geometry shaders without tessellationShader feature enabled.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08474");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    auto shaderObjectFeatures = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    features2.features.tessellationShader = VK_FALSE;
    features2.features.geometryShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    // Binding vertex shader as tessellation control shader to get around VUID-VkShaderCreateInfoEXT-stage-08419
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);

    m_errorMonitor->VerifyFound();

    // Binding vertex shader as geometry shader to get around VUID-VkShaderCreateInfoEXT-stage-08420
    stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08475");
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ComputeShaderNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use compute shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08476");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_COMPUTE_BIT);

    if (!graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkCommandPoolObj command_pool(m_device, graphics_queue_family_index.value());
    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GraphicsShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use graphics shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08477");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkCommandPoolObj command_pool(m_device, non_graphics_queue_family_index.value());
    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GraphicsMeshShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use mesh shaders with unsupported command pool.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08478");

    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    auto meshShaderFeatures = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    InitBasicShaderObject(&meshShaderFeatures, VK_API_VERSION_1_2);
    if (::testing::Test::IsSkipped()) return;
    if (meshShaderFeatures.meshShader == VK_FALSE) {
        GTEST_SKIP() << "meshShader not supported.";
    }

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyMatching(0u, VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, "main", nullptr, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkCommandPoolObj command_pool(m_device, non_graphics_queue_family_index.value());
    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &createInfo.stage, &shaderHandle);
    command_buffer.end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindTaskAndMeshShadersWithoutEnabledFeature) {
    TEST_DESCRIPTION("Bind task and mesh shaders without tessellationShader feature enabled.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08490");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    auto shaderObjectFeatures = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>();
    GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &shaderObjectFeatures));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    // Binding vertex shader as tessellation control shader to get around create shaders VUID
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_TASK_BIT_EXT;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);

    m_errorMonitor->VerifyFound();

    // Binding vertex shader as geometry shader to get around create shaders VUID
    stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pShaders-08491");
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, NonUniqueShadersBind) {
    TEST_DESCRIPTION("Bind multiple shaders with same stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08463");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader1(*m_device, createInfo);
    vk_testing::Shader shader2(*m_device, createInfo);

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08464");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL_GRAPHICS;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidRayTracingShaderStageBind) {
    TEST_DESCRIPTION("Bind shader with invalid stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08465");

    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidHuaweiShaderStageBind) {
    TEST_DESCRIPTION("Bind shader with invalid stage.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08467");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadersEXT-pStages-08468");

    stage = VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shaderHandle);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GetShaderBinaryDataInvalidPointer) {
    TEST_DESCRIPTION("Get shader binary data with invalid pointer.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetShaderBinaryDataEXT-None-08499");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    vk_testing::Shader shader(*m_device, createInfo);
    VkShaderEXT shaderHandle = shader.handle();

    size_t dataSize = 0;
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, nullptr);
    std::vector<uint8_t> data(dataSize + 1u);
    auto ptr = reinterpret_cast<std::uintptr_t>(data.data()) + sizeof(unsigned char);
    void* dataPtr = reinterpret_cast<void*>(ptr);
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, dataPtr);

    m_errorMonitor->VerifyFound();
}
