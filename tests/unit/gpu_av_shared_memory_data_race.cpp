/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 * Copyright (c) 2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_helper.h"
#include "cooperative_matrix_helper.h"

class NegativeGpuAVSharedMemoryDataRace : public GpuAVSharedMemoryDataRaceTest {
  protected:
    void TestHelper(const char* source, int source_type, uint32_t count, VkScopeKHR coopmat_scope = VK_SCOPE_DEVICE_KHR,
                    const char* error = "SharedMemoryDataRace");
    // Runs a compute pipeline built from `shader_source` and asserts the race error matches
    // `full_regex`. The pipeline/buffer are kept alive on the stack here so they outlive
    // SubmitAndWait.
    void RunAndAssertErrorRegex(const char* shader_source, const std::string& full_regex,
                                SpvSourceType source_type = SPV_SOURCE_GLSL);
};

void NegativeGpuAVSharedMemoryDataRace::RunAndAssertErrorRegex(const char* shader_source, const std::string& full_regex,
                                                               SpvSourceType source_type) {
    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredErrorRegex("", full_regex);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

void NegativeGpuAVSharedMemoryDataRace::TestHelper(const char* shader_source, int source_type, uint32_t count,
                                                   VkScopeKHR coopmat_scope, const char* error) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace(count));
    if (source_type == SPV_SOURCE_SLANG) {
        RETURN_IF_SKIP(CheckSlangSupport());
    }
    if (coopmat_scope != VK_SCOPE_DEVICE_KHR) {
        CooperativeMatrixHelper helper(*this);
        if (!helper.HasValidProperty(coopmat_scope, 16, 16, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
            GTEST_SKIP() << "16x16 float16 Property not found";
        }
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, (SpvSourceType)source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError(error, count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SingleScalar) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            temp = 0;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SingleElementArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[1];
        void main() {
            temp[0] = 0;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoThreadsShareValuesThroughArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp[2];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoDimensionalArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[3][2];
        void main() {
            temp[gl_LocalInvocationIndex][1] = 0;
            uint x = temp[(gl_LocalInvocationIndex + 1) % 3][1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 3);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, BasicStruct) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        struct S { uint a, b; };
        shared S temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp.a = 0;
            } else {
                temp.b = 0;
            }
            if (gl_LocalInvocationIndex == 0) {
                temp.b = 0;
            } else {
                temp.a = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, StructVsScalar) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        struct S { uint a, b; };
        shared S temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                S t2;
                temp = t2;
            }
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.b;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VectorVsScalar) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uvec4 temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp = uvec4(0);
            }
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.z;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoVariables) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint a, b;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                a = 0;
            } else {
                b = 0;
            }
            if (gl_LocalInvocationIndex == 1) {
                a = 0;
            } else {
                b = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoVectors) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uvec4 a, b;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                a = uvec4(0);
            } else {
                b = uvec4(0);
            }
            if (gl_LocalInvocationIndex == 1) {
                a = uvec4(0);
            } else {
                b = uvec4(0);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VectorArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 4) in;
        shared uvec4 arr[4];
        void main() {
            arr[gl_LocalInvocationIndex] = uvec4(gl_LocalInvocationIndex);
            uvec4 sum;
            for (uint i = 0; i < 4; ++i) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, ShortIndex) {
    AddRequiredFeature(vkt::Feature::shaderInt16);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable

        layout(local_size_x = 4) in;
        shared uvec4 arr[4];
        void main() {
            arr[gl_LocalInvocationIndex] = uvec4(gl_LocalInvocationIndex);
            uvec4 sum;
            for (int16_t i = int16_t(0); i < 4; ++i) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SpecConstantArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        layout(constant_id = 0) const uint N = 2;
        shared uint temp[N];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VariableName) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        layout(constant_id = 0) const uint N = 2;
        uint other0 = 0;
        shared uint temp0[N];
        shared uint temp1[N];
        shared uint temp2[N];
        shared uint temp3[N];
        shared uint temp4[N];
        shared uint temp5[N];
        shared uint temp6[N];
        shared uint temp7[N];
        shared uint temp8[N];
        shared uint temp9[N];
        shared uint temp10[N];
        uint other1 = 0;
        void main() {
            uint other2 = 0;
            temp0[gl_LocalInvocationIndex] = 0;
            temp1[gl_LocalInvocationIndex] = 0;
            temp2[gl_LocalInvocationIndex] = 0;
            temp3[gl_LocalInvocationIndex] = 0;
            temp4[gl_LocalInvocationIndex] = 0;
            temp5[gl_LocalInvocationIndex] = 0;
            temp6[gl_LocalInvocationIndex] = 0;
            temp7[gl_LocalInvocationIndex] = 0;
            temp8[gl_LocalInvocationIndex] = 0;
            temp9[gl_LocalInvocationIndex] = 0;
            temp10[gl_LocalInvocationIndex] = 0;
            uint x = temp6[gl_LocalInvocationIndex ^ 1];
            uint other3 = 0;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 2, VK_SCOPE_DEVICE_KHR, "temp6");
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, AtomicStoreAndStore) {
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                atomicStore(temp, 0u, gl_ScopeWorkgroup, 0, 0);
            } else {
                temp = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, AtomicAddAndLoad) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                atomicAdd(temp, 1);
            } else {
                temp = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, MultipleFunction) {
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 2) in;
        shared uint temp[4];
        void A() {
            if (gl_LocalInvocationIndex == 0) {
                temp[1] = 1;
            }
        }
        void B() {
            temp[2] = 2; // race
        }
        void C() {
            if (gl_LocalInvocationIndex == 1) {
                temp[3] = 3;
            }
            B();
        }
        void main() {
            if (gl_LocalInvocationIndex == 2) {
                temp[0] = 0; // not touched
            }
            A();
            C();
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, WorkgroupSize) {
    const char* shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, WorkgroupSizeSpecConstant) {
    const char* shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpDecorate %x SpecId 0
               OpDecorate %y SpecId 1
               OpDecorate %z SpecId 2
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
          %x = OpSpecConstant %uint 2
          %y = OpSpecConstant %uint 1
          %z = OpSpecConstant %uint 1
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %x %y %z
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, LocalSizeId) {
    AddRequiredFeature(vkt::Feature::maintenance4);
    const char* shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, LocalSizeIdSpecConstant) {
    AddRequiredFeature(vkt::Feature::maintenance4);
    const char* shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpExecutionModeId %main LocalSizeId %x %y %z
               OpDecorate %x SpecId 0
               OpDecorate %y SpecId 1
               OpDecorate %z SpecId 2
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
          %x = OpSpecConstant %uint 2
          %y = OpSpecConstant %uint 1
          %z = OpSpecConstant %uint 1
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SlangScalar) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<uint> outputBuffer;

        [numthreads(2, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
        {
            static groupshared uint temp;
            temp = 0;
            GroupMemoryBarrierWithGroupSync();

            if (groupThreadID.x == 0) {
                outputBuffer[0] = temp;
            }
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SlangNestedStruct) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(2, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
        {
            struct A {
                uint32_t b;
                uint32_t c;
            };
            struct S {
                uint32_t x;
                uint32_t y;
                A z[2];
            };

            static groupshared S temp;
            temp.z[groupThreadID.x].b = 0;

            outputBuffer[groupThreadID.x] = temp.z[1 - groupThreadID.x].b;
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SlangMultiEntryPoint) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<uint4> outputBuffer;
        groupshared uint4 a;
        groupshared uint4 b;

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main(uint3 groupThreadID : SV_GroupThreadID)
        {
            b = uint4(0);
            GroupMemoryBarrierWithGroupSync();

            if (groupThreadID.x == 0) {
                outputBuffer[0] = b;
            }
        }

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main_good(uint localIndex : SV_GroupIndex)
        {
            // not called
            if (localIndex == 0) {
                a = uint4(0);
            } else {
                b = uint4(0);
            }

            GroupMemoryBarrierWithGroupSync();
            outputBuffer[0] = a + b;
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, NoOpName) {
    const char* shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpExecutionMode %main LocalSize 2 1 1
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
         %14 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, DebugGlobalVariable) {
    const char* shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp
               OpExecutionMode %main LocalSize 2 1 1
     %string = OpString "MyAwesomeVariable"
      %empty = OpString ""
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
         %14 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1

   %d_source = OpExtInst %void %2 DebugSource %empty %empty
     %d_unit = OpExtInst %void %2 DebugCompilationUnit %uint_0 %uint_0 %d_source %uint_0
    %d_basic = OpExtInst %void %2 DebugTypeBasic %empty %uint_0 %uint_0 %uint_0
    %d_array = OpExtInst %void %2 DebugTypeArray %d_basic %uint_0
      %d_var = OpExtInst %void %2 DebugGlobalVariable %string %d_array %d_source %uint_0 %uint_0 %d_unit %empty %temp %uint_0

       %main = OpFunction %void None %3
          %6 = OpLabel
               OpStore %temp %uint_0
               OpReturn
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1, VK_SCOPE_DEVICE_KHR, "MyAwesomeVariable");
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, ShaderDebugInfo) {
    const char* shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp1 %temp2 %temp3 %outputBuffer %gl_LocalInvocationID
               OpExecutionMode %main LocalSize 2 1 1
          %1 = OpString "RWStructuredBuffer<uint> outputBuffer;
[[vk::constant_id(0)]]
const uint TEMP1_SIZE = 2;
[[vk::constant_id(1)]]
const uint TEMP2_SIZE = 2;
[[vk::constant_id(2)]]
const uint TEMP3_SIZE = 2;

[numthreads(2, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
    static groupshared uint temp1[TEMP1_SIZE];
    static groupshared uint temp2[TEMP2_SIZE];
    static groupshared uint temp3[TEMP3_SIZE];
    if (groupThreadID.x == 0) {
        temp1[0] = 0;
        temp2[1] = 0;
    }
    temp3[0] = 0;

    GroupMemoryBarrierWithGroupSync();

    if (groupThreadID.x == 0) {
        outputBuffer[0] = temp1[0];
        outputBuffer[1] = temp1[1];
        outputBuffer[2] = temp2[0];
        outputBuffer[3] = temp2[1];
        outputBuffer[4] = temp3[0];
        outputBuffer[5] = temp3[1];
    }
}"
          %5 = OpString "my_clever_code.slang"
               OpSource Slang 1
         %19 = OpString "uint"
         %30 = OpString "main"
         %43 = OpString "slangc"
         %44 = OpString "-target spir -matrix-layout-column-major -stage compute -entry main -g2"
         %46 = OpString "groupThreadID"
         %55 = OpString "dispatchThreadID"
         %94 = OpString "temp1"
        %108 = OpString "temp2"
        %123 = OpString "temp3"
        %145 = OpString "unnamed"
        %147 = OpString "RWStructuredBuffer"
        %149 = OpString "outputBuffer"
               OpName %groupThreadID "groupThreadID"
               OpName %dispatchThreadID "dispatchThreadID"
               OpName %TEMP1_SIZE "TEMP1_SIZE"
               OpName %temp1 "temp1"
               OpName %TEMP2_SIZE "TEMP2_SIZE"
               OpName %temp2 "temp2"
               OpName %TEMP3_SIZE "TEMP3_SIZE"
               OpName %temp3 "temp3"
               OpName %RWStructuredBuffer "RWStructuredBuffer"
               OpName %outputBuffer "outputBuffer"
               OpName %main "main"
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %TEMP1_SIZE SpecId 0
               OpDecorate %TEMP2_SIZE SpecId 1
               OpDecorate %TEMP3_SIZE SpecId 2
               OpDecorate %_ptr_StorageBuffer_uint ArrayStride 4
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %outputBuffer Binding 0
               OpDecorate %outputBuffer DescriptorSet 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_11 = OpConstant %uint 11
     %uint_5 = OpConstant %uint 5
   %uint_100 = OpConstant %uint 100
         %12 = OpTypeFunction %void
     %v3uint = OpTypeVector %uint 3
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
%uint_131072 = OpConstant %uint 131072
     %uint_3 = OpConstant %uint 3
     %uint_7 = OpConstant %uint 7
     %uint_0 = OpConstant %uint 0
    %uint_10 = OpConstant %uint 10
     %uint_2 = OpConstant %uint 2
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
     %uint_1 = OpConstant %uint 1
    %uint_12 = OpConstant %uint 12
    %uint_24 = OpConstant %uint 24
    %uint_25 = OpConstant %uint 25
    %uint_13 = OpConstant %uint 13
    %uint_14 = OpConstant %uint 14
    %uint_15 = OpConstant %uint 15
       %bool = OpTypeBool
    %uint_26 = OpConstant %uint 26
    %uint_16 = OpConstant %uint 16
     %uint_9 = OpConstant %uint 9
 %TEMP1_SIZE = OpSpecConstant %uint 2
        %int = OpTypeInt 32 1
%_arr_uint_88 = OpTypeArray %uint %TEMP1_SIZE
%_ptr_Workgroup__arr_uint_88 = OpTypePointer Workgroup %_arr_uint_88
%uint_2147483647 = OpConstant %uint 2147483647
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
      %int_0 = OpConstant %int 0
    %uint_17 = OpConstant %uint 17
 %TEMP2_SIZE = OpSpecConstant %uint 2
%_arr_uint_103 = OpTypeArray %uint %TEMP2_SIZE
%_ptr_Workgroup__arr_uint_103 = OpTypePointer Workgroup %_arr_uint_103
      %int_1 = OpConstant %int 1
    %uint_19 = OpConstant %uint 19
 %TEMP3_SIZE = OpSpecConstant %uint 2
%_arr_uint_118 = OpTypeArray %uint %TEMP3_SIZE
%_ptr_Workgroup__arr_uint_118 = OpTypePointer Workgroup %_arr_uint_118
    %uint_21 = OpConstant %uint 21
   %uint_264 = OpConstant %uint 264
    %uint_23 = OpConstant %uint 23
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_2 = OpConstant %int 2
    %uint_27 = OpConstant %uint 27
      %int_3 = OpConstant %int 3
    %uint_28 = OpConstant %uint 28
      %int_4 = OpConstant %int 4
    %uint_29 = OpConstant %uint 29
      %int_5 = OpConstant %int 5
    %uint_31 = OpConstant %uint 31
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
      %temp1 = OpVariable %_ptr_Workgroup__arr_uint_88 Workgroup
      %temp2 = OpVariable %_ptr_Workgroup__arr_uint_103 Workgroup
      %temp3 = OpVariable %_ptr_Workgroup__arr_uint_118 Workgroup
%outputBuffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
        %227 = OpExtInst %void %2 DebugInfoNone
         %48 = OpExtInst %void %2 DebugExpression
          %4 = OpExtInst %void %2 DebugSource %5 %1
         %10 = OpExtInst %void %2 DebugCompilationUnit %uint_100 %uint_5 %4 %uint_11
         %18 = OpExtInst %void %2 DebugTypeBasic %19 %uint_32 %uint_6 %uint_131072
         %23 = OpExtInst %void %2 DebugTypeVector %18 %uint_3
         %25 = OpExtInst %void %2 DebugTypePointer %23 %uint_7 %uint_131072
         %27 = OpExtInst %void %2 DebugTypeFunction %uint_0 %void %25 %25
         %29 = OpExtInst %void %2 DebugFunction %30 %27 %4 %uint_10 %uint_6 %10 %30 %uint_0 %uint_10
         %42 = OpExtInst %void %2 DebugEntryPoint %29 %10 %43 %44
%groupThreadID = OpExtInst %void %2 DebugLocalVariable %46 %23 %4 %uint_10 %uint_6 %29 %uint_0 %uint_2
%dispatchThreadID = OpExtInst %void %2 DebugLocalVariable %55 %23 %4 %uint_10 %uint_6 %29 %uint_0 %uint_1
         %91 = OpExtInst %void %2 DebugTypeArray %18 %uint_2147483647
        %106 = OpExtInst %void %2 DebugTypeArray %18 %uint_2147483647
        %121 = OpExtInst %void %2 DebugTypeArray %18 %uint_2147483647
        %143 = OpExtInst %void %2 DebugTypeArray %18 %uint_0
        %144 = OpExtInst %void %2 DebugTypeMember %145 %143 %4 %uint_0 %uint_0 %uint_0 %uint_0 %uint_0
        %146 = OpExtInst %void %2 DebugTypeComposite %147 %uint_1 %4 %uint_0 %uint_0 %10 %147 %uint_0 %uint_131072 %144
         %93 = OpExtInst %void %2 DebugGlobalVariable %94 %91 %4 %uint_0 %uint_0 %10 %94 %temp1 %uint_0
        %107 = OpExtInst %void %2 DebugGlobalVariable %108 %106 %4 %uint_0 %uint_0 %10 %108 %temp2 %uint_0
        %122 = OpExtInst %void %2 DebugGlobalVariable %123 %121 %4 %uint_0 %uint_0 %10 %123 %temp3 %uint_0
        %148 = OpExtInst %void %2 DebugGlobalVariable %149 %146 %4 %uint_1 %uint_26 %10 %149 %outputBuffer %uint_0
       %main = OpFunction %void None %12
         %13 = OpLabel
         %32 = OpExtInst %void %2 DebugFunctionDefinition %29 %main
        %228 = OpExtInst %void %2 DebugScope %29
         %61 = OpLoad %v3uint %gl_LocalInvocationID
         %62 = OpExtInst %void %2 DebugLine %4 %uint_12 %uint_12 %uint_5 %uint_6
         %65 = OpExtInst %void %2 DebugLine %4 %uint_12 %uint_12 %uint_24 %uint_25
         %68 = OpExtInst %void %2 DebugLine %4 %uint_13 %uint_13 %uint_24 %uint_25
         %70 = OpExtInst %void %2 DebugLine %4 %uint_14 %uint_14 %uint_24 %uint_25
         %72 = OpExtInst %void %2 DebugLine %4 %uint_15 %uint_15 %uint_5 %uint_6
         %74 = OpCompositeExtract %uint %61 0
         %76 = OpIEqual %bool %74 %uint_0
         %77 = OpExtInst %void %2 DebugLine %4 %uint_15 %uint_15 %uint_25 %uint_26
        %229 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %36 None
               OpBranchConditional %76 %34 %36
         %34 = OpLabel
        %230 = OpExtInst %void %2 DebugScope %29
         %80 = OpExtInst %void %2 DebugLine %4 %uint_16 %uint_16 %uint_9 %uint_10
         %96 = OpAccessChain %_ptr_Workgroup_uint %temp1 %int_0
               OpStore %96 %uint_0
         %99 = OpExtInst %void %2 DebugLine %4 %uint_17 %uint_17 %uint_9 %uint_10
        %109 = OpAccessChain %_ptr_Workgroup_uint %temp2 %int_1
               OpStore %109 %uint_0
               OpBranch %36
         %36 = OpLabel
        %231 = OpExtInst %void %2 DebugScope %29
        %113 = OpExtInst %void %2 DebugLine %4 %uint_19 %uint_19 %uint_5 %uint_6
        %124 = OpAccessChain %_ptr_Workgroup_uint %temp3 %int_0
               OpStore %124 %uint_0
        %126 = OpExtInst %void %2 DebugLine %4 %uint_21 %uint_21 %uint_5 %uint_6
               OpControlBarrier %uint_2 %uint_2 %uint_264
        %130 = OpExtInst %void %2 DebugLine %4 %uint_23 %uint_23 %uint_5 %uint_6
        %132 = OpExtInst %void %2 DebugLine %4 %uint_23 %uint_23 %uint_25 %uint_26
        %232 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %40 None
               OpBranchConditional %76 %38 %40
         %38 = OpLabel
        %233 = OpExtInst %void %2 DebugScope %29
        %134 = OpExtInst %void %2 DebugLine %4 %uint_24 %uint_24 %uint_9 %uint_10
        %138 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_0
        %150 = OpAccessChain %_ptr_Workgroup_uint %temp1 %int_0
        %151 = OpLoad %uint %150
               OpStore %138 %151
        %153 = OpExtInst %void %2 DebugLine %4 %uint_25 %uint_25 %uint_9 %uint_10
        %154 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_1
        %155 = OpAccessChain %_ptr_Workgroup_uint %temp1 %int_1
        %156 = OpLoad %uint %155
               OpStore %154 %156
        %158 = OpExtInst %void %2 DebugLine %4 %uint_26 %uint_26 %uint_9 %uint_10
        %160 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_2
        %161 = OpAccessChain %_ptr_Workgroup_uint %temp2 %int_0
        %162 = OpLoad %uint %161
               OpStore %160 %162
        %164 = OpExtInst %void %2 DebugLine %4 %uint_27 %uint_27 %uint_9 %uint_10
        %167 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_3
        %168 = OpAccessChain %_ptr_Workgroup_uint %temp2 %int_1
        %169 = OpLoad %uint %168
               OpStore %167 %169
        %171 = OpExtInst %void %2 DebugLine %4 %uint_28 %uint_28 %uint_9 %uint_10
        %174 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_4
        %175 = OpLoad %uint %124
               OpStore %174 %175
        %177 = OpExtInst %void %2 DebugLine %4 %uint_29 %uint_29 %uint_9 %uint_10
        %180 = OpAccessChain %_ptr_StorageBuffer_uint %outputBuffer %int_0 %int_5
        %181 = OpAccessChain %_ptr_Workgroup_uint %temp3 %int_1
        %182 = OpLoad %uint %181
               OpStore %180 %182
               OpBranch %40
         %40 = OpLabel
        %234 = OpExtInst %void %2 DebugScope %29
        %185 = OpExtInst %void %2 DebugLine %4 %uint_31 %uint_31 %uint_1 %uint_2
               OpReturn
        %235 = OpExtInst %void %2 DebugNoScope
               OpFunctionEnd
    )";

    TestHelper(shader_source, SPV_SOURCE_ASM, 1, VK_SCOPE_DEVICE_KHR, "temp3[0] = 0;");
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, LongVectorArrayRace) {
    AddRequiredFeature(vkt::Feature::longVector);
    AddRequiredExtensions(VK_EXT_SHADER_LONG_VECTOR_EXTENSION_NAME);
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_long_vector : enable

        layout(constant_id = 0) const uint N = 5;
        layout(local_size_x = 5) in;
        shared vector<uint, N> v;
        void main() {
            v[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
            uint sum;
            for (uint i = 0; i < N; ++i) {
                sum += v[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, StoreCoopMatLoad) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

        layout(local_size_x = 128) in;
        shared float16_t arr[16*16];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            if (gl_LocalInvocationIndex == 40) {
                arr[17] = float16_t(40);
            }
            coopMatLoad(mat, arr, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatStoreLoad) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared float16_t arr[16*16];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            }
            if (gl_LocalInvocationIndex == 40) {
                float16_t sum = arr[17];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreOverlap) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared float16_t arr[32*32];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store overlapping in an 8x8 region
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
            } else {
                coopMatLoad(mat, arr, 8 + 32 * 8, 32, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, StoreCoopMatLoadWorkgroup) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

        layout(local_size_x = 128) in;
        shared float16_t arr[32*32];
        coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseA> mat;
        void main() {
            if (gl_LocalInvocationIndex == 40) {
                arr[17] = float16_t(40);
            }
            coopMatLoad(mat, arr, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_WORKGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatStoreLoadWorkgroup) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

        layout(local_size_x = 128) in;
        shared float16_t arr[32*32];
        coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseA> mat;
        void main() {
            coopMatStore(mat, arr, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
            if (gl_LocalInvocationIndex == 40) {
                float16_t sum = arr[17];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_WORKGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreOverlapFloat) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared float arr[32*32/2];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store overlapping in an 8x8 region
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 32/2, gl_CooperativeMatrixLayoutRowMajor);
            } else {
                coopMatLoad(mat, arr, (8 + 32 * 8)/2, 32/2, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreOverlapVec4) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared vec4 arr[32*32/8];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store overlapping in an 8x8 region
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 32/8, gl_CooperativeMatrixLayoutRowMajor);
            } else {
                coopMatLoad(mat, arr, (8 + 32 * 8)/8, 32/8, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreOverlapUint8) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared uint8_t arr[32*32*2];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store overlapping in an 8x8 region
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 32*2, gl_CooperativeMatrixLayoutRowMajor);
            } else {
                coopMatLoad(mat, arr, (8 + 32 * 8)*2, 32*2, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreOverlapUint8ColMajor) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared uint8_t arr[32*32*2];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store overlapping in an 8x8 region
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 32*2, gl_CooperativeMatrixLayoutColumnMajor);
            } else {
                coopMatLoad(mat, arr, (8 + 32 * 8)*2, 32*2, gl_CooperativeMatrixLayoutColumnMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, 1, VK_SCOPE_SUBGROUP_KHR);
}

// Race between atomicAdd and coopMatStore on overlapping shared memory. Both paths
// write THREAD_ID_MASK as the offender's thread_id, so the message should say
// "unknown invocation" regardless of which side detects the race.
TEST_F(NegativeGpuAVSharedMemoryDataRace, AtomicVsCoopMatStore) {
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSharedMemoryDataRace());
    CooperativeMatrixHelper helper(*this);
    if (!helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 16, 16, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "16x16 float16 Property not found";
    }

    // 16x16 f16 coopmat over a uint backing: each uint holds 2 f16s, so the matrix
    // covers 128 uints with stride 8 uints/row. atomicAdd targets a uint that the
    // coopMatStore also writes.
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_shader_subgroup_basic : enable

        layout(local_size_x = 128) in;
        shared uint arr[16*16/2];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            if (gl_LocalInvocationIndex == 40) {
                atomicAdd(arr[5], 1);
            }
            if (gl_SubgroupID == 0) {
                coopMatStore(mat, arr, 0, 16/2, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    RunAndAssertErrorRegex(shader_source, R"((?=[\s\S]*SharedMemoryDataRace))"
                                          R"((?=[\s\S]*Likely against unknown invocation))"
                                          R"((?=[\s\S]*The other access in this race was at:))");
}

// The next few tests verify the offender's SPIR-V op kind is reported correctly.
// They use GLSL without debug info so the offender renders as "SPIR-V Instruction:
// Op<Kind>", which is easy to grep for.

// Two invocations execute the same OpStore. The detector's and offender's inst_offset
// match, so the message should call out the same-instruction case instead of repeating
// the source line.
TEST_F(NegativeGpuAVSharedMemoryDataRace, SelfRaceSameInstruction) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());

    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            temp = 0;
        }
    )glsl";

    RunAndAssertErrorRegex(shader_source, R"((?=[\s\S]*SharedMemoryDataRace-RaceOnStore))"
                                          R"((?=[\s\S]*This race is between two invocations executing the same instruction))");
}

