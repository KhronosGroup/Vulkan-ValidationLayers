/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "cooperative_matrix_helper.h"
#include "shader_helper.h"

class NegativeShaderCooperativeMatrix : public CooperativeMatrixTest {};

TEST_F(NegativeShaderCooperativeMatrix, SpecInfo) {
    TEST_DESCRIPTION("Test VK_KHR_cooperative_matrix.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    // https://godbolt.org/z/Ys7faYaav but now validated at GLSL level, so have SPIR-V
    const char *cs_source = R"asm(
            OpCapability Shader
            OpCapability Float16
            OpCapability VulkanMemoryModel
            OpCapability CooperativeMatrixKHR
            OpExtension "SPV_KHR_cooperative_matrix"
            OpMemoryModel Logical Vulkan
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main LocalSize 32 1 1
            OpDecorate %C0 SpecId 0
            OpDecorate %C1 SpecId 1
    %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %half = OpTypeFloat 16
    %uint = OpTypeInt 32 0
    %uint_3 = OpConstant %uint 3
    %uint_5 = OpConstant %uint 5
    %uint_2 = OpConstant %uint 2
        %11 = OpTypeCooperativeMatrixKHR %half %uint_3 %uint_3 %uint_5 %uint_2
%_ptr_Function_11 = OpTypePointer Function %11
%half_0x0p_0 = OpConstant %half 0x0p+0
        %15 = OpConstantComposite %11 %half_0x0p_0
        %C0 = OpSpecConstant %uint 1
        %C1 = OpSpecConstant %uint 1
    %uint_0 = OpConstant %uint 0
        %19 = OpTypeCooperativeMatrixKHR %half %uint_3 %C0 %C1 %uint_0
%_ptr_Function_19 = OpTypePointer Function %19
    %uint_1 = OpConstant %uint 1
        %24 = OpTypeCooperativeMatrixKHR %half %uint_3 %C0 %C1 %uint_1
%_ptr_Function_24 = OpTypePointer Function %24
        %28 = OpTypeCooperativeMatrixKHR %half %uint_3 %C0 %C1 %uint_2
%_ptr_Function_28 = OpTypePointer Function %28
    %v3uint = OpTypeVector %uint 3
%uint_32 = OpConstant %uint 32
        %35 = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
    %main = OpFunction %void None %3
        %5 = OpLabel
%badSize = OpVariable %_ptr_Function_11 Function
        %A = OpVariable %_ptr_Function_19 Function
        %B = OpVariable %_ptr_Function_24 Function
        %C = OpVariable %_ptr_Function_28 Function
            OpStore %badSize %15
        %22 = OpLoad %19 %A
        %27 = OpLoad %24 %B
        %31 = OpLoad %28 %C
        %32 = OpCooperativeMatrixMulAddKHR %28 %22 %27 %31
            OpReturn
            OpFunctionEnd
    )asm";

    const uint32_t spec_data[] = {
        63,
        65,
    };
    VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };

    VkSpecializationInfo spec_info = {
        2,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM, &spec_info);
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, SpecInfoNV) {
    TEST_DESCRIPTION("Test VK_NV_cooperative_matrix.");
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    // The NV and KHR share the same feature name, so set it without AddRequiredFeature
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    // https://godbolt.org/z/Kbx1PsraY but now validated at GLSL level, so have SPIR-V
    const char *cs_source = R"asm(
               OpCapability Shader
               OpCapability Float16
               OpCapability VulkanMemoryModel
               OpCapability CooperativeMatrixNV
               OpExtension "SPV_NV_cooperative_matrix"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 32 1 1
               OpDecorate %C0 SpecId 0
               OpDecorate %C1 SpecId 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %half = OpTypeFloat 16
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
     %uint_5 = OpConstant %uint 5
         %10 = OpTypeCooperativeMatrixNV %half %uint_3 %uint_3 %uint_5
%_ptr_Function_10 = OpTypePointer Function %10
%half_0x0p_0 = OpConstant %half 0x0p+0
         %14 = OpConstantComposite %10 %half_0x0p_0
         %C0 = OpSpecConstant %uint 1
         %C1 = OpSpecConstant %uint 1
         %17 = OpTypeCooperativeMatrixNV %half %uint_3 %C0 %C1
%_ptr_Function_17 = OpTypePointer Function %17
     %v3uint = OpTypeVector %uint 3
    %uint_32 = OpConstant %uint 32
     %uint_1 = OpConstant %uint 1
         %29 = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
    %badSize = OpVariable %_ptr_Function_10 Function
          %A = OpVariable %_ptr_Function_17 Function
          %B = OpVariable %_ptr_Function_17 Function
          %C = OpVariable %_ptr_Function_17 Function
               OpStore %badSize %14
         %20 = OpLoad %17 %A
         %22 = OpLoad %17 %B
         %24 = OpLoad %17 %C
         %25 = OpCooperativeMatrixMulAddNV %17 %20 %22 %24
               OpReturn
               OpFunctionEnd
    )asm";

    const uint32_t spec_data[] = {
        16,
        8,
    };
    VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };

    VkSpecializationInfo spec_info = {
        2,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM, &spec_info);
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, UnsupportedStageUint32) {
    TEST_DESCRIPTION("Test error using cooperative matrix in unsupported stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);
    InitRenderTarget();

    if (!helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 16, 16, 16, VK_COMPONENT_TYPE_UINT32_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    if (helper.SupportsStage(VK_SHADER_STAGE_VERTEX_BIT)) {
        GTEST_SKIP() << "Cannot execute test due to vertex stage expected to be unsupported";
    }

    const char *vs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        void main() {
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> A;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> B;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> C;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
       }
    )glsl";

    CreatePipelineHelper pipe(*this);
    pipe.vs_ = std::make_unique<VkShaderObj>(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixSupportedStages-08985");
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, UnsupportedStageFloat16) {
    TEST_DESCRIPTION("Test error using cooperative matrix in unsupported stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);
    InitRenderTarget();

    if (!helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 8, 8, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    if (helper.SupportsStage(VK_SHADER_STAGE_VERTEX_BIT)) {
        GTEST_SKIP() << "Cannot execute test due to vertex stage expected to be unsupported";
    }

    const char *vs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, 8, 16, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
       }
    )glsl";

    CreatePipelineHelper pipe(*this);
    pipe.vs_ = std::make_unique<VkShaderObj>(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixSupportedStages-08985");
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, ParametersMatchProperties) {
    TEST_DESCRIPTION("Test that parameters match one of the matrices in any of the supported VkCooperativeMatrixPropertiesKHR");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.subgroupSize > 32) {
        GTEST_SKIP() << "local_size_x (32) is not a multiple of subgroupSize";
    }

    // Tests are assume that Float16 3*5 is not available
    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator> badSize = coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator>(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, DimXMultipleSubgroupSize) {
    TEST_DESCRIPTION("Local workgroup size in the X dimension of the pipeline multiple of subgroupSize");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 16, 16, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x_id = 0, local_size_y = 1, local_size_z = 1) in;
        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    const uint32_t spec_data[] = {
        31,
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
    };
    const VkSpecializationInfo spec_info = {
        1,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info);

    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-module-08987", 3);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, DimXMultipleSubgroupSizeWorkgroupScope) {
    TEST_DESCRIPTION(
        "Local workgroup size in the X dimension of the pipeline multiple of subgroupSize and less than or equal to "
        "cooperativeMatrixWorkgroupScopeMaxWorkgroupSize");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_WORKGROUP_KHR, 32, 32, 32, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x_id = 0, local_size_y = 1, local_size_z = 1) in;
        void main() {
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV props2 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props2);

    const uint32_t spec_data[] = {
        props2.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize + 1,
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
    };
    const VkSpecializationInfo spec_info = {
        1,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10165", 3);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10166");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-module-10169", 6);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, SameScope) {
    TEST_DESCRIPTION("In OpCooperativeMatrixMulAddKHR all matrices should have same scope");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 16, 16, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(constant_id = 0) const uint scope0 = gl_ScopeSubgroup;
        layout(constant_id = 1) const uint scope1 = gl_ScopeSubgroup;
        layout(local_size_x = 64) in;
        void main() {
            coopmat<float16_t, scope0, 16, 16, gl_MatrixUseA> A;
            coopmat<float16_t, scope1, 16, 16, gl_MatrixUseB> B;
            coopmat<float16_t, scope0, 16, 16, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, scope0, 16, 16, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    const uint32_t spec_data[] = {
        3,  // gl_ScopeSubgroup
        4,  // gl_ScopeInvocation
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };
    const VkSpecializationInfo spec_info = {
        2,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info);

    // SPIR-V code is expected to be bad after specialization, due to scopes are different
    // Need to ignore the spirv-val
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");

    // The scope will be invalid
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
    // Expect gl_ScopeInvocation will not be found in the implementation since it is not allowed in Vulkan
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, WorkgroupScope) {
    TEST_DESCRIPTION("Workgroup scope requires cooperativeMatrixWorkgroupScope");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_WORKGROUP_KHR, 32, 32, 32, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 64) in;
        void main() {
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixWorkgroupScope-10164", 3);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, WorkgroupScopeMaxDimensions) {
    TEST_DESCRIPTION("Matrix dimensions must be less than or equal to cooperativeMatrixFlexibleDimensionsMaxDimension");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_WORKGROUP_KHR, 32, 32, 32, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(constant_id = 0) const uint dim = 32;
        layout(local_size_x = 64) in;
        void main() {
            coopmat<float16_t, gl_ScopeWorkgroup, dim, dim, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeWorkgroup, dim, dim, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeWorkgroup, dim, dim, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeWorkgroup, dim, dim, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV props2 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props2);
    uint32_t dim = props2.cooperativeMatrixFlexibleDimensionsMaxDimension;
    dim /= 32;
    dim++;
    dim *= 32;

    const uint32_t spec_data[] = {
        dim,
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
    };
    const VkSpecializationInfo spec_info = {
        1,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensionsMaxDimension-10167", 3);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, WorkgroupScopeMaxSharedMemory) {
    TEST_DESCRIPTION("cooperativeMatrixWorkgroupScopeReservedSharedMemory limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    if (!helper.HasValidProperty(VK_SCOPE_WORKGROUP_KHR, 32, 32, 32, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property not found";
    }

    const char *cs_source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(constant_id = 0) const uint dim = 32;
        shared uint8_t sh[dim];
        layout(local_size_x = 64) in;
        void main() {
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeWorkgroup, 32, 32, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV props2 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props2);

    uint32_t shmem_size =
        m_device->Physical().limits_.maxComputeSharedMemorySize - props2.cooperativeMatrixWorkgroupScopeReservedSharedMemory + 1;

    const uint32_t spec_data[] = {
        shmem_size,
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
    };
    const VkSpecializationInfo spec_info = {
        1,
        entries,
        sizeof(spec_data),
        spec_data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-maxComputeSharedMemorySize-10168");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, MatchSizeWithProperties) {
    TEST_DESCRIPTION("Check size match properties");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.subgroupSize > 32) {
        GTEST_SKIP() << "local_size_x (32) is not a multiple of subgroupSize";
    }

    if (helper.HasValidProperty(VK_SCOPE_SUBGROUP_KHR, 8, 8, 16, VK_COMPONENT_TYPE_FLOAT16_KHR)) {
        GTEST_SKIP() << "Valid Property found, need invalid to test";
    }

    const char *source = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(local_size_x = 32) in;
        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, 8, 16, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> C;
            coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
       }
    )glsl";

    // There is no way to avoid this message
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163");

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, SignedCheck) {
    TEST_DESCRIPTION("Test that if component type of is signed check that appropriate MatrixSignedComponents is present");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    // OpExtension "SPV_KHR_storage_buffer_storage_class"
    const std::string cs_source_template = R"glsl(
        OpCapability Shader
        OpCapability VulkanMemoryModel
        OpCapability CooperativeMatrixKHR
        OpExtension "SPV_KHR_cooperative_matrix"
        OpExtension "SPV_KHR_vulkan_memory_model"
        %1 = OpExtInstImport "GLSL.std.450"
        OpMemoryModel Logical Vulkan
        OpEntryPoint GLCompute %4 "main" %13 %17 %22 %24
        OpExecutionMode %4 LocalSize 64 1 1
        OpDecorate %29 BuiltIn WorkgroupSize
        %2 = OpTypeVoid
        %3 = OpTypeFunction %2
        %6 = OpTypeInt 32 1
        %7 = OpTypeInt 32 0
        %8 = OpConstant %7 3
        %9 = OpConstant %7 16
        %10 = OpConstant %7 2
        %11 = OpTypeCooperativeMatrixKHR %6 %8 %9 %9 %10
        %12 = OpTypePointer Private %11
        %13 = OpVariable %12 Private
        %14 = OpConstant %7 0
        %15 = OpTypeCooperativeMatrixKHR %6 %8 %9 %9 %14
        %16 = OpTypePointer Private %15
        %17 = OpVariable %16 Private
        %19 = OpConstant %7 1
        %20 = OpTypeCooperativeMatrixKHR %6 %8 %9 %9 %19
        %21 = OpTypePointer Private %20
        %22 = OpVariable %21 Private
        %24 = OpVariable %12 Private
        %27 = OpTypeVector %7 3
        %28 = OpConstant %7 64
        %29 = OpConstantComposite %27 %28 %19 %19
        %4 = OpFunction %2 None %3
        %5 = OpLabel
        %18 = OpLoad %15 %17
        %23 = OpLoad %20 %22
        %25 = OpLoad %11 %24
        %26 = OpCooperativeMatrixMulAddKHR %11 %18 %23 %25 MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR|MatrixCSignedComponentsKHR|MatrixResultSignedComponentsKHR
        OpStore %13 %26
        OpReturn
        OpFunctionEnd
    )glsl";

    const auto remove_str = [](const std::string &shader_template, const std::string &removestr) {
        std::string result = shader_template;
        auto position = result.find(removestr);
        assert(position != std::string::npos);
        result.replace(position, removestr.length(), std::string(""));
        return result;
    };
    const struct {
        const char *remove;
        const char *expect;
    } subtests[] = {
        {"MatrixASignedComponentsKHR|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060"},
        {"MatrixBSignedComponentsKHR|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060"},
        {"MatrixCSignedComponentsKHR|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060"},
        {"|MatrixResultSignedComponentsKHR", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060"},
    };

    for (const auto &x: subtests) {
        const std::string cs_source_str = remove_str(cs_source_template, std::string(x.remove));
        const char *css = cs_source_str.c_str();
        CreateComputePipelineHelper pipe(*this);

        pipe.cs_ = VkShaderObj(*m_device, css, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

        for (const auto &y : subtests) {
            if (x.remove == y.remove) {
                // Set expected message
                m_errorMonitor->SetDesiredError(y.expect);
            } else {
                // Ignore messages that types and sizes are unsupported by implementation
                m_errorMonitor->SetAllowedFailureMsg(y.expect);
            }
        }

        // Ignore messages that types and sizes are unsupported by implementation
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163");

        pipe.CreateComputePipeline();

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderCooperativeMatrix, RequiredVulkanVersionPipeline) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/spirv/SPIR-V/-/issues/847");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::subgroupSizeControl);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This makes assumption about possible coop matrix subgroup size and support.";
    }

    const vkt::DescriptorSetLayout dsl(*m_device, {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&dsl});

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
         layout(local_size_x = 32) in;
         layout(set=0, binding=0) coherent buffer InputA { uint32_t x[]; } inputA;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
             coopMatLoad(matA, inputA.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";
    VkShaderObj cs(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = cs.GetStageCreateInfo();
    pipe.cp_ci_.layout = pipeline_layout;
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10770");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, RequiredVulkanVersionShaderObject) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/spirv/SPIR-V/-/issues/847");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::subgroupSizeControl);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This makes assumption about possible coop matrix subgroup size and support.";
    }

    const vkt::DescriptorSetLayout dsl(*m_device, {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
         layout(local_size_x = 32) in;
         layout(set=0, binding=0) coherent buffer InputA { uint32_t x[]; } inputA;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
             coopMatLoad(matA, inputA.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source, SPV_ENV_VULKAN_1_1);
    auto shader_ci = ShaderCreateInfoNoNextStage(spv, VK_SHADER_STAGE_COMPUTE_BIT, 1, &dsl.handle());

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10771");
    const vkt::Shader comp_shader(*m_device, shader_ci);
    m_errorMonitor->VerifyFound();
}
