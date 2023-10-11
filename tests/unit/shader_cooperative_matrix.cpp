/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativeShaderCooperativeMatrix, KHRSpecInfo) {
    TEST_DESCRIPTION("Test VK_KHR_cooperative_matrix.");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        layout(constant_id = 0) const uint C0 = 1;
        layout(constant_id = 1) const uint C1 = 1;
        void main() {
            // Bad type
            coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator> badSize = coopmat<float16_t, gl_ScopeSubgroup, 3, 5, gl_MatrixUseAccumulator>(float16_t(0.0));
            // Not a valid multiply when C0 != C1
            coopmat<float16_t, gl_ScopeSubgroup, C0, C1, gl_MatrixUseA> A;
            coopmat<float16_t, gl_ScopeSubgroup, C0, C1, gl_MatrixUseB> B;
            coopmat<float16_t, gl_ScopeSubgroup, C0, C1, gl_MatrixUseAccumulator> C;
            coopMatMulAdd(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        63,
        65,
    };
    VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };

    VkSpecializationInfo specInfo = {
        2,
        entries,
        sizeof(specData),
        specData,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, KHRUnsupportedStage) {
    TEST_DESCRIPTION("Test error using cooperative matrix in unsupported stage");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);

    VkPhysicalDeviceCooperativeMatrixPropertiesKHR props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);

    if ((props.cooperativeMatrixSupportedStages & VK_SHADER_STAGE_VERTEX_BIT) != 0) {
        GTEST_SKIP() << "Cannot execute test due to vertex stage expected to be unsupported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));
    InitRenderTarget();

    char const *vtSource = R"glsl(
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
    pipe.vs_ = std::make_unique<VkShaderObj>(this, vtSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08739");
    // Ignore messages that types and sizes are unsupported by implementation
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08975");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-KSize-08977");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08979");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08981");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08976");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08978");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08980");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08982");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-cooperativeMatrixSupportedStages-08985");
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, KHRParametersMatchProperties) {
    TEST_DESCRIPTION("Test that parameters match one of the matrices in any of the supported VkCooperativeMatrixPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    // Tests are assume that Float16 3*5 is not available
    char const *csSource = R"glsl(
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
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-08974");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, KHRDimXMultipleSubgroupSize) {
    TEST_DESCRIPTION("Local workgroup size in the X dimension of the pipeline multiple of subgroupSize");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    // Tests are assume that Float16 3*5 is not available
    char const *csSource = R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x_id = 0, local_size_y = 1, local_size_z = 1) in;
        void main() {
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> A;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> B;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> C;
            coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        31,
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
    };
    const VkSpecializationInfo specInfo = {
        1,
        entries,
        sizeof(specData),
        specData,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo);
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {});
    // Ignore messages that types and sizes are unsupported by implementation
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08975");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-KSize-08977");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08979");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08981");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08976");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08978");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08980");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08982");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-module-08987");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCooperativeMatrix, KHRSameScope) {
    TEST_DESCRIPTION("In OpCooperativeMatrixMulAddKHR all matrices should have same scope");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    char const *csSource = R"glsl(
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
            coopmat<float32_t, scope0, 16, 16, gl_MatrixUseA> A;
            coopmat<float32_t, scope1, 16, 16, gl_MatrixUseB> B;
            coopmat<float32_t, scope0, 16, 16, gl_MatrixUseAccumulator> C;
            coopmat<float32_t, scope0, 16, 16, gl_MatrixUseAccumulator> D = coopMatMulAdd(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        3, // gl_ScopeSubgroup
        4, // gl_ScopeInvocation
    };
    const VkSpecializationMapEntry entries[] = {
        {0, sizeof(uint32_t) * 0, sizeof(uint32_t)},
        {1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
    };
    const VkSpecializationInfo specInfo = {
        2,
        entries,
        sizeof(specData),
        specData,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo);
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {});
    // Ignore messages that types and sizes are unsupported by implementation
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08975");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-KSize-08977");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08979");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08981");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08976");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08978");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08980");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08982");

    // SPIR-V code is expected to be bad after specialization, due to scopes are different
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");

    // Expect gl_ScopeInvocation will not be found in the implementation
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-08974");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-scope-08984");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

