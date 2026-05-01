/*
 * Copyright (c) 2024-2026 Valve Corporation
 * Copyright (c) 2024-2026 LunarG, Inc.
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
#include "../framework/buffer_helper.h"
#include "utils/math_utils.h"

void GpuAVDescriptorBuffer::InitBasicDescriptorBuffer(std::vector<VkLayerSettingEXT> layer_settings, bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings, safe_mode));
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
}

class PositiveGpuAVDescriptorBuffer : public GpuAVDescriptorBuffer {};

TEST_F(PositiveGpuAVDescriptorBuffer, BasicCompute) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer({}, false));

    vkt::Buffer buffer_data(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    uint32_t* data = (uint32_t*)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_data, 16);
    void* mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info, descriptor_buffer_properties.storageBufferDescriptorSize, mapped_descriptor_data);

    const char* cs_source = R"glsl(
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
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer({}, false));
    InitRenderTarget();

    vkt::Buffer bda(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo_0(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ubo_1(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    auto ssbo_ptr = (VkDeviceAddress*)ssbo_0.Memory().Map();
    ssbo_ptr[0] = bda.Address();  // ptr
    uint32_t* data = (uint32_t*)ubo_1.Memory().Map();
    data[0] = 0;  // index

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDeviceSize ds_layout_size = ds_layout.GetDescriptorBufferSize();
    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);
    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::DescriptorGetInfo get_info_0(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ssbo_0, 16);
    uint8_t* mapped_descriptor_data = (uint8_t*)descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), get_info_0, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(0));

    vkt::DescriptorGetInfo get_info_1(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo_1, 16);
    vk::GetDescriptorEXT(device(), get_info_1, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         mapped_descriptor_data + ds_layout.GetDescriptorBufferBindingOffset(1));

    const char* fs_source = R"glsl(
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

    data = (uint32_t*)bda.Memory().Map();
    ASSERT_TRUE(data[0] == 11);
}

TEST_F(PositiveGpuAVDescriptorBuffer, IndexBuffer) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    if (descriptor_buffer_properties.maxResourceDescriptorBufferBindings < 2) {
        GTEST_SKIP() << "maxResourceDescriptorBufferBindings is not 2";
    }

    const char* vsSource = R"glsl(
        #version 450

        layout(set=0, binding=0) buffer InData {
            vec4 pos;
        } in_data;
        layout(set=1, binding=0) buffer OutData {
            vec4 pos;
        } out_data;

        void main() {
            gl_Position = vec4(in_data.pos);
            out_data.pos = in_data.pos;
        }
    )glsl";

    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

    const VkDescriptorBindingFlags ds_binding_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1u;
    flags_create_info.pBindingFlags = &ds_binding_flags;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout, &ds_layout});

    vkt::Buffer in_buffer(*m_device, sizeof(float) * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    float* in_buffer_ptr = (float*)in_buffer.Memory().Map();
    in_buffer_ptr[0] = 1.0f;
    in_buffer_ptr[1] = 2.0f;
    in_buffer_ptr[2] = 3.0f;
    in_buffer_ptr[3] = 4.0f;
    vkt::Buffer out_buffer(*m_device, sizeof(float) * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    vkt::Buffer out_descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t* in_descriptor_data = reinterpret_cast<uint8_t*>(in_descriptor_buffer.Memory().Map());
    uint8_t* out_descriptor_data = reinterpret_cast<uint8_t*>(out_descriptor_buffer.Memory().Map());

    VkDeviceSize in_buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);
    VkDeviceSize out_buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo in_buffer_get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, in_buffer, in_buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, in_buffer_get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         in_descriptor_data + in_buffer_offset);
    vkt::DescriptorGetInfo out_buffer_get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, out_buffer, out_buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, out_buffer_get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         out_descriptor_data + out_buffer_offset);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkDescriptorBufferBindingInfoEXT buffer_binding_infos[2];
    buffer_binding_infos[0] = vku::InitStructHelper();
    buffer_binding_infos[0].address = in_descriptor_buffer.Address();
    buffer_binding_infos[0].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    buffer_binding_infos[1] = vku::InitStructHelper();
    buffer_binding_infos[1].address = out_descriptor_buffer.Address();
    buffer_binding_infos[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 2u, buffer_binding_infos);
    uint32_t buffer_indices[2] = {0u, 1u};
    VkDeviceSize offsets[2] = {0u, 0u};
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 2u, buffer_indices,
                                         offsets);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, vvl::kU32Max, 42});

    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    float* out_buffer_ptr = (float*)out_buffer.Memory().Map();
    for (uint32_t i = 0; i < 4; ++i) {
        ASSERT_EQ(in_buffer_ptr[i], out_buffer_ptr[i]);
    }
}

TEST_F(PositiveGpuAVDescriptorBuffer, PostProcessAliasImageBindingPartiallyBound) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer({}, false));

    const char* csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D float_textures[2];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[2];
        layout(set = 0, binding = 1) buffer output_buffer {
            uint index;
            vec4 data;
        };

        void main() {
            const vec4 value = texelFetch(float_textures[index], ivec2(0), 0);
            const uint mask = texelFetch(uint_textures[index + 1], ivec2(0), 0).x;
            data = mask > 0 ? value : vec4(0.0);
        }
    )glsl";

    const VkDescriptorSetLayoutBinding bindings[2] = {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const VkDescriptorBindingFlags ds_binding_flags[2] = {VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, 0u};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2u;
    flags_create_info.pBindingFlags = ds_binding_flags;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 2u;
    ds_layout_ci.pBindings = bindings;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = VkShaderObj(*m_device, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image float_image(*m_device, image_ci);
    float_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci);
    uint_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    struct Payload {
        uint32_t index;
        uint32_t padding[3];
        float data[4];
    };
    Payload* payload = (Payload*)buffer.Memory().Map();
    payload->index = 0;

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    uint8_t* descriptor_data = reinterpret_cast<uint8_t*>(descriptor_buffer.Memory().Map());

    VkDeviceSize image_offset = ds_layout.GetDescriptorBufferBindingOffset(0);
    VkDeviceSize buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(1);

    vkt::DescriptorGetInfo float_image_get_info(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, float_image_view,
                                                VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(*m_device, float_image_get_info, descriptor_buffer_properties.sampledImageDescriptorSize,
                         descriptor_data + image_offset);

    vkt::DescriptorGetInfo uint_image_get_info(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE, uint_image_view,
                                               VK_IMAGE_LAYOUT_GENERAL);
    vk::GetDescriptorEXT(*m_device, uint_image_get_info, descriptor_buffer_properties.sampledImageDescriptorSize,
                         descriptor_data + image_offset + descriptor_buffer_properties.sampledImageDescriptorSize);

    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer, sizeof(Payload));
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    VkClearColorValue float_clear;
    float_clear.float32[0] = 0.2f;
    float_clear.float32[1] = 0.4f;
    float_clear.float32[2] = 0.6f;
    float_clear.float32[3] = 0.8f;
    VkClearColorValue uint_clear;
    uint_clear.uint32[0] = 1u;
    uint_clear.uint32[1] = 0u;
    uint_clear.uint32[2] = 1u;
    uint_clear.uint32[3] = 0u;
    VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

    m_command_buffer.Begin();
    vk::CmdClearColorImage(m_command_buffer, float_image, VK_IMAGE_LAYOUT_GENERAL, &float_clear, 1u, &subresource_range);
    vk::CmdClearColorImage(m_command_buffer, uint_image, VK_IMAGE_LAYOUT_GENERAL, &uint_clear, 1u, &subresource_range);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0u, 1u, &buffer_index,
                                         &offset);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    for (uint32_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(float_clear.float32[i], payload->data[i]);
    }
}

TEST_F(PositiveGpuAVDescriptorBuffer, MultipleAccessChainsBDA) {
    TEST_DESCRIPTION("Slang will produce a chain of OpAccessChains");
    RETURN_IF_SKIP(CheckSlangSupport());
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer({}, false));

    const char* slang_shader = R"slang(
        struct Data {
            uint x;
            uint payload[16]; // last item is OOB
        }
        uniform Data* bda;
        [numthreads(1,1,1)]
        void main() {
           uint a = bda->payload[0] * bda->payload[14];
           bda->x = a;
        }
    )slang";
    vkt::Buffer bda_buffer(*m_device, 64, 0, vkt::device_address);
    vkt::Buffer ubo_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);

    auto ubo_buffer_ptr = static_cast<VkDeviceAddress*>(ubo_buffer.Memory().Map());
    ubo_buffer_ptr[0] = bda_buffer.Address();

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

    const VkDescriptorBindingFlags ds_binding_flags = 0;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1u;
    flags_create_info.pBindingFlags = &ds_binding_flags;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cs_ = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_SLANG);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t* descriptor_data = reinterpret_cast<uint8_t*>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo_buffer, ubo_buffer.CreateInfo().size);
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0u, 1u, &buffer_index,
                                         &offset);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorBuffer, ForceNullDescriptor) {
    TEST_DESCRIPTION(
        "Never set anything in the descriptor buffer, turn on gpuav_force_on_robustness, then pray hard that GPU-AV will prevent "
        "crashing");
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_force_on_robustness", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    RETURN_IF_SKIP(InitBasicDescriptorBuffer(layer_settings, false));

    std::vector<VkDescriptorSetLayoutBinding> bindings = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                          {3, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    vkt::DescriptorSetLayout ds_layout(*m_device, bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer descriptor_buffer_host(
        *m_device, 65536, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
        vkt::device_address);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 65536;
    buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer descriptor_buffer_device(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    // Few extra allocation to stress our internal way to call vkCmdFillBuffer
    {
        buffer_ci.size = 1024;
        vkt::Buffer dummy_buffer_1(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);
        vkt::Buffer dummy_buffer_2(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);
    }

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform A { vec4 a; };
        layout(set = 0, binding = 1) buffer B { vec4 b; };
        layout(set = 0, binding = 2) uniform texture2D t;
        layout(set = 0, binding = 3) uniform sampler s;
        void main() {
            b = texture(sampler2D(t, s), vec2(0.5f)) * a;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info[2];
    descriptor_buffer_binding_info[0] = vku::InitStructHelper();
    descriptor_buffer_binding_info[0].address = descriptor_buffer_host.Address();
    descriptor_buffer_binding_info[0].usage =
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptor_buffer_binding_info[1] = vku::InitStructHelper();
    descriptor_buffer_binding_info[1].address = descriptor_buffer_device.Address();
    descriptor_buffer_binding_info[1].usage =
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 2, descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 4, 1, 4);

    buffer_index = 1;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &buffer_index,
                                         &buffer_offset);
    vk::CmdDispatch(m_command_buffer, 2, 8, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}