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

void ShaderObjectTest::InitBasicShaderObject(void *pNextFeatures, APIVersion targetApiVersion, bool coreFeatures) {
    SetTargetApiVersion(targetApiVersion);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(pNextFeatures);
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_object_features);
    if (!coreFeatures) {
        features2 = vku::InitStructHelper(&shader_object_features);
    }
    if (!shader_object_features.shaderObject) {
        GTEST_SKIP() << "Test requires (unsupported) shaderObject , skipping.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
}

void ShaderObjectTest::InitBasicMeshShaderObject(void *pNextFeatures, APIVersion targetApiVersion, bool taskShader,
                                                 bool meshShader) {
    SetTargetApiVersion(targetApiVersion);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance4Features maintenance_4_features = vku::InitStructHelper(pNextFeatures);
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper(&maintenance_4_features);
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&mesh_shader_features);
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_object_features);
    if (!shader_object_features.shaderObject) {
        GTEST_SKIP() << "Test requires (unsupported) shaderObject , skipping.";
    }
    if (meshShader && !mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shaders are required";
    }
    if (taskShader && !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Task shaders are required";
    }
    if (!maintenance_4_features.maintenance4) {
        GTEST_SKIP() << "maintenance4 not supported";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
}

void ShaderObjectTest::BindVertFragShader(const vkt::Shader &vertShader, const vkt::Shader &fragShader) {
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
}

void ShaderObjectTest::BindCompShader(const vkt::Shader &compShader) {
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const VkShaderEXT shaders[] = {compShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, stages, shaders);
}

