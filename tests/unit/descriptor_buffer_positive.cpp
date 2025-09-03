/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

void DescriptorBufferTest::InitBasicDescriptorBuffer(void *instance_pnext) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init(nullptr, nullptr, instance_pnext));

    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
}

class PositiveDescriptorBuffer : public DescriptorBufferTest {};

TEST_F(PositiveDescriptorBuffer, BasicUsage) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(Init());

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
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, UnusedBoundBuffer) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10290");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    // Pipeline has no descriptor
    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2(*this);
    pipe2.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe2.CreateGraphicsPipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // unused
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, PipelineFlags2) {
    TEST_DESCRIPTION("Use descriptor buffer with pipeline created with VkPipelineCreateFlags2CreateInfo");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    VkPipelineCreateFlags2CreateInfo flags_2_ci = vku::InitStructHelper();
    flags_2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT;

    CreatePipelineHelper pipe(*this, &flags_2_ci);
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, BindingMidBuffer) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    m_command_buffer.Begin();

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.Address() + descriptor_buffer_properties.descriptorBufferOffsetAlignment;
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &dbbi);
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, Basic) {
    TEST_DESCRIPTION("Tries to use a full workflow (For Resource descriptors).");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
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
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[2] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, BasicSampler) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer result_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(
        *m_device, ds_layout_size,
        VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
        vkt::device_address);
    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();

    vkt::DescriptorGetInfo get_info_sampler(&sampler.handle());
    vk::GetDescriptorEXT(device(), get_info_sampler, descriptor_buffer_properties.samplerDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_image(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(device(), get_info_image, descriptor_buffer_properties.sampledImageDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    vkt::DescriptorGetInfo get_info_combined(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler, image_view,
                                             VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(device(), get_info_combined, descriptor_buffer_properties.combinedImageSamplerDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(2));

    vkt::DescriptorGetInfo get_info_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, result_buffer, 32);
    vk::GetDescriptorEXT(device(), get_info_buffer, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(3));

    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler s;
        layout(set = 0, binding = 1) uniform texture2D t; // sampled image
        layout(set = 0, binding = 2) uniform sampler2D c; // combined
        layout(set = 0, binding = 3) buffer SSBO { vec4 result; };

        void main() {
            result = texture(c, vec2(0));
            result *= texture(sampler2D(t, s), vec2(0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage =
        VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, MultipleDescriptors) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer result_buffer(*m_device, 32, 0, vkt::device_address);
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer sampler_descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                                          vkt::device_address);
    vkt::Buffer resource_descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                           vkt::device_address);

    uint8_t *mapped_descriptor_data = (uint8_t *)resource_descriptor_buffer.Memory().Map();
    vkt::DescriptorGetInfo get_info_image(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(device(), get_info_image, descriptor_buffer_properties.sampledImageDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, result_buffer, 32);
    vk::GetDescriptorEXT(device(), get_info_buffer, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    mapped_descriptor_data = (uint8_t *)sampler_descriptor_buffer.Memory().Map();
    vkt::DescriptorGetInfo get_info_sampler(&sampler.handle());
    vk::GetDescriptorEXT(device(), get_info_sampler, descriptor_buffer_properties.samplerDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 2) uniform sampler s;
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) buffer SSBO { vec4 result; };

        void main() {
            result = texture(sampler2D(t, s), vec2(0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info[2];
    descriptor_buffer_binding_info[0] = vku::InitStructHelper();
    descriptor_buffer_binding_info[0].address = sampler_descriptor_buffer.Address();
    descriptor_buffer_binding_info[0].usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptor_buffer_binding_info[1] = vku::InitStructHelper();
    descriptor_buffer_binding_info[1].address = resource_descriptor_buffer.Address();
    descriptor_buffer_binding_info[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 2, descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);

    buffer_index = 1;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);

    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, MultipleSet) {
    TEST_DESCRIPTION("Have a single VkBuffer of data spread across 3 different sets.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout, &ds_layout, &ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size * 3, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 4);
    // Sets data_buffer[0] to set 0
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    // Sets data_buffer[1] to set 1
    get_info.address_info.address += 4;
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    // Sets data_buffer[2] to set 2
    get_info.address_info.address += 4;
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
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
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index[3] = {0, 0, 0};
    VkDeviceSize buffer_offset[3] = {0, ds_layout_size, ds_layout_size * 2};
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 3, buffer_index,
                                         buffer_offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[2] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, MultipleBinding) {
    TEST_DESCRIPTION("Have a single VkBuffer of data spread across 3 different bindings in the same set.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 4);
    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    // Sets data_buffer[0] to binding 0
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    // Sets data_buffer[1] to binding 1
    get_info.address_info.address += 4;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    // Sets data_buffer[2] to binding 2
    get_info.address_info.address += 4;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(2));

    const char *cs_source = R"glsl(
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
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, DescriptorIndexing) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_0(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer buffer_2(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_0.Memory().Map();
    data[0] = 8;
    data = (uint32_t *)buffer_1.Memory().Map();
    data[0] = 12;
    data = (uint32_t *)buffer_2.Memory().Map();
    data[0] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    size_t descriptor_size = descriptor_buffer_properties.storageBufferDescriptorSize;
    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_0, 16);
    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_size, mapped_descriptor_data);

    get_info.address_info.address = buffer_1.Address();
    mapped_descriptor_data += descriptor_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_size, mapped_descriptor_data);

    get_info.address_info.address = buffer_2.Address();
    mapped_descriptor_data += descriptor_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_size, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        } x[3];

        void main() {
            x[2].data = x[0].data + x[1].data;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[0] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, BindingInfoUsage2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9228");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = 4096;

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    vkt::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &allocate_flag_info);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &dbbi);
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, DescriptorBufferBindingInfoUsage2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9228");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper(&buffer_usage_flags);
    dbbi.address = buffer.Address();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &dbbi);
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorBuffer, TexelBuffer) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, vkt::device_address);
    vkt::BufferView storage_buffer_view(*m_device, storage_buffer, VK_FORMAT_R32_UINT);
    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);
    vkt::BufferView uniform_buffer_view(*m_device, uniform_buffer, VK_FORMAT_R32_UINT);

    uint32_t *data = (uint32_t *)uniform_buffer.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;
    data[3] = 4;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info_s(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, storage_buffer, 32, VK_FORMAT_R32_UINT);
    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info_s, descriptor_buffer_properties.storageTexelBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_u(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, uniform_buffer, 32, VK_FORMAT_R32_UINT);
    vk::GetDescriptorEXT(device(), get_info_u, descriptor_buffer_properties.uniformTexelBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32ui) uniform uimageBuffer s_buffer;
        layout(set = 0, binding = 1) uniform usamplerBuffer u_buffer;

        void main() {
            imageStore(s_buffer, 0, texelFetch(u_buffer, 0));
            imageStore(s_buffer, 1, texelFetch(u_buffer, 1));
            imageStore(s_buffer, 2, texelFetch(u_buffer, 2));
            imageStore(s_buffer, 3, texelFetch(u_buffer, 3));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    data = (uint32_t *)storage_buffer.Memory().Map();
    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 1);
        ASSERT_TRUE(data[3] == 4);
    }
}

