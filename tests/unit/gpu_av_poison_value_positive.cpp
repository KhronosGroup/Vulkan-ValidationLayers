/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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
#include "../framework/cooperative_matrix_helper.h"

class PositiveGpuAVPoisonValue : public GpuAVTest {
  protected:
    void InitPoisonValue();
    void SimpleComputeTest(const char* shader, SpvSourceType source_type = SPV_SOURCE_GLSL,
                           spv_target_env spv_env = SPV_ENV_VULKAN_1_1);
};

void PositiveGpuAVPoisonValue::InitPoisonValue() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
}

void PositiveGpuAVPoisonValue::SimpleComputeTest(const char* shader, SpvSourceType source_type, spv_target_env spv_env) {
    RETURN_IF_SKIP(InitPoisonValue());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader, VK_SHADER_STAGE_COMPUTE_BIT, spv_env, source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void* in_ptr = in_buffer.Memory().Map();
    memset(in_ptr, 0, 256);

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

TEST_F(PositiveGpuAVPoisonValue, InitializedVariable) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x = 42;
            output_val = x;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, WriteBeforeRead) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint input_val;
            uint output_val;
        };
        void main() {
            uint x;
            x = input_val;
            output_val = x;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, NoUninitializedVars) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint input_val;
            uint output_val;
        };
        void main() {
            output_val = input_val + 1;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, InitializedWithZero) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x = 0;
            if (x == 0) {
                output_val = 1;
            }
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, InitializedBranch) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            bool cond = true;
            if (cond) {
                output_val = 1;
            }
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, ArrayInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint arr[4] = uint[4](1, 2, 3, 4);
            output_val = arr[0];
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, ContaminatedWithClean) {
    // Initialized variable overwritten with a clean value should not trigger
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint input_val;
            uint output_val;
        };
        void main() {
            uint x = 0u;
            uint y = 0u;
            y = x;
            output_val = y;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, VectorWriteBeforeRead) {
    // Uninitialized vec4, fully written from buffer before read
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 input_val;
            vec4 output_val;
        };
        void main() {
            vec4 v;
            v = input_val;
            output_val = v;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, PartialVectorCleanExtract) {
    // Construct vec4 with 1 poison component, extract a clean component
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            float d;
            vec4 v = vec4(1.0, 2.0, 3.0, d);
            output_val = uint(v.x);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, StructInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S {
            uint a;
            float b;
        };
        void main() {
            S s = S(42u, 3.14);
            output_val = s.a;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, LoopInitializes) {
    // Variable uninitialized before loop, written on first iteration
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            for (int i = 0; i < 1; i++) {
                x = 42u;
            }
            output_val = x;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, PhiBothClean) {
    // Uninitialized variable, written to clean values in both branches
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint cond;
            uint output_val;
        };
        void main() {
            uint x;
            if (cond == 0) {
                x = 1u;
            } else {
                x = 2u;
            }
            output_val = x;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, PhiScalarBothCleanAsm) {
    // Hand-written SPIR-V with an actual OpPhi. Both incoming values are
    // clean constants, so the phi result should be clean.
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
   %void_fn  = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_0  = OpConstant %uint 0
    %uint_1  = OpConstant %uint 1
    %uint_2  = OpConstant %uint 2
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %cond_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
  %cond_val  = OpLoad %uint %cond_ptr
  %is_zero   = OpIEqual %bool %cond_val %uint_0
               OpSelectionMerge %merge None
               OpBranchConditional %is_zero %true_br %false_br
   %true_br  = OpLabel
               OpBranch %merge
  %false_br  = OpLabel
               OpBranch %merge
      %merge = OpLabel
     %result = OpPhi %uint %uint_1 %true_br %uint_2 %false_br
  %out_ptr   = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_1
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM);
}

TEST_F(PositiveGpuAVPoisonValue, PhiVectorBothCleanAsm) {
    // Hand-written SPIR-V OpPhi with vec4 type. Both branches provide
    // clean constant vectors.
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 16
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
   %void_fn  = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_0  = OpConstant %uint 0
    %uint_1  = OpConstant %uint 1
    %uint_2  = OpConstant %uint 2
     %v4uint = OpTypeVector %uint 4
    %vec_a   = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1
    %vec_b   = OpConstantComposite %v4uint %uint_2 %uint_2 %uint_2 %uint_2
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %v4uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_ssbo_v  = OpTypePointer StorageBuffer %v4uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %cond_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
  %cond_val  = OpLoad %uint %cond_ptr
  %is_zero   = OpIEqual %bool %cond_val %uint_0
               OpSelectionMerge %merge None
               OpBranchConditional %is_zero %true_br %false_br
   %true_br  = OpLabel
               OpBranch %merge
  %false_br  = OpLabel
               OpBranch %merge
      %merge = OpLabel
     %result = OpPhi %v4uint %vec_a %true_br %vec_b %false_br
  %out_ptr   = OpAccessChain %ptr_ssbo_v %ssbo_var %uint_1
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM);
}

