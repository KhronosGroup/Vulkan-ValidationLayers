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

#include <gtest/gtest.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_helper.h"
#include "generated/vk_function_pointers.h"
#include "cooperative_matrix_helper.h"

void GpuAVSharedMemoryDataRaceTest::InitSharedMemoryDataRace(uint32_t message_limit) {
    SetTargetApiVersion(VK_API_VERSION_1_3);

    // some tests can report a variable number of errors, depending on the order invocations
    // execute the instructions (max one error reported per invocation). message_limit should
    // be set to the minimum number of expected errors.
    RETURN_IF_SKIP(InitGpuAvFramework(
        {{OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &message_limit}}, false));
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVSharedMemoryDataRace : public GpuAVSharedMemoryDataRaceTest {
  protected:
    void TestHelper(const char* source, int source_type, spv_target_env env = SPV_ENV_VULKAN_1_2,
                    VkScopeKHR coopmat_scope = VK_SCOPE_DEVICE_KHR);
};

void PositiveGpuAVSharedMemoryDataRace::TestHelper(const char* shader_source, int source_type, spv_target_env env,
                                                   VkScopeKHR coopmat_scope) {
    RETURN_IF_SKIP(InitSharedMemoryDataRace());
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
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, env, (SpvSourceType)source_type);
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

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SingleScalar) {
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            atomicStore(temp, 0u, gl_ScopeWorkgroup, 0, 0);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SingleElementAccess) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp[2];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, TwoThreadsShareValuesThroughArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp[2];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            barrier();
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, TwoDimensionalArrayBarrier) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[3][2];
        void main() {
            temp[gl_LocalInvocationIndex][1] = 0;
            barrier();
            uint x = temp[(gl_LocalInvocationIndex + 1) % 3][1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, TwoDimensionalArray) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[3][2];
        void main() {
            temp[gl_LocalInvocationIndex][0] = 0;
            uint x = temp[(gl_LocalInvocationIndex + 1) % 3][1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, BasicStructBarrier) {
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
            barrier();
            if (gl_LocalInvocationIndex == 0) {
                temp.b = 0;
            } else {
                temp.a = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, StructVsScalarBarrier) {
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
            barrier();
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.b;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, VectorVsScalarBarrier) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uvec4 temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp = uvec4(0);
            }
            barrier();
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.z;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, TwoVariablesBarrier) {
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
            barrier();
            if (gl_LocalInvocationIndex == 1) {
                a = 0;
            } else {
                b = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, TwoVectorsBarrier) {
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
            barrier();
            if (gl_LocalInvocationIndex == 1) {
                a = uvec4(0);
            } else {
                b = uvec4(0);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, MultiLoad) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp[2];
        void main() {
            uint a = temp[gl_LocalInvocationIndex];
            uint b = temp[gl_LocalInvocationIndex ^ 1];
            uint c = temp[gl_LocalInvocationIndex];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, VectorArrayBarrier) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 4) in;
        shared uvec4 arr[4];
        void main() {
            arr[gl_LocalInvocationIndex] = uvec4(gl_LocalInvocationIndex);
            barrier();
            uvec4 sum;
            for (uint i = 0; i < 4; ++i) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SpecConstantArrayBarrier) {
    const char* shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        layout(constant_id = 0) const uint N = 2;
        shared uint temp[N];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            barrier();
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, NoLocalSize) {
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable

        shared uint temp;
        void main() {
            temp = 0u;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SpvEnvVulkan13) {
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            atomicStore(temp, 0u, gl_ScopeWorkgroup, 0, 0);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_3);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SharedMemoryDeclaredNotUsed) {
    const char* shader_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer foo {
            uint x;
        };
        shared uint temp; // not used, glslang still declares it
        void main() {
            x = 0;
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SlangBasic) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<uint> outputBuffer;

        [numthreads(2, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
        {
            static groupshared uint temp;
            if (groupThreadID.x == 0) {
                temp = 0;
            }
            GroupMemoryBarrierWithGroupSync();

            if (groupThreadID.x == 0) {
                outputBuffer[0] = temp;
            }
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SlangNestedStruct) {
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
            if (groupThreadID.x == 0) {
                temp.z[1].b = 0;
            }
            GroupMemoryBarrierWithGroupSync();

            if (groupThreadID.x == 0) {
                outputBuffer[0] = temp.z[1].b;
            }
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SlangMultiEntryPoint) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<uint4> outputBuffer;
        groupshared uint4 a;
        groupshared uint4 b;

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint localIndex : SV_GroupIndex)
        {
            if (localIndex == 0) {
                a = uint4(0);
            } else {
                b = uint4(0);
            }

            GroupMemoryBarrierWithGroupSync();
            outputBuffer[0] = a + b;
        }

        groupshared uint4 c;

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main_b(uint localIndex : SV_GroupIndex)
        {
            if (localIndex == 1) {
                a = uint4(0);
            } else {
                c = uint4(0);
            }

            GroupMemoryBarrierWithGroupSync();
            outputBuffer[0] = a + c;
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, SlangMultiEntryPoint2) {
    const char* shader_source = R"slang(
        RWStructuredBuffer<uint4> outputBuffer;
        groupshared uint4 a;
        groupshared uint4 b;

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main(uint localIndex : SV_GroupIndex)
        {
            if (localIndex == 0) {
                a = uint4(0);
            } else {
                b = uint4(0);
            }

            GroupMemoryBarrierWithGroupSync();
            outputBuffer[0] = a + b;
        }

        [shader("compute")]
        [numthreads(2, 1, 1)]
        void main_bad(uint3 groupThreadID : SV_GroupThreadID)
        {
            // not called
            b = uint4(0);
            GroupMemoryBarrierWithGroupSync();

            if (groupThreadID.x == 0) {
                outputBuffer[0] = b;
            }
        }
    )slang";

    TestHelper(shader_source, SPV_SOURCE_SLANG);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, LongVectorArrayBarrier) {
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
            barrier();
            uint sum;
            for (uint i = 0; i < N; ++i) {
                sum += v[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, StoreBarrierCoopMatLoad) {
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
            for (uint i = gl_LocalInvocationIndex; i < 16*16; i += gl_WorkGroupSize.x) {
                arr[i] = float16_t(i);
            }
            barrier();
            coopMatLoad(mat, arr, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatStoreBarrierLoad) {
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
            if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID % 2;
                uint sy = gl_SubgroupID / 2;
                coopMatStore(mat, arr, sx * 16 + sy * 32 * 16, 32, gl_CooperativeMatrixLayoutRowMajor);
            }
            barrier();
            float16_t sum = float16_t(0);
            for (uint i = gl_LocalInvocationIndex; i < 32*32; i += gl_WorkGroupSize.x) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreDisjoint) {
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
            // load/store disjoint quadrants
            if (gl_SubgroupID < 2) {
                uint sx = gl_SubgroupID & 1;
                uint sy = gl_SubgroupID & 1;
                coopMatStore(mat, arr, sx * 16 + sy * 32 * 16, 32, gl_CooperativeMatrixLayoutRowMajor);
            } else if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID & 1;
                uint sy = 1 - (gl_SubgroupID & 1);
                coopMatLoad(mat, arr, sx * 16 + sy * 32 * 16, 32, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, StoreBarrierCoopMatLoadWorkgroup) {
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
            for (uint i = gl_LocalInvocationIndex; i < 32*32; i += gl_WorkGroupSize.x) {
                arr[i] = float16_t(i);
            }
            barrier();
            coopMatLoad(mat, arr, 0, 32, gl_CooperativeMatrixLayoutRowMajor);
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_WORKGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatStoreBarrierLoadWorkgroup) {
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
            barrier();
            float16_t sum = float16_t(0);
            for (uint i = gl_LocalInvocationIndex; i < 32*32; i += gl_WorkGroupSize.x) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_WORKGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreDisjointVector) {
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
        shared f16vec4 arr[32*32/4];
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
            // load/store disjoint quadrants
            if (gl_SubgroupID < 2) {
                uint sx = gl_SubgroupID & 1;
                uint sy = gl_SubgroupID & 1;
                coopMatStore(mat, arr, (sx * 16 + sy * 32 * 16)/4, 32/4, gl_CooperativeMatrixLayoutRowMajor);
            } else if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID & 1;
                uint sy = 1 - (gl_SubgroupID & 1);
                coopMatLoad(mat, arr, (sx * 16 + sy * 32 * 16)/4, 32/4, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreDisjointFloat) {
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
            // load/store disjoint quadrants
            if (gl_SubgroupID < 2) {
                uint sx = gl_SubgroupID & 1;
                uint sy = gl_SubgroupID & 1;
                coopMatStore(mat, arr, (sx * 16 + sy * 32 * 16)/2, 32/2, gl_CooperativeMatrixLayoutRowMajor);
            } else if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID & 1;
                uint sy = 1 - (gl_SubgroupID & 1);
                coopMatLoad(mat, arr, (sx * 16 + sy * 32 * 16)/2, 32/2, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreDisjointVec4) {
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
            // load/store disjoint quadrants
            if (gl_SubgroupID < 2) {
                uint sx = gl_SubgroupID & 1;
                uint sy = gl_SubgroupID & 1;
                coopMatStore(mat, arr, (sx * 16 + sy * 32 * 16)/8, 32/8, gl_CooperativeMatrixLayoutRowMajor);
            } else if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID & 1;
                uint sy = 1 - (gl_SubgroupID & 1);
                coopMatLoad(mat, arr, (sx * 16 + sy * 32 * 16)/8, 32/8, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}

TEST_F(PositiveGpuAVSharedMemoryDataRace, CoopMatLoadCoopMatStoreDisjointUint8) {
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
            // load/store disjoint quadrants
            if (gl_SubgroupID < 2) {
                uint sx = gl_SubgroupID & 1;
                uint sy = gl_SubgroupID & 1;
                coopMatStore(mat, arr, (sx * 16 + sy * 32 * 16)*2, 32*2, gl_CooperativeMatrixLayoutRowMajor);
            } else if (gl_SubgroupID < 4) {
                uint sx = gl_SubgroupID & 1;
                uint sy = 1 - (gl_SubgroupID & 1);
                coopMatLoad(mat, arr, (sx * 16 + sy * 32 * 16)*2, 32*2, gl_CooperativeMatrixLayoutRowMajor);
            }
        }
    )glsl";

    TestHelper(shader_source, SPV_SOURCE_GLSL, SPV_ENV_VULKAN_1_2, VK_SCOPE_SUBGROUP_KHR);
}
