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

// TODO - Need to figure out to handle vectors
// https://gitlab.khronos.org/spirv/SPIR-V/-/issues/900
TEST_F(NegativeGpuAVShaderSanitizer, DISABLED_DivideByZeroUDivVector) {
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

// TODO - Need to figure out to handle vectors
// https://gitlab.khronos.org/spirv/SPIR-V/-/issues/900
TEST_F(NegativeGpuAVShaderSanitizer, DISABLED_DivideByZeroSDivVector) {
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

TEST_F(NegativeGpuAVShaderSanitizer, DivideByZeroUDivConstant) {
    const char *cs_source = R"asm(
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
       %uint = OpTypeInt 32 0
       %SSBO = OpTypeStruct %uint %uint
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
     %uint_0 = OpConstant %uint 0
       %main = OpFunction %void None %4
          %6 = OpLabel
         %15 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_0
         %16 = OpLoad %uint %15
         %18 = OpUDiv %uint %16 %uint_0
         %19 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_1
               OpStore %19 %18
               OpReturn
               OpFunctionEnd
    )asm";

    SimpleZeroComputeTest(cs_source, SPV_SOURCE_ASM, "UNASSIGNED-SPIRV-Sanitizer-Divide-By-Zero");
}
