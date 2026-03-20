/* Copyright (c) 2026 The Khronos Group Inc.
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

class NegativeGpuAVPoisonValue : public GpuAVTest {
  protected:
    void InitPoisonValue();
    void SimpleComputeTest(const char* shader, const char* expected_error, uint32_t error_count = 1,
                           SpvSourceType source_type = SPV_SOURCE_GLSL, spv_target_env spv_env = SPV_ENV_VULKAN_1_1);
};

void NegativeGpuAVPoisonValue::InitPoisonValue() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
}

void NegativeGpuAVPoisonValue::SimpleComputeTest(const char* shader, const char* expected_error, uint32_t error_count,
                                                 SpvSourceType source_type, spv_target_env spv_env) {
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

    m_errorMonitor->SetDesiredError(expected_error, error_count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVPoisonValue, BranchOnPoison) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            bool x;
            if (x) {
                output_val = 1;
            } else {
                output_val = 0;
            }
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-BranchOnPoison");
}

TEST_F(NegativeGpuAVPoisonValue, SelectPoisonSideSelected) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint cond;
            uint output_val;
        };
        void main() {
            uint x;
            uint y = 42;
            uint result = (cond == 0) ? x : y;
            output_val = result;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateArith) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            uint y = x + 1;
            output_val = y;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonAccessChainIndex) {
    // Hand-written SPIR-V: OpAccessChain with a poison index.
    // The resulting pointer is never dereferenced, so only the access chain
    // error fires (each thread can only report one error).
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %arr_type ArrayStride 4
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_42 = OpConstant %uint 42
    %uint_16 = OpConstant %uint 16
   %arr_type = OpTypeArray %uint %uint_16
       %SSBO = OpTypeStruct %arr_type
   %ptr_ssbo = OpTypePointer StorageBuffer %SSBO
  %ptr_ssbo_u = OpTypePointer StorageBuffer %uint
  %ptr_func_u = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %idx_var = OpVariable %ptr_func_u Function
        %idx = OpLoad %uint %idx_var
        %ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0 %idx
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-AddressFromPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonExternalStore) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            output_val = x;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonVectorSwizzle) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            vec4 v;
            float f = v.x;
            output_val = uint(f);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonMultipleOperands) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint a;
            uint b;
            output_val = a + b;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PrivateScopeUninitialized) {
    // Global variable in GLSL becomes Private storage class in SPIR-V
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint private_var;
        void main() {
            output_val = private_var;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, ContaminatedVariable) {
    // Poison flows through a store into an initialized variable
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            uint y = 0u;
            y = x;
            output_val = y;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

// --- A1: Composite types ---

TEST_F(NegativeGpuAVPoisonValue, PoisonVectorStore) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 output_val;
        };
        void main() {
            vec4 v;
            output_val = v;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructMember) {
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
            S s;
            output_val = s.a;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonMatrixElement) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            float output_val;
        };
        void main() {
            mat4 m;
            output_val = m[0][0];
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonArrayElement) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint arr[4];
            output_val = arr[0];
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

// --- A2: Pass-through instructions ---

TEST_F(NegativeGpuAVPoisonValue, PoisonCompositeConstruct) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 output_val;
        };
        void main() {
            float a;
            float b;
            float c;
            float d;
            output_val = vec4(a, b, c, d);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonCompositeInsert) {
    // Inserting a poison scalar into an initialized vector
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 output_val;
        };
        void main() {
            float poison_val;
            vec4 v = vec4(1.0, 2.0, 3.0, 4.0);
            v.x = poison_val;
            output_val = v;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonVectorShuffleMixed) {
    // Shuffle components from one uninitialized and one initialized vector
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 output_val;
        };
        void main() {
            vec4 poison_vec;
            vec4 clean_vec = vec4(1.0, 2.0, 3.0, 4.0);
            output_val = vec4(poison_vec.xy, clean_vec.zw);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPhi) {
    // Poison flows through one branch of if/else, merges via phi
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint cond;
            uint output_val;
        };
        void main() {
            uint x;
            uint result;
            if (cond == 0) {
                result = x;
            } else {
                result = 42u;
            }
            output_val = result;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPhiScalarAsm) {
    // Hand-written SPIR-V with an actual OpPhi.
    // One branch loads from an uninitialized Function variable (poison),
    // the other uses a constant (clean). The OpPhi merges them, and the
    // result is stored to the SSBO -> should trigger an error when the
    // poison path is taken (cond==0).
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
   %uint_42  = OpConstant %uint 42
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %uninit_var = OpVariable %ptr_func_u Function
  %cond_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
  %cond_val  = OpLoad %uint %cond_ptr
  %is_zero   = OpIEqual %bool %cond_val %uint_0
               OpSelectionMerge %merge None
               OpBranchConditional %is_zero %true_br %false_br
   %true_br  = OpLabel
  %poison_val = OpLoad %uint %uninit_var
               OpBranch %merge
  %false_br  = OpLabel
               OpBranch %merge
      %merge = OpLabel
     %result = OpPhi %uint %poison_val %true_br %uint_42 %false_br
  %out_ptr   = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPhiVectorAsm) {
    // OpPhi with vec4 type. One branch carries a poison vector (loaded from
    // an uninitialized Function variable), the other a clean constant.
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
     %v4uint = OpTypeVector %uint 4
  %clean_vec = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %v4uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_ssbo_v  = OpTypePointer StorageBuffer %v4uint
%ptr_func_v  = OpTypePointer Function %v4uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %uninit_var = OpVariable %ptr_func_v Function
  %cond_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
  %cond_val  = OpLoad %uint %cond_ptr
  %is_zero   = OpIEqual %bool %cond_val %uint_0
               OpSelectionMerge %merge None
               OpBranchConditional %is_zero %true_br %false_br
   %true_br  = OpLabel
  %poison_vec = OpLoad %v4uint %uninit_var
               OpBranch %merge
  %false_br  = OpLabel
               OpBranch %merge
      %merge = OpLabel
     %result = OpPhi %v4uint %poison_vec %true_br %clean_vec %false_br
  %out_ptr   = OpAccessChain %ptr_ssbo_v %ssbo_var %uint_1
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPhiBothPoisonAsm) {
    // OpPhi where BOTH incoming values are poison (from two different
    // uninitialized Function variables). Should trigger regardless of
    // which branch is taken.
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
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
 %uninit_a   = OpVariable %ptr_func_u Function
 %uninit_b   = OpVariable %ptr_func_u Function
  %cond_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
  %cond_val  = OpLoad %uint %cond_ptr
  %is_zero   = OpIEqual %bool %cond_val %uint_0
               OpSelectionMerge %merge None
               OpBranchConditional %is_zero %true_br %false_br
   %true_br  = OpLabel
  %val_a     = OpLoad %uint %uninit_a
               OpBranch %merge
  %false_br  = OpLabel
  %val_b     = OpLoad %uint %uninit_b
               OpBranch %merge
      %merge = OpLabel
     %result = OpPhi %uint %val_a %true_br %val_b %false_br
  %out_ptr   = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_1
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

// --- A3: Propagation instructions ---

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateFloat) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            float f;
            float g = f * 2.0;
            output_val = uint(g);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateBitwise) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            uint y = x & 0xFFu;
            output_val = y;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateComparison) {
    // Poison propagates through comparison, then used as branch condition
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            if (x > 5u) {
                output_val = 1;
            } else {
                output_val = 0;
            }
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-BranchOnPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateConversion) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            int x;
            float y = float(x);
            output_val = uint(y);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPropagateDot) {
    // Dot product reduces vec4 to float, exercises dimensionality reduction
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            vec4 v;
            float d = dot(v, vec4(1.0));
            output_val = uint(d);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

// --- A4: UB trigger instructions ---

TEST_F(NegativeGpuAVPoisonValue, PoisonSwitchSelector) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            switch (x) {
                case 0:  output_val = 0; break;
                case 1:  output_val = 1; break;
                default: output_val = 2; break;
            }
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-BranchOnPoison");
}

// --- A5: Chaining and complex flow ---

TEST_F(NegativeGpuAVPoisonValue, PoisonMultiStepChain) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            uint y = x * 2u;
            uint z = y + 1u;
            uint w = z & 0xFFu;
            output_val = w;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonThroughLoop) {
    // Poison initial value accumulated through loop iterations
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint sum;
            for (int i = 0; i < 4; i++) {
                sum += uint(i);
            }
            output_val = sum;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonPartialVectorPoison) {
    // Per-component: 3 clean + 1 poison, extract the poison component
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            float d;
            vec4 v = vec4(1.0, 2.0, 3.0, d);
            output_val = uint(v.w);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonNestedStruct) {
    // Struct containing an array, all uninitialized
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct Inner {
            uint data[4];
        };
        void main() {
            Inner s;
            output_val = s.data[0];
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonContaminatedChain) {
    // Contamination + further arithmetic propagation
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint x;
            uint y = 0u;
            y = x;
            uint z = y + 1u;
            output_val = z;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructOfStruct) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct Inner { uint x; float y; };
        struct Outer { Inner i; uint z; };
        void main() {
            Outer o;
            output_val = o.i.x;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonArrayOfStructs) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { uint x; float y; };
        void main() {
            S arr[3];
            output_val = arr[1].x;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructWithVector) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { vec4 v; uint x; };
        void main() {
            S s;
            output_val = uint(s.v.x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructWithMatrix) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { mat4 m; };
        void main() {
            S s;
            output_val = uint(s.m[0][0]);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonArrayOfArrays) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            uint arr[2][3];
            output_val = arr[0][0];
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonDeepNesting) {
    // 3 levels: struct of struct of struct
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct L1 { uint val; };
        struct L2 { L1 inner; };
        struct L3 { L2 mid; };
        void main() {
            L3 s;
            output_val = s.mid.inner.val;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonLargeStruct) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct Big {
            uint a; uint b; uint c; uint d;
            uint e; uint f; uint g; uint h;
        };
        void main() {
            Big s;
            output_val = s.h;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonDoubleVector) {
    AddRequiredFeature(vkt::Feature::shaderFloat64);
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            dvec4 v;
            output_val = uint(v.x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonSpecConstantArray) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        layout(constant_id = 0) const int N = 4;
        void main() {
            uint arr[N];
            output_val = arr[0];
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructArrayOfVectors) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { vec3 positions[2]; };
        void main() {
            S s;
            output_val = uint(s.positions[0].x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonThroughFunctionCall) {
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint identity(uint x) { return x; }
        void main() {
            uint x;
            output_val = identity(x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonVectorThroughFunctionCall) {
    // Only one component is poison; the shadow reduction should still taint the result
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        float sum_vec(vec4 v) { return v.x + v.y + v.z + v.w; }
        void main() {
            float d;
            vec4 v = vec4(1.0, 2.0, 3.0, d);
            output_val = uint(sum_vec(v));
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonStructThroughFunctionCall) {
    // Only one member is poison; shadow reduction should detect it
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        struct S { uint a; float b; };
        uint get_a(S s) { return s.a; }
        void main() {
            float uninit_f;
            S s = S(42u, uninit_f);
            output_val = get_a(s);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonMixedArgsFunctionCall) {
    // Two args, only one is poison
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint add(uint a, uint b) { return a + b; }
        void main() {
            uint x;
            uint y = 42u;
            output_val = add(x, y);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonReturnFromFunction) {
    // Function has its own uninit local and returns it
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint make_poison() {
            uint x;
            return x;
        }
        void main() {
            output_val = make_poison();
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ReturnOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonFunctionParamOut) {
    // Callee has uninit local and stores it to an out parameter
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void make_poison(out uint result) {
            uint x;
            result = x;
        }
        void main() {
            uint val;
            make_poison(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-StoreToFunctionParam");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonFunctionParamInout) {
    // Callee has uninit local and stores it to an inout parameter
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void replace_with_poison(inout uint val) {
            uint x;
            val = x;
        }
        void main() {
            uint val = 42u;
            replace_with_poison(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-StoreToFunctionParam");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonFunctionParamConstIn) {
    // const in passes by value; callee returns it, caller stores externally
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint identity(const in uint x) { return x; }
        void main() {
            uint x;
            output_val = identity(x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonNestedFunctionCall) {
    // Poison flows through two levels of function calls
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint inner(uint x) { return x + 1u; }
        uint outer(uint x) { return inner(x); }
        void main() {
            uint x;
            output_val = outer(x);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonFunctionCallChain) {
    // Result of one function call feeds into another
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint add_one(uint x) { return x + 1u; }
        uint double_it(uint x) { return x * 2u; }
        void main() {
            uint x;
            output_val = double_it(add_one(x));
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonMultipleOutParams) {
    // Function writes poison to multiple out parameters
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void make_two_poisons(out uint a, out uint b) {
            uint x;
            a = x;
            b = x;
        }
        void main() {
            uint a, b;
            make_two_poisons(a, b);
            output_val = 0u;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-StoreToFunctionParam");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonOutParamPartialInit) {
    // Function writes poison to one out param and clean to another;
    // the poison one should be detected
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void partial_init(out uint good, out uint bad) {
            uint x;
            good = 42u;
            bad = x;
        }
        void main() {
            uint a, b;
            partial_init(a, b);
            output_val = 0u;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-StoreToFunctionParam");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonNestedOutParam) {
    // Inner function writes poison to out param, outer passes it through
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void make_poison(out uint result) {
            uint x;
            result = x;
        }
        void wrapper(out uint result) {
            make_poison(result);
        }
        void main() {
            uint val;
            wrapper(val);
            output_val = val;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-StoreToFunctionParam");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonReturnThenStore) {
    // Function returns poison, caller stores the result externally
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        uint get_poison() {
            uint x;
            return x;
        }
        uint relay() {
            return get_poison();
        }
        void main() {
            output_val = relay();
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ReturnOfPoison");
}

// --- BDA and variable pointer poison ---

TEST_F(NegativeGpuAVPoisonValue, PoisonBdaPointerStore) {
    // Uninitialized uint64 converted to BDA pointer, then stored through.
    // The pointer value is poison; dereferencing it is UB.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability Int64
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %ssbo_type Block
               OpMemberDecorate %ssbo_type 0 Offset 0
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint64 = OpTypeInt 64 0
  %ssbo_type = OpTypeStruct %uint
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
 %ptr_sb_uint = OpTypePointer StorageBuffer %uint
%ptr_psb_uint = OpTypePointer PhysicalStorageBuffer %uint
%ptr_func_u64 = OpTypePointer Function %uint64
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
      %idx_0 = OpConstant %uint 0
    %val_42u = OpConstant %uint 42
       %main = OpFunction %void None %func
      %entry = OpLabel
   %addr_var = OpVariable %ptr_func_u64 Function
       %addr = OpLoad %uint64 %addr_var
        %ptr = OpConvertUToPtr %ptr_psb_uint %addr
               OpStore %ptr %val_42u Aligned 4
      %out_p = OpAccessChain %ptr_sb_uint %ssbo %idx_0
               OpStore %out_p %val_42u
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-PoisonPointerDereference", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonBdaPointerLoad) {
    // Uninitialized uint64 converted to BDA pointer, then loaded through.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability Int64
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %ssbo_type Block
               OpMemberDecorate %ssbo_type 0 Offset 0
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint64 = OpTypeInt 64 0
  %ssbo_type = OpTypeStruct %uint
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
 %ptr_sb_uint = OpTypePointer StorageBuffer %uint
%ptr_psb_uint = OpTypePointer PhysicalStorageBuffer %uint
%ptr_func_u64 = OpTypePointer Function %uint64
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
      %idx_0 = OpConstant %uint 0
       %main = OpFunction %void None %func
      %entry = OpLabel
   %addr_var = OpVariable %ptr_func_u64 Function
       %addr = OpLoad %uint64 %addr_var
        %ptr = OpConvertUToPtr %ptr_psb_uint %addr
        %val = OpLoad %uint %ptr Aligned 4
      %out_p = OpAccessChain %ptr_sb_uint %ssbo %idx_0
               OpStore %out_p %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-PoisonPointerDereference", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, ChainedAccessChainLoadAsm) {
    // Chained access chains (OpAccessChain of OpAccessChain) to an
    // uninitialized variable. The poison pass should still detect the load
    // as coming from a tracked variable despite the indirection.
    const char* cs_chained = R"asm(
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
    %uint_3  = OpConstant %uint 3
   %arr_type = OpTypeArray %uint %uint_3
 %arr2_type  = OpTypeArray %arr_type %uint_3
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_a2 = OpTypePointer Function %arr2_type
%ptr_func_a  = OpTypePointer Function %arr_type
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %arr_var = OpVariable %ptr_func_a2 Function
  %inner_ptr = OpAccessChain %ptr_func_a %arr_var %uint_0
  %outer_ptr = OpAccessChain %ptr_func_u %inner_ptr %uint_0
      %val   = OpLoad %uint %outer_ptr
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_chained, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, CopyObjectPointerLoadAsm) {
    // OpCopyObject on a pointer to an uninitialized variable.
    // TraceToVariable must walk through OpCopyObject to find the base variable.
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
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
      %local = OpVariable %ptr_func_u Function
       %copy = OpCopyObject %ptr_func_u %local
        %val = OpLoad %uint %copy
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, CopyObjectBetweenAccessChainsAsm) {
    // OpCopyObject between two levels of access chain into an uninitialized array.
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
    %uint_3  = OpConstant %uint 3
   %arr_type = OpTypeArray %uint %uint_3
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_a  = OpTypePointer Function %arr_type
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %arr_var = OpVariable %ptr_func_a Function
    %ac_arr  = OpAccessChain %ptr_func_u %arr_var %uint_0
       %copy = OpCopyObject %ptr_func_u %ac_arr
        %val = OpLoad %uint %copy
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PtrAccessChainPoisonIndexAsm) {
    // OpPtrAccessChain with an uninitialized Element index. The poison pass
    // should detect the poison index and report AddressFromPoison.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::variablePointersStorageBuffer);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability VariablePointersStorageBuffer
               OpExtension "SPV_KHR_variable_pointers"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %ssbo_type Block
               OpMemberDecorate %ssbo_type 0 Offset 0
               OpDecorate %arr_type ArrayStride 4
               OpDecorate %ssbo DescriptorSet 0
               OpDecorate %ssbo Binding 0
               OpDecorate %ptr_sb_uint ArrayStride 4
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_42 = OpConstant %uint 42
     %uint_4 = OpConstant %uint 4
   %arr_type = OpTypeArray %uint %uint_4
  %ssbo_type = OpTypeStruct %arr_type
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
 %ptr_sb_arr = OpTypePointer StorageBuffer %arr_type
 %ptr_sb_uint = OpTypePointer StorageBuffer %uint
%ptr_func_uint = OpTypePointer Function %uint
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
       %main = OpFunction %void None %func
      %entry = OpLabel
  %local_idx = OpVariable %ptr_func_uint Function
 %poison_idx = OpLoad %uint %local_idx
    %arr_ptr = OpAccessChain %ptr_sb_arr %ssbo %uint_0
   %first_p  = OpAccessChain %ptr_sb_uint %arr_ptr %uint_0
   %elem_ptr = OpPtrAccessChain %ptr_sb_uint %first_p %poison_idx
      %out_p = OpAccessChain %ptr_sb_uint %arr_ptr %uint_0
               OpStore %out_p %uint_42
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-AddressFromPoison", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, PtrAccessChainStructArrayPoisonStoreAsm) {
    // Store a poison value through a StorageBuffer pointer derived via
    // OpPtrAccessChain on a struct array. The Element index offsets to a
    // struct element, then OpAccessChain descends into a struct member.
    // The poison value (from an uninitialized local) stored through this
    // pointer chain should trigger ExternalStoreOfPoison.
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
     %uint_4 = OpConstant %uint 4
     %Elem_S = OpTypeStruct %uint %uint
   %arr_type = OpTypeArray %Elem_S %uint_4
       %SSBO = OpTypeStruct %arr_type
 %ptr_func_u = OpTypePointer Function %uint
%ptr_sb_SSBO = OpTypePointer StorageBuffer %SSBO
 %ptr_sb_arr = OpTypePointer StorageBuffer %arr_type
  %ptr_sb_S  = OpTypePointer StorageBuffer %Elem_S
  %ptr_sb_u  = OpTypePointer StorageBuffer %uint
   %ssbo_var = OpVariable %ptr_sb_SSBO StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
  %local_var = OpVariable %ptr_func_u Function
 %poison_val = OpLoad %uint %local_var
    %arr_ptr = OpAccessChain %ptr_sb_arr %ssbo_var %uint_0
   %elem0_p  = OpAccessChain %ptr_sb_S %arr_ptr %uint_0
   %elem1_p  = OpPtrAccessChain %ptr_sb_S %elem0_p %uint_1
  %member_p  = OpAccessChain %ptr_sb_u %elem1_p %uint_0
               OpStore %member_p %poison_val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, PartialInitStillPoisonAsm) {
    // Only element 0 of an array is written via access chain; element 1
    // remains uninitialized. Loading element 1 should detect poison.
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
   %uint_42  = OpConstant %uint 42
     %uint_4 = OpConstant %uint 4
   %arr_type = OpTypeArray %uint %uint_4
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_arr = OpTypePointer Function %arr_type
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %arr_var = OpVariable %ptr_func_arr Function
  %elem_ptr0 = OpAccessChain %ptr_func_u %arr_var %uint_0
               OpStore %elem_ptr0 %uint_42
  %elem_ptr1 = OpAccessChain %ptr_func_u %arr_var %uint_1
      %val   = OpLoad %uint %elem_ptr1
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %val
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, LoadBeforeStoreNotInitAsm) {
    // OpLoad from the variable before OpStore in the entry block.
    // The load-before-store means the variable is NOT treated as initialized.
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
   %uint_42  = OpConstant %uint 42
       %SSBO = OpTypeStruct %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_u  = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
    %local   = OpVariable %ptr_func_u Function
   %poison   = OpLoad %uint %local
               OpStore %local %uint_42
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %poison
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, CrossBlockPropagationAsm) {
    // Poison value loaded in the entry block, then used in a different block
    // (via direct SSA reference, not through memory). Verifies that the static
    // analysis and runtime shadow tracking work across basic block boundaries.
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
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
       %SSBO = OpTypeStruct %uint
   %ptr_ssbo = OpTypePointer StorageBuffer %SSBO
 %ptr_ssbo_u = OpTypePointer StorageBuffer %uint
 %ptr_func_u = OpTypePointer Function %uint
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
      %local = OpVariable %ptr_func_u Function
     %poison = OpLoad %uint %local
               OpBranch %next
       %next = OpLabel
    %out_ptr = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %poison
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonLongVector) {
    AddRequiredExtensions(VK_EXT_SHADER_LONG_VECTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::longVector);
    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_long_vector : enable
        layout(set=0, binding=0) buffer SSBO {
            vector<uint, 8> output_val;
        };
        void main() {
            vector<uint, 8> v;
            output_val = v;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonVariablePointerSelect) {
    // OpSelect on SSBO pointers with a poison condition (uninitialized bool).
    // The resulting pointer is poison; dereferencing it is UB.
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
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
 %ptr_sb_uint = OpTypePointer StorageBuffer %uint
%ptr_func_bool = OpTypePointer Function %bool
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
      %idx_0 = OpConstant %uint 0
      %idx_1 = OpConstant %uint 1
    %val_42u = OpConstant %uint 42
       %main = OpFunction %void None %func
      %entry = OpLabel
   %cond_var = OpVariable %ptr_func_bool Function
       %cond = OpLoad %bool %cond_var
      %ptr_a = OpAccessChain %ptr_sb_uint %ssbo %idx_0
      %ptr_b = OpAccessChain %ptr_sb_uint %ssbo %idx_1
     %chosen = OpSelect %ptr_sb_uint %cond %ptr_a %ptr_b
               OpStore %chosen %val_42u
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-PoisonPointerDereference", 1, SPV_SOURCE_ASM);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonSelectConditionScalar) {
    // OpSelect where the condition is poison (uninitialized bool), scalar result.
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            uint output_val;
        };
        void main() {
            bool cond;
            uint result = cond ? 1u : 2u;
            output_val = result;
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonSelectConditionVector) {
    // OpSelect where the condition is poison (uninitialized bvec4), vector result.
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO {
            vec4 output_val;
        };
        void main() {
            bvec4 cond;
            vec4 a = vec4(1.0);
            vec4 b = vec4(2.0);
            output_val = mix(a, b, cond);
        }
    )glsl";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison");
}

TEST_F(NegativeGpuAVPoisonValue, PoisonCopyLogicalExtractPoisonMemberAsm) {
    // Construct a struct with one poison member (x) and one clean member (y).
    // OpCopyLogical to a different struct type, extract the POISON member,
    // and store it externally.
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
  %poison_m0 = OpCompositeExtract %uint %dst 0
    %out_ptr = OpAccessChain %ptr_sb_u %ssbo %uint_0
               OpStore %out_ptr %poison_m0
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonCompositeConstructReplicateAsm) {
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
  %ssbo_type = OpTypeStruct %v4uint
%ptr_sb_struct = OpTypePointer StorageBuffer %ssbo_type
%ptr_sb_v4uint = OpTypePointer StorageBuffer %v4uint
%ptr_func_uint = OpTypePointer Function %uint
       %ssbo = OpVariable %ptr_sb_struct StorageBuffer
       %main = OpFunction %void None %func
      %entry = OpLabel
    %uninit_p = OpVariable %ptr_func_uint Function
   %poison_u = OpLoad %uint %uninit_p
     %result = OpCompositeConstructReplicateEXT %v4uint %poison_u
    %out_ptr = OpAccessChain %ptr_sb_v4uint %ssbo %uint_0
               OpStore %out_ptr %result
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}

TEST_F(NegativeGpuAVPoisonValue, PoisonSelectConditionStructAsm) {
    // OpSelect (SPIR-V 1.4+) where the condition is poison, struct result.
    SetTargetApiVersion(VK_API_VERSION_1_2);
    const char* cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %ssbo_var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpDecorate %ssbo_var DescriptorSet 0
               OpDecorate %ssbo_var Binding 0
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %bool = OpTypeBool
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
   %my_struct = OpTypeStruct %uint %uint
       %SSBO = OpTypeStruct %uint %uint
%ptr_ssbo    = OpTypePointer StorageBuffer %SSBO
%ptr_ssbo_u  = OpTypePointer StorageBuffer %uint
%ptr_func_bool = OpTypePointer Function %bool
   %ssbo_var = OpVariable %ptr_ssbo StorageBuffer
       %main = OpFunction %void None %void_fn
      %entry = OpLabel
   %cond_var = OpVariable %ptr_func_bool Function
       %cond = OpLoad %bool %cond_var
     %val_a  = OpCompositeConstruct %my_struct %uint_10 %uint_10
     %val_b  = OpCompositeConstruct %my_struct %uint_20 %uint_20
   %selected = OpSelect %my_struct %cond %val_a %val_b
   %member_0 = OpCompositeExtract %uint %selected 0
   %out_ptr  = OpAccessChain %ptr_ssbo_u %ssbo_var %uint_0
               OpStore %out_ptr %member_0
               OpReturn
               OpFunctionEnd
    )asm";
    SimpleComputeTest(cs_source, "SPIRV-PoisonValue-ExternalStoreOfPoison", 1, SPV_SOURCE_ASM, SPV_ENV_VULKAN_1_2);
}