void ShaderObjectTest::SetDefaultDynamicStates(const std::vector<VkDynamicState> &exclude, bool tessellation,
                                               VkCommandBuffer commandBuffer) {
    const auto excluded = [&exclude](VkDynamicState state) {
        return std::find(exclude.begin(), exclude.end(), state) != exclude.end();
    };
    vertexBuffer.init(*m_device, 32u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer cmdBuffer = commandBuffer ? commandBuffer : m_commandBuffer->handle();
    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    if (!excluded(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)) vk::CmdSetViewportWithCountEXT(cmdBuffer, 1u, &viewport);
    if (!excluded(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) vk::CmdSetScissorWithCountEXT(cmdBuffer, 1u, &scissor);
    if (!excluded(VK_DYNAMIC_STATE_LINE_WIDTH)) vk::CmdSetLineWidth(cmdBuffer, 1.0f);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BIAS)) vk::CmdSetDepthBias(cmdBuffer, 1.0f, 0.0f, 1.0f);
    float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    if (!excluded(VK_DYNAMIC_STATE_BLEND_CONSTANTS)) vk::CmdSetBlendConstants(cmdBuffer, blendConstants);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BOUNDS)) vk::CmdSetDepthBounds(cmdBuffer, 0.0f, 1.0f);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK))
        vk::CmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK))
        vk::CmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_REFERENCE))
        vk::CmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    VkDeviceSize offset = 0u;
    VkDeviceSize size = sizeof(float);
    if (!excluded(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE))
        vk::CmdBindVertexBuffers2EXT(cmdBuffer, 0, 1, &vertexBuffer.handle(), &offset, &size, &size);
    if (!excluded(VK_DYNAMIC_STATE_CULL_MODE)) vk::CmdSetCullModeEXT(cmdBuffer, VK_CULL_MODE_NONE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) vk::CmdSetDepthBoundsTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP)) vk::CmdSetDepthCompareOpEXT(cmdBuffer, VK_COMPARE_OP_NEVER);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) vk::CmdSetDepthTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE)) vk::CmdSetDepthWriteEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_FRONT_FACE)) vk::CmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_CLOCKWISE);
    if (!excluded(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY))
        vk::CmdSetPrimitiveTopologyEXT(cmdBuffer,
                                       tessellation ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_OP))
        vk::CmdSetStencilOpEXT(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
                               VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) vk::CmdSetStencilTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) vk::CmdSetDepthBiasEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) vk::CmdSetPrimitiveRestartEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)) vk::CmdSetRasterizerDiscardEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) vk::CmdSetVertexInputEXT(cmdBuffer, 0u, nullptr, 0u, nullptr);
    if (!excluded(VK_DYNAMIC_STATE_LOGIC_OP_EXT)) vk::CmdSetLogicOpEXT(cmdBuffer, VK_LOGIC_OP_COPY);
    if (!excluded(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) vk::CmdSetPatchControlPointsEXT(cmdBuffer, 4u);
    if (!excluded(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT))
        vk::CmdSetTessellationDomainOriginEXT(cmdBuffer, VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) vk::CmdSetDepthClampEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_POLYGON_MODE_EXT)) vk::CmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    if (!excluded(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) vk::CmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    if (!excluded(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)) vk::CmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    if (!excluded(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT)) vk::CmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT)) vk::CmdSetAlphaToOneEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) vk::CmdSetLogicOpEnableEXT(cmdBuffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    if (!excluded(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)) vk::CmdSetColorBlendEnableEXT(cmdBuffer, 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    if (!excluded(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT))
        vk::CmdSetColorBlendEquationEXT(cmdBuffer, 0u, 1u, &colorBlendEquation);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (!excluded(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) vk::CmdSetColorWriteMaskEXT(cmdBuffer, 0u, 1u, &colorWriteMask);
}

TEST_F(PositiveShaderObject, CreateAndDestroyShaderObject) {
    TEST_DESCRIPTION("Create and destroy shader object.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);

    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
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

    RETURN_IF_SKIP(InitBasicShaderObject())

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    const vkt::Shader vertShader(*m_device, stage, GLSLToSPV(stage, kVertexMinimalGlsl));

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &vertShader.handle());
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawWithVertAndFragShaderObjects) {
    TEST_DESCRIPTION("Draw with only vertex and fragment shader objects bound.");

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
}

TEST_F(PositiveShaderObject, DrawWithVertAndFragBinaryShaderObjects) {
    TEST_DESCRIPTION("Draw with binary vertex and fragment shader objects bound.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    size_t vertDataSize;
    vk::GetShaderBinaryDataEXT(*m_device, vertShader.handle(), &vertDataSize, nullptr);
    std::vector<uint8_t> vertData(vertDataSize);
    vk::GetShaderBinaryDataEXT(*m_device, vertShader.handle(), &vertDataSize, vertData.data());

    size_t fragDataSize;
    vk::GetShaderBinaryDataEXT(*m_device, fragShader.handle(), &fragDataSize, nullptr);
    std::vector<uint8_t> fragData(fragDataSize);
    vk::GetShaderBinaryDataEXT(*m_device, fragShader.handle(), &fragDataSize, fragData.data());

    vkt::Shader binaryVertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vertData);
    vkt::Shader binaryFragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, fragData);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(binaryVertShader, binaryFragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, LinkedVertexAndFragmentShaders) {
    TEST_DESCRIPTION("Create linked vertex and fragment shaders.");

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
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[1].pCode = frag_spv.data();
    createInfos[1].pName = "main";

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2u, createInfos, nullptr, shaders);

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, LinkedGraphicsShaders) {
    TEST_DESCRIPTION("Create linked vertex and fragment shaders.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const auto tesc_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    const auto tese_spv = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    const auto geom_spv = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[5];
    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfos[0].nextStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    createInfos[0].pCode = vert_spv.data();
    createInfos[0].pName = "main";
    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    createInfos[1].nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tesc_spv.size() * sizeof(tesc_spv[0]);
    createInfos[1].pCode = tesc_spv.data();
    createInfos[1].pName = "main";
    createInfos[2] = vku::InitStructHelper();
    createInfos[2].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[2].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    createInfos[2].nextStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfos[2].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[2].codeSize = tese_spv.size() * sizeof(tese_spv[0]);
    createInfos[2].pCode = tese_spv.data();
    createInfos[2].pName = "main";
    createInfos[3] = vku::InitStructHelper();
    createInfos[3].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[3].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    createInfos[3].nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[3].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[3].codeSize = geom_spv.size() * sizeof(geom_spv[0]);
    createInfos[3].pCode = geom_spv.data();
    createInfos[3].pName = "main";
    createInfos[4] = vku::InitStructHelper();
    createInfos[4].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[4].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfos[4].nextStage = 0u;
    createInfos[4].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[4].codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    createInfos[4].pCode = frag_spv.data();
    createInfos[4].pName = "main";

    VkShaderEXT shaders[5];
    vk::CreateShadersEXT(m_device->handle(), 5u, createInfos, nullptr, shaders);

    for (uint32_t i = 0; i < 5; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, MissingCmdSetDepthBiasEnable) {
    TEST_DESCRIPTION("Draw with shaders without setting depth bias enable.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const vkt::Shader vertShader(*m_device, stages[0], GLSLToSPV(stages[0], kVertexMinimalGlsl));
    const vkt::Shader fragShader(*m_device, stages[1], GLSLToSPV(stages[1], kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE});
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, VertFragShaderDraw) {
    TEST_DESCRIPTION("Test drawing with a vertex and fragment shader");

    RETURN_IF_SKIP(InitBasicShaderObject())

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    VkShaderStageFlagBits unusedShaderStages[] = {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT};

    const vkt::Shader vertShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], vert_src));
    const vkt::Shader fragShader(*m_device, shaderStages[1], GLSLToSPV(shaderStages[1], frag_src));

    VkShaderEXT shaders[] = {vertShader.handle(), fragShader.handle()};

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkImageCreateInfo imageInfo = vku::InitStructHelper();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent.width = static_cast<uint32_t>(m_width);
    begin_rendering_info.renderArea.extent.height = static_cast<uint32_t>(m_height);
    begin_rendering_info.layerCount = 1u;
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vk::CmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2u, shaderStages, shaders);
    for (const auto &unusedShader : unusedShaderStages) {
        VkShaderEXT null_shader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &unusedShader, &null_shader);
    }
    SetDefaultDynamicStates();
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vk::CmdEndRenderingKHR(m_commandBuffer->handle());

    {
        VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
    }

    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0u;
    copyRegion.imageSubresource.baseArrayLayer = 0u;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageOffset.x = static_cast<int32_t>(m_width / 2) + 1;
    copyRegion.imageOffset.y = static_cast<int32_t>(m_height / 2) + 1;
    copyRegion.imageExtent.width = 1u;
    copyRegion.imageExtent.height = 1u;
    copyRegion.imageExtent.depth = 1u;

    vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u, &copyRegion);

    m_commandBuffer->end();

    VkCommandBuffer commandBufferHandle = m_commandBuffer->handle();
    VkSubmitInfo submitInfo = vku::InitStructHelper();
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    vk::QueueSubmit(m_default_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveShaderObject, DrawWithAllGraphicsShaderStagesUsed) {
    TEST_DESCRIPTION("Test drawing using all graphics shader");

    RETURN_IF_SKIP(InitBasicShaderObject())
    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);
    if (!features.tessellationShader || !features.geometryShader) {
        GTEST_SKIP() << "Required shaders not supported.";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[5];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    const vkt::Shader vertShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], vert_src));
    const vkt::Shader tescShader(*m_device, shaderStages[1], GLSLToSPV(shaderStages[1], tesc_src));
    const vkt::Shader teseShader(*m_device, shaderStages[2], GLSLToSPV(shaderStages[2], tese_src));
    const vkt::Shader geomShader(*m_device, shaderStages[3], GLSLToSPV(shaderStages[3], geom_src));
    const vkt::Shader fragShader(*m_device, shaderStages[4], GLSLToSPV(shaderStages[4], frag_src));

    VkShaderEXT shaders[5] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), geomShader.handle(),
                              fragShader.handle()};

    VkImageCreateInfo imageInfo = vku::InitStructHelper();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

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

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vk::CmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, shaderStages, shaders);
    SetDefaultDynamicStates();
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    vk::CmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    VkCommandBuffer commandBufferHandle = m_commandBuffer->handle();
    VkSubmitInfo submitInfo = vku::InitStructHelper();
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    vk::QueueSubmit(m_default_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveShaderObject, ComputeShader) {
    TEST_DESCRIPTION("Test dispatching with compute shader");

    RETURN_IF_SKIP(InitBasicShaderObject())

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

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_COMPUTE_BIT};

    vkt::Buffer storageBuffer(*m_device, sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);

    VkDescriptorBufferInfo storage_buffer_info = {storageBuffer.handle(), 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptorWrite = vku::InitStructHelper();
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.pBufferInfo = &storage_buffer_info;

    vk::UpdateDescriptorSets(m_device->handle(), 1u, &descriptorWrite, 0u, nullptr);

    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {dsl_binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});

    VkDescriptorSetLayout descriptorSetLayout = descriptor_set_layout.handle();

    const vkt::Shader compShader(*m_device, shaderStages[0], GLSLToSPV(shaderStages[0], comp_src), &descriptorSetLayout);

    m_commandBuffer->begin();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                              &descriptorSet, 0u, nullptr);

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, shaderStages, &compShader.handle());
    // setdefaul(vertexBuffer.handle(), false);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);

    m_commandBuffer->end();

    VkCommandBuffer commandBufferHandle = m_commandBuffer->handle();
    VkSubmitInfo submitInfo = vku::InitStructHelper();
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    vk::QueueSubmit(m_default_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveShaderObject, TaskMeshShadersDraw) {
    TEST_DESCRIPTION("Test drawing using task and mesh shaders");

    RETURN_IF_SKIP(InitBasicMeshShaderObject(nullptr, VK_API_VERSION_1_3));

    VkPhysicalDeviceFeatures features;
    GetPhysicalDeviceFeatures(&features);

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

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    const vkt::Shader taskShader(*m_device, shaderStages[0],
                                 GLSLToSPV(shaderStages[0], task_src, "main", nullptr, SPV_ENV_VULKAN_1_3));
    const vkt::Shader meshShader(*m_device, shaderStages[1],
                                 GLSLToSPV(shaderStages[1], mesh_src, "main", nullptr, SPV_ENV_VULKAN_1_3));
    const vkt::Shader fragShader(*m_device, shaderStages[2], GLSLToSPV(shaderStages[2], frag_src));

    VkShaderEXT shaders[3] = {taskShader.handle(), meshShader.handle(), fragShader.handle()};

    VkImageCreateInfo imageInfo = vku::InitStructHelper();
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0u;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj image(m_device);
    image.init(&imageInfo);
    VkImageView view = image.targetView(imageInfo.format);

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

    m_commandBuffer->begin();

    {
        VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image.handle();
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
        imageMemoryBarrier.subresourceRange.levelCount = 1u;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
        imageMemoryBarrier.subresourceRange.layerCount = 1u;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                               &imageMemoryBarrier);
    }
    vk::CmdBeginRenderingKHR(m_commandBuffer->handle(), &begin_rendering_info);
    std::vector<VkShaderStageFlagBits> nullStages = {VK_SHADER_STAGE_VERTEX_BIT};
    if (features.tessellationShader) {
        nullStages.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        nullStages.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }
    if (features.geometryShader) {
        nullStages.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    for (const auto stage : nullStages) {
        VkShaderEXT nullShader = VK_NULL_HANDLE;
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &nullShader);
    }

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 3u, shaderStages, shaders);
    SetDefaultDynamicStates();
    vk::CmdDrawMeshTasksEXT(m_commandBuffer->handle(), 1, 1, 1);
    vk::CmdEndRenderingKHR(m_commandBuffer->handle());

    m_commandBuffer->end();

    VkCommandBuffer commandBufferHandle = m_commandBuffer->handle();
    VkSubmitInfo submitInfo = vku::InitStructHelper();
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    vk::QueueSubmit(m_default_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveShaderObject, FailCreateShaders) {
    TEST_DESCRIPTION("Test failing to create shaders");

    RETURN_IF_SKIP(InitBasicShaderObject())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    constexpr uint32_t stages_count = 5;
    constexpr uint32_t shaders_count = 20;
    constexpr uint32_t fail_index = 15;

    VkShaderStageFlagBits shaderStages[stages_count] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                        VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[stages_count];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkShaderEXT shaders[shaders_count];

    VkShaderCreateInfoEXT createInfos[shaders_count];
    for (uint32_t i = 0; i < shaders_count; ++i) {
        createInfos[i] = vku::InitStructHelper();
        createInfos[i].stage = shaderStages[i % stages_count];
        createInfos[i].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfos[i].codeSize = spv[i % stages_count].size() * sizeof(uint32_t);
        createInfos[i].pCode = spv[i % stages_count].data();
        createInfos[i].pName = "main";
    }

    // Binary code must be aligned to 16 bytes
    std::vector<uint8_t> garbage(createInfos[fail_index].codeSize + 16);
    auto pCode = reinterpret_cast<std::uintptr_t>(garbage.data());
    while (pCode % 16 != 0) {
        pCode += 1;
    }
    std::memcpy(reinterpret_cast<void *>(pCode), createInfos[fail_index].pCode, createInfos[fail_index].codeSize);
    createInfos[fail_index].codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfos[fail_index].pCode = reinterpret_cast<const void *>(pCode);

    VkResult res = vk::CreateShadersEXT(m_device->handle(), 20u, createInfos, nullptr, shaders);
    ASSERT_EQ(res, VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);

    for (uint32_t i = 0; i < shaders_count; ++i) {
        if (i < fail_index) {
            vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
        }
    }
}

