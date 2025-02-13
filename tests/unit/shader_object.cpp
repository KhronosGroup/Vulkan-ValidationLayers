/*
 * Copyright (c) 2023-2025 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/shader_helper.h"

class NegativeShaderObject : public ShaderObjectTest {};

TEST_F(NegativeShaderObject, SpirvCodeSize) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]) - 2u;
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeSize-08735");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedComputeShader) {
    TEST_DESCRIPTION("Create compute shader with linked flag.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08412");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidFlags) {
    TEST_DESCRIPTION("Create shader with invalid flags.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08992");
    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08485");
    create_info.flags = VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08486");
    create_info.flags = VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08488");
    create_info.flags = VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidMeshShaderExtFlags) {
    TEST_DESCRIPTION("Create mesh shader with invalid flags.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08414");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, VertexNextStage) {
    TEST_DESCRIPTION("Create vertex shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08427");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TessellationControlNextStage) {
    TEST_DESCRIPTION("Create tessellation control shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    create_info.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08430");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TessellationEvaluationNextStage) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    create_info.nextStage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08431");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryNextStage) {
    TEST_DESCRIPTION("Create geometry shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    create_info.nextStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08433");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, FragmentNextStage) {
    TEST_DESCRIPTION("Create fragment shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.nextStage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08434");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TaskNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_2));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    create_info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08435");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_2));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    create_info.nextStage = VK_SHADER_STAGE_COMPUTE_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08436");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TaskNVNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid next stage.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_2));

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_NV, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_TASK_BIT_NV;
    create_info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08435");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshNVNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid next stage.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features_nv = vku::InitStructHelper();
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&mesh_shader_features_nv);
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_object_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    if (mesh_shader_features_nv.meshShader == VK_FALSE) {
        GTEST_SKIP() << "shadingRateImage not supported.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_NV, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_MESH_BIT_NV;
    create_info.nextStage = VK_SHADER_STAGE_COMPUTE_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = spv.data();
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08436");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BinaryCodeAlignment) {
    TEST_DESCRIPTION("Create binary shader with invalid binary code alignment.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    std::vector<uint32_t> spv(256);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(uint8_t);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = reinterpret_cast<void*>(ptr);
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08492");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpirvCodeAlignment) {
    TEST_DESCRIPTION("Create shader with invalid binary code alignment.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    auto ptr = reinterpret_cast<std::uintptr_t>(spv.data()) + sizeof(uint8_t);

    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spv.size() * sizeof(spv[0]);
    create_info.pCode = reinterpret_cast<void*>(ptr);
    create_info.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08493");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidStage) {
    TEST_DESCRIPTION("Create shader with invalid stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderCreateInfoEXT-stage-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08425");
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderCreateInfoEXT-stage-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08426");

    createInfo.stage = VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindVertexAndTaskShaders) {
    TEST_DESCRIPTION("Bind vertex and task shaders in the same call.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto task_spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);

    vkt::Shader vert_shader(*m_device, ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT));
    vkt::Shader task_shader(*m_device, ShaderCreateInfo(task_spv, VK_SHADER_STAGE_TASK_BIT_EXT));
    VkShaderEXT shaders[] = {
        vert_shader.handle(),
        task_shader.handle(),
    };

    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_TASK_BIT_EXT,
    };
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08470");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 2u, stages, shaders);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, BindVertexAndMeshShaders) {
    TEST_DESCRIPTION("Bind vertex and mesh shaders in the same call.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);

    vkt::Shader vert_shader(*m_device, ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT));
    vkt::Shader mesh_shader(*m_device, ShaderCreateInfo(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT));
    VkShaderEXT shaders[] = {
        vert_shader.handle(),
        mesh_shader.handle(),
    };

    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_MESH_BIT_EXT,
    };
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08471");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 2u, stages, shaders);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, CreateShadersWithoutEnabledFeatures) {
    TEST_DESCRIPTION("Create tessellation shader without tessellationShader feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);

    RETURN_IF_SKIP(Init());

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08419");
        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
        vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT));
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08420");
        const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
        vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_GEOMETRY_BIT));
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, CreateMeshShadersWithoutEnabledFeatures) {
    TEST_DESCRIPTION("Create mesh and task shaders without features enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08421");
        const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);
        vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_TASK_BIT_EXT));
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-stage-08422");
        const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
        vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_MESH_BIT_EXT));
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, ComputeShaderNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use compute shaders with unsupported command pool.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const std::optional<uint32_t> transfer_queue_family_index = m_device->TransferOnlyQueueFamily();
    if (!transfer_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);
    vkt::Shader shader(*m_device, create_info);

    vkt::CommandPool command_pool(*m_device, transfer_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08476");
    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &create_info.stage, &shader.handle());
    m_errorMonitor->VerifyFound();

    command_buffer.End();
}

TEST_F(NegativeShaderObject, GraphicsShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use graphics shaders with unsupported command pool.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    vkt::Shader shader(*m_device, create_info);

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08477");
    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &create_info.stage, &shader.handle());
    m_errorMonitor->VerifyFound();

    command_buffer.End();
}

TEST_F(NegativeShaderObject, GraphicsMeshShadersNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use mesh shaders with unsupported command pool.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_MESH_BIT_EXT);
    vkt::Shader shader(*m_device, create_info);

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08478");
    vk::CmdBindShadersEXT(command_buffer.handle(), 1u, &create_info.stage, &shader.handle());
    m_errorMonitor->VerifyFound();

    command_buffer.End();
}

TEST_F(NegativeShaderObject, NonUniqueShadersBind) {
    TEST_DESCRIPTION("Bind multiple shaders with same stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    vkt::Shader shader1(*m_device, create_info);
    vkt::Shader shader2(*m_device, create_info);

    VkShaderEXT shaders[] = {
        shader1.handle(),
        shader2.handle(),
    };
    VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_VERTEX_BIT,
    };

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pStages-08463");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 2u, stages, shaders);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidShaderStageBind) {
    TEST_DESCRIPTION("Bind shader with invalid stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT));
    VkShaderEXT shaderHandle = shader.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL_GRAPHICS;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08469");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pStages-08464");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &shaderHandle);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, GetShaderBinaryDataInvalidPointer) {
    TEST_DESCRIPTION("Get shader binary data with invalid pointer.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GetShaderBinaryDataEXT not implemented";
    }

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::Shader shader(*m_device, ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT));
    VkShaderEXT shaderHandle = shader.handle();

    size_t dataSize = 0;
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, nullptr);
    std::vector<uint8_t> data(dataSize + 1u);
    auto ptr = reinterpret_cast<std::uintptr_t>(data.data()) + sizeof(uint8_t);
    void* dataPtr = reinterpret_cast<void*>(ptr);

    m_errorMonitor->SetDesiredError("VUID-vkGetShaderBinaryDataEXT-None-08499");
    vk::GetShaderBinaryDataEXT(m_device->handle(), shaderHandle, &dataSize, dataPtr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithNoShadersBound) {
    TEST_DESCRIPTION("Call vkCmdDraw when there are no shaders or pipeline bound.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08684");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08688");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DrawWithMissingShaders) {
    TEST_DESCRIPTION("Draw without setting all of the shader objects.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08687");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DrawWithoutBindingMeshShadersWhenEnabled) {
    TEST_DESCRIPTION("Draw without binding all of the shader objects supported by graphics.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));

    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08689");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08690");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, FlagAttachmentFragmentShadingRate) {
    TEST_DESCRIPTION("Create shader with invalid flags.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT);
    VkShaderEXT shader;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08487");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, FlagFragmentDensityMap) {
    TEST_DESCRIPTION("Create shader with invalid flags.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT);
    VkShaderEXT shader;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08489");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLinkStageBit) {
    TEST_DESCRIPTION("Create a linked and non-linked shader in the same call.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    createInfos[1] = ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08402");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLinkStageBitMesh) {
    TEST_DESCRIPTION("Create a linked and non-linked mesh shader in the same call.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfo(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT);
    createInfos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08403");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedVertexAndMeshStages) {
    TEST_DESCRIPTION("Attempt to create linked vertex and mesh stages.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT);
    createInfos[1] = ShaderCreateInfoLink(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08404");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedTaskAndMeshNoTaskShaders) {
    TEST_DESCRIPTION("Attempt to create linked task shader and linked mesh shader with no task shader flag.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    const auto task_spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);
    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(task_spv, VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT);
    createInfos[1] = ShaderCreateInfoLink(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT);
    createInfos[1].flags |= VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08405");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingNextStage) {
    TEST_DESCRIPTION("Attempt to linked vertex and fragment shaders with missing nextStage.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT);
    createInfos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08409");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08409");
    createInfos[0].nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SameLinkedStage) {
    TEST_DESCRIPTION("Create multiple linked shaders with the same stage.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_VERTEX_BIT);
    createInfos[1] = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08410");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LinkedStagesWithDifferentCodeType) {
    TEST_DESCRIPTION("Create linked shaders with different code types.");

    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08411");

    RETURN_IF_SKIP(InitBasicShaderObject());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, GetShaderBinaryDataEXT not implemented";
    }

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[2];
    createInfos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    createInfos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
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

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);

    RETURN_IF_SKIP(Init());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoLink(spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderEXT shader;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08428");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    create_info.nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08429");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTessellationControlNextStage) {
    TEST_DESCRIPTION("Create tessellation control shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoLink(spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08430");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTessellationEvaluationNextStage) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoLink(spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08431");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidGeometryNextStage) {
    TEST_DESCRIPTION("Create geometry shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08433");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidFragmentNextStage) {
    TEST_DESCRIPTION("Create fragment shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08434");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidTaskNextStage) {
    TEST_DESCRIPTION("Create task shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08435");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidMeshNextStage) {
    TEST_DESCRIPTION("Create mesh shader with invalid nextStage.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-nextStage-08436");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, BindInvalidShaderStage) {
    TEST_DESCRIPTION("Bind shader with different stage than it was created with.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pShaders-08469");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &vert_shader.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DrawWithShadersOutsideRenderPass) {
    TEST_DESCRIPTION("Draw with shaders outside of a render pass.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    CreateMinimalShaders();

    m_command_buffer.Begin();
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderpass");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DrawWithShadersInNonDynamicRenderPass) {
    TEST_DESCRIPTION("Draw with shaders inside a non-dynamic render pass.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    CreateMinimalShaders();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view.handle());

    VkClearValue clear_value;
    clear_value.color.float32[0] = 0.25f;
    clear_value.color.float32[1] = 0.25f;
    clear_value.color.float32[2] = 0.25f;
    clear_value.color.float32[3] = 0.0f;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, &clear_value);
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08876");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, IncompatibleDescriptorSet) {
    TEST_DESCRIPTION("Bind an incompatible descriptor set.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
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

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_src, &descriptor_set.layout_.handle());
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentUniformGlsl, &descriptor_set.layout_.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, NotSettingViewportAndScissor) {
    TEST_DESCRIPTION("Draw with shader object without setting viewport and scissor.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    // not setting the viewport count also registers as a zero
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewportCount-03417");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewportCount-03419");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DifferentViewportAndScissorCount) {
    TEST_DESCRIPTION("Draw with shader object with different viewport and scissor count.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewportCount-03419");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidViewportWScaling) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetViewportWScaling.");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 2u, scissors);
    VkViewportWScalingNV viewportWScaling = {1.0f, 1.0f};
    vk::CmdSetViewportWScalingEnableNV(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetViewportWScalingNV(m_command_buffer.handle(), 0u, 1u, &viewportWScaling);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08636");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidShadingRatePaletteViewportCount) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetViewportShadingRatePaletteNV.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 2u, scissors);
    VkShadingRatePaletteEntryNV defaultShadingRatePaletteEntry = VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV;
    VkShadingRatePaletteNV shadingRatePalette;
    shadingRatePalette.shadingRatePaletteEntryCount = 1u;
    shadingRatePalette.pShadingRatePaletteEntries = &defaultShadingRatePaletteEntry;
    vk::CmdSetShadingRateImageEnableNV(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetViewportShadingRatePaletteNV(m_command_buffer.handle(), 0u, 1u, &shadingRatePalette);
    vk::CmdSetCoarseSampleOrderNV(m_command_buffer.handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08637");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetExclusiveScissorEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetExclusiveScissorEnableNV.");

    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::exclusiveScissor);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07878");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidExclusiveScissorCount) {
    TEST_DESCRIPTION("Draw with shader object with invalid viewport count in vkCmdSetExclusiveScissorNV.");

    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::exclusiveScissor);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (!DeviceExtensionSupported(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_NV_scissor_exclusive version 2";
    }
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 2u, scissors);
    VkBool32 exclusiveScissorEnable = VK_TRUE;
    vk::CmdSetExclusiveScissorEnableNV(m_command_buffer.handle(), 0u, 1u, &exclusiveScissorEnable);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07879");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizerDiscardEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetRasterizerDiscardEnable.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04876");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBiasEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetDepthBiasEnable.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04877");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetLogicOp) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetLogicOp.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::logicOp);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LOGIC_OP_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    vk::CmdSetLogicOpEnableEXT(m_command_buffer.handle(), VK_TRUE);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-logicOp-04878");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, BlendEnabledWithNonBlendableFormat) {
    TEST_DESCRIPTION("Draw with shader objects with blend enabled for attachment format that does not support blending.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_R32_UINT, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0 ||
        (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) != 0) {
        GTEST_SKIP() << "color attachment format not suitable.";
    }

    InitDynamicRenderTarget(VK_FORMAT_R32_UINT);
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 enabled = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 1, &enabled);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08643");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, RasterizationSamplesMismatch) {
    TEST_DESCRIPTION("Draw with shader objects with invalid rasterization samples in vkCmdSetRasterizationSamplesEXT().");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_2_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08644");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingColorWriteEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorWriteEnableEXT().");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::colorWriteEnable);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08646");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, ColorWriteEnableAttachmentCount) {
    TEST_DESCRIPTION("Draw with shader objects without setting color write enable for all attachments.");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::colorWriteEnable);
    RETURN_IF_SKIP(InitBasicShaderObject());

    vkt::Image img1(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img2(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::ImageView view1 = img1.CreateView();
    vkt::ImageView view2 = img2.CreateView();
    CreateMinimalShaders();

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

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 2u;
    renderingInfo.pColorAttachments = attachments;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 colorWriteEnable = VK_TRUE;
    vk::CmdSetColorWriteEnableEXT(m_command_buffer.handle(), 1u, &colorWriteEnable);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 1u, 1u, &colorWriteMask);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 1u, 1u, &colorBlendEnable);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08647");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleEnableEXT) {
    TEST_DESCRIPTION("Draw with shaders without setting vkCmdSetDiscardRectangleEnableEXT.");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07880");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDiscardRectangleModeEXT().");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitDynamicRenderTarget();
    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);
    std::vector<VkRect2D> discard_rectangles(discard_rectangle_properties.maxDiscardRectangles);
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDiscardRectangleEXT(m_command_buffer.handle(), 0u, discard_rectangles.size(), discard_rectangles.data());
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07881");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDiscardRectangleEXT().");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDiscardRectangleModeEXT(m_command_buffer.handle(), VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09236");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDiscardRectangleMaxDiscardRectangles) {
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(discard_rectangle_properties);
    if (discard_rectangle_properties.maxDiscardRectangles < 2) {
        GTEST_SKIP() << "Need at least maxDiscardRectangles of 2";
    }

    const uint32_t count = discard_rectangle_properties.maxDiscardRectangles - 1;
    std::vector<VkRect2D> discard_rectangles(count);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDiscardRectangleModeEXT(m_command_buffer.handle(), VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT);
    vk::CmdSetDiscardRectangleEXT(m_command_buffer.handle(), 0u, count, discard_rectangles.data());
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09236");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClampEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClampEnableEXT().");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::depthClamp);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07620");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetPolygonModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPolygonModeEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_POLYGON_MODE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07621");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizationSamplesEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRasterizationSamplesEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07622");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleMaskEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_SAMPLE_MASK_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07623");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetAlphaToCoverageEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAlphaToCoverageEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07624");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetAlphaToOneEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAlphaToOneEnableEXT.");

    AddRequiredFeature(vkt::Feature::alphaToOne);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07625");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetLogicOpEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLogicOpEnableEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::logicOp);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07626");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08657");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09417");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEnableEXTForActiveAttachment) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 enable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 1u, 1u, &enable);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09417");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEquationEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEquationEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEnable);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08658");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09418");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendEquationEXTActiveAttachments) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendEquationEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 1u, 1u, &colorBlendEquation);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08658");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorBlendAdvancedEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorBlendAdvancedEXT.");

    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT});
    VkBool32 colorBlendEnable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEnable);
    VkColorBlendAdvancedEXT colorBlendAdvanced;
    colorBlendAdvanced.advancedBlendOp = VK_BLEND_OP_ADD;
    colorBlendAdvanced.srcPremultiplied = VK_FALSE;
    colorBlendAdvanced.dstPremultiplied = VK_FALSE;
    colorBlendAdvanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
    colorBlendAdvanced.clampResults = VK_FALSE;
    vk::CmdSetColorBlendAdvancedEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendAdvanced);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_CONSTANT_COLOR,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 1u, 1u, &colorBlendEquation);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08658");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetFragmentShadingRateKHR) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetFragmentShadingRateKHR.");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-pipelineFragmentShadingRate-09238");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorWriteMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorWriteMaskEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08659");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09419");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetColorWriteMaskEXTActiveAttachments) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetColorWriteMaskEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 1u, 1u, &colorWriteMask);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09419");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetRasterizationStreamEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRasterizationStreamEXT.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const vkt::Shader geom_shader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, geom_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07630");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetConservativeRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetConservativeRasterizationModeEXT.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07631");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetExtraPrimitiveOverestimationSizeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetExtraPrimitiveOverestimationSizeEXT.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetConservativeRasterizationModeEXT(m_command_buffer.handle(), VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07632");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClipEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClipEnableEXT.");

    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::depthClipEnable);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07633");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleLocationsEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleLocationsEnableEXT.");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07634");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetProvokingVertexModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetProvokingVertexModeEXT.");

    AddRequiredExtensions(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07636");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPolygonModeCmdSetLineRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineRasterizationModeEXT.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::fillModeNonSolid);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetPolygonModeEXT(m_command_buffer.handle(), VK_POLYGON_MODE_LINE);
    vk::CmdSetLineStippleEnableEXT(m_command_buffer.handle(), VK_FALSE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08666");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyCmdSetLineRasterizationModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineRasterizationModeEXT.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdSetLineStippleEnableEXT(m_command_buffer.handle(), VK_FALSE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08667");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPolygonModeCmdSetLineStippleEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEnableEXT.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    AddRequiredFeature(vkt::Feature::fillModeNonSolid);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetPolygonModeEXT(m_command_buffer.handle(), VK_POLYGON_MODE_LINE);
    vk::CmdSetLineRasterizationModeEXT(m_command_buffer.handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08669");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyCmdSetLineStippleEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEnableEXT.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vk::CmdSetLineRasterizationModeEXT(m_command_buffer.handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08670");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetLineStippleEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineStippleEXT.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetLineStippleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07849");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthClipNegativeOneToOneEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthClipNegativeOneToOneEXT.");

    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::depthClipControl);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07639");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportWScalingEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportWScalingEnableNV.");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07640");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportWScalingNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportWScalingNV.");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetViewportWScalingEnableNV(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewportCount-04138");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08636");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportSwizzleNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportSwizzleNV.");

    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07641");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageToColorEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageToColorEnableNV.");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07642");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageToColorLocationNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageToColorLocationNV.");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageToColorEnableNV(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07643");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationModeNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationModeNV.");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageModulationTableEnableNV(m_command_buffer.handle(), VK_FALSE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07644");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationTableEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationTableEnableNV.");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageModulationModeNV(m_command_buffer.handle(), VK_COVERAGE_MODULATION_MODE_RGBA_NV);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07645");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageModulationTableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageModulationTableNV.");

    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageModulationModeNV(m_command_buffer.handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
    vk::CmdSetCoverageModulationTableEnableNV(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07646");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetShadingRateImageEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetShadingRateImageEnableNV.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoarseSampleOrderNV(m_command_buffer.handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07647");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetViewportShadingRatePaletteNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetViewportShadingRatePaletteNV.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoarseSampleOrderNV(m_command_buffer.handle(), VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV, 0u, nullptr);
    vk::CmdSetShadingRateImageEnableNV(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-shadingRateImage-09234");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoarseSampleOrderNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoarseSampleOrderNV.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shadingRateImage);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetShadingRateImageEnableNV(m_command_buffer.handle(), VK_FALSE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-shadingRateImage-09233");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetRepresentativeFragmentTestEnableNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetRepresentativeFragmentTestEnableNV.");

    AddRequiredExtensions(VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::representativeFragmentTest);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07648");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCoverageReductionModeNV) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCoverageReductionModeNV.");

    AddRequiredExtensions(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::coverageReductionMode);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageModulationModeNV(m_command_buffer.handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
    vk::CmdSetCoverageModulationTableEnableNV(m_command_buffer.handle(), VK_FALSE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07649");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingVertexShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding vertex shader.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                  GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    const VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &frag_shader.handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08684");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingTessellationControlBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding tessellation control shader.");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {m_vert_shader.handle(), VK_NULL_HANDLE, m_frag_shader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 3u, stages, shaders);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08685");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingTessellationEvaluationBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding tessellation evaluation shader.");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {m_vert_shader.handle(), VK_NULL_HANDLE, m_frag_shader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 3u, stages, shaders);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08686");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingGeometryBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding geometry shader.");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08687");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingFragmentShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding fragment shader.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    const VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &vert_shader.handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08688");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingTaskShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding task shader.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    {
        VkShaderStageFlagBits meshStage = VK_SHADER_STAGE_MESH_BIT_EXT;
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &meshStage, &nullShader);
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08689");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingMeshShaderBind) {
    TEST_DESCRIPTION("Draw with shader objects without binding mesh shader.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_1));
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    m_command_buffer.BindMeshShaders({}, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, VertAndMeshShaderBothBound) {
    TEST_DESCRIPTION("Draw with both vertex and mesh shader objects bound.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT mesh_create_info =
        ShaderCreateInfoFlag(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);
    const vkt::Shader mesh_shader(*m_device, mesh_create_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindMeshShaders(mesh_shader, m_frag_shader);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08693");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08696");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08885");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MeshShaderWithMissingTaskShader) {
    TEST_DESCRIPTION("Draw with a mesh shader that was created without the no task shader flag, but no task shader bound.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    InitDynamicRenderTarget();

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    const vkt::Shader mesh_shader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindMeshShaders(mesh_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-08694");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, TaskAndMeshShaderWithNoTaskFlag) {
    TEST_DESCRIPTION("Draw with a task and a mesh shader that was created with the no task shader flag.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    InitDynamicRenderTarget();

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    const vkt::Shader task_shader(*m_device, VK_SHADER_STAGE_TASK_BIT_EXT, kTaskMinimalGlsl);

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT mesh_create_info =
        ShaderCreateInfoFlag(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);
    const vkt::Shader mesh_shader(*m_device, mesh_create_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindMeshShaders(task_shader, mesh_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-08695");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingLinkedShaderBind) {
    TEST_DESCRIPTION("Draw with not all linked shaders bound.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT create_infos[2];
    create_infos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    create_infos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(*m_device, 2u, create_infos, nullptr, shaders);

    const vkt::Shader vert_shader(*m_device, shaders[0]);
    const vkt::Shader frag_shader(*m_device, shaders[1]);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, {});

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08698");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, BindShaderBetweenLinkedShaders) {
    TEST_DESCRIPTION("Draw when a shader is bound between linked shaders.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT create_infos[2];
    create_infos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    create_infos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(*m_device, 2u, create_infos, nullptr, shaders);

    const vkt::Shader vert_shader(*m_device, shaders[0]);
    const vkt::Shader geom_shader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    const vkt::Shader frag_shader(*m_device, shaders[1]);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, geom_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08699");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DifferentShaderPushConstantRanges) {
    TEST_DESCRIPTION("Draw with shaders that have different push constant ranges.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    VkPushConstantRange pushConstRange;
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstRange.offset = 0u;
    pushConstRange.size = sizeof(uint32_t);

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, nullptr, &pushConstRange);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08878");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DifferentShaderDescriptorLayouts) {
    TEST_DESCRIPTION("Draw with shaders that have different descriptor layouts.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, &descriptor_set.layout_.handle());
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08879");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetAttachmentFeedbackLoopEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetAttachmentFeedbackLoopEnableEXT.");

    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopDynamicState);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08877");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetPrimitiveTopologyEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPrimitiveTopologyEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07842");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetPatchControlPointsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPatchControlPointsEXT.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const vkt::Shader tesc_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    const vkt::Shader tese_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT}, true);
    m_command_buffer.BindShaders(m_vert_shader, tesc_shader, tese_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04875");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetTessellationDomainOriginEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetTessellationDomainOriginEXT.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const vkt::Shader tesc_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    const vkt::Shader tese_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT});
    m_command_buffer.BindShaders(m_vert_shader, tesc_shader, tese_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07619");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetPrimitiveRestartEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetPrimitiveRestartEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04879");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetVertexInput) {
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_VERTEX_INPUT_EXT});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04914");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DrawWithGraphicsShadersWhenMeshShaderIsBound) {
    TEST_DESCRIPTION("Draw with graphics shader objects when a mesh shader is bound.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    InitDynamicRenderTarget();

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    const auto mesh_spv = GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, kMeshMinimalGlsl, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT mesh_create_info =
        ShaderCreateInfoFlag(mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT);
    const vkt::Shader mesh_shader(*m_device, mesh_create_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindMeshShaders(mesh_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08885");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPolygonLineCmdSetLineWidthEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineWidthEXT when polygon mode is line.");

    AddRequiredFeature(vkt::Feature::fillModeNonSolid);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_WIDTH});
    vk::CmdSetPolygonModeEXT(m_command_buffer.handle(), VK_POLYGON_MODE_LINE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08617");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingPrimitiveTopologyLineCmdSetLineWidthEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetLineWidthEXT when primitive topology is line.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_WIDTH});
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08618");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBiasEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBiasEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BIAS});
    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07834");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetBlendConstantsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetBlendConstantsEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_BLEND_CONSTANTS});
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_CONSTANT_COLOR,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0u, 1u, &colorBlendEquation);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkBool32 color_blend_enable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &color_blend_enable);
    VkColorBlendAdvancedEXT color_blend_advanced;
    color_blend_advanced.advancedBlendOp = VK_BLEND_OP_ADD;
    color_blend_advanced.srcPremultiplied = VK_FALSE;
    color_blend_advanced.dstPremultiplied = VK_FALSE;
    color_blend_advanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
    color_blend_advanced.clampResults = VK_FALSE;
    vk::CmdSetColorBlendAdvancedEXT(m_command_buffer.handle(), 0u, 1u, &color_blend_advanced);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08621");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBoundsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBoundsEXT.");

    AddRequiredFeature(vkt::Feature::depthBounds);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BOUNDS});
    vk::CmdSetDepthBoundsTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07836");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilCompareMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilCompareMaskEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK});
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07837");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilWriteMaskEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilWriteMaskEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_STENCIL_WRITE_MASK});
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07838");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilReferenceEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilReferenceEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_STENCIL_REFERENCE});
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07839");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetSampleLocationsEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetSampleLocationsEXT.");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT});
    vk::CmdSetSampleLocationsEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06666");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetCullModeEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetCullModeEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_CULL_MODE});

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07840");
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetFrontFaceEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetFrontFaceEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_FRONT_FACE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    vk::CmdSetCullModeEXT(m_command_buffer.handle(), VK_CULL_MODE_BACK_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07841");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthTestEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthTestEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07843");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthWriteEnableEXT) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthWriteEnableEXT.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07844");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthCompareOp) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthCompareOp.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_COMPARE_OP});
    vk::CmdSetDepthTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07845");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetDepthBoundsTestEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetDepthBoundsTestEnable.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::depthBounds);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07846");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilTestEnable) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilTestEnable.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE});
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07847");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingCmdSetStencilOp) {
    TEST_DESCRIPTION("Draw with shader objects without setting vkCmdSetStencilOp.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_STENCIL_OP});
    vk::CmdSetStencilTestEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07848");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, ComputeShaderGroupCount) {
    TEST_DESCRIPTION("Dispatch with group count higher than maxComputeWorkGroupCount.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    uint32_t x_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[0];
    uint32_t y_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[1];
    uint32_t z_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[2];

    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);

    m_command_buffer.Begin();

    m_command_buffer.BindCompShader(comp_shader);

    if (x_count_limit != std::numeric_limits<uint32_t>::max()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountX-00386");
        vk::CmdDispatch(m_command_buffer.handle(), x_count_limit + 1u, 1u, 1u);
        m_errorMonitor->VerifyFound();
    }

    if (y_count_limit != std::numeric_limits<uint32_t>::max()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountY-00387");
        vk::CmdDispatch(m_command_buffer.handle(), 1u, y_count_limit + 1u, 1u);
        m_errorMonitor->VerifyFound();
    }

    if (z_count_limit != std::numeric_limits<uint32_t>::max()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountZ-00388");
        vk::CmdDispatch(m_command_buffer.handle(), 1u, 1u, z_count_limit + 1u);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, ComputeShaderMissingPushConst) {
    TEST_DESCRIPTION("Dispatch with a shader object using push const, but not setting it.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    VkPushConstantRange push_const_range;
    push_const_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_const_range.offset = 0u;
    push_const_range.size = sizeof(int);

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

    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {push_const_range});

    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, kComputeShaderGlsl, &descriptor_set.layout_.handle(),
                                  &push_const_range);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    m_command_buffer.BindCompShader(comp_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-maintenance4-08602");
    vk::CmdDispatch(m_command_buffer.handle(), 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, SharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream cs_src;
    // Make sure compute pipeline has a compute shader stage set
    cs_src << R"glsl(
        #version 450
        shared int a[)glsl";
    cs_src << (max_shared_ints + 16);
    cs_src << R"glsl(];
        void main(){
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src.str().c_str());

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InvalidRequireFullSubgroupsFlag) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08992");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMapEntryOffset) {
    TEST_DESCRIPTION("Create shader with invalid specialization map entry offset.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry map_entry = {};
    map_entry.constantID = 0u;
    map_entry.offset = sizeof(int) * 2;
    map_entry.size = sizeof(int);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &map_entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    create_info.pSpecializationInfo = &specialization_info;

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationInfo-offset-00773");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMapEntrySize) {
    TEST_DESCRIPTION("Create shader with specialization map entry out of bounds.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry map_entry = {};
    map_entry.constantID = 0u;
    map_entry.offset = sizeof(int) / 2;
    map_entry.size = sizeof(int);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &map_entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    create_info.pSpecializationInfo = &specialization_info;

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationInfo-pMapEntries-00774");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationMismatch) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data[2] = {0u, 0u};

    VkSpecializationMapEntry map_entry = {};
    map_entry.constantID = 0u;
    map_entry.offset = 0u;
    map_entry.size = sizeof(int) * 2u;

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &map_entry;
    specialization_info.dataSize = sizeof(uint32_t) * 2;
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    create_info.pSpecializationInfo = &specialization_info;

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationMapEntry-constantID-00776");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationSameConstantId) {
    TEST_DESCRIPTION("Create shader with non unique specialization map entries.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char kVertexSource[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int v = 0;
        void main() {
           gl_Position = vec4(v);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexSource);

    int data = 0;

    VkSpecializationMapEntry map_entries[2] = {};
    map_entries[0].constantID = 0u;
    map_entries[0].offset = 0u;
    map_entries[0].size = sizeof(int);
    map_entries[1].constantID = 0u;
    map_entries[1].offset = 0u;
    map_entries[1].size = sizeof(int);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = map_entries;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    create_info.pSpecializationInfo = &specialization_info;

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkSpecializationInfo-constantID-04911");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingEntrypoint) {
    TEST_DESCRIPTION("Create shader with invalid spirv code size.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);
    create_info.pName = "invalid";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pName-08440");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    RETURN_IF_SKIP(InitBasicShaderObject());

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

    VkSpecializationMapEntry map_entry = {};
    map_entry.constantID = 0u;
    map_entry.offset = 0u;
    map_entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &map_entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    create_info.pSpecializationInfo = &specialization_info;

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08460");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MinTexelGatherOffset) {
    TEST_DESCRIPTION("Create shader with texel gather offset lower than minimum.");

    RETURN_IF_SKIP(InitBasicShaderObject());

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

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(cs_spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06377");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, UnsupportedSpirvCapability) {
    TEST_DESCRIPTION("Create shader with unsupported spirv capability.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);

    RETURN_IF_SKIP(Init());

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

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08740");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, UnsupportedSpirvExtension) {
    TEST_DESCRIPTION("Create shader with unsupported spirv extension.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());

    const char* vs_src = R"(
               OpCapability Shader
               OpExtension "GL_EXT_scalar_block_layout"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource GLSL 450
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)";

    std::vector<uint32_t> vs_spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, vs_src, vs_spv);

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08741");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, SpirvExtensionRequirementsNotMet) {
    TEST_DESCRIPTION("Create shader with extension requirements not met.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());

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

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(cs_spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08742");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MemoryModelNotEnabled) {
    TEST_DESCRIPTION("Create shader with unsupported spirv extension.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);

    RETURN_IF_SKIP(Init());

    char const* cs_src = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
       }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-vulkanMemoryModel-06265");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MaxTransformFeedbackStream) {
    TEST_DESCRIPTION("Test maxTransformFeedbackStream with shader objects.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    RETURN_IF_SKIP(InitBasicShaderObject());

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

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(gs_spv, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpEmitStreamVertex-06310");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TransformFeedbackStride) {
    TEST_DESCRIPTION("Test maxTransformFeedbackStream with shader objects.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    RETURN_IF_SKIP(InitBasicShaderObject());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    // seen sometimes when using profiles and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "maxTransformFeedbackStreams is zero";
    }

    std::stringstream vs_src;
    vs_src << R"asm(
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
    vs_src << transform_feedback_props.maxTransformFeedbackBufferDataStride + 4;
    vs_src << R"asm(
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
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, vs_src.str().c_str(), vs_spv);

    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-XfbStride-06313");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MeshOutputVertices) {
    TEST_DESCRIPTION("Create mesh shader with output vertices higher than max.");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

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
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, mesh_src.c_str(), spv);

    VkShaderCreateInfoEXT create_info = ShaderCreateInfoLink(spv, VK_SHADER_STAGE_MESH_BIT_EXT, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07115");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, Atomics) {
    TEST_DESCRIPTION("Test atomics with shader objects.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderSharedInt64Atomics);  // to allow OpCapability Int64Atomics
    RETURN_IF_SKIP(InitBasicShaderObject());

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

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src.c_str());
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());
    VkShaderEXT shader;

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06278");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ExtendedTypesDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::shaderFloat16);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceSubgroupProperties subgroup_prop = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_prop);
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT)) {
        GTEST_SKIP() << "Required features not supported";
    }

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

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_src, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-None-06275");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());

    char const* vs_src = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src, SPV_ENV_VULKAN_1_3);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSubgroupClock-06267");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, WriteLessComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitBasicShaderObject());

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

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, cs_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &descriptor_set.layout_.handle());

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImageWrite-07112");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LocalSizeIdExecutionMode) {
    TEST_DESCRIPTION("Test LocalSizeId spirv execution mode.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

    RETURN_IF_SKIP(InitBasicShaderObject());

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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-LocalSizeId-06434");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ZeroInitializeWorkgroupMemory) {
    TEST_DESCRIPTION("Test initializing workgroup memory in compute shader.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    ASMtoSPV(SPV_ENV_VULKAN_1_2, 0, cs_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingNonReadableDecorationFormatRead) {
    TEST_DESCRIPTION("Create a shader with a storage image without an image format not marked as non readable.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-apiVersion-07954");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-apiVersion-07955");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    if (m_device->Physical().limits_.maxSampleMaskWords > 1) {
        GTEST_SKIP() << "maxSampleMaskWords is greater than 1";
    }

    // layout(location = 0) out vec4 uFragColor;
    // void main(){
    //     int x = gl_SampleMaskIn[2];
    //     int y = gl_SampleMaskIn[0];
    //     uFragColor = vec4(0,1,0,1) * x * y;
    // }
    char const* fs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_SampleMaskIn %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %gl_SampleMaskIn Flat
               OpDecorate %gl_SampleMaskIn BuiltIn SampleMask
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_int_uint_3 = OpTypeArray %int %uint_3
%_ptr_Input__arr_int_uint_3 = OpTypePointer Input %_arr_int_uint_3
%gl_SampleMaskIn = OpVariable %_ptr_Input__arr_int_uint_3 Input
      %int_2 = OpConstant %int 2
%_ptr_Input_int = OpTypePointer Input %int
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %28 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
         %16 = OpAccessChain %_ptr_Input_int %gl_SampleMaskIn %int_2
         %17 = OpLoad %int %16
               OpStore %x %17
         %20 = OpAccessChain %_ptr_Input_int %gl_SampleMaskIn %int_0
         %21 = OpLoad %int %20
               OpStore %y %21
         %29 = OpLoad %int %x
         %30 = OpConvertSToF %float %29
         %31 = OpVectorTimesScalar %v4float %28 %30
         %32 = OpLoad %int %y
         %33 = OpConvertSToF %float %32
         %34 = OpVectorTimesScalar %v4float %31 %33
               OpStore %uFragColor %34
               OpReturn
               OpFunctionEnd
    )";

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_3, 0, fs_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08451");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ConservativeRasterizationPostDepthCoverage) {
    TEST_DESCRIPTION("Make sure conservativeRasterizationPostDepthCoverage is set if needed.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, LocalSizeExceedLimits) {
    TEST_DESCRIPTION("Create shader where local size exceeds limits.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    uint32_t x_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[0];

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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-x-06429");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-x-06432");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingLineWidthSet) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line width dynamic state.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    const vkt::Shader geom_shader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_WIDTH});
    m_command_buffer.BindShaders(m_vert_shader, geom_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08619");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidViewportCount) {
    TEST_DESCRIPTION("Draw with a shader that uses PrimitiveShadingRateKHR with invalid viewport count.");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(InitBasicShaderObject());
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

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vsSource);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    VkViewport viewports[2];
    viewports[0] = {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    viewports[1] = {0.0f, 100.0f, 100.0f, 100.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    VkRect2D scissors[2];
    scissors[0] = {{0, 0}, {100u, 100u}};
    scissors[1] = {{0, 100}, {100u, 100u}};
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 2u, scissors);
    VkExtent2D fragment_size = {1u, 1u};
    VkFragmentShadingRateCombinerOpKHR combiner_ops[2] = {VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                                                          VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};
    vk::CmdSetFragmentShadingRateKHR(m_command_buffer.handle(), &fragment_size, combiner_ops);
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-08642");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, AlphaToCoverage) {
    TEST_DESCRIPTION("Draw with fragment shader missing alpha to coverage.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 1) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0,1,0,1);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_WIDTH});
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdSetAlphaToCoverageEnableEXT(m_command_buffer.handle(), VK_TRUE);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-alphaToCoverageEnable-08920");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingLineRasterizationMode) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line rasterization mode dynamic state.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    const vkt::Shader geom_shader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, geom_shader, m_frag_shader);
    vk::CmdSetLineStippleEnableEXT(m_command_buffer.handle(), VK_FALSE);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08668");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingLineStippleEnable) {
    TEST_DESCRIPTION("Draw with shaders outputing lines but not setting line stipple enable dynamic state.");

    AddRequiredExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::stippledRectangularLines);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(triangles) in;
        layout(line_strip, max_vertices=2) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    const vkt::Shader geom_shader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT});
    m_command_buffer.BindShaders(m_vert_shader, geom_shader, m_frag_shader);
    vk::CmdSetLineRasterizationModeEXT(m_command_buffer.handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08671");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidColorWriteMask) {
    TEST_DESCRIPTION("Draw with invalid color write mask.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    CreateMinimalShaders();

    VkFormat format = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), format, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 not supported as color attachment.";
    }

    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = format;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper();
    auto res = vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical(), &image_format_info, &image_format_properties);
    if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "image format not supported as color attachment.";
    }

    vkt::Image image(*m_device, 256, 256, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(image_view, GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 0u, 1u, &colorWriteMask);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09116");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, Mismatched64BitAttributeType) {
    TEST_DESCRIPTION("Draw with vertex format not matching vertex input format.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const VkFormat format = VK_FORMAT_R64_SINT;

    VkFormatProperties2 format_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(m_device->Physical(), format, &format_properties);

    if ((format_properties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "format not supported.";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        layout(location = 0) in int pos;
        void main() {
           gl_Position = vec4(pos);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    VkVertexInputBindingDescription2EXT vertex_binding_description = vku::InitStructHelper();
    vertex_binding_description.binding = 0u;
    vertex_binding_description.stride = 16u;
    vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_binding_description.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertex_attribute_description = vku::InitStructHelper();
    vertex_attribute_description.location = 0u;
    vertex_attribute_description.binding = 0u;
    vertex_attribute_description.format = format;
    vertex_attribute_description.offset = 0u;

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1u, &vertex_binding_description, 1u, &vertex_attribute_description);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-format-08936");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, Mismatched32BitAttributeType) {
    TEST_DESCRIPTION("Draw with vertex format not matching vertex input format.");

    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(location = 0) in int64_t pos;
        void main() {
           gl_Position = vec4(pos);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    VkVertexInputBindingDescription2EXT vertex_binding_description = vku::InitStructHelper();
    vertex_binding_description.binding = 0u;
    vertex_binding_description.stride = 16u;
    vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_binding_description.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertex_attribute_description = vku::InitStructHelper();
    vertex_attribute_description.location = 0u;
    vertex_attribute_description.binding = 0u;
    vertex_attribute_description.format = VK_FORMAT_R32_SINT;
    vertex_attribute_description.offset = 0u;

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1u, &vertex_binding_description, 1u, &vertex_attribute_description);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-format-08937");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MismatchedFormat64Components) {
    TEST_DESCRIPTION("Draw with vertex format components not matching vertex input format components.");

    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const VkFormat format = VK_FORMAT_R64G64B64_SINT;

    VkFormatProperties2 format_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(m_device->Physical(), format, &format_properties);

    if ((format_properties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
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

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    VkVertexInputBindingDescription2EXT vertex_binding_description = vku::InitStructHelper();
    vertex_binding_description.binding = 0u;
    vertex_binding_description.stride = 16u;
    vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_binding_description.divisor = 1u;

    VkVertexInputAttributeDescription2EXT vertex_attribute_description = vku::InitStructHelper();
    vertex_attribute_description.location = 0u;
    vertex_attribute_description.binding = 0u;
    vertex_attribute_description.format = format;
    vertex_attribute_description.offset = 0u;

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1u, &vertex_binding_description, 1u, &vertex_attribute_description);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09203");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MismatchedAttributeType) {
    TEST_DESCRIPTION("Draw with vertex format not matching vertex input format.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        layout(location=0) in int x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vert_shader(*m_device, stages[0], GLSLToSPV(stages[0], vert_src));
    const vkt::Shader frag_shader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.stride = 4;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;

    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.format = VK_FORMAT_R32_SFLOAT;

    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-Input-08734");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DescriptorNotUpdated) {
    TEST_DESCRIPTION("Draw with shaders using a descriptor set that was never updated.");

    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicShaderObject());
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

    const vkt::Shader vert_shader(*m_device, ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 2, descriptor_set_layouts));
    const vkt::Shader frag_shader(*m_device, ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 2, descriptor_set_layouts));

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1u, 1u,
                              &frag_descriptor_set.set_, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, ComputeVaryingAndFullSubgroups) {
    TEST_DESCRIPTION("Dispatch with compute shader using required full subgroups and allow varying subgroup size flags.");

    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::subgroupSizeControl);
    AddRequiredFeature(vkt::Feature::computeFullSubgroups);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfoFlag(
        spv, VK_SHADER_STAGE_COMPUTE_BIT,
        VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT | VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08416");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, ComputeVaryingSubgroups) {
    TEST_DESCRIPTION("Dispatch with compute shader using required full subgroups and allow varying subgroup size flags.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeFullSubgroups);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_COMPUTE_BIT, VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-08417");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryShaderMaxOutputVertices) {
    TEST_DESCRIPTION("Create geometry shader with output vertices higher than maximum.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    std::string geom_src = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices )";
    geom_src += std::to_string(m_device->Physical().limits_.maxGeometryOutputVertices + 1);
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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08454");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, GeometryShaderMaxInvocations) {
    TEST_DESCRIPTION("Create geometry shader with invocations higher than maximum.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    std::string geom_src = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations )";
    geom_src += std::to_string(m_device->Physical().limits_.maxGeometryShaderInvocations + 1);
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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08455");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingImageFilterLinearBit) {
    TEST_DESCRIPTION("Draw with shaders sampling from an image which does not have required filter linear bit.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    VkFormat format = VK_FORMAT_R16_SINT;
    VkFormatProperties props = {};
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &props);

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

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, &descriptor_set.layout_.handle());

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_src, &descriptor_set.layout_.handle());

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.compareEnable = VK_FALSE;
    vkt::Sampler sampler(*m_device, sampler_info);

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vert_shader, frag_shader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-magFilter-04553");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MaxMultiviewInstanceIndex) {
    TEST_DESCRIPTION("Draw with a read only depth stencil attachment and invalid stencil op.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    VkPhysicalDeviceMultiviewProperties multiview_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_properties);

    vkt::Image img(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView view = img.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {100u, 100u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.viewMask = 0x1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, multiview_properties.maxMultiviewInstanceIndex);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MaxFragmentDualSrcAttachmentsDynamicBlendEnable) {
    TEST_DESCRIPTION(
        "Test drawing with dual source blending with too many fragment output attachments, but using dynamic blending.");
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();

    const uint32_t count = m_device->Physical().limits_.maxFragmentDualSrcAttachments + 1;
    if (count != 2) {
        GTEST_SKIP() << "Test is designed for a maxFragmentDualSrcAttachments of 1";
    }

    const char* fs_src = R"glsl(
        #version 460
        layout(location = 0) out vec4 c0;
        layout(location = 1) out vec4 c1;
        void main() {
            c0 = vec4(0.0f);
            c1 = vec4(0.0f);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, fs_src);

    vkt::Image img1(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img2(*m_device, m_width, m_height, 1, m_render_target_fmt,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::ImageView view1 = img1.CreateView();
    vkt::ImageView view2 = img2.CreateView();

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

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 2u;
    renderingInfo.pColorAttachments = attachments;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);

    SetDefaultDynamicStatesExclude();

    VkBool32 color_blend_enabled[2] = {VK_TRUE, VK_TRUE};
    VkColorBlendEquationEXT dual_color_blend_equation = {
        VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    VkColorBlendEquationEXT normal_color_blend_equation = {
        VK_BLEND_FACTOR_ONE,       VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    VkColorComponentFlags color_component_flags[2] = {VK_COLOR_COMPONENT_R_BIT, VK_COLOR_COMPONENT_R_BIT};

    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 2, color_blend_enabled);
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0, 1, &normal_color_blend_equation);
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 1, 1, &dual_color_blend_equation);
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 0, 2, color_component_flags);

    m_command_buffer.BindShaders(vert_shader, frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maxFragmentDualSrcAttachments-09239");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, PrimitivesGeneratedQuery) {
    TEST_DESCRIPTION("Draw with primitives generated query.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);

    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_TRUE);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, CooperativeMatrix) {
    TEST_DESCRIPTION("Test cooperative matrix with shader objects");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(InitBasicShaderObject());

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.subgroupSize > 32) {
        GTEST_SKIP() << "local_size_x (32) is not a multiple of subgroupSize";
    }

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
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationSubdivision) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different subdivision.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    createInfos[0] =
        ShaderCreateInfoLink(tesc_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    createInfos[1] = ShaderCreateInfoLink(tese_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08867");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationOrientation) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different orientations.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    createInfos[0] =
        ShaderCreateInfoLink(tesc_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    createInfos[1] = ShaderCreateInfoLink(tese_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08868");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationPointMode) {
    TEST_DESCRIPTION("Create linked tessellation control with point mode and evaluation shader without.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    createInfos[0] =
        ShaderCreateInfoLink(tesc_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    createInfos[1] = ShaderCreateInfoLink(tese_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08869");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MismatchedTessellationSpacing) {
    TEST_DESCRIPTION("Create linked tessellation control and evaluation shaders with different spacing.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

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
    createInfos[0] =
        ShaderCreateInfoLink(tesc_spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    createInfos[1] = ShaderCreateInfoLink(tese_spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkShaderEXT shaders[2];
    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-08870");
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingSubgroupSizeControlFeature) {
    TEST_DESCRIPTION("Create shader with invalid flags when subgroupSizeControl is not enabled.");

    RETURN_IF_SKIP(InitBasicShaderObject());

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_COMPUTE_BIT, VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-09404");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingComputeFullSubgroups) {
    TEST_DESCRIPTION("Create shader with invalid flags when computeFullSubgroups is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitBasicShaderObject());

    VkPhysicalDeviceVulkan11Properties properties11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(properties11);

    std::string comp_src = R"glsl(
        #version 460
        layout(local_size_x = )glsl";
    comp_src += std::to_string(properties11.subgroupSize);
    comp_src += R"glsl() in;
        void main() {}
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src.c_str());
    VkShaderCreateInfoEXT create_info =
        ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_COMPUTE_BIT, VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-09405");
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, CoverageToColorInvalidFormat) {
    TEST_DESCRIPTION("Use coverage to color with invalid format.");

    AddRequiredExtensions(VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetCoverageToColorEnableNV(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetCoverageToColorLocationNV(m_command_buffer.handle(), 0u);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizerDiscardEnable-09420");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidViewportSwizzleCount) {
    TEST_DESCRIPTION("Set invalid viewport count for viewport swizzle.");

    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    VkViewport viewports[2] = {{0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f},
                               {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f}};
    VkRect2D scissors[2] = {{{0, 0}, {m_width, m_height}}, {{0, 0}, {m_width, m_height}}};
    VkViewportSwizzleNV viewportSwizzle = {
        VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
        VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdSetViewportWithCountEXT(m_command_buffer.handle(), 2u, viewports);
    vk::CmdSetScissorWithCountEXT(m_command_buffer.handle(), 2u, scissors);
    vk::CmdSetViewportSwizzleNV(m_command_buffer.handle(), 0u, 1u, &viewportSwizzle);
    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewportCount-09421");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);

    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, MissingTessellationEvaluationSubdivision) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with missing subdivision.");

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-08872");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    const char* tese_src = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
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

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingTessellationEvaluationOrientation) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with missing orientation.");

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-08873");
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

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

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, MissingTessellationEvaluationSpacing) {
    TEST_DESCRIPTION("Create tessellation evaluation shader with missing spacing.");

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-08874");
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    const char* tese_src = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
               OpExecutionMode %main Triangles
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

    std::vector<uint32_t> spv;
    ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tese_src, spv);
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, TessellationPatchSize) {
    TEST_DESCRIPTION("Create tessellation shader with invalid patch size.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());

    for (uint32_t i = 0; i < 2; ++i) {
        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pCode-08453");

        std::string tesc_src = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices )";
        tesc_src += i == 0 ? std::string("0") : std::to_string(m_device->Physical().limits_.maxTessellationPatchSize + 1u);
        tesc_src += R"(
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
               OpFunctionEnd
    )";

        std::vector<uint32_t> spv;
        ASMtoSPV(SPV_ENV_VULKAN_1_0, 0, tesc_src.c_str(), spv);
        VkShaderCreateInfoEXT create_info =
            ShaderCreateInfoLink(spv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        VkShaderEXT shader;
        vk::CreateShadersEXT(m_device->handle(), 1u, &create_info, nullptr, &shader);

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderObject, DispatchBaseFlag) {
    TEST_DESCRIPTION("Compute dispatch without VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT");
    RETURN_IF_SKIP(InitBasicShaderObject());
    const vkt::Shader compShader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, kMinimalShaderGlsl);
    m_command_buffer.Begin();
    m_command_buffer.BindCompShader(compShader);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-baseGroupX-00427");
    vk::CmdDispatchBase(m_command_buffer.handle(), 1, 1, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, SetPrimitiveTopologyNonPatch) {
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitBasicShaderObject());
    InitDynamicRenderTarget();
    CreateMinimalShaders();

    const vkt::Shader tesc_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    const vkt::Shader tese_shader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(m_vert_shader, tesc_shader, tese_shader, m_frag_shader);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitiveTopology-10286");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DescriptorWrongStage) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    // wrong stage
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=16, local_size_x=1, local_size_x=1) in;
        layout(binding = 0) buffer Output {
            uint values[16];
        } buffer_out;

        void main() {
            buffer_out.values[gl_LocalInvocationID.x] = gl_LocalInvocationID.x;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10383");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorWrongStageMultipleBindings) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_x=1, local_size_x=1) in;
        layout(set = 0, binding = 0) buffer SSBO_0 { uint a; };
        layout(set = 0, binding = 1) buffer SSBO_1 { uint b; };
        layout(set = 0, binding = 2) buffer SSBO_2 { uint c; };
        void main() {
            a = b + c;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10383");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorWrongStageMultipleSets) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    OneOffDescriptorSet descriptor_set0(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    OneOffDescriptorSet descriptor_set1(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet descriptor_set2(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=1, local_size_x=1, local_size_x=1) in;
        layout(set = 0, binding = 0) buffer SSBO_0 { uint a; };
        layout(set = 1, binding = 0) buffer SSBO_1 { uint b; };
        layout(set = 2, binding = 0) buffer SSBO_2 { uint c; };
        void main() {
            a = b + c;
        }
    )glsl";

    const auto comp_spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src);
    VkDescriptorSetLayout dsl[3] = {descriptor_set0.layout_.handle(), descriptor_set1.layout_.handle(),
                                    descriptor_set2.layout_.handle()};
    VkShaderCreateInfoEXT create_info = ShaderCreateInfo(comp_spv, VK_SHADER_STAGE_COMPUTE_BIT, 3, dsl);
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10383");
    const vkt::Shader comp_shader(*m_device, create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorNotProvided) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char comp_src[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO_0 { uint a; };
        void main() {
            a = 0;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10383");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src));
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorTypeMismatch) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char comp_src[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO_0 { uint a; };
        void main() {
            a = 0;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10384");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, DescriptorCount) {
    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char comp_src[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO_0 { uint a; } x[3];
        void main() {
            x[2].a = 0;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}});
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10385");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, InlineUniformBlockArray) {
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitBasicShaderObject());

    static const char comp_src[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer SSBO0 { uint ssbo; };
        layout(set = 0, binding = 1) uniform InlineUBO { uint x; } inlineArray[4];
        void main() {
            ssbo = inlineArray[0].x;
        }
    )glsl";

    VkDescriptorPoolInlineUniformBlockCreateInfo pool_inline_info = vku::InitStructHelper();
    pool_inline_info.maxInlineUniformBlockBindings = 1;
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, nullptr, 0, nullptr, &pool_inline_info);

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10386");
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src),
                                  &descriptor_set.layout_.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderObject, PushConstantNotDeclared) {
    TEST_DESCRIPTION("Test missing push constant declaration.");

    RETURN_IF_SKIP(InitBasicShaderObject());
    InitRenderTarget();

    char const* vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {};
    // Set to the wrong stage to challenge core_validation
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = 4u;

    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    const auto vspv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = vspv.size() * sizeof(vspv[0]);
    createInfo.pCode = vspv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10064");
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    createInfo.pushConstantRangeCount = 1u;
    createInfo.pPushConstantRanges = &push_constant_range;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10064");
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 4u;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-codeType-10065");
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    m_errorMonitor->VerifyFound();

    push_constant_range.offset = 0u;
    const vkt::Shader validShader(*m_device, createInfo);
}

TEST_F(NegativeShaderObject, BindWithoutFeature) {
    TEST_DESCRIPTION("Use vkCmdBindShadersEXT without enabling shaderObject feature");

    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    m_command_buffer.Begin();
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkShaderEXT handle = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-None-08462");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &handle);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, InvalidRayTracingStage) {
    TEST_DESCRIPTION("Use vkCmdBindShadersEXT without enabling shaderObject feature");

    RETURN_IF_SKIP(InitBasicShaderObject());

    m_command_buffer.Begin();
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    VkShaderEXT handle = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindShadersEXT-pStages-08465");
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &handle);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, TaskMeshShadersDrawWithoutBindingVertex) {
    TEST_DESCRIPTION("Test drawing using task and mesh shaders without binding anything to vertex stage");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));

    static const char task_src[] = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    static const char mesh_src[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3, 1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[1].gl_Position = vec4( 3.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[2].gl_Position = vec4(-1.0,  3.0, 0.0f, 1.0f);
            gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT,
                                             VK_SHADER_STAGE_FRAGMENT_BIT};

    const vkt::Shader task_shader(*m_device, shader_stages[0], task_src);
    const vkt::Shader mesh_shader(*m_device, shader_stages[1], mesh_src);
    const vkt::Shader frag_shader(*m_device, shader_stages[2], frag_src);

    VkShaderEXT shaders[3] = {task_shader.handle(), mesh_shader.handle(), frag_shader.handle()};

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 3u, shader_stages, shaders);
    SetDefaultDynamicStatesExclude();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-08607");
    // Todo: should be VUID-vkCmdDrawMeshTasksEXT-None-08684 when it is added into the spec
    m_errorMonitor->SetDesiredError("VUID_Undefined");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderObject, DrawMeshTasksWithoutMeshShader) {
    TEST_DESCRIPTION("Test calling vkCmdDrawMeshTasksEXT when VK_NULL_HANDLE is bound to mesh stage");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    CreateMinimalShaders();

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0u;
    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.viewMask = 0x0;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &begin_rendering_info);
    std::vector<VkShaderStageFlagBits> null_stages = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT};
    VkShaderEXT null_shader = VK_NULL_HANDLE;
    for (const auto stage : null_stages) {
        vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, &stage, &null_shader);
    }

    m_command_buffer.BindShaders(m_vert_shader, m_frag_shader);
    SetDefaultDynamicStatesExclude();
    m_errorMonitor->SetDesiredError("UNASSIGNED-vkCmdDrawMeshTasks-pre-raster-stages");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeShaderObject, VertAndMeshShaderBothNotBound) {
    TEST_DESCRIPTION("Draw with a neither a vertex or mesh bound");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(VK_API_VERSION_1_3));
    InitDynamicRenderTarget();

    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_TASK_BIT_EXT,
                                            VK_SHADER_STAGE_MESH_BIT_EXT,
                                            VK_SHADER_STAGE_VERTEX_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,      VK_NULL_HANDLE,
                                   VK_NULL_HANDLE, VK_NULL_HANDLE, frag_shader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 7u, stages, shaders);
    // TODO - Should be 08693
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08607");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-08607");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}