TEST_F(PositiveDescriptorBuffer, BindingOffsets) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    // Will write "42" to offset[0], [64], and [256]
    vkt::Buffer buffer_data(*m_device, 1024, 0, vkt::device_address);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size * 3, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    get_info.address_info.address += 64;
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    get_info.address_info.address += (256 - 64);
    mapped_descriptor_data += ds_layout_size;
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint data;
        };
        void main() {
            data = 42;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
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

    buffer_offset += ds_layout_size;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    buffer_offset += ds_layout_size;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[0] == 42);   // [0]
        ASSERT_TRUE(data[16] == 42);  // [64]
        ASSERT_TRUE(data[64] == 42);  // [256]
    }
}

TEST_F(PositiveDescriptorBuffer, ShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
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

    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source),
                         &ds_layout.handle());

    m_command_buffer.Begin();
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer, 1, stages, &cs.handle());

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

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, NotInvalidatedLegacy) {
    TEST_DESCRIPTION("Case 3 from https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7504#note_549388");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer legacy_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    OneOffDescriptorSet legacy_ds(m_device, {binding});
    vkt::PipelineLayout pipeline_layout(*m_device, {&legacy_ds.layout_});
    legacy_ds.WriteDescriptorBufferInfo(0, legacy_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    legacy_ds.UpdateDescriptorSets();

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);

    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 { uint x; };
        void main() {
            x = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &legacy_ds.set_, 0, nullptr);

    // Does not invalidate the legacy by itself
    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}