TEST_F(PositiveShaderObject, DrawMinimalDynamicStates) {
    TEST_DESCRIPTION("Draw with only required dynamic states set.");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_1, false))

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1u, &scissor);
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetPolygonModeEXT(m_commandBuffer->handle(), VK_POLYGON_MODE_FILL);
    vk::CmdSetRasterizationSamplesEXT(m_commandBuffer->handle(), VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 1u;
    vk::CmdSetSampleMaskEXT(m_commandBuffer->handle(), VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vk::CmdSetAlphaToCoverageEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetCullModeEXT(m_commandBuffer->handle(), VK_CULL_MODE_NONE);
    vk::CmdSetDepthTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetDepthBoundsTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetDepthBiasEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 0u, nullptr, 0u, nullptr);
    vk::CmdSetPrimitiveRestartEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0u, 1u, &colorBlendEnable);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_commandBuffer->handle(), 0u, 1u, &colorWriteMask);

    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawMinimalDynamicStatesRasterizationDisabled) {
    TEST_DESCRIPTION("Draw with only required dynamic states set.");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_1, false))

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    vk::CmdSetViewportWithCountEXT(m_commandBuffer->handle(), 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(m_commandBuffer->handle(), 1u, &scissor);
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdSetStencilTestEnableEXT(m_commandBuffer->handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_commandBuffer->handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 0u, nullptr, 0u, nullptr);
    vk::CmdSetPrimitiveRestartEnableEXT(m_commandBuffer->handle(), VK_FALSE);

    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, ShadersDescriptorSets) {
    TEST_DESCRIPTION("Draw with shaders using multiple descriptor sets.");

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

    auto image_ci =
        VkImageObj::ImageCreateInfo2D(64, 64, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_info);

    frag_descriptor_set.WriteDescriptorImageInfo(0, view, sampler.handle());
    frag_descriptor_set.UpdateDescriptorSets();

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
}

