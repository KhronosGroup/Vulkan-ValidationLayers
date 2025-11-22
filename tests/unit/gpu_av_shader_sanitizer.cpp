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
#include "pipeline_helper.h"
#include "shader_helper.h"

class NegativeGpuAVShaderSanitizer : public GpuAVGpuAVShaderSanitizer {};

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroUDivScalar) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            uint index;
            uint result;
        };

        void main() {
            result = 5 / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroSDivScalar) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            int index;
            int result;
        };

        void main() {
            result = 5 / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroUDivVector) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            uvec4 index;
            uvec4 result;
        };

        void main() {
            result = uvec4(5) / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroSDivVector) {
    const char *cs_source = R"glsl(
        #version 450 core
        layout(set=0, binding=0) buffer SSBO {
            ivec2 index;
            ivec2 result;
        };

        void main() {
            result = ivec2(5) / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroUDivVector64) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        layout(set=0, binding=0) buffer SSBO {
            u64vec3 index;
            u64vec3 result;
        };

        void main() {
            result = u64vec3(5) / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroUDiv8Bit) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
        layout(set=0, binding=0) buffer SSBO {
            int8_t index;
            int8_t result;
        };

        void main() {
            result = int8_t(5) / index;
        }
    )glsl";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_GLSL, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroIntDivConstant) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void* in_ptr = in_buffer.Memory().Map();
    memset(in_ptr, 0, 256);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    std::string base = R"asm(
               OpCapability Shader
               OpCapability Int64
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpMemberDecorate %SSBO 3 Offset 16
               OpMemberDecorate %SSBO 4 Offset 32
               OpMemberDecorate %SSBO 5 Offset 48
               OpDecorate %var Binding 0
               OpDecorate %var DescriptorSet 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint64 = OpTypeInt 64 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v3int = OpTypeVector %int 3
     %v4uint = OpTypeVector %uint 4

      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
  %uint_null = OpConstantNull %uint
   %uint64_0 = OpConstant %uint64 0

%u32v2_1_null = OpConstantComposite %v2uint %uint_1 %uint_null
   %i32v3_000 = OpConstantComposite %v3int %int_0 %int_0 %int_0
  %u32v4_1101 = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_0 %uint_1
  %u32v4_null = OpConstantNull %v4uint

       %SSBO = OpTypeStruct %uint %int %uint64 %v2uint %v3int %v4uint
   %ptr_ssbo = OpTypePointer StorageBuffer %SSBO
        %var = OpVariable %ptr_ssbo StorageBuffer
%ptr_ssbo_v2uint = OpTypePointer StorageBuffer %v2uint
%ptr_ssbo_v3int = OpTypePointer StorageBuffer %v3int
%ptr_ssbo_v4uint = OpTypePointer StorageBuffer %v4uint
%ptr_ssbo_uint64 = OpTypePointer StorageBuffer %uint64
%ptr_ssbo_uint = OpTypePointer StorageBuffer %uint
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %func
      %label = OpLabel
    )asm";

    std::string end = R"asm(
        OpStore %4 %3 ; Prevents DCE
        OpReturn
        OpFunctionEnd
    )asm";

    auto test = [this, base, end, &pipeline_layout, &descriptor_set](std::string source) {
        std::string full_source = base + source + end;
        CreateComputePipelineHelper pipe(*this);
        pipe.cp_ci_.layout = pipeline_layout;
        pipe.cs_ = VkShaderObj(this, full_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                                  nullptr);
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_command_buffer.End();

        m_errorMonitor->SetDesiredError("UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
        m_default_queue->SubmitAndWait(m_command_buffer);
        m_errorMonitor->VerifyFound();
    };

    // UDiv 32-bit
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_uint %var %int_0
        %2 = OpLoad %uint %1
        %3 = OpUDiv %uint %2 %uint_0
        %4 = OpAccessChain %ptr_ssbo_uint %var %int_0
    )asm");

    // UDiv 32-bit Null
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_uint %var %int_0
        %2 = OpLoad %uint %1
        %3 = OpUDiv %uint %2 %uint_null
        %4 = OpAccessChain %ptr_ssbo_uint %var %int_0
    )asm");

    // SDiv 32-bit
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_int %var %int_1
        %2 = OpLoad %int %1
        %3 = OpSDiv %int %2 %uint_0
        %4 = OpAccessChain %ptr_ssbo_int %var %int_1
    )asm");

    // UDiv 64-bit Null
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_uint64 %var %int_2
        %2 = OpLoad %uint64 %1
        %3 = OpUDiv %uint64 %2 %uint64_0
        %4 = OpAccessChain %ptr_ssbo_uint64 %var %int_2
    )asm");

    // UDiv uvec2
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_v2uint %var %int_3
        %2 = OpLoad %v2uint %1
        %3 = OpUDiv %v2uint %2 %u32v2_1_null
        %4 = OpAccessChain %ptr_ssbo_v2uint %var %int_3
    )asm");

    // SDiv ivec3
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_v3int %var %int_4
        %2 = OpLoad %v3int %1
        %3 = OpSDiv %v3int %2 %i32v3_000
        %4 = OpAccessChain %ptr_ssbo_v3int %var %int_4
    )asm");

    // UDiv uvec4
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_v4uint %var %int_5
        %2 = OpLoad %v4uint %1
        %3 = OpUDiv %v4uint %2 %u32v4_1101
        %4 = OpAccessChain %ptr_ssbo_v4uint %var %int_5
    )asm");

    // UDiv uvec4 Null
    test(R"asm(
        %1 = OpAccessChain %ptr_ssbo_v4uint %var %int_5
        %2 = OpLoad %v4uint %1
        %3 = OpUDiv %v4uint %2 %u32v4_null
        %4 = OpAccessChain %ptr_ssbo_v4uint %var %int_5
    )asm");
}
