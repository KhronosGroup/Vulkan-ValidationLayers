/*
 * Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
 * Copyright (c) 2020-2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"

// Until we land the glsl/spirv tooling, these are based on
//
// layout (set = 0, binding = 0) buffer SSBO {
//     uint data;
// } x;
//
// layout (set = 0, binding = 1) uniform UBO {
//     uint data;
// } y;
//
// void main() {
//     uint a = y.data;
//     x.data = a;
// }
//

// void main() {
//     DebugDescriptor(x);
//     uint a = y.data;
//     x.data = a;
// }
[[maybe_unused]] static const uint32_t spv_simple_size = 168;
[[maybe_unused]] static const uint32_t spv_simple_data[168] = {
    0x07230203, 0x00010500, 0x00070000, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0008000a, 0x5f565053, 0x5f52484b,
    0x5f6e6f6e, 0x616d6573, 0x6369746e, 0x666e695f, 0x0000006f, 0x0009000b, 0x00000001, 0x536e6f4e, 0x6e616d65, 0x2e636974,
    0x75626544, 0x73654467, 0x70697263, 0x00726f74, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000005, 0x00000002,
    0x6e69616d, 0x00000000, 0x00000003, 0x00000004, 0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
    0x00030047, 0x00000005, 0x00000002, 0x00050048, 0x00000005, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000003,
    0x00000021, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00030047, 0x00000006, 0x00000002, 0x00050048,
    0x00000006, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000004, 0x00000021, 0x00000000, 0x00040047, 0x00000004,
    0x00000022, 0x00000000, 0x00020013, 0x00000007, 0x00030021, 0x00000008, 0x00000007, 0x00040015, 0x00000009, 0x00000020,
    0x00000000, 0x00040020, 0x0000000a, 0x00000007, 0x00000009, 0x0003001e, 0x00000005, 0x00000009, 0x00040020, 0x0000000b,
    0x00000002, 0x00000005, 0x0004003b, 0x0000000b, 0x00000003, 0x00000002, 0x00040015, 0x0000000c, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020, 0x0000000e, 0x00000002, 0x00000009, 0x0003001e, 0x00000006,
    0x00000009, 0x00040020, 0x0000000f, 0x0000000c, 0x00000006, 0x0004003b, 0x0000000f, 0x00000004, 0x0000000c, 0x00040020,
    0x00000010, 0x0000000c, 0x00000009, 0x00050036, 0x00000007, 0x00000002, 0x00000000, 0x00000008, 0x000200f8, 0x00000011,
    0x0004003b, 0x0000000a, 0x00000012, 0x00000007, 0x0008000c, 0x00000007, 0x00000013, 0x00000001, 0x00000001, 0x00000004,
    0x0000000d, 0x0000000d, 0x00050041, 0x0000000e, 0x00000014, 0x00000003, 0x0000000d, 0x0004003d, 0x00000009, 0x00000015,
    0x00000014, 0x0003003e, 0x00000012, 0x00000015, 0x0004003d, 0x00000009, 0x00000016, 0x00000012, 0x00050041, 0x00000010,
    0x00000017, 0x00000004, 0x0000000d, 0x0003003e, 0x00000017, 0x00000016, 0x000100fd, 0x00010038};


// void main() {
//     uint a = y.data;
//     x.data = a;
//     DebugDescriptorAll();
// }
[[maybe_unused]] static const uint32_t spv_simple_all_size = 166;
[[maybe_unused]] static const uint32_t spv_simple_all_data[166] = {
    0x07230203, 0x00010500, 0x00070000, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0008000a, 0x5f565053, 0x5f52484b,
    0x5f6e6f6e, 0x616d6573, 0x6369746e, 0x666e695f, 0x0000006f, 0x0009000b, 0x00000001, 0x536e6f4e, 0x6e616d65, 0x2e636974,
    0x75626544, 0x73654467, 0x70697263, 0x00726f74, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000005, 0x00000002,
    0x6e69616d, 0x00000000, 0x00000003, 0x00000004, 0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
    0x00030047, 0x00000005, 0x00000002, 0x00050048, 0x00000005, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000003,
    0x00000021, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00030047, 0x00000006, 0x00000002, 0x00050048,
    0x00000006, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000004, 0x00000021, 0x00000000, 0x00040047, 0x00000004,
    0x00000022, 0x00000000, 0x00020013, 0x00000007, 0x00030021, 0x00000008, 0x00000007, 0x00040015, 0x00000009, 0x00000020,
    0x00000000, 0x00040020, 0x0000000a, 0x00000007, 0x00000009, 0x0003001e, 0x00000005, 0x00000009, 0x00040020, 0x0000000b,
    0x00000002, 0x00000005, 0x0004003b, 0x0000000b, 0x00000003, 0x00000002, 0x00040015, 0x0000000c, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020, 0x0000000e, 0x00000002, 0x00000009, 0x0003001e, 0x00000006,
    0x00000009, 0x00040020, 0x0000000f, 0x0000000c, 0x00000006, 0x0004003b, 0x0000000f, 0x00000004, 0x0000000c, 0x00040020,
    0x00000010, 0x0000000c, 0x00000009, 0x00050036, 0x00000007, 0x00000002, 0x00000000, 0x00000008, 0x000200f8, 0x00000011,
    0x0004003b, 0x0000000a, 0x00000012, 0x00000007, 0x00050041, 0x0000000e, 0x00000013, 0x00000003, 0x0000000d, 0x0004003d,
    0x00000009, 0x00000014, 0x00000013, 0x0003003e, 0x00000012, 0x00000014, 0x0004003d, 0x00000009, 0x00000015, 0x00000012,
    0x00050041, 0x00000010, 0x00000016, 0x00000004, 0x0000000d, 0x0003003e, 0x00000016, 0x00000015, 0x0006000c, 0x00000007,
    0x00000017, 0x00000001, 0x00000002, 0x0000000d, 0x000100fd, 0x00010038};

// void main() {
//     DebugDescriptor(x, gl_earlyReturn);
//     uint a = y.data;
//     x.data = a;
// }
[[maybe_unused]] static const uint32_t spv_return_early_size = 172;
[[maybe_unused]] static const uint32_t spv_return_early_data[172] = {
    0x07230203, 0x00010500, 0x00070000, 0x00000019, 0x00000000, 0x00020011, 0x00000001, 0x0008000a, 0x5f565053, 0x5f52484b,
    0x5f6e6f6e, 0x616d6573, 0x6369746e, 0x666e695f, 0x0000006f, 0x0009000b, 0x00000001, 0x536e6f4e, 0x6e616d65, 0x2e636974,
    0x75626544, 0x73654467, 0x70697263, 0x00726f74, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000005, 0x00000002,
    0x6e69616d, 0x00000000, 0x00000003, 0x00000004, 0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
    0x00030047, 0x00000005, 0x00000002, 0x00050048, 0x00000005, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000003,
    0x00000021, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00030047, 0x00000006, 0x00000002, 0x00050048,
    0x00000006, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000004, 0x00000021, 0x00000000, 0x00040047, 0x00000004,
    0x00000022, 0x00000000, 0x00020013, 0x00000007, 0x00030021, 0x00000008, 0x00000007, 0x00040015, 0x00000009, 0x00000020,
    0x00000000, 0x00040020, 0x0000000a, 0x00000007, 0x00000009, 0x0003001e, 0x00000005, 0x00000009, 0x00040020, 0x0000000b,
    0x00000002, 0x00000005, 0x0004003b, 0x0000000b, 0x00000003, 0x00000002, 0x00040015, 0x0000000c, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x0004002b, 0x0000000c, 0x0000000e, 0x00000001, 0x00040020, 0x0000000f,
    0x00000002, 0x00000009, 0x0003001e, 0x00000006, 0x00000009, 0x00040020, 0x00000010, 0x0000000c, 0x00000006, 0x0004003b,
    0x00000010, 0x00000004, 0x0000000c, 0x00040020, 0x00000011, 0x0000000c, 0x00000009, 0x00050036, 0x00000007, 0x00000002,
    0x00000000, 0x00000008, 0x000200f8, 0x00000012, 0x0004003b, 0x0000000a, 0x00000013, 0x00000007, 0x0008000c, 0x00000007,
    0x00000014, 0x00000001, 0x00000001, 0x00000004, 0x0000000d, 0x0000000e, 0x00050041, 0x0000000f, 0x00000015, 0x00000003,
    0x0000000d, 0x0004003d, 0x00000009, 0x00000016, 0x00000015, 0x0003003e, 0x00000013, 0x00000016, 0x0004003d, 0x00000009,
    0x00000017, 0x00000013, 0x00050041, 0x00000011, 0x00000018, 0x00000004, 0x0000000d, 0x0003003e, 0x00000018, 0x00000017,
    0x000100fd, 0x00010038};

// void main() {
//     DebugDescriptorAll(gl_earlyReturn);
//     uint a = y.data;
//     x.data = a;
// }
[[maybe_unused]] static const uint32_t spv_return_early_all_size = 170;
[[maybe_unused]] static const uint32_t spv_return_early_all_data[170] = {
    0x07230203, 0x00010500, 0x00070000, 0x00000019, 0x00000000, 0x00020011, 0x00000001, 0x0008000a, 0x5f565053, 0x5f52484b,
    0x5f6e6f6e, 0x616d6573, 0x6369746e, 0x666e695f, 0x0000006f, 0x0009000b, 0x00000001, 0x536e6f4e, 0x6e616d65, 0x2e636974,
    0x75626544, 0x73654467, 0x70697263, 0x00726f74, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000005, 0x00000002,
    0x6e69616d, 0x00000000, 0x00000003, 0x00000004, 0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
    0x00030047, 0x00000005, 0x00000002, 0x00050048, 0x00000005, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000003,
    0x00000021, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00030047, 0x00000006, 0x00000002, 0x00050048,
    0x00000006, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000004, 0x00000021, 0x00000000, 0x00040047, 0x00000004,
    0x00000022, 0x00000000, 0x00020013, 0x00000007, 0x00030021, 0x00000008, 0x00000007, 0x00040015, 0x00000009, 0x00000020,
    0x00000000, 0x00040020, 0x0000000a, 0x00000007, 0x00000009, 0x0003001e, 0x00000005, 0x00000009, 0x00040020, 0x0000000b,
    0x00000002, 0x00000005, 0x0004003b, 0x0000000b, 0x00000003, 0x00000002, 0x00040015, 0x0000000c, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x0004002b, 0x0000000c, 0x0000000e, 0x00000001, 0x00040020, 0x0000000f,
    0x00000002, 0x00000009, 0x0003001e, 0x00000006, 0x00000009, 0x00040020, 0x00000010, 0x0000000c, 0x00000006, 0x0004003b,
    0x00000010, 0x00000004, 0x0000000c, 0x00040020, 0x00000011, 0x0000000c, 0x00000009, 0x00050036, 0x00000007, 0x00000002,
    0x00000000, 0x00000008, 0x000200f8, 0x00000012, 0x0004003b, 0x0000000a, 0x00000013, 0x00000007, 0x0006000c, 0x00000007,
    0x00000014, 0x00000001, 0x00000002, 0x0000000e, 0x00050041, 0x0000000f, 0x00000015, 0x00000003, 0x0000000d, 0x0004003d,
    0x00000009, 0x00000016, 0x00000015, 0x0003003e, 0x00000013, 0x00000016, 0x0004003d, 0x00000009, 0x00000017, 0x00000013,
    0x00050041, 0x00000011, 0x00000018, 0x00000004, 0x0000000d, 0x0003003e, 0x00000018, 0x00000017, 0x000100fd, 0x00010038};

static const VkLayerSettingEXT kDescriptorSetting = {OBJECT_LAYER_NAME, "debug_descriptor", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                     &kVkTrue};
static VkLayerSettingsCreateInfoEXT kDescriptorSettingCreateInfo = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                    &kDescriptorSetting};

class NegativeDebugDescriptor : public VkLayerTest {
  public:
    void InitDebugDescriptor();
};

void NegativeDebugDescriptor::InitDebugDescriptor() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&kDescriptorSettingCreateInfo));
    RETURN_IF_SKIP(InitState());

    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
}

TEST_F(NegativeDebugDescriptor, BufferBasic) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugDescriptor());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size * 4, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info_ssbo(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ssbo_buffer, 32);
    uint8_t* mapped_descriptor_data = (uint8_t*)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info_ssbo, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_ubo(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo_buffer, 16);
    vk::GetDescriptorEXT(device(), get_info_ubo, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    VkShaderModuleCreateInfo module_ci = vku::InitStructHelper();
    module_ci.codeSize = spv_simple_size * sizeof(uint32_t);
    module_ci.pCode = spv_simple_data;

    // spirv-val will not be happy without our spirv headers added
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08737");
    vkt::ShaderModule module(*m_device, module_ci);

    VkPipelineShaderStageCreateInfo stage_info = vku::InitStructHelper();
    stage_info.flags = 0;
    stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_info.module = module;
    stage_info.pName = "main";

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cp_ci_.stage = stage_info;
    pipe.CreateComputePipeline(false);

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
}
