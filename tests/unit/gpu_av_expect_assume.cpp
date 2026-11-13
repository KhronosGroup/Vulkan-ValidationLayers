/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "shader_helper.h"

class NegativeGpuAVExpectAssume : public GpuAVExpectAssume {};

TEST_F(NegativeGpuAVExpectAssume, AssumeFalse) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint result;
        };

        void main() {
            assumeEXT(count > 0);
            result = count;
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseUnsafeMode) {
    RETURN_IF_SKIP(InitExpectAssume(false));
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint result;
        };

        void main() {
            assumeEXT(count > 0);
            result = count;
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseInLoop) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint data[4];
        };

        void main() {
            for (uint i = 0; i < 4; i++) {
                assumeEXT(data[i] > 0);
                count += data[i];
            }
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseInLoopHeader) {
    TEST_DESCRIPTION("Other passes (currently) skip loop header blocks, this one doesn't need to");
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"(
               OpCapability Shader
               OpCapability ExpectAssumeKHR
               OpExtension "SPV_KHR_expect_assume"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %uint
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_uint Function
               OpStore %i %uint_0
               OpBranch %header
     %header = OpLabel
     %count_ptr = OpAccessChain %_ptr_StorageBuffer_uint %_ %uint_0
     %count = OpLoad %uint %count_ptr
     %cond = OpUGreaterThan %bool %count %uint_0
               OpAssumeTrueKHR %cond
               OpLoopMerge %merge %continue None
               OpBranch %check
      %check = OpLabel
         %15 = OpLoad %uint %i
         %18 = OpULessThan %bool %15 %uint_4
               OpBranchConditional %18 %body %merge
       %body = OpLabel
     %result_ptr = OpAccessChain %_ptr_StorageBuffer_uint %_ %uint_1
         %31 = OpLoad %uint %result_ptr
         %29 = OpLoad %uint %i
         %32 = OpIAdd %uint %31 %29
               OpStore %result_ptr %32
               OpBranch %continue
   %continue = OpLabel
         %34 = OpLoad %uint %i
         %35 = OpIAdd %uint %34 %uint_1
               OpStore %i %35
               OpBranch %header
      %merge = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    ZeroComputeTest(cs_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseInFunction) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint count;
            uint result;
        };

        uint Load() {
            assumeEXT(count > 0);
            return count;
        }

        void main() {
            result = Load();
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseTwoInvocations) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(local_size_x = 2) in;
        layout(set=0, binding=0) buffer SSBO {
            uint a;
            uint b;
            uint result[2];
        };

        void main() {
            if (gl_LocalInvocationIndex == 0) {
                assumeEXT(a == 1);
                result[0] = a;
            } else {
                assumeEXT(b == 2);
                result[1] = b;
            }
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 2);
}

TEST_F(NegativeGpuAVExpectAssume, AssumeFalseTwiceSameInvocation) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(set=0, binding=0) buffer SSBO {
            uint a;
            uint b;
            uint result;
        };

        void main() {
            assumeEXT(a == 1);
            assumeEXT(b == 2);
            result = a + b;
        }
    )glsl";

    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVExpectAssume, SpecConstantFalse) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        layout(constant_id = 0) const bool kCondition = true;
        layout(set=0, binding=0) buffer SSBO {
            uint result;
        };

        void main() {
            assumeEXT(kCondition);
            result = 1;
        }
    )glsl";

    const VkBool32 condition = VK_FALSE;
    const VkSpecializationMapEntry entry = {0, 0, sizeof(VkBool32)};
    const VkSpecializationInfo spec_info = {1, &entry, sizeof(VkBool32), &condition};
    ZeroComputeTest(cs_source, SPV_SOURCE_GLSL, 1, "SPIRV-ExpectAssume-AssumeTrue", &spec_info);
}

TEST_F(NegativeGpuAVExpectAssume, DebugSource) {
    RETURN_IF_SKIP(InitExpectAssume());
    const char* cs_source = R"(
               OpCapability Shader
               OpCapability ExpectAssumeKHR
               OpExtension "SPV_KHR_expect_assume"
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_expect_assume : enable
layout(set = 0, binding = 0) buffer SSBO { uint count; uint result; };
void main() {
    assumeEXT(count > 0);
    result = count;
}
"
               OpSourceExtension "GL_EXT_expect_assume"
               OpName %main "main"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "count"
               OpMemberName %SSBO 1 "result"
               OpName %_ ""
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %SSBO = OpTypeStruct %uint %uint
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
               OpLine %1 4 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 5 0
         %14 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
         %15 = OpLoad %uint %14
         %18 = OpUGreaterThan %bool %15 %uint_0
               OpAssumeTrueKHR %18
               OpLine %1 6 0
         %20 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
         %21 = OpLoad %uint %20
         %22 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_1
               OpStore %22 %21
               OpLine %1 7 0
               OpReturn
               OpFunctionEnd
    )";

    ZeroComputeTest(cs_source, SPV_SOURCE_ASM, 1, "assumeEXT(count > 0);");
}

TEST_F(NegativeGpuAVExpectAssume, VertexShader) {
    RETURN_IF_SKIP(InitExpectAssume());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char* vs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_expect_assume : enable
        void main() {
            assumeEXT(gl_VertexIndex != 1);
            gl_Position = vec4(0.0);
        }
    )glsl";

    VkShaderObj vs(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("SPIRV-ExpectAssume-AssumeTrue");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
