/*
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveShaderImageAccess, FunctionParameterToVariable) {
    TEST_DESCRIPTION("Test getting a ImageAccess from a OpFunctionParameter to a OpVariable");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : enable
        layout(set = 0, binding = 0) uniform texture2D texture_image;

        int foo(texture2D func_texture) {
            return textureSize(func_texture, 0).x;
        }

        void main() {
            int x = textureSize(texture_image, 0).x;
            x += foo(texture_image);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, MultipleFunctionParameterToVariable) {
    TEST_DESCRIPTION("Test getting a ImageAccess from a chain of OpFunctionParameter to a OpVariable");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : enable
        layout(set = 0, binding = 0) uniform texture2D texture_image;

        int bar(int detail, texture2D func_texture) {
            return textureSize(func_texture, detail).x;
        }

        int foo(texture2D func_texture, int detail) {
            int y = textureSize(func_texture, detail).x;
            return y + bar(detail, func_texture);
        }

        void main() {
            int x = textureSize(texture_image, 0).x;
            x += foo(texture_image, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, DifferentFunctionParameterToVariable) {
    TEST_DESCRIPTION("Test getting a different ImageAccess from the same OpFunctionParameter to a OpVariable");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : enable

        layout(set = 0, binding = 0) uniform texture2D texture_image_a;
        layout(set = 0, binding = 1) uniform texture2D texture_image_b;

        int foo(texture2D func_texture, int detail) {
            return textureSize(func_texture, detail).x;
        }

        void main() {
            int x = foo(texture_image_a, 0) + foo(texture_image_b, 1);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, FunctionParameterToLoad) {
    TEST_DESCRIPTION("Test getting a ImageAccess from a OpFunctionParameter to a OpLoad");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    // This is
    //    int foo(texture2D func_texture) { return textureSize(func_texture, 0).x; }
    //    void main() {  int x = foo(texture_image); }
    // But replaced so the OpFunctionCall takes a OpLoad instead of OpVariable
    char const *csSource = R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %texture_image DescriptorSet 0
               OpDecorate %texture_image Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
 %type_image = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_texture = OpTypePointer UniformConstant %type_image
        %int = OpTypeInt 32 1
         %10 = OpTypeFunction %int %type_image
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%ptr_func_int = OpTypePointer Function %int
%texture_image = OpVariable %ptr_texture UniformConstant

       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %ptr_func_int Function
       %load = OpLoad %type_image %texture_image
         %26 = OpFunctionCall %int %foo_t21_ %load
               OpStore %x %26
               OpReturn
               OpFunctionEnd

   %foo_t21_ = OpFunction %int None %10
%func_texture = OpFunctionParameter %type_image
         %13 = OpLabel
         %17 = OpImageQuerySizeLod %v2int %func_texture %int_0
         %20 = OpCompositeExtract %int %17 0
               OpReturnValue %20
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, FunctionParameterToVariableSampledImage) {
    TEST_DESCRIPTION("Test getting a OpSampledImage ImageAccess from a OpFunctionParameter to a OpVariable");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : enable

        layout(set = 0, binding = 0) uniform sampler sampler_descriptor;
        layout(set = 0, binding = 1) uniform texture2D texture_image;

        vec4 foo(texture2D func_texture, sampler func_sampler) {
            return texture(sampler2D(func_texture,  func_sampler), vec2(0.0));
        }

        void main() {
            vec4 x = foo(texture_image, sampler_descriptor);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, FunctionParameterToLoadSampledImage) {
    TEST_DESCRIPTION("Test getting a OpSampledImage ImageAccess from a OpFunctionParameter to a OpLoad");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    // This is
    //    int foo(texture2D func_texture) { return texture(sampler2D(func_texture,  func_sampler), vec2(0.0)); }
    //    void main() {  vec4 x = foo(texture_image, sampler_descriptor); }
    // But replaced so the OpFunctionCall takes a OpLoad instead of OpVariable
    char const *csSource = R"(
                OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %texture_image DescriptorSet 0
               OpDecorate %texture_image Binding 1
               OpDecorate %sampler_descriptor DescriptorSet 0
               OpDecorate %sampler_descriptor Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
 %type_image = OpTypeImage %float 2D 0 0 0 1 Unknown
  %ptr_image = OpTypePointer UniformConstant %type_image
%type_sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %type_sampler
    %v4float = OpTypeVector %float 4
         %12 = OpTypeFunction %v4float %type_image %type_sampler
         %19 = OpTypeSampledImage %type_image
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%texture_image = OpVariable %ptr_image UniformConstant
%sampler_descriptor = OpVariable %ptr_sampler UniformConstant

       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %_ptr_Function_v4float Function
         %17 = OpLoad %type_image %texture_image
         %18 = OpLoad %type_sampler %sampler_descriptor
         %31 = OpFunctionCall %v4float %foo %17 %18
               OpStore %x %31
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %v4float None %12
%func_texture = OpFunctionParameter %type_image
%func_sampler = OpFunctionParameter %type_sampler
         %16 = OpLabel
         %20 = OpSampledImage %19 %func_texture %func_sampler
         %24 = OpImageSampleExplicitLod %v4float %20 %23 Lod %float_0
               OpReturnValue %24
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, CopyObjectFromLoad) {
    TEST_DESCRIPTION("Use a OpCopyObject from a OpLoad");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    // This is simple
    //    int x = textureSize(texture_image, 0).x;
    // but with inserted OpCopyObject calls
    char const *csSource = R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %texture_image DescriptorSet 0
               OpDecorate %texture_image Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%ptr_func_int = OpTypePointer Function %int
      %float = OpTypeFloat 32
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_texture = OpTypePointer UniformConstant %10
%texture_image = OpVariable %ptr_texture UniformConstant
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %ptr_func_int Function
         %13 = OpLoad %10 %texture_image
      %copy1 = OpCopyObject %10 %13
      %copy2 = OpCopyObject %10 %copy1
         %16 = OpImageQuerySizeLod %v2int %copy2 %int_0
         %19 = OpCompositeExtract %int %16 0
               OpStore %x %19
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, UndefImage) {
    TEST_DESCRIPTION("A OpSampledImage has the Image ID pointing to a OpUndef");

    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %image DescriptorSet 0
               OpDecorate %image Binding 1
               OpDecorate %sampler DescriptorSet 0
               OpDecorate %sampler Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%ptr_var_func = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
  %ptr_image = OpTypePointer UniformConstant %10
      %image = OpVariable %ptr_image UniformConstant
         %14 = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %14
    %sampler = OpVariable %ptr_sampler UniformConstant
         %18 = OpTypeSampledImage %10
      %undef = OpUndef %10
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
       %data = OpVariable %ptr_var_func Function
         %13 = OpLoad %14 %sampler
         %19 = OpSampledImage %18 %undef %13
         %23 = OpImageSampleExplicitLod %v4float %19 %22 Lod %float_0
               OpStore %data %23
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderImageAccess, ComponentTypeMismatchFunctionTwoArgs) {
    TEST_DESCRIPTION("Pass a signed and unsinged sampler, and use the correct one.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s; // not accessed (so ignored)
        layout(set=0, binding=1) uniform usampler2D u; // accessed
        layout(location=0) out vec4 color;

        vec4 foo(isampler2D _s, usampler2D _u) {
            return texelFetch(_u, ivec2(0), 0);
        }
        void main() {
           color = foo(s, u);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.WriteDescriptorImageInfo(1, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderImageAccess, SamplerNeverAccessed) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat good_format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat bad_format = VK_FORMAT_B8G8R8A8_UNORM;

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), bad_format, &formatProps);
    formatProps.optimalTilingFeatures = (formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), bad_format, formatProps);

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), good_format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), good_format, formatProps);

    vkt::Image bad_image(*m_device, 128, 128, 1, bad_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView bad_view = bad_image.CreateView();

    vkt::Image good_image(*m_device, 128, 128, 1, good_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView good_view = good_image.CreateView();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.minFilter = VK_FILTER_LINEAR;  // turned off feature bit for test
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.compareEnable = VK_FALSE;
    vkt::Sampler sampler(*m_device, sampler_ci);

    char const *fs_source = R"glsl(
        #version 450
        layout (set=0, binding=0) uniform sampler2D bad; // never accessed
        layout (set=0, binding=1) uniform sampler2D good;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(good, gl_FragCoord.xy);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, bad_view, sampler.handle());
    descriptor_set.WriteDescriptorImageInfo(1, good_view, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveShaderImageAccess, AliasImageBinding) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    RETURN_IF_SKIP(Init());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2D float_textures[2];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[2];
        layout(set = 0, binding = 1) buffer output_buffer { vec4 data; }; // avoid optimization

        void main() {
            const vec4 value = texelFetch(float_textures[0], ivec2(0), 0);
            const uint mask = texelFetch(uint_textures[1], ivec2(0), 0).x;
            data = mask > 0 ? value : vec4(0.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, float_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 0);
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, uint_image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL, 1);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}