/*
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
// stype-check off
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "descriptor_helper.h"
#include "pipeline_helper.h"

static const VkLayerSettingEXT kDescriptorSetting = {OBJECT_LAYER_NAME, "gpu_dump_descriptors", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                     &kVkTrue};
class NegativeGpuDump : public VkLayerTest {};

TEST_F(NegativeGpuDump, DumpDescriptors) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkLayerSettingsCreateInfoEXT setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &kDescriptorSetting};
    RETURN_IF_SKIP(InitFramework(&setting_ci));
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::Buffer buffer_data(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);

    void* mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };
        void main() {
            a = 0;
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
    m_errorMonitor->SetDesiredInfo("GPU-DUMP");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeGpuDump, DumpCopyMemoryIndirect) {
    AddRequiredExtensions(VK_KHR_COPY_MEMORY_INDIRECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::indirectMemoryCopy);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    vkt::Buffer src_buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    const VkDeviceAddress src_address = src_buffer.Address();
    const VkDeviceAddress dst_address = dst_buffer.Address();
    VkCopyMemoryIndirectCommandKHR cmds[6] = {{src_address + 0, dst_address, 4},        {src_address + 32, dst_address + 32, 32},
                                              {src_address + 0, dst_address + 64, 16},  {src_address + 64, dst_address + 128, 64},
                                              {src_address + 0, dst_address + 256, 16}, {src_address + 128, dst_address + 512, 4}};
    const VkDeviceSize indirect_buffer_size = sizeof(cmds);

    vkt::Buffer indirect_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    void* indirect_buffer_data = indirect_buffer.Memory().Map();
    memcpy(indirect_buffer_data, cmds, indirect_buffer_size);

    VkStridedDeviceAddressRangeKHR address_range = {};
    address_range.address = indirect_buffer.Address();
    address_range.size = indirect_buffer_size;
    address_range.stride = sizeof(VkCopyMemoryIndirectCommandKHR);

    VkCopyMemoryIndirectInfoKHR copy_info = vku::InitStructHelper();
    copy_info.copyCount = 6;
    copy_info.copyAddressRange = address_range;
    copy_info.srcCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;
    copy_info.dstCopyFlags = VK_ADDRESS_COPY_DEVICE_LOCAL_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryIndirectKHR(m_command_buffer, &copy_info);
    m_command_buffer.End();
}
