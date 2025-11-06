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

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"

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

TEST_F(PositiveDescriptorBuffer, MeshShader) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(set=0, binding=0) buffer Buffer {
            uint vertexCount;
            uint primitiveCount;
            uint padding;
            uint sum;
        } count;
        void main() {
            SetMeshOutputsEXT(count.vertexCount, count.primitiveCount);
            count.sum = count.vertexCount + count.primitiveCount;
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr};

    const VkDescriptorBindingFlags ds_binding_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1u;
    flags_create_info.pBindingFlags = &ds_binding_flags;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    data[0] = 3u;
    data[1] = 1u;
    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &buffer_index,
                                         &offset);
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        ASSERT_EQ(data[0] + data[1], data[3]);
    }
}

TEST_F(PositiveDescriptorBuffer, GraphicsPipelineLibrary) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding1;
    vkt::DescriptorSetLayout ds_layout1(*m_device, ds_layout_ci);
    ds_layout_ci.pBindings = &binding2;
    vkt::DescriptorSetLayout ds_layout2(*m_device, ds_layout_ci);

    vkt::PipelineLayout pipeline_layout_vs(*m_device, {&ds_layout1, &ds_layout1, nullptr}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_fs(*m_device, {&ds_layout1, nullptr, &ds_layout2}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_null(*m_device, {&ds_layout1, nullptr, nullptr}, {},
                                             VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    vkt::Buffer uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = ds_layout1.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform_buffer, uniform_buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const char vs_src[] = R"glsl(
            #version 450
            layout(set=0, binding=0) uniform foo { float x; } bar;
            void main() {
            gl_Position = vec4(bar.x);
            }
        )glsl";
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.gp_ci_.layout = pipeline_layout_vs;
        pre_raster_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    exe_pipe_ci.layout = pipeline_layout_null;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Draw with pipeline created with null set
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_null, 0u, 1u,
                                         &buffer_index, &offset);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, GraphicsPipelineLibraryIndependent) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding1;
    vkt::DescriptorSetLayout ds_layout1(*m_device, ds_layout_ci);
    ds_layout_ci.pBindings = &binding2;
    vkt::DescriptorSetLayout ds_layout2(*m_device, ds_layout_ci);

    vkt::Buffer uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = ds_layout1.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform_buffer, uniform_buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    vkt::PipelineLayout pipeline_layout_vs(*m_device, {&ds_layout1, &ds_layout1, nullptr}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_fs(*m_device, {&ds_layout1, nullptr, &ds_layout2}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vkt::PipelineLayout pipeline_layout_link(*m_device, {&ds_layout1, &ds_layout1, &ds_layout2}, {},
                                             VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    vkt::PipelineLayout pipeline_layout_ds(*m_device, {&ds_layout1, &ds_layout1, &ds_layout2}, {},
                                           VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const char vs_src[] = R"glsl(
            #version 450
            layout(set=0, binding=0) uniform foo { float x; } bar;
            void main() {
            gl_Position = vec4(bar.x);
            }
        )glsl";
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_src);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
        pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
        pre_raster_lib.gp_ci_.layout = pipeline_layout_vs;
        pre_raster_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
        frag_shader_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
        frag_shader_lib.gp_ci_.layout = pipeline_layout_fs;
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib,
        pre_raster_lib,
        frag_shader_lib,
        frag_out_lib,
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    exe_pipe_ci.layout = pipeline_layout_link;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_ds, 0u, 1u,
                                         &buffer_index, &offset);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, EmbeddedSamplers) {
    TEST_DESCRIPTION("Bind descriptor buffer and set descriptor offset then draw.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const char *fsSource = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler samp;
        layout(set = 1, binding = 0) uniform texture2D tex;
        layout(location=0) out vec4 color;
        void main() {
            color = texture(sampler2D(tex, samp), vec2(0.5f));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Buffer result_buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R32G32B32A32_SFLOAT,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorSetLayoutBinding sampler_binding = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                          &sampler.handle()};
    const VkDescriptorSetLayoutBinding image_binding = {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        nullptr};

    VkDescriptorSetLayoutCreateInfo sampler_ds_layout_ci = vku::InitStructHelper();
    sampler_ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                 VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
    sampler_ds_layout_ci.bindingCount = 1u;
    sampler_ds_layout_ci.pBindings = &sampler_binding;
    vkt::DescriptorSetLayout sampler_ds_layout(*m_device, sampler_ds_layout_ci);

    VkDescriptorSetLayoutCreateInfo image_ds_layout_ci = vku::InitStructHelper();
    image_ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    image_ds_layout_ci.bindingCount = 1u;
    image_ds_layout_ci.pBindings = &image_binding;
    vkt::DescriptorSetLayout image_ds_layout(*m_device, image_ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&sampler_ds_layout, &image_ds_layout});

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = image_ds_layout.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo image_get_info(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(*m_device, image_get_info, descriptor_buffer_properties.sampledImageDescriptorSize,
                         descriptor_data + buffer_offset);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    VkClearColorValue clear_color_value;
    clear_color_value.float32[0] = 0.2f;
    clear_color_value.float32[1] = 0.4f;
    clear_color_value.float32[2] = 0.6f;
    clear_color_value.float32[3] = 0.8f;

    m_command_buffer.Begin();
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1u, &range);

    VkMemoryBarrier memory_barrier_1 = vku::InitStructHelper();
    memory_barrier_1.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memory_barrier_1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 1u,
                           &memory_barrier_1, 0u, nullptr, 0u, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = buffer_offset;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1u, 1u, &buffer_index,
                                         &offset);
    vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    VkMemoryBarrier memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 1u,
                           &memory_barrier_2, 0u, nullptr, 0u, nullptr);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageExtent = {1u, 1u, 1u};
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, result_buffer, 1, &region);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float *data = (float *)result_buffer.Memory().Map();
    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_NEAR(data[i], clear_color_value.float32[i], 0.0001f);
        }
    }
}