TEST_F(PositiveShaderObject, MultiplePushConstants) {
    TEST_DESCRIPTION("Draw with shaders using multiple push constants.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        layout (push_constant) uniform constants {
	        int pos;
        } pushConst;
        void main() {
            gl_Position = vec4(pushConst.pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout (push_constant) uniform constants {
	        layout(offset = 4) float c;
        } pushConst;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(pushConst.c);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkPushConstantRange push_constant_ranges[2];
    push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_ranges[0].offset = 0u;
    push_constant_ranges[0].size = sizeof(int);
    push_constant_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges[1].offset = sizeof(int);
    push_constant_ranges[1].size = sizeof(float);
    vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_ranges[0], push_constant_ranges[1]});

    VkShaderCreateInfoEXT vert_create_info = vku::InitStructHelper();
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vert_create_info.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vert_create_info.pCode = vert_spv.data();
    vert_create_info.pName = "main";
    vert_create_info.pushConstantRangeCount = 2u;
    vert_create_info.pPushConstantRanges = push_constant_ranges;

    VkShaderCreateInfoEXT frag_create_info = vku::InitStructHelper();
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    frag_create_info.codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    frag_create_info.pCode = frag_spv.data();
    frag_create_info.pName = "main";
    frag_create_info.pushConstantRangeCount = 2u;
    frag_create_info.pPushConstantRanges = push_constant_ranges;

    const vkt::Shader vertShader(*m_device, vert_create_info);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    int pos = 1;
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(int), &pos);
    float color = 1.0f;
    vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(int),
                         sizeof(float), &color);

    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, MultipleSpecializationConstants) {
    TEST_DESCRIPTION("Draw with shaders using multiple specialization constants.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        layout (constant_id = 0) const int pos = 1;
        void main() {
            gl_Position = vec4(pos);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout (constant_id = 1) const float c = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(c);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src);

    VkSpecializationMapEntry map_entries[2];
    map_entries[0].constantID = 0u;
    map_entries[0].offset = 0u;
    map_entries[0].size = sizeof(int);
    map_entries[1].constantID = 1u;
    map_entries[1].offset = sizeof(int);
    map_entries[1].size = sizeof(float);

    struct Data {
        int pos = 0u;
        float color = 1.0f;
    } data;

    VkSpecializationInfo specialization_info;
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = map_entries;
    specialization_info.dataSize = sizeof(int) + sizeof(float);
    specialization_info.pData = &data;

    VkShaderCreateInfoEXT vert_create_info = vku::InitStructHelper();
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vert_create_info.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vert_create_info.pCode = vert_spv.data();
    vert_create_info.pName = "main";
    vert_create_info.pSpecializationInfo = &specialization_info;

    VkShaderCreateInfoEXT frag_create_info = vku::InitStructHelper();
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    frag_create_info.codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    frag_create_info.pCode = frag_spv.data();
    frag_create_info.pName = "main";
    frag_create_info.pSpecializationInfo = &specialization_info;

    const vkt::Shader vertShader(*m_device, vert_create_info);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, IndirectDraw) {
    TEST_DESCRIPTION("Draw with all 5 shaders stages using indirect draw and seconary command buffers.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        void main() {
            vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
            gl_Position = vec4(pos - 0.5f, 0.0f, 1.0f);;
        }
    )glsl";

    static const char tesc_src[] = R"glsl(
        #version 450
        layout(vertices = 4) out;
        void main (void) {
            if (gl_InvocationID == 0) {
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 1.0;
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;
                gl_TessLevelOuter[3] = 1.0;
            }
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    static const char tese_src[] = R"glsl(
        #version 450
        layout(quads, equal_spacing) in;
        void main (void) {
            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;
            float omu = 1.0f - u;
            float omv = 1.0f - v;
            gl_Position = omu * omv * gl_in[0].gl_Position + u * omv * gl_in[2].gl_Position + u * v * gl_in[3].gl_Position + omu * v * gl_in[1].gl_Position;
            gl_Position.x *= 1.5f;
        }
    )glsl";

    static const char geom_src[] = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 4) out;

        void main(void)
        {
            gl_Position = gl_in[0].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[1].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            gl_Position = gl_in[2].gl_Position;
            gl_Position.y *= 1.5f;
            gl_Position.z = 0.5f;
            EmitVertex();
            EndPrimitive();
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src));
    const vkt::Shader tescShader(*m_device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, tesc_src));
    const vkt::Shader teseShader(*m_device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, tese_src));
    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, geom_src));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src));

    vkt::Buffer indirect_buffer(*m_device, 32, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    SetDefaultDynamicStates();

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), tescShader.handle(), teseShader.handle(), geomShader.handle(),
                                   fragShader.handle()};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);

    vk::CmdDrawIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0u, 1u, 0u);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawInSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Draw in secondary command buffers.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, 0u);

    vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    command_buffer.begin();
    command_buffer.BeginRenderingColor(GetDynamicRenderTarget());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStates({}, true, command_buffer.handle());
    vk::CmdDraw(command_buffer.handle(), 4, 1, 0, 0);
    command_buffer.EndRendering();
    command_buffer.end();

    m_commandBuffer->begin();
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &command_buffer.handle());
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, OutputToMultipleAttachments) {
    TEST_DESCRIPTION("Draw with fragment shader writing to multiple attachments.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    InitDynamicRenderTarget();

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor1;
        layout(location = 1) out vec4 uFragColor2;
        void main(){
           uFragColor1 = vec4(0,1,0,1);
           uFragColor2 = vec4(1,0,1,0);
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_src));

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
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawWithNonBlendableFormat) {
    TEST_DESCRIPTION("Draw with shader objects to an attachment format that does not support blending.");

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
    VkBool32 enabled = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_commandBuffer->handle(), 0, 1, &enabled);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawInSecondaryCommandBuffersWithRenderPassContinue) {
    TEST_DESCRIPTION("Draw in secondary command buffers with render pass continue flag.");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_3))

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    const std::optional<uint32_t> graphics_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, 0u);

    vkt::CommandPool command_pool(*m_device, graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(m_device, &command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferInheritanceRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &m_render_target_fmt;
    rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper(&rendering_info);
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &hinfo;
    command_buffer.begin(&begin_info);
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fragShader.handle()};
    vk::CmdBindShadersEXT(command_buffer.handle(), 5u, stages, shaders);
    SetDefaultDynamicStates({}, true, command_buffer.handle());
    vk::CmdDraw(command_buffer.handle(), 4, 1, 0, 0);
    command_buffer.end();

    m_commandBuffer->begin();

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = GetDynamicRenderTarget();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->BeginRendering(renderingInfo);

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &command_buffer.handle());
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawRebindingShaders) {
    TEST_DESCRIPTION("Draw after rebinding only some shaders.");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_3))

    InitDynamicRenderTarget();

    const VkShaderStageFlagBits vertStage = VK_SHADER_STAGE_VERTEX_BIT;
    const VkShaderStageFlagBits tescStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    const VkShaderStageFlagBits teseStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    const VkShaderStageFlagBits geomStage = VK_SHADER_STAGE_GEOMETRY_BIT;
    const VkShaderStageFlagBits fragStage = VK_SHADER_STAGE_FRAGMENT_BIT;

    const vkt::Shader vertShader(*m_device, vertStage, GLSLToSPV(vertStage, kVertexMinimalGlsl));
    const vkt::Shader tescShader(*m_device, tescStage, GLSLToSPV(tescStage, kTessellationControlMinimalGlsl));
    const vkt::Shader teseShader(*m_device, teseStage, GLSLToSPV(teseStage, kTessellationEvalMinimalGlsl));
    const vkt::Shader geomShader(*m_device, geomStage, GLSLToSPV(geomStage, kGeometryMinimalGlsl));
    const vkt::Shader fragShader(*m_device, fragStage, GLSLToSPV(fragStage, kFragmentMinimalGlsl));

    const VkShaderEXT nullShader = VK_NULL_HANDLE;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());

    SetDefaultDynamicStates();

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &vertStage, &vertShader.handle());
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &tescStage, &nullShader);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &teseStage, &nullShader);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &geomStage, &nullShader);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &fragStage, &fragShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &geomStage, &geomShader.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &tescStage, &tescShader.handle());
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &teseStage, &teseShader.handle());
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &geomStage, &nullShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &fragStage, &nullShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4u, 1u, 0u, 0u);

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, TestVertexAttributeMatching) {
    TEST_DESCRIPTION("Test vertex inputs.");

    VkPhysicalDeviceVulkanMemoryModelFeatures vulkanMemoryModelFeatures = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicShaderObject(&vulkanMemoryModelFeatures))
    if (!vulkanMemoryModelFeatures.vulkanMemoryModel || !vulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope) {
        GTEST_SKIP() << "vulkanMemoryModel or vulkanMemoryModelDeviceScope not supported.";
    }
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 460
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(location = 0) in int pos;
        layout(location = 0) out int64_t pos1;
        void main() {
            gl_Position = vec4(pos);
            pos1 = 0;
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
    vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_UINT;
    vertexAttributeDescription.offset = 0u;

    vk::CmdSetVertexInputEXT(m_commandBuffer->handle(), 1u, &vertexBindingDescription, 1u, &vertexAttributeDescription);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, DrawWithBinaryShaders) {
    TEST_DESCRIPTION("Draw using binary shaders.");

    RETURN_IF_SKIP(InitBasicShaderObject())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    InitDynamicRenderTarget();

    VkShaderStageFlagBits shaderStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    std::vector<uint32_t> spv[5];
    spv[0] = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    spv[1] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, kTessellationControlMinimalGlsl);
    spv[2] = GLSLToSPV(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, kTessellationEvalMinimalGlsl);
    spv[3] = GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, kGeometryMinimalGlsl);
    spv[4] = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkShaderEXT shaders[5];
    VkShaderEXT binaryShaders[5];
    for (uint32_t i = 0; i < 5u; ++i) {
        VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
        createInfo.stage = shaderStages[i];
        createInfo.nextStage = 0u;
        if (i < 4) {
            createInfo.nextStage = shaderStages[i + 1];
        }
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.codeSize = spv[i].size() * sizeof(spv[i][0]);
        createInfo.pCode = spv[i].data();
        createInfo.pName = "main";

        vk::CreateShadersEXT(*m_device, 1u, &createInfo, nullptr, &shaders[i]);
        size_t dataSize;
        vk::GetShaderBinaryDataEXT(*m_device, shaders[i], &dataSize, nullptr);
        std::vector<uint8_t> data(dataSize);
        vk::GetShaderBinaryDataEXT(*m_device, shaders[i], &dataSize, data.data());

        createInfo.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
        createInfo.codeSize = dataSize;
        createInfo.pCode = data.data();
        vk::CreateShadersEXT(*m_device, 1u, &createInfo, nullptr, &binaryShaders[i]);
    }

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, shaderStages, binaryShaders);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < 5; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
        vk::DestroyShaderEXT(*m_device, binaryShaders[i], nullptr);
    }
}

