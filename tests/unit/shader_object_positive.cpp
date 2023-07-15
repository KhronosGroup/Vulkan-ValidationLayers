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

void ShaderObjectTest::InitBasicShaderObject(void* pNextFeatures, APIVersion targetApiVersion) {
    SetTargetApiVersion(targetApiVersion);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < m_attempted_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_attempted_api_version.minor() << " is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto shader_object_features = LvlInitStruct<VkPhysicalDeviceShaderObjectFeaturesEXT>(pNextFeatures);
    GetPhysicalDeviceFeatures2(shader_object_features);
    if (!shader_object_features.shaderObject) {
        GTEST_SKIP() << "Test requires (unsupported) shaderObject , skipping.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &shader_object_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
}

TEST_F(PositiveShaderObject, CreateAndDestroyShaderObject) {
    TEST_DESCRIPTION("Create and destroy shader object.");

    InitBasicShaderObject();
    if (::testing::Test::IsSkipped()) return;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = LvlInitStruct<VkShaderCreateInfoEXT>();
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";

    VkShaderEXT shader;
    vk::CreateShadersEXT(m_device->handle(), 1u, &createInfo, nullptr, &shader);
    vk::DestroyShaderEXT(m_device->handle(), shader, nullptr);
}

TEST_F(PositiveShaderObject, BindShaderObject) {
    TEST_DESCRIPTION("Use graphics shaders with unsupported command pool.");

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

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &createInfo.stage, &shaderHandle);
    m_commandBuffer->end();
}