TEST_F(PositiveDescriptorBuffer, InputAttachment) {
    TEST_DESCRIPTION("Test reading from a descriptor that uses same image view as framebuffer input attachment");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 3, format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
    VkImageView image_view_handle = image_view;
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Framebuffer framebuffer(*m_device, rp, 1, &image_view_handle);

    const char *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0, rgba8) readonly uniform image2D image1;
            layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput inputColor;
            void main(){
                color = subpassLoad(inputColor) + imageLoad(image1, ivec2(0));
            }
        )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout descriptor_set_layout(*m_device, ds_layout_ci);
    binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    vkt::DescriptorSetLayout descriptor_set_layout2(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout, &descriptor_set_layout2});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.gp_ci_.renderPass = rp;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize image_offset = descriptor_set_layout.GetDescriptorBufferBindingOffset(0);
    VkDeviceSize input_attachment_offset =
        descriptor_set_layout2.GetDescriptorBufferBindingOffset(0) + descriptor_buffer_properties.storageImageDescriptorSize;

    vkt::DescriptorGetInfo image_get_info(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, sampler, image_view, VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(*m_device, image_get_info, descriptor_buffer_properties.storageImageDescriptorSize,
                         descriptor_data + image_offset);
    vkt::DescriptorGetInfo input_attachment_get_info(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, sampler, image_view,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(*m_device, input_attachment_get_info, descriptor_buffer_properties.inputAttachmentDescriptorSize,
                         descriptor_data + input_attachment_offset);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, framebuffer, 32, 32, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &buffer_index,
                                         &offset);

    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, ImageLayoutIgnored) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    AddRequiredFeature(vkt::Feature::descriptorBufferImageLayoutIgnored);
    InitRenderTarget();

    const VkFormat format = FindSupportedDepthStencilFormat(gpu_);

    auto image_create_info = vkt::Image::ImageCreateInfo2D(32, 32, 1, 3, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view =
        image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const char *fs_source = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0) uniform sampler s;
            layout(set = 0, binding = 1) uniform texture2D t;
            void main(){
                color = texture(sampler2D(t, s), vec2(0));
            }
        )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = bindings.size();
    ds_layout_ci.pBindings = bindings.data();
    vkt::DescriptorSetLayout descriptor_set_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer descriptor_buffer(
        *m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
        vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());

    vkt::DescriptorGetInfo get_info_sampler(&sampler.handle());
    vk::GetDescriptorEXT(device(), get_info_sampler, descriptor_buffer_properties.samplerDescriptorSize,
                         descriptor_data + descriptor_set_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_image(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, image_view,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vk::GetDescriptorEXT(device(), get_info_image, descriptor_buffer_properties.sampledImageDescriptorSize,
                         descriptor_data + descriptor_set_layout.GetDescriptorBufferBindingOffset(1));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage =
        VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &buffer_index,
                                         &offset);

    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorBuffer, ComputeAndGraphics) {
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer1(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer2(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                                  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    vkt::DescriptorGetInfo get_info1(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer1, 16u);
    vkt::DescriptorGetInfo get_info2(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer2, 16u);

    const VkDeviceSize offset1 = 0;
    VkDeviceSize offset2 = descriptor_buffer_properties.storageBufferDescriptorSize;
    if (offset2 % descriptor_buffer_properties.descriptorBufferOffsetAlignment != 0) {
        offset2 += descriptor_buffer_properties.descriptorBufferOffsetAlignment -
                   (offset2 % descriptor_buffer_properties.descriptorBufferOffsetAlignment);
    }

    uint8_t *mapped_descriptor_data = (uint8_t*)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info1, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data + offset1);
    vk::GetDescriptorEXT(device(), get_info2, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data + offset2);

    const char *vsSource = R"glsl(
            #version 450
            layout (set = 0, binding = 0) buffer Buf {
                vec4 data;
            } buf;
            void main() {
                vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
                gl_Position = vec4(pos, 0.0f, 1.0f);
                buf.data[gl_VertexIndex] = 1.0f;
            }
        )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer Buf {
            vec4 data;
        } buf;

        void main() {
            buf.data.x = gl_LocalInvocationIndex + 1.0f;
            buf.data.y = gl_LocalInvocationIndex + 2.0f;
            buf.data.z = gl_LocalInvocationIndex + 3.0f;
            buf.data.w = gl_LocalInvocationIndex + 4.0f;
        }
    )glsl";

    CreateComputePipelineHelper pipe2(*this);
    pipe2.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe2.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe2.cp_ci_.layout = pipeline_layout;
    pipe2.CreateComputePipeline();

    const uint32_t index = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset1);
    vk::CmdDraw(m_command_buffer, 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &index, &offset2);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float *data1 = (float *)buffer1.Memory().Map();
    float *data2 = (float *)buffer2.Memory().Map();
    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_EQ(data1[i], 1.0f);
            ASSERT_EQ(data2[i], float(i + 1));
        }
    }
}

