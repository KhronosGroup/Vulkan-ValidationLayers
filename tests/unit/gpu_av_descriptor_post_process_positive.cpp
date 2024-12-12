/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveGpuAVDescriptorPostProcess : public GpuAVDescriptorIndexingTest {};

TEST_F(PositiveGpuAVDescriptorPostProcess, MixingProtectedResources) {
    TEST_DESCRIPTION("Have protected resources, but don't actually access it so no VU is triggered");
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);
    if (protected_memory_properties.protectedNoFault) {
        GTEST_SKIP() << "protectedNoFault is supported";
    }

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vkt::Image image_protected(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs_image_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_image_protected);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs_image_protected.size;
    if (!m_device->Physical().SetMemoryType(mem_reqs_image_protected.memoryTypeBits, &alloc_info,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_image_protected(*m_device, alloc_info);
    vk::BindImageMemory(device(), image_protected.handle(), memory_image_protected.handle(), 0);
    image_protected.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_protected = image_protected.CreateView();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view_protected, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

        // [0] non-protected
        // [1] protected
        layout(set = 0, binding = 1) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageBindingPartiallyBound) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
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

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
         {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    *data = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, uint_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageBindingRuntimeArray) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D float_textures[];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[];
        layout(set = 0, binding = 1) buffer output_buffer {
            uint index;
            vec4 data;
        };

        void main() {
            const vec4 value = texelFetch(float_textures[nonuniformEXT(index)], ivec2(0), 0);
            const uint mask = texelFetch(uint_textures[nonuniformEXT(index + 1)], ivec2(0), 0).x;
            data = mask > 0 ? value : vec4(0.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci);
    float_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci);
    uint_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    *data = 0;
    buffer.Memory().Unmap();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, float_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 0);
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, uint_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 1);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisample) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        layout (constant_id = 0) const int path = 0; // always zero, but prevents dead code elimination

        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
            if (path > 10) {
                dummy += texelFetch(BaseTextureMS, ivec2(0), 0);
            }
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDescriptorSets) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across two descriptor sets");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";

    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorSet descriptor_set0(m_device, {
                                                      {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  });
    const vkt::PipelineLayout pipeline_layout0(*m_device, {&descriptor_set0.layout_});
    descriptor_set0.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set0.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    OneOffDescriptorSet descriptor_set1(m_device, {
                                                      {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  });
    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set1.layout_});
    descriptor_set1.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set1.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout0.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout1.handle();
    pipe_ms.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout1.handle(), 0, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDescriptorSetsPartiallyBound) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across two descriptor sets");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";
    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";
    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorIndexingSet descriptor_set0(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout0(*m_device, {&descriptor_set0.layout_});
    descriptor_set0.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set0.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    OneOffDescriptorIndexingSet descriptor_set1(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set1.layout_});
    descriptor_set1.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set1.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout0.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout1.handle();
    pipe_ms.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout1.handle(), 0, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, AliasImageMultisampleDispatches) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across dispatches");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };

        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";

    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };

        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout.handle();
    pipe_ms.CreateComputePipeline();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    descriptor_set.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorPostProcess, NonMultisampleMismatchWithPipeline) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image bad_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView bad_image_view = bad_image.CreateView();

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image good_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView good_image_view = good_image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        // mySampler[0] is bad
        // mySampler[1] is good
        layout(set=0, binding=0) uniform sampler2D mySampler[2];
        layout(set=0, binding=1) buffer SSBO {
            int index;
            vec4 out_value;
        };
        void main() {
           out_value = texelFetch(mySampler[index], ivec2(0), 0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