// Concurrent loaders racing one store. Either side can win: if the loaders go first the
// offender is OpLoad (multi-load atomicOr path), if the store goes first it's OpStore.
// The point is that whichever wins, inst_offset survived through the atomicOr.
TEST_F(NegativeGpuAVSharedMemoryDataRace, OffenderIsOpLoadFromMultipleLoaders) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());

    // 15 loaders + 1 storer reliably exercises the multi-load promotion path.
    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 16) in;
        shared uint temp;
        layout(set = 0, binding = 0) buffer SSBO { uint out_val; } ssbo;
        void main() {
            if (gl_LocalInvocationIndex > 0) {
                ssbo.out_val = temp;
            }
            if (gl_LocalInvocationIndex == 0) {
                temp = 42;
            }
        }
    )glsl";

    // OpLoad produces a result id ("%<n> = OpLoad ..."); OpStore does not.
    RunAndAssertErrorRegex(shader_source,
                           R"((?=[\s\S]*SharedMemoryDataRace))"
                           R"((?=[\s\S]*The other access in this race was at:\s*SPIR-V Instruction: (?:%\d+ = OpLoad|OpStore)))");
}

// Atomic vs store race. The offender is OpAtomicIAdd or OpStore depending on which side
// wins; the regex accepts either.
TEST_F(NegativeGpuAVSharedMemoryDataRace, OffenderIsOpAtomic) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());

    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                atomicAdd(temp, 1);
            } else {
                temp = 0;
            }
        }
    )glsl";

    // OpAtomicIAdd has a result id ("%<n> = OpAtomicIAdd ..."); OpStore does not.
    RunAndAssertErrorRegex(
        shader_source, R"((?=[\s\S]*SharedMemoryDataRace-RaceOnLoadStoreVsAtomic))"
                       R"((?=[\s\S]*The other access in this race was at:\s*SPIR-V Instruction: (?:%\d+ = OpAtomicIAdd|OpStore)))");
}

