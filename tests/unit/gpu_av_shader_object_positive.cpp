/*
 * Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"

TEST_F(PositiveGpuAVShaderObject, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    validation_features.pNext = &layer_settings_create_info;
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&shader_object_features);
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.robustBufferAccess) {
        GTEST_SKIP() << "Not safe to write outside of buffer memory";
    }
    // Robust buffer access will be on by default
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    InitState(nullptr, &features2, pool_flags);
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_});

    static const char vert_src[] = R"glsl(
        #version 460
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
            Data.data[4] = 0xdeadca71;
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_.handle()};

    VkShaderCreateInfoEXT vert_create_info = vku::InitStructHelper();
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vert_create_info.codeSize = vert_spv.size() * sizeof(vert_spv[0]);
    vert_create_info.pCode = vert_spv.data();
    vert_create_info.pName = "main";
    vert_create_info.setLayoutCount = 1;
    vert_create_info.pSetLayouts = descriptor_set_layouts;

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    vert_create_info.pNext = &features;

    VkShaderCreateInfoEXT frag_create_info = vku::InitStructHelper();
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    frag_create_info.codeSize = frag_spv.size() * sizeof(frag_spv[0]);
    frag_create_info.pCode = frag_spv.data();
    frag_create_info.pName = "main";
    frag_create_info.setLayoutCount = 1;
    frag_create_info.pSetLayouts = descriptor_set_layouts;

    // TODO - Once the spec is changed to allow this, the SetUnexpectedError call can be removed
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderCreateInfoEXT-pNext-pNext");
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    // Should get a warning since shader was instrumented
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "VUID-vkCmdDraw-None-08613");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vert_create_info.pNext = nullptr;
    const vkt::Shader vertShader2(*m_device, vert_create_info);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStates();
    BindVertFragShader(vertShader2, fragShader);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 4, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    // Should not get a warning since shader was not instrumented
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
}
