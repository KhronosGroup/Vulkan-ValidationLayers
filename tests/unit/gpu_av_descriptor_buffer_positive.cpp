/*
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/gpu_av_helper.h"
#include "utils/math_utils.h"

class PositiveGpuAVDescriptorBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVDescriptorBuffer, BasicCompute) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    m_errorMonitor->SetAllowedFailureMsg("WARNING-Setting-Limit-Adjusted");

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer_data(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 1;
    pipe_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);
    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
            uint b;
            uint c;
        };

        void main() {
            c = a + b;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    ASSERT_TRUE(data[0] == 8);
    ASSERT_TRUE(data[1] == 12);
    ASSERT_TRUE(data[2] == 20);
}

TEST_F(PositiveGpuAVDescriptorBuffer, BasicGraphics) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    m_errorMonitor->SetAllowedFailureMsg("WARNING-Setting-Limit-Adjusted");

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer bda(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo_0(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ubo_1(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    auto ssbo_ptr = (VkDeviceAddress *)ssbo_0.Memory().Map();
    ssbo_ptr[0] = bda.Address();  // ptr
    uint32_t *data = (uint32_t *)ubo_1.Memory().Map();
    data[0] = 0;  // index

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 1;
    pipe_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info_0(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ssbo_0, 16);
    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info_0, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_1(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo_1, 16);
    vk::GetDescriptorEXT(device(), get_info_1, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    const char *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(buffer_reference, std430) buffer Ptr {
            uint i;
        };
        layout (set = 0, binding = 0) buffer SSBO_0 {
            Ptr ptr;
        } SSBO[];
        layout (set = 0, binding = 1) uniform UBO_1 {
            uint index;
        };

        void main() {
            SSBO[index].ptr.i = 11;
        }
    )glsl";

    VkShaderObj vs(*m_device, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    data = (uint32_t *)bda.Memory().Map();
    ASSERT_TRUE(data[0] == 11);
}

TEST_F(PositiveGpuAVDescriptorBuffer, NoPipelineLayout) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    // Once other GPU-AV checks are enabled, don't need to enable to debug printf
    VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
    layer_settings_create_info.settingCount = 1;
    layer_settings_create_info.pSettings = &setting;
    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
    }
    RETURN_IF_SKIP(InitState());

    vkt::Buffer descriptor_buffer(*m_device, 1024, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    const char *cs_source = R"glsl(
        #version 450
        layout (constant_id = 0) const uint c = 3;
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("c == %u\n", c);
        }
    )glsl";
    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source));

    m_command_buffer.Begin();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer, 1, stages, &cs.handle());

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    // Will still force DescriptorModeBuffer
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}