TEST_F(PositiveGpuAVPoisonValue, NestedStructInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct Inner { uint x; float y; };
        struct Outer { Inner i; uint z; };
        void main() {
            Inner i = Inner(42u, 3.14);
            Outer o = Outer(i, 7u);
            output_val = o.i.x;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, ArrayOfArraysInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint arr[2][3] = uint[2][3](uint[3](1,2,3), uint[3](4,5,6));
            output_val = arr[0][0];
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, MatrixInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            mat4 m = mat4(1.0);
            output_val = uint(m[0][0]);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, DeepNestingInitialized) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct L1 { uint val; };
        struct L2 { L1 inner; };
        struct L3 { L2 mid; };
        void main() {
            L1 l1 = L1(42u);
            L2 l2 = L2(l1);
            L3 l3 = L3(l2);
            output_val = l3.mid.inner.val;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, SpecConstantArrayInitialized) {
    // Can't use aggregate initializer with spec-constant-sized arrays in GLSL,
    // so initialize elements individually
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        layout(constant_id = 0) const int N = 4;
        void main() {
            uint arr[N];
            arr[0] = 1u;
            arr[1] = 2u;
            arr[2] = 3u;
            arr[3] = 4u;
            output_val = arr[0];
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionCall) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint identity(uint x) { return x; }
        void main() {
            uint x = 42u;
            output_val = identity(x);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionCallComposite) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { uint a; float b; };
        uint process(vec4 v, S s) { return uint(v.x) + s.a; }
        void main() {
            vec4 v = vec4(1.0, 2.0, 3.0, 4.0);
            S s = S(10u, 3.14);
            output_val = process(v, s);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CooperativeMatrix) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CooperativeMatrixHelper helper(*this);
    bool found = false;
    for (const auto& prop : helper.coop_matrix_props) {
        if (prop.scope == VK_SCOPE_SUBGROUP_KHR && prop.AType == VK_COMPONENT_TYPE_FLOAT16_KHR && prop.MSize == 16 &&
            prop.KSize == 16) {
            found = true;
            break;
        }
    }
    if (!found) {
        GTEST_SKIP() << "float16 16x16 coopmat property not found";
    }

    const char* cs_source = R"glsl(
        #version 450 core
        #pragma use_vulkan_memory_model
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(local_size_x = 64) in;
        layout(set=0, binding=0) coherent buffer SSBO { float16_t x[]; } buf;
        coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> mat;
        void main() {
           coopMatLoad(mat, buf.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
           coopMatStore(mat, buf.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    void* ptr = buffer.Memory().Map();
    memset(ptr, 0, 1024);

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionParamOut) {
    // Callee writes an initialized value to an out parameter
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void make_clean(out uint result) {
            result = 42u;
        }
        void main() {
            uint val;
            make_clean(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionParamInout) {
    // Callee writes an initialized value via inout parameter
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void add_one(inout uint val) {
            val = val + 1u;
        }
        void main() {
            uint val = 42u;
            add_one(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionParamConstIn) {
    // const in passes by value with an initialized value
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint identity(const in uint x) { return x; }
        void main() {
            uint x = 42u;
            output_val = identity(x);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanNestedFunctionCall) {
    // Clean value through two levels of function calls
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint inner(uint x) { return x + 1u; }
        uint outer(uint x) { return inner(x); }
        void main() {
            uint x = 42u;
            output_val = outer(x);
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanFunctionCallChain) {
    // Result of one function feeds into another, all clean
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint add_one(uint x) { return x + 1u; }
        uint double_it(uint x) { return x * 2u; }
        void main() {
            uint x = 10u;
            output_val = double_it(add_one(x));
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanMultipleOutParams) {
    // Function writes clean values to multiple out parameters
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void init_pair(out uint a, out uint b) {
            a = 1u;
            b = 2u;
        }
        void main() {
            uint a, b;
            init_pair(a, b);
            output_val = a + b;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, OutParamInitializesVariable) {
    // Variable is uninitialized but out param initializes it before use
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void init_val(out uint result) {
            result = 99u;
        }
        void main() {
            uint val;
            init_val(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, CleanNestedOutParam) {
    // Inner function writes clean to out param, outer passes it through
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void make_clean(out uint result) {
            result = 42u;
        }
        void wrapper(out uint result) {
            make_clean(result);
        }
        void main() {
            uint val;
            wrapper(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source);
}

TEST_F(PositiveGpuAVPoisonValue, PtrAccessChainStructArrayInitializedStoreAsm) {
    // Store an initialized value through a StorageBuffer pointer derived via
    // OpPtrAccessChain on a struct. No poison error should fire.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::variablePointersStorageBuffer);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability VariablePointersStorageBuffer
               OpExtension "SPV_KHR_variable_pointers"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo_var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %Elem_S 0 Offset 0
               OpMemberDecorate %Elem_S 1 Offset 4
               OpDecorate %arr_type ArrayStride 8
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
               OpDecorate %ptr_sb_S ArrayStride 8
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %uint_42 = OpConstant %uint 42
     %uint_4 = OpConstant %uint 4
     %Elem_S = OpTypeStruct %uint %uint
   %arr_type = OpTypeArray %Elem_S %uint_4
       %SSBO = OpTypeStruct %arr_type
%ptr_sb_SSBO = OpTypePointer StorageBuffer %SSBO
 %ptr_sb_arr = OpTypePointer StorageBuffer %arr_type
  %ptr_sb_S  = OpTypePointer StorageBuffer %Elem_S
  %ptr_sb_u  = OpTypePointer StorageBuffer %uint
   %ssbo_var = OpVariable %ptr_sb_SSBO StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %arr_ptr = OpAccessChain %ptr_sb_arr %ssbo_var %uint_0
   %elem0_p  = OpAccessChain %ptr_sb_S %arr_ptr %uint_0
   %elem1_p  = OpPtrAccessChain %ptr_sb_S %elem0_p %uint_1
  %member_p  = OpAccessChain %ptr_sb_u %elem1_p %uint_0
               OpStore %member_p %uint_42
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(PositiveGpuAVPoisonValue, BufferDeviceAddress) {
    // BDA shader with initialized pointer — poison pass should not crash.
    // NOTE: poison pointer dereference (e.g. OpConvertUToPtr from an
    // uninitialized uint64) is a known unhandled case; see poison_pass.cpp.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_buffer_reference2 : enable
        layout(buffer_reference, std430, buffer_reference_align = 4) buffer BdaBuffer {
            uint data;
        };
        layout(set=0, binding=0) buffer SSBO {
            BdaBuffer ptr;
        };
        void main() {
            uint val = 42u;
            ptr.data = val;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 256, 0, vkt::device_address);
    VkDeviceAddress bda_addr = bda_buffer.Address();

    vkt::Buffer ssbo(*m_device, sizeof(VkDeviceAddress), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto* ssbo_ptr = static_cast<VkDeviceAddress*>(ssbo.Memory().Map());
    *ssbo_ptr = bda_addr;
    ssbo.Memory().Unmap();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, ssbo, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVPoisonValue, BufferDeviceAddressLocalVar) {
    // BDA shader with a local variable that IS initialized — test that
    // the poison pass correctly instruments around BDA loads/stores without
    // false positives.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_EXT_buffer_reference2 : enable
        layout(buffer_reference, std430, buffer_reference_align = 4) buffer BdaBuffer {
            uint data;
        };
        layout(set=0, binding=0) buffer SSBO {
            BdaBuffer ptr;
        };
        void main() {
            uint local_val = ptr.data;
            local_val += 1u;
            ptr.data = local_val;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer bda_buffer(*m_device, 256, 0, vkt::device_address);
    VkDeviceAddress bda_addr = bda_buffer.Address();

    vkt::Buffer ssbo(*m_device, sizeof(VkDeviceAddress), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto* ssbo_ptr = static_cast<VkDeviceAddress*>(ssbo.Memory().Map());
    *ssbo_ptr = bda_addr;
    ssbo.Memory().Unmap();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, ssbo, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVPoisonValue, EntryBlockStoreInitializesAsm) {
    // OpVariable without initializer, followed by OpStore in the entry block
    // before any load. The entry-block store should be treated as an initializer.
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
   %void_fn  = OpTypeFunction %void
       %uint = OpTypeInt 32 0
   %uint_42  = OpConstant %uint 42
    %uint_0  = OpConstant %uint 0
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %local   = OpVariable %ptr_func_u Function
               OpStore %local %uint_42
      %val   = OpLoad %uint %local
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM);
}

TEST_F(PositiveGpuAVPoisonValue, EntryBlockStoreMultipleVarsAsm) {
    // Two uninitialized variables, both stored to in the entry block before
    // any load. Both should be treated as initialized.
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
   %void_fn  = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_0  = OpConstant %uint 0
    %uint_1  = OpConstant %uint 1
    %uint_2  = OpConstant %uint 2
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
      %var_a = OpVariable %ptr_func_u Function
      %var_b = OpVariable %ptr_func_u Function
               OpStore %var_a %uint_1
               OpStore %var_b %uint_2
    %val_a   = OpLoad %uint %var_a
    %val_b   = OpLoad %uint %var_b
      %sum   = OpIAdd %uint %val_a %val_b
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %sum
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM);
}

TEST_F(PositiveGpuAVPoisonValue, CopyLogicalPreservesCleanMemberAsm) {
    // Construct a struct with one poison member (x) and one clean member (y).
    // OpCopyLogical to a different struct type with the same shape.
    // Extract and store only the clean member — should NOT trigger an error.
    // With lossy (generic) handling, the entire result would be treated as
    // poison, causing a false positive.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_42 = OpConstant %uint 42
    %StructA = OpTypeStruct %uint %uint
    %StructB = OpTypeStruct %uint %uint
       %SSBO = OpTypeStruct %uint
  %ptr_sb_SSBO = OpTypePointer StorageBuffer %SSBO
   %ptr_sb_u = OpTypePointer StorageBuffer %uint
 %ptr_func_u = OpTypePointer Function %uint
       %ssbo = OpVariable %ptr_sb_SSBO StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %poison_var = OpVariable %ptr_func_u Function
   %poison_x = OpLoad %uint %poison_var
        %src = OpCompositeConstruct %StructA %poison_x %uint_42
        %dst = OpCopyLogical %StructB %src
    %clean_y = OpCompositeExtract %uint %dst 1
    %out_ptr = OpAccessChain %ptr_sb_u %ssbo %uint_0
               OpStore %out_ptr %clean_y
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(PositiveGpuAVPoisonValue, CompositeConstructReplicateInitializedAsm) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_REPLICATED_COMPOSITES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderReplicatedComposites);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability ReplicatedCompositesEXT
               OpExtension "SPV_EXT_replicated_composites"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %ssbo_type Block
               OpMemberDecorate %ssbo_type 0 Offset 0
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
     %uint_0 = OpConstant %uint 0
    %uint_42 = OpConstant %uint 42
  %ssbo_type = OpTypeStruct %v4uint
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
%ptr_sb_v4uint = OpTypePointer StorageBuffer %v4uint
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
       %main = OpFunction %void None %func
      %entry = OpLabel
     %result = OpCompositeConstructReplicateEXT %v4uint %uint_42
    %out_ptr = OpAccessChain %ptr_sb_v4uint %ssbo %uint_0
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(PositiveGpuAVPoisonValue, VariablePointersStorageBuffer) {
    // Shader using VariablePointersStorageBuffer capability (OpSelect on
    // SSBO pointers). Verifies the poison pass doesn't crash on pointer-
    // typed values flowing through OpSelect.
    // NOTE: poison pointer dereference via variable pointers is a known
    // unhandled case; see poison_pass.cpp.
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::variablePointersStorageBuffer);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability VariablePointersStorageBuffer
               OpExtension "SPV_KHR_variable_pointers"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %ssbo_type Block
               OpMemberDecorate %ssbo_type 0 Offset 0
               OpMemberDecorate %ssbo_type 1 Offset 4
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %bool = OpTypeBool
  %ssbo_type = OpTypeStruct %uint %uint
%ptr_ssbo_struct = OpTypePointer StorageBuffer %ssbo_type
 %ptr_ssbo_uint = OpTypePointer StorageBuffer %uint
       %ssbo = OpVariable %ptr_ssbo_struct StorageBuffer
      %idx_0 = OpConstant %uint 0
      %idx_1 = OpConstant %uint 1
    %val_42u = OpConstant %uint 42
      %true  = OpConstantTrue %bool
       %main = OpFunction %void None %func
      %entry = OpLabel
     %ptr_a  = OpAccessChain %ptr_ssbo_uint %ssbo %idx_0
     %ptr_b  = OpAccessChain %ptr_ssbo_uint %ssbo %idx_1
     %chosen = OpSelect %ptr_ssbo_uint %true %ptr_a %ptr_b
               OpStore %chosen %val_42u
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, SPV_SOURCE_ASM);
}