// Tests fail in CI from failing spirv-as
// Need to rewrite tests without subtests logic
TEST_F(NegativeShaderCooperativeMatrix, DISABLED_MatchSizeWithProperties) {
    TEST_DESCRIPTION("Check size match properties");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    // OpExtension "SPV_KHR_storage_buffer_storage_class"
    const std::string csSourceTemplate = R"glsl(
        OpCapability Shader
        OpCapability VulkanMemoryModel
        OpCapability CooperativeMatrixKHR
        OpExtension "SPV_KHR_cooperative_matrix"
        OpExtension "SPV_KHR_vulkan_memory_model"
        %import = OpExtInstImport "GLSL.std.450"
        OpMemoryModel Logical Vulkan
        OpEntryPoint GLCompute %main "main"
        OpExecutionMode %main LocalSize 64 1 1
        OpDecorate %wgs BuiltIn WorkgroupSize
        %void  = OpTypeVoid
        %func  = OpTypeFunction %void
        %sint  = OpTypeInt 32 1
        %uint  = OpTypeInt 32 0
        %uvec3 = OpTypeVector %uint 3
        %c_3u  = OpConstant %uint 3
        %c_64u = OpConstant %uint 64
        %wgs   = OpConstantComposite %uvec3 %c64_u %one %one

        %rows  = OpConstant %uint 16

        %A_col = OpConstant %uint $A_VAL
        %A_use = OpConstant %uint 0
        %A_t   = OpTypeCooperativeMatrixKHR %sint %c_3u %rows %A_col %A_use
        %A_ptr = OpTypePointer Private %A_t
        %A_var = OpVariable %A_ptr Private

        %B_col = OpConstant %uint $B_VAL
        %B_use = OpConstant %uint 1
        %B_t   = OpTypeCooperativeMatrixKHR %sint %c_3u %rows %B_col %B_use
        %B_ptr = OpTypePointer Private %B_t
        %B_var = OpVariable %B_ptr Private

        %C_col = OpConstant %uint $C_VAL
        %C_use = OpConstant %uint 2
        %C_t   = OpTypeCooperativeMatrixKHR %sint %c_3u %rows %C_col %C_use
        %C_ptr = OpTypePointer Private %C_t
        %C_var = OpVariable %C_ptr Private

        %R_col = OpConstant %uint $R_VAL
        %R_use = OpConstant %uint 2
        %R_t   = OpTypeCooperativeMatrixKHR %sint %c_3u %rows %R_col %R_use
        %R_ptr = OpTypePointer Private %R_t
        %R_var = OpVariable %R_ptr Private

        %main  = OpFunction %void None %func
        %body  = OpLabel
        %A     = OpLoad %A_t %A_var
        %B     = OpLoad %B_t %B_var
        %C     = OpLoad %C_t %C_var
        %R     = OpCooperativeMatrixMulAddKHR %R_t %A %B %C MatrixASignedComponents|MatrixBSignedComponents|MatrixCSignedComponents|MatrixResultSignedComponents
        OpStore %R_var %R
        OpReturn
        OpFunctionEnd
    )glsl";

    const struct {
        const char *var;
        const char *name;
    } subtests[] = {
        {"$A_VAL", "VUID-RuntimeSpirv-MSize-08975"},
        {"$B_VAL", "VUID-RuntimeSpirv-KSize-08977"},
        {"$C_VAL", "VUID-RuntimeSpirv-MSize-08979"},
        {"$R_VAL", "VUID-RuntimeSpirv-MSize-08981"},
    };
    auto replace = [](std::string &str, const std::string &from, const std::string &to) {
        size_t pos;
        while ((pos = str.find(from)) != std::string::npos) str.replace(pos, from.length(), to);
    };

    for (const auto &x : subtests) {
        std::string css(csSourceTemplate);

        for (const auto &y : subtests) {
            if (x.var == y.var) {
                // Set expected message
                replace(css, y.var, "17");
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, y.name);
            } else {
                // Ignore messages that types and sizes are unsupported by implementation
                replace(css, y.var, "16");
                m_errorMonitor->SetAllowedFailureMsg(y.name);
            }
        }

        // Allow SPIR-V tests to be accepted as Vertex Shader, while it essentially Compute Shader
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08737");

        // There is no way to avoid this message
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-08974");

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, css.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                 SPV_SOURCE_ASM, nullptr);
        pipe.InitState();
        pipe.CreateComputePipeline();
        m_errorMonitor->VerifyFound();
    }
}

// Tests fail in CI from failing spirv-as
// Need to rewrite tests without subtests logic
TEST_F(NegativeShaderCooperativeMatrix, DISABLED_KHRSignedCheck) {
    TEST_DESCRIPTION("Test that if component type of is signed check that appropriate MatrixSignedComponents is present");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_features = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperative_matrix_features = vku::InitStructHelper(&float16_features);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memory_model_features = vku::InitStructHelper(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    RETURN_IF_SKIP(InitState(nullptr, &memory_model_features));

    // OpExtension "SPV_KHR_storage_buffer_storage_class"
    const std::string csSourceTemplate = R"glsl(
        OpCapability Shader
        OpCapability VulkanMemoryModel
        OpCapability CooperativeMatrixKHR
        OpExtension "SPV_KHR_cooperative_matrix"
        OpExtension "SPV_KHR_vulkan_memory_model"
        %1 = OpExtInstImport "GLSL.std.450"
        OpMemoryModel Logical Vulkan
        OpEntryPoint GLCompute %4 "main"
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
        %26 = OpCooperativeMatrixMulAddKHR %11 %18 %23 %25 MatrixASignedComponents|MatrixBSignedComponents|MatrixCSignedComponents|MatrixResultSignedComponents
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
        {"MatrixASignedComponents|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08976"},
        {"MatrixBSignedComponents|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08978"},
        {"MatrixCSignedComponents|", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08980"},
        {"|MatrixResultSignedComponents", "VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-08982"},
    };

    for (const auto &x: subtests) {
        const std::string csSourceStr = remove_str(csSourceTemplate, std::string(x.remove));
        const char *css = csSourceStr.c_str();
        CreateComputePipelineHelper pipe(*this);

        pipe.cs_ =
            std::make_unique<VkShaderObj>(this, css, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr);
        pipe.InitState();

        for (const auto &y : subtests) {
            if (x.remove == y.remove) {
                // Set expected message
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, y.expect);
            } else {
                // Ignore messages that types and sizes are unsupported by implementation
                m_errorMonitor->SetAllowedFailureMsg(y.expect);
            }
        }

        // Ignore messages that types and sizes are unsupported by implementation
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08975");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-KSize-08977");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08979");
        m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MSize-08981");

        pipe.CreateComputePipeline();

        m_errorMonitor->VerifyFound();
    }
}
