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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../../layers/gpu_shaders/gpu_shaders_constants.h"

void GpuAVDescriptorIndexingTest::InitGpuVUDescriptorIndexing() {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    AddRequiredFeature(vkt::Feature::shaderStorageBufferArrayNonUniformIndexing);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitState());
}

TEST_F(PositiveGpuAVDescriptorIndexing, Basic) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    // send index to select in image array
    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 1;
    buffer.memory().unmap();

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

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

        layout(set = 0, binding = 1) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BasicHLSL) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);
    // send index to select in image array
    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 1;
    buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    // only indexing into textures[1]
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[4] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
     %uint_4 = OpConstant %uint 4
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_4 = OpTypeArray %type_2d_image %uint_4
%_ptr_UniformConstant__arr_type_2d_image_uint_4 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_4
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_4 UniformConstant
       %main = OpFunction %void None %27
         %31 = OpLabel
         %32 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %33 = OpLoad %uint %32
         %34 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %33
         %35 = OpLoad %type_2d_image %34
         %36 = OpLoad %type_sampler %ss
         %37 = OpSampledImage %type_sampled_image %35 %36
         %38 = OpImageSampleExplicitLod %v4float %37 %18 Lod %float_0
         %39 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %39 %38
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BasicHLSLRuntimeArray) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL via runtime array");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);
    // send index to select in image array
    uint32_t *data = (uint32_t *)buffer.memory().map();
    data[0] = 7;
    buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    // only indexing into textures[7]
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 7);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_EXT_descriptor_indexing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_type_2d_image = OpTypeRuntimeArray %type_2d_image
%_ptr_UniformConstant__runtimearr_type_2d_image = OpTypePointer UniformConstant %_runtimearr_type_2d_image
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__runtimearr_type_2d_image UniformConstant
       %main = OpFunction %void None %26
         %30 = OpLabel
         %31 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %32 = OpLoad %uint %31
         %33 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %32
         %34 = OpLoad %type_2d_image %33
         %35 = OpLoad %type_sampler %ss
         %36 = OpSampledImage %type_sampled_image %34 %35
         %37 = OpImageSampleExplicitLod %v4float %36 %18 Lod %float_0
         %38 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, UnInitImage) {
    TEST_DESCRIPTION(
        "Make sure there's not a crash if the sampler of a combined image sampler is initialized but the image isn't.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[3] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 3;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &layout_createinfo_binding_flags,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer0.handle(), 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 2) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 1;
    buffer0.memory().unmap();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, ImageMultiBinding) {
    TEST_DESCRIPTION("Make sure multiple variables using a single binding does not produce false errors.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[3] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 3;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &layout_createinfo_binding_flags,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer0.handle(), 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
	#version 450
	#extension GL_EXT_nonuniform_qualifier : enable

	layout(set = 0, binding = 2) uniform sampler3D tex3d[];
	layout(set = 0, binding = 2) uniform sampler2D tex[];
	layout(location = 0) out vec4 uFragColor;
	layout(location = 0) in flat uint index;
	void main() {
	    if ((index & 1) != 0) {
                uFragColor = texture(tex[index], vec2(0, 0));
            } else {
                uFragColor = texture(tex3d[index], vec3(0, 0, 0));
            }
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 1;
    buffer0.memory().unmap();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}
