/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveGpuAVDescriptorClassTexelBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVDescriptorClassTexelBuffer, ImageLoadStoreTexelFetch) {
    TEST_DESCRIPTION("index into an image Load, Store, and texelFetch");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) uniform samplerBuffer u_buffer; // texel_buffer[5]
        layout(set = 0, binding = 1, r32f) uniform imageBuffer s_buffer;  // texel_buffer[6]

        void main() {
            vec4 x = texelFetch(u_buffer, 4);
            x *= imageLoad(s_buffer, 5);
           imageStore(s_buffer, 5, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer uniform_texel_buffer(*m_device, 20, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::BufferView uniform_buffer_view(*m_device, uniform_texel_buffer, VK_FORMAT_R32_SFLOAT);

    vkt::Buffer storage_texel_buffer(*m_device, 24, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::BufferView storage_buffer_view(*m_device, storage_texel_buffer, VK_FORMAT_R32_SFLOAT);

    pipe.descriptor_set_.WriteDescriptorBufferView(0, uniform_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    pipe.descriptor_set_.WriteDescriptorBufferView(1, storage_buffer_view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassTexelBuffer, AtomicImageLoadStore) {
    TEST_DESCRIPTION("index into an atomic image Load and Store");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // Not sure if anyone actually support buffer texel on atomic
    if ((m_device->FormatFeaturesBuffer(VK_FORMAT_R32_SFLOAT) & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) == 0) {
        GTEST_SKIP() << "No atomic texel buffer support";
    }

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_atomic_float : enable

        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            float x = imageAtomicLoad(s_buffer, 3, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsAcquire);
            imageAtomicStore(s_buffer, 3, x, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelease);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::BufferView storage_buffer_view(*m_device, storage_texel_buffer, VK_FORMAT_R32_SFLOAT);

    pipe.descriptor_set_.WriteDescriptorBufferView(0, storage_buffer_view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassTexelBuffer, AtomicImageLoadStoreDescriptorBuffer) {
    TEST_DESCRIPTION("index into an atomic image Load and Store");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // Not sure if anyone actually support buffer texel on atomic
    if ((m_device->FormatFeaturesBuffer(VK_FORMAT_R32_SFLOAT) & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) == 0) {
        GTEST_SKIP() << "No atomic texel buffer support";
    }

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_atomic_float : enable

        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            float x = imageAtomicLoad(s_buffer, 3, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsAcquire);
            imageAtomicStore(s_buffer, 3, x, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelease);
        }
    )glsl";

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

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

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, vkt::device_address);

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());

    VkDeviceSize buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);
    vkt::DescriptorGetInfo buffer_get_info(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, storage_texel_buffer,
                                           storage_texel_buffer.CreateInfo().size, VK_FORMAT_R32_SFLOAT);
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.storageTexelBufferDescriptorSize,
                         descriptor_data + buffer_offset);

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1u, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0u, 1u,
                                         &buffer_index, &offset);

    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassTexelBuffer, TexelFetchArray) {
    TEST_DESCRIPTION("index into texelFetch OOB for an array of TexelBuffers");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
            vec4 b;
        } out_buffer;

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer[2];

        void main() {
            out_buffer.a = texelFetch(u_buffer[0], 4); // valid
            // Will not ever get into this check
            if (gl_WorkGroupID.x == 987) {
                out_buffer.b = texelFetch(u_buffer[1], 4); // invalid
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer out_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer;
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView full_buffer_view(*m_device, bvci);
    bvci.range = 16;  // only fills 4, but are accessing index[4]
    vkt::BufferView partial_buffer_view(*m_device, bvci);

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, out_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.WriteDescriptorBufferView(1, full_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0);
    pipe.descriptor_set_.WriteDescriptorBufferView(1, partial_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassTexelBuffer, Robustness) {
    TEST_DESCRIPTION("Use robustness to prevent crash and have no error");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) uniform samplerBuffer u_buffer; // texel_buffer[5]
        layout(set = 0, binding = 1, r32f) uniform imageBuffer s_buffer;  // texel_buffer[6]

        void main() {
            vec4 x = texelFetch(u_buffer, 64);
            x *= imageLoad(s_buffer, 128);
           imageStore(s_buffer, 512, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::BufferView uniform_buffer_view(*m_device, uniform_texel_buffer, VK_FORMAT_R32_SFLOAT);

    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::BufferView storage_buffer_view(*m_device, storage_texel_buffer, VK_FORMAT_R32_SFLOAT);

    pipe.descriptor_set_.WriteDescriptorBufferView(0, uniform_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    pipe.descriptor_set_.WriteDescriptorBufferView(1, storage_buffer_view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}