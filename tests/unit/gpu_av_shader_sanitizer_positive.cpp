/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
#include "shader_helper.h"

class PositiveGpuAVShaderSanitizer : public GpuAVGpuAVShaderSanitizer {};

// Set a single SSBO to all zero
void GpuAVGpuAVShaderSanitizer::SimpleZeroComputeTest(const char *shader, int source_type, const char *expected_error,
                                                      uint32_t error_count) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, (SpvSourceType)source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void *in_ptr = in_buffer.Memory().Map();
    memset(in_ptr, 0, 256);

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    if (expected_error) m_errorMonitor->SetDesiredError(expected_error, error_count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    if (expected_error) m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVShaderSanitizer, DivideByOne) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            uint u_index;
            int s_index;
            uvec4 v_index;
            float f_index;

            uint u_result;
            int s_result;
            uvec4 v_result;
            float f_result;
        };

        void main() {
            u_result = 5 / (u_index + 1);
            s_result = 5 / (s_index - 1);
            v_result = uvec4(1) / (uvec4(1) + v_index);

            u_result = u_result / 2;
            f_result = 1 / f_index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVShaderSanitizer, Mod) {
    const char* cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            int a[2];
            uvec4 b[2];
            ivec3 c[2];
        };

        void main() {
            a[0] = 5 % (a[1] - 1);
            b[0] = uvec4(1) % (uvec4(1) + b[0]);
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVShaderSanitizer, SRem) {
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %SSBO = OpTypeStruct %int %int
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %15 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_0
         %16 = OpLoad %int %15
         %17 = OpIAdd %int %16 %int_1
         %18 = OpSRem %int %int_1 %17
         %19 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %19 %18
               OpReturn
               OpFunctionEnd
    )asm";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_ASM);
}

TEST_F(PositiveGpuAVShaderSanitizer, ImageGather) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // if (condition != 0) {
    //     result = textureGather(tex, vec2(0), -1);
    // }
    const char *cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_ %tex
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 16
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
               OpDecorate %tex Binding 0
               OpDecorate %tex DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %uint %v4float
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
     %int_n1 = OpConstant %int -1
         %24 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %25 = OpTypeSampledImage %24
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
        %tex = OpVariable %_ptr_UniformConstant_25 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %main = OpFunction %void None %4
          %6 = OpLabel
         %16 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
         %17 = OpLoad %uint %16
         %20 = OpINotEqual %bool %17 %uint_0
               OpSelectionMerge %22 None
               OpBranchConditional %20 %21 %22
         %21 = OpLabel
         %28 = OpLoad %25 %tex
         %32 = OpImageGather %v4float %28 %31 %int_n1
         %34 = OpAccessChain %_ptr_StorageBuffer_v4float %_ %int_1
               OpStore %34 %32
               OpBranch %22
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void *in_ptr = buffer.Memory().Map();
    memset(in_ptr, 0, 64);

    vkt::Image image(*m_device, 16, 16, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVShaderSanitizer, MultiplePass) {
    TEST_DESCRIPTION("Make sure multiple functions passes work");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char* cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            int a;
            vec4 b;
            float c;
        };

        layout(set=0, binding=1) uniform sampler2D tex;

        void main() {
            a = 2 / a;
            vec4 b = textureGather(tex, vec2(0), 0);
            c = mod(4.0, c);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();
}