TEST_F(PositiveDescriptorBuffer, ComputeAndGraphics2) {
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(float) * 8u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                                  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout, &set_layout});

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, 32u);

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);

    const char *vsSource = R"glsl(
            #version 450
            layout (set = 0, binding = 0) buffer Buf {
                vec4 data[2];
            } buf;
            void main() {
                vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
                gl_Position = vec4(pos, 0.0f, 1.0f);
                buf.data[0][gl_VertexIndex] = 1.0f;
            }
        )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 1, binding = 0) buffer Buf {
            vec4 data[2];
        } buf;

        void main() {
            buf.data[1].x = gl_LocalInvocationIndex + 1.0f;
            buf.data[1].y = gl_LocalInvocationIndex + 2.0f;
            buf.data[1].z = gl_LocalInvocationIndex + 3.0f;
            buf.data[1].w = gl_LocalInvocationIndex + 4.0f;
        }
    )glsl";

    CreateComputePipelineHelper pipe2(*this);
    pipe2.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe2.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe2.cp_ci_.layout = pipeline_layout;
    pipe2.CreateComputePipeline();

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index,
                                         &offset);
    vk::CmdDraw(m_command_buffer, 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 1, 1, &index, &offset);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float *data = (float *)buffer.Memory().Map();
    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_EQ(data[i], 1.0f);
            ASSERT_EQ(data[i + 4], float(i + 1));
        }
    }
}

TEST_F(PositiveDescriptorBuffer, PushDescriptor) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBufferPushDescriptors);
    AddRequiredFeature(vkt::Feature::pushDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data0(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer_data1(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data0.Memory().Map();
    data[0] = 8;
    data[1] = 12;

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout descriptor_set(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::DescriptorSetLayout descriptor_set_push(
        *m_device, {binding},
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set, &descriptor_set_push});

    VkDeviceSize ds_layout_size = descriptor_set.GetDescriptorBufferSize();
    ds_layout_size = std::max(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);
    VkBufferUsageFlags descriptor_buffer_usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    if (!descriptor_buffer_properties.bufferlessPushDescriptors) {
        descriptor_buffer_usage |= VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
    }
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, descriptor_buffer_usage, vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data0, 16);
    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
            uint b;
        };
        layout (set = 1, binding = 0) buffer SSBO_1 {
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

    VkDescriptorBufferBindingPushDescriptorBufferHandleEXT descriptor_buffer_push_descriptor_buffer_handle =
        vku::InitStructHelper();
    descriptor_buffer_push_descriptor_buffer_handle.buffer = descriptor_buffer;

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    if (!descriptor_buffer_properties.bufferlessPushDescriptors) {
        descriptor_buffer_binding_info.pNext = &descriptor_buffer_push_descriptor_buffer_handle;
    }
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &descriptor_buffer_binding_info);

    const uint32_t index = 0;
    const VkDeviceSize offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &index, &offset);

    VkDescriptorBufferInfo buffer_info = {buffer_data1, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = VK_NULL_HANDLE;
    descriptor_write.dstBinding = 0u;
    descriptor_write.dstArrayElement = 0u;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;
    vk::CmdPushDescriptorSetKHR(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 1u, 1u, &descriptor_write);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        data = (uint32_t *)buffer_data1.Memory().Map();
        ASSERT_TRUE(data[0] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, PushDescriptorOnly) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBufferPushDescriptors);
    AddRequiredFeature(vkt::Feature::pushDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Buffer buffer_data(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    OneOffDescriptorSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        },
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

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

    VkDescriptorBufferInfo buffer_info = {buffer_data, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = VK_NULL_HANDLE;
    descriptor_write.dstBinding = 0u;
    descriptor_write.dstArrayElement = 0u;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;
    vk::CmdPushDescriptorSetKHR(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0u, 1u, &descriptor_write);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        ASSERT_TRUE(data[2] == 20);
    }
}

