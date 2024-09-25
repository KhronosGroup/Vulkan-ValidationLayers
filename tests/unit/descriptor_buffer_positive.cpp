/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "utils/vk_layer_utils.h"

void DescriptorBufferTest::InitBasicDescriptorBuffer() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(Init());
}

class PositiveDescriptorBuffer : public DescriptorBufferTest {};

TEST_F(PositiveDescriptorBuffer, BasicUsage) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    // *descriptorBufferAddressSpaceSize properties are guaranteed to be 2^27
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;

    {
        buffer_ci.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        vkt::Buffer buffer(*m_device, buffer_ci);
    }

    {
        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        vkt::Buffer buffer(*m_device, buffer_ci);
    }

    {
        buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                          VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        vkt::Buffer buffer(*m_device, buffer_ci);
    }
}

TEST_F(PositiveDescriptorBuffer, BindBufferAndSetOffset) {
    TEST_DESCRIPTION("Bind descriptor buffer and set descriptor offset then draw.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
}

TEST_F(PositiveDescriptorBuffer, PipelineFlags2) {
    TEST_DESCRIPTION("Use descriptor buffer with pipeline created with VkPipelineCreateFlags2CreateInfoKHR");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    VkPipelineCreateFlags2CreateInfoKHR flags_2_ci = vku::InitStructHelper();
    flags_2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT;

    CreatePipelineHelper pipe(*this, &flags_2_ci);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
}

TEST_F(PositiveDescriptorBuffer, BindingMidBuffer) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    m_command_buffer.begin();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.address() + descriptor_buffer_properties.descriptorBufferOffsetAlignment;
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_command_buffer.end();
}

TEST_F(PositiveDescriptorBuffer, Basic) {
    TEST_DESCRIPTION("Tries to use a full workflow.");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.memory().map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;
    buffer_data.memory().unmap();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 1;
    pipe_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = 0;
    vk::GetDescriptorSetLayoutSizeEXT(device(), ds_layout.handle(), &ds_layout_size);

    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    VkDescriptorAddressInfoEXT addr_info = vku::InitStructHelper();
    addr_info.address = buffer_data.address();
    addr_info.range = 16;
    addr_info.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT buffer_descriptor_info = vku::InitStructHelper();
    buffer_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_info.data.pStorageBuffer = &addr_info;

    void *mapped_descriptor_data = descriptor_buffer.memory().map();
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);
    descriptor_buffer.memory().unmap();

    char const *cs_source = R"glsl(
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
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                                         &buffer_index, &buffer_offset);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    if (!IsPlatformMockICD()) {
        data = (uint32_t *)buffer_data.memory().map();
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
        buffer_data.memory().unmap();
    }
}

TEST_F(PositiveDescriptorBuffer, MultipleSet) {
    TEST_DESCRIPTION("Have a single VkBuffer of data spread across 3 different sets.");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.memory().map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;
    buffer_data.memory().unmap();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    const VkDescriptorSetLayout set_layouts[3] = {ds_layout.handle(), ds_layout.handle(), ds_layout.handle()};
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 3;
    pipe_layout_ci.pSetLayouts = set_layouts;
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = 0;
    vk::GetDescriptorSetLayoutSizeEXT(device(), ds_layout.handle(), &ds_layout_size);

    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size * 3, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    VkDescriptorAddressInfoEXT addr_info = vku::InitStructHelper();
    addr_info.address = buffer_data.address();
    addr_info.range = 4;
    addr_info.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT buffer_descriptor_info = vku::InitStructHelper();
    buffer_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_info.data.pStorageBuffer = &addr_info;

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.memory().map();
    // Sets data_buffer[0] to set 0
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);

    // Sets data_buffer[1] to set 1
    addr_info.address += 4;
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);

    // Sets data_buffer[2] to set 2
    addr_info.address += 4;
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);

    descriptor_buffer.memory().unmap();

    char const *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };
        layout (set = 1, binding = 0) buffer SSBO_1 {
            uint b;
        };
        layout (set = 2, binding = 0) buffer SSBO_2 {
            uint c;
        };

        void main() {
            c = a + b;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                                         &buffer_index, &buffer_offset);

    buffer_offset += ds_layout_size;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 1, 1,
                                         &buffer_index, &buffer_offset);

    buffer_offset += ds_layout_size;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 2, 1,
                                         &buffer_index, &buffer_offset);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    if (!IsPlatformMockICD()) {
        data = (uint32_t *)buffer_data.memory().map();
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
        buffer_data.memory().unmap();
    }
}

TEST_F(PositiveDescriptorBuffer, MultipleBinding) {
    TEST_DESCRIPTION("Have a single VkBuffer of data spread across 3 different bindings in the same set.");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.memory().map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;
    buffer_data.memory().unmap();

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 1;
    pipe_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = 0;
    vk::GetDescriptorSetLayoutSizeEXT(device(), ds_layout.handle(), &ds_layout_size);

    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    VkDeviceSize ds_layout_binding_offsets[3];
    vk::GetDescriptorSetLayoutBindingOffsetEXT(device(), ds_layout.handle(), 0, &ds_layout_binding_offsets[0]);
    vk::GetDescriptorSetLayoutBindingOffsetEXT(device(), ds_layout.handle(), 1, &ds_layout_binding_offsets[1]);
    vk::GetDescriptorSetLayoutBindingOffsetEXT(device(), ds_layout.handle(), 2, &ds_layout_binding_offsets[2]);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    VkDescriptorAddressInfoEXT addr_info = vku::InitStructHelper();
    addr_info.address = buffer_data.address();
    addr_info.range = 4;
    addr_info.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT buffer_descriptor_info = vku::InitStructHelper();
    buffer_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_info.data.pStorageBuffer = &addr_info;

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.memory().map();
    // Sets data_buffer[0] to binding 0
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);

    // Sets data_buffer[1] to binding 1
    addr_info.address += 4;
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout_binding_offsets[1]);

    // Sets data_buffer[2] to binding 2
    addr_info.address += 4;
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout_binding_offsets[2]);

    descriptor_buffer.memory().unmap();

    char const *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };
        layout (set = 0, binding = 1) buffer SSBO_1 {
            uint b;
        };
        layout (set = 0, binding = 2) buffer SSBO_2 {
            uint c;
        };

        void main() {
            c = a + b;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                                         &buffer_index, &buffer_offset);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    if (!IsPlatformMockICD()) {
        data = (uint32_t *)buffer_data.memory().map();
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
        buffer_data.memory().unmap();
    }
}