// Race that straddles a function boundary: one store in main, the other in a helper.
// Verifies the offender's source location is resolved regardless of which function it's in.
TEST_F(NegativeGpuAVSharedMemoryDataRace, RaceAcrossFunctions) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());

    const char* shader_source = R"glsl(
        #version 450
        layout(local_size_x = 2) in;
        shared uint temp;
        void helper() {
            if (gl_LocalInvocationIndex == 1) {
                temp = 1;
            }
        }
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp = 0;
            }
            helper();
        }
    )glsl";

    // Without debug info both render as "OpStore"; we just need the offender to resolve
    // to a real instruction, not the "not recorded" fallback.
    RunAndAssertErrorRegex(shader_source, R"((?=[\s\S]*SharedMemoryDataRace-RaceOnStore))"
                                          R"((?=[\s\S]*The other access in this race was at:\s*SPIR-V Instruction: OpStore))");
}

// Hand-written SPIR-V with DebugLine on both the store and the load, racing on two
// distinct source lines. Verifies both halves of the race render with the correct
// "Shader validation error occurred at <file>:<line>" snippet (the existing
// ShaderDebugInfo test is a self-race, so it only ever exercises one line).
TEST_F(NegativeGpuAVSharedMemoryDataRace, DebugInfoTwoSourceLines) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());

    const char* shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %temp %gl_LocalInvocationID
               OpExecutionMode %main LocalSize 2 1 1
        %src = OpString "void main() {
    if (gl_LocalInvocationIndex == 0) { temp = 0; }
    if (gl_LocalInvocationIndex == 1) { uint x = temp; }
}
"
       %file = OpString "race_negative_debug.glsl"
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %bool = OpTypeBool
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
       %temp = OpVariable %_ptr_Workgroup_uint Workgroup
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_5 = OpConstant %uint 5
    %uint_11 = OpConstant %uint 11
   %uint_100 = OpConstant %uint 100
   %d_source = OpExtInst %void %2 DebugSource %file %src
     %d_unit = OpExtInst %void %2 DebugCompilationUnit %uint_100 %uint_5 %d_source %uint_11
       %main = OpFunction %void None %3
      %entry = OpLabel
       %tidv = OpLoad %v3uint %gl_LocalInvocationID
        %tid = OpCompositeExtract %uint %tidv 0
    %is_zero = OpIEqual %bool %tid %uint_0
               OpSelectionMerge %after_store None
               OpBranchConditional %is_zero %do_store %after_store
   %do_store = OpLabel
        %dls = OpExtInst %void %2 DebugLine %d_source %uint_2 %uint_2 %uint_0 %uint_0
               OpStore %temp %uint_0
               OpBranch %after_store