TEST_F(PositiveDescriptorBuffer, DeviceLocal) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    const uint32_t descriptor_buffer_size = 4096u;

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer src_descriptor_buffer(*m_device, descriptor_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer device_local_descriptor_buffer(*m_device, 4096,
                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                   VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, 16u);

    const VkDeviceSize offset = 0;

    uint8_t *mapped_descriptor_data = (uint8_t *)src_descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char *vsSource = R"glsl(
            #version 450
            layout (set = 0, binding = 0) buffer Buf {
                vec4 data;
            } buf;
            void main() {
                vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
                gl_Position = vec4(pos, 0.0f, 1.0f);
                buf.data[gl_VertexIndex] = 1.0f;
            }
        )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;

    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = 0u;
    buffer_copy.dstOffset = 0u;
    buffer_copy.size = descriptor_buffer_size;

    VkBufferMemoryBarrier2 buffer_memory_barrier = vku::InitStructHelper();
    buffer_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_memory_barrier.dstAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
    buffer_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    buffer_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.buffer = device_local_descriptor_buffer;
    buffer_memory_barrier.offset = 0u;
    buffer_memory_barrier.size = descriptor_buffer_size;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.bufferMemoryBarrierCount = 1u;
    dependency_info.pBufferMemoryBarriers = &buffer_memory_barrier;

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = device_local_descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, src_descriptor_buffer, device_local_descriptor_buffer, 1u, &buffer_copy);
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer, 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float *data = (float *)buffer.Memory().Map();
    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_EQ(data[i], 1.0f);
        }
    }
}

TEST_F(PositiveDescriptorBuffer, DestroyDescriptor) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer descriptor_buffer(*m_device, 4096u, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout});

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, 16u);

    const VkDeviceSize offset = 0;

    uint8_t *mapped_descriptor_data = (uint8_t *)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    buffer.Destroy();
    set_layout.Destroy();

    const char *vsSource = R"glsl(
            #version 450
            layout (set = 0, binding = 0) buffer Buf {
                vec4 data;
            } buf;
            void main() {
                vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
                gl_Position = vec4(pos, 0.0f, 1.0f);
                buf.data[gl_VertexIndex] = 1.0f;
            }
        )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const uint32_t index = 0;

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &index, &offset);
    vk::CmdDraw(m_command_buffer, 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    float *data = (float *)buffer.Memory().Map();
    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_EQ(data[i], 1.0f);
        }
    }
}

TEST_F(PositiveDescriptorBuffer, SharedSet) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 2u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer copy_buffer(*m_device, 4096u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout, &set_layout});

    const VkDeviceSize ds_layout_size = set_layout.GetDescriptorBufferSize();
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size * 2u,
                                  VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, buffer.CreateInfo().size);

    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    data[0] = 5u;
    data[1] = 7u;

    void *mapped_copy_data = copy_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_copy_data);

    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    memcpy(mapped_descriptor_data, mapped_copy_data, descriptor_buffer_properties.storageBufferDescriptorSize);

    const char *vsSource = R"glsl(
            #version 450
            layout(set = 0, binding = 0) buffer SSBO_0 {
                uint a_0;
                uint b_0;
            };
            layout(set = 1, binding = 0) buffer SSBO_1 {
                uint a_1;
                uint b_1;
            };
            void main() {
                vec2 pos = vec2(float(gl_VertexIndex & 1), float((gl_VertexIndex >> 1) & 1));
                gl_Position = vec4(pos, 0.0f, 1.0f);
                b_1 = a_0;
            }
        )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    const uint32_t buffer_indices[] = {0u, 0u};
    const VkDeviceSize buffer_offsets[] = {0u, ds_layout_size};

    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = 0u;
    buffer_copy.dstOffset = ds_layout_size;
    buffer_copy.size = ds_layout_size;

    VkBufferMemoryBarrier2 buffer_memory_barrier = vku::InitStructHelper();
    buffer_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_memory_barrier.dstAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
    buffer_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    buffer_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.buffer = descriptor_buffer;
    buffer_memory_barrier.offset = ds_layout_size;
    buffer_memory_barrier.size = ds_layout_size;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.bufferMemoryBarrierCount = 1u;
    dependency_info.pBufferMemoryBarriers = &buffer_memory_barrier;

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, copy_buffer, descriptor_buffer, 1u, &buffer_copy);
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 2u, buffer_indices,
                                         buffer_offsets);
    vk::CmdDraw(m_command_buffer, 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        for (uint32_t i = 0; i < 2; i++) {
            ASSERT_EQ(data[i], 5.0f);
        }
    }
}