TEST_F(PositiveShaderObject, NotSettingDepthBounds) {
    TEST_DESCRIPTION("Draw without setting depth bounds.");

    RETURN_IF_SKIP(InitBasicShaderObject())

    InitDynamicRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates({VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_BOUNDS});
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    BindVertFragShader(vertShader, fragShader);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderObject, CreateAndDrawLinkedAndUnlinkedShaders) {
    TEST_DESCRIPTION("Create and draw with some linked and some unlinked shaders.");

    RETURN_IF_SKIP(InitBasicShaderObject(nullptr, VK_API_VERSION_1_3))

    InitDynamicRenderTarget();

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};

    const auto vertSpirv = GLSLToSPV(stages[0], kVertexMinimalGlsl);
    const auto tescSpirv = GLSLToSPV(stages[1], kTessellationControlMinimalGlsl);
    const auto teseSpirv = GLSLToSPV(stages[2], kTessellationEvalMinimalGlsl);
    const auto geomSpirv = GLSLToSPV(stages[3], kGeometryMinimalGlsl);
    const auto fragSpirv = GLSLToSPV(stages[4], kFragmentMinimalGlsl);

    VkShaderCreateInfoEXT createInfos[5];

    createInfos[0] = vku::InitStructHelper();
    createInfos[0].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[0].stage = stages[0];
    createInfos[0].nextStage = stages[1];
    createInfos[0].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[0].codeSize = vertSpirv.size() * sizeof(vertSpirv[0]);
    createInfos[0].pCode = vertSpirv.data();
    createInfos[0].pName = "main";

    createInfos[1] = vku::InitStructHelper();
    createInfos[1].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[1].stage = stages[1];
    createInfos[1].nextStage = stages[2];
    createInfos[1].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[1].codeSize = tescSpirv.size() * sizeof(tescSpirv[0]);
    createInfos[1].pCode = tescSpirv.data();
    createInfos[1].pName = "main";

    createInfos[2] = vku::InitStructHelper();
    createInfos[2].flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    createInfos[2].stage = stages[2];
    createInfos[2].nextStage = stages[3] | stages[4];
    createInfos[2].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[2].codeSize = teseSpirv.size() * sizeof(teseSpirv[0]);
    createInfos[2].pCode = teseSpirv.data();
    createInfos[2].pName = "main";

    createInfos[3] = vku::InitStructHelper();
    createInfos[3].stage = stages[3];
    createInfos[3].nextStage = stages[4];
    createInfos[3].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[3].codeSize = geomSpirv.size() * sizeof(geomSpirv[0]);
    createInfos[3].pCode = geomSpirv.data();
    createInfos[3].pName = "main";

    createInfos[4] = vku::InitStructHelper();
    createInfos[4].stage = stages[4];
    createInfos[4].nextStage = 0u;
    createInfos[4].codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfos[4].codeSize = fragSpirv.size() * sizeof(fragSpirv[0]);
    createInfos[4].pCode = fragSpirv.data();
    createInfos[4].pName = "main";

    VkShaderEXT shaders[5];
    vk::CreateShadersEXT(*m_device, 3u, createInfos, nullptr, shaders);
    for (uint32_t i = 3u; i < 5u; ++i) {
        vk::CreateShadersEXT(*m_device, 1u, &createInfos[i], nullptr, &shaders[i]);
    }

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    SetDefaultDynamicStates();

    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);

    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < 5u; ++i) {
        vk::DestroyShaderEXT(*m_device, shaders[i], nullptr);
    }
}