%after_store = OpLabel
     %is_one = OpIEqual %bool %tid %uint_1
               OpSelectionMerge %after_load None
               OpBranchConditional %is_one %do_load %after_load
    %do_load = OpLabel
        %dll = OpExtInst %void %2 DebugLine %d_source %uint_3 %uint_3 %uint_0 %uint_0
        %val = OpLoad %uint %temp
               OpBranch %after_load
 %after_load = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    // The offender's line follows the "other access" header in the per-pass body, then
    // the framework's wrapper appends the detector's line at the end of the message.
    // Order of which side detects is non-deterministic, so accept either pairing - but
    // require the two lines to actually differ.
    RunAndAssertErrorRegex(
        shader_source,
        R"((?=[\s\S]*SharedMemoryDataRace))"
        R"((?:)"
        R"((?=[\s\S]*The other access in this race was at:\s*Shader validation error occurred at race_negative_debug\.glsl:2[\s\S]*Shader validation error occurred at race_negative_debug\.glsl:3))"
        R"(|)"
        R"((?=[\s\S]*The other access in this race was at:\s*Shader validation error occurred at race_negative_debug\.glsl:3[\s\S]*Shader validation error occurred at race_negative_debug\.glsl:2))"
        R"())",
        SPV_SOURCE_ASM);
}
