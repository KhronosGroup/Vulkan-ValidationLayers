/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2026 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <spirv/unified1/spirv.hpp>
#include <algorithm>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "shader_object_helper.h"
#include "cooperative_matrix_helper.h"

void CooperativeMatrixTest::InitCooperativeMatrixKHR() {
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());
}

class PositiveShaderCooperativeMatrix : public CooperativeMatrixTest {};

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrixKHR) {
    TEST_DESCRIPTION("Test VK_KHR_cooperative_matrix.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    VkCooperativeMatrixPropertiesKHR subgroup_prop = vku::InitStructHelper();
    bool found_scope_subgroup = false;
    for (const auto& prop : helper.coop_matrix_props) {
        // We only have the 16-bit features enabled, but 32-bit also works
        if (prop.scope == VK_SCOPE_SUBGROUP_KHR && !helper.Has8BitComponentType(prop) && !helper.Has64BitComponentType(prop)) {
            found_scope_subgroup = true;
            subgroup_prop = prop;
            break;
        }
    }
    if (!found_scope_subgroup) {
        GTEST_SKIP() << "VK_SCOPE_SUBGROUP_KHR not Found";
    }

    const vkt::DescriptorSetLayout dsl(*m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    std::string css = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer InputA { %type_A% x[]; } inputA;
         layout(set=0, binding=1) coherent buffer InputB { %type_B% x[]; } inputB;
         layout(set=0, binding=2) coherent buffer InputC { %type_C% x[]; } inputC;
         layout(set=0, binding=3) coherent buffer Output { %type_R% x[]; } outputO;
         coopmat<%type_A%, gl_ScopeSubgroup, %M%, %K%, gl_MatrixUseA> matA;
         coopmat<%type_B%, gl_ScopeSubgroup, %K%, %N%, gl_MatrixUseB> matB;
         coopmat<%type_C%, gl_ScopeSubgroup, %M%, %N%, gl_MatrixUseAccumulator> matC;
         coopmat<%type_R%, gl_ScopeSubgroup, %M%, %N%, gl_MatrixUseAccumulator> matO;
         void main()
         {
             coopMatLoad(matA, inputA.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matB, inputB.x, 0, %K%, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matC, inputC.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
             matO = coopMatMulAdd(matA, matB, matC);
             coopMatStore(matO, outputO.x, 0, %M%, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    auto replace = [](std::string& str, const std::string& from, const std::string& to) {
        size_t pos;
        while ((pos = str.find(from)) != std::string::npos) str.replace(pos, from.length(), to);
    };
    replace(css, "%M%", std::to_string(subgroup_prop.MSize));
    replace(css, "%N%", std::to_string(subgroup_prop.NSize));
    replace(css, "%K%", std::to_string(subgroup_prop.KSize));
    replace(css, "%type_A%", helper.VkComponentTypeToGLSL(subgroup_prop.AType));
    replace(css, "%type_B%", helper.VkComponentTypeToGLSL(subgroup_prop.BType));
    replace(css, "%type_C%", helper.VkComponentTypeToGLSL(subgroup_prop.CType));
    replace(css, "%type_R%", helper.VkComponentTypeToGLSL(subgroup_prop.ResultType));

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, css.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&dsl});
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, RequiredSubgroupSize) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9843");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::subgroupSizeControl);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This makes assumption about possible coop matrix subgroup size and support.";
    }

    const vkt::DescriptorSetLayout dsl(*m_device, {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&dsl});

    const char* cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
         layout(local_size_x = 16) in;
         layout(set=0, binding=0) coherent buffer InputA { uint32_t x[]; } inputA;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
             coopMatLoad(matA, inputA.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";
    VkShaderObj cs(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);

    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_properties = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper(&subgroup_properties);
    GetPhysicalDeviceProperties2(props11);
    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "Required shader stage not present in requiredSubgroupSizeStages";
    }

    if (subgroup_properties.minSubgroupSize != 16) {
        GTEST_SKIP() << "Testing when we go under the limit";
    }

    VkPipelineShaderStageRequiredSubgroupSizeCreateInfo subgroup_size_control = vku::InitStructHelper();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = cs.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &subgroup_size_control;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveShaderCooperativeMatrix, RequiredVulkanVersionPipeline) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/spirv/SPIR-V/-/issues/847");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::computeFullSubgroups);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This makes assumption about possible coop matrix subgroup size and support.";
    }

    const vkt::DescriptorSetLayout dsl(*m_device, {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&dsl});

    const char* cs_source = R"glsl(
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
    pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveShaderCooperativeMatrix, RequiredVulkanVersionShaderObject) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/spirv/SPIR-V/-/issues/847");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::computeFullSubgroups);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This makes assumption about possible coop matrix subgroup size and support.";
    }

    const vkt::DescriptorSetLayout dsl(*m_device,
                                       {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});

    const char* cs_source = R"glsl(
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
    shader_ci.flags = VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    const vkt::Shader comp_shader(*m_device, shader_ci);
}

TEST_F(PositiveShaderCooperativeMatrix, BFloat16) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_BFLOAT16_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderBFloat16Type);
    AddRequiredFeature(vkt::Feature::shaderBFloat16CooperativeMatrix);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_bfloat16 : require
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_KHR_cooperative_matrix : enable
        layout(local_size_x = 64) in;
        void main() {
            coopmat<bfloat16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> cmA = coopmat<bfloat16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA>(3.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCooperativeMatrix, Float8) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_FLOAT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderFloat8);
    AddRequiredFeature(vkt::Feature::shaderFloat8CooperativeMatrix);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    CooperativeMatrixHelper helper(*this);
    if (!helper.HasSupportedMatrixUse(VK_SCOPE_SUBGROUP_KHR, 16, 32, VK_COMPONENT_TYPE_FLOAT_E4M3_NV,
                                      spv::CooperativeMatrixUseMatrixAKHR)) {
        GTEST_SKIP() << "desired VkCooperativeMatrixPropertiesKHR not found";
    }

    const char* cs_source = R"glsl(
        #version 450 core
        #extension GL_EXT_float_e4m3 : require
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_KHR_cooperative_matrix : enable
        layout(local_size_x = 32) in;
        void main() {
            coopmat<floate4m3_t, gl_ScopeSubgroup, 16, 32, gl_MatrixUseA> cmA = coopmat<floate4m3_t, gl_ScopeSubgroup, 16, 32, gl_MatrixUseA>(3.0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCooperativeMatrix, Int8) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "desired VkCooperativeMatrixPropertiesKHR not found";
    }

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pl(*m_device, {&descriptor_set.layout_});

    std::string css = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer InputA { uint8_t x[]; } inputA;
         layout(set=0, binding=1) coherent buffer InputB { uint8_t x[]; } inputB;
         layout(set=0, binding=2) coherent buffer InputC { uint32_t x[]; } inputC;
         layout(set=0, binding=3) coherent buffer Output { uint32_t x[]; } outputO;
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> matB;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matO;
         void main() {
             coopMatLoad(matA, inputA.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matB, inputB.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
             coopMatLoad(matC, inputC.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
             matO = coopMatMulAdd(matA, matB, matC);
             coopMatStore(matO, outputO.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, css.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.cp_ci_.layout = pl;
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, WorkgroupScopeLocalSizeIdSpecConstant) {
    TEST_DESCRIPTION(
        "Pre-specialization skips the unknown LocalSizeId; specialization matches a flexible-dimensions property. "
        "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12363");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());
    CooperativeMatrixHelper helper(*this);

    constexpr uint32_t kRows = 32;
    constexpr uint32_t kCols = 32;
    constexpr uint32_t kK = 32;

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV props2 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props2);

    const auto fixed_property_matches_mul_add = [&](const VkCooperativeMatrixPropertiesKHR& prop) {
        return prop.scope == VK_SCOPE_WORKGROUP_KHR && prop.AType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
               prop.BType == VK_COMPONENT_TYPE_FLOAT16_KHR && prop.CType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
               prop.ResultType == VK_COMPONENT_TYPE_FLOAT16_KHR && prop.MSize == kRows && prop.NSize == kCols && prop.KSize == kK;
    };

    const VkCooperativeMatrixFlexibleDimensionsPropertiesNV* flexible_prop = nullptr;
    for (const auto& prop : helper.coop_matrix_flex_props) {
        if (prop.scope == VK_SCOPE_WORKGROUP_KHR && prop.AType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            prop.BType == VK_COMPONENT_TYPE_FLOAT16_KHR && prop.CType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            prop.ResultType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            prop.workgroupInvocations <= props2.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize &&
            prop.workgroupInvocations <= m_device->Physical().limits_.maxComputeWorkGroupInvocations &&
            prop.workgroupInvocations <= m_device->Physical().limits_.maxComputeWorkGroupSize[0] &&
            (kRows % prop.MGranularity) == 0 && (kCols % prop.NGranularity) == 0 && (kK % prop.KGranularity) == 0 &&
            std::none_of(helper.coop_matrix_props.begin(), helper.coop_matrix_props.end(), fixed_property_matches_mul_add)) {
            flexible_prop = &prop;
            break;
        }
    }
    if (!flexible_prop) {
        GTEST_SKIP() << "desired VkCooperativeMatrixFlexibleDimensionsPropertiesNV not found";
    }

    std::ostringstream spv_source;
    spv_source << R"asm(
               OpCapability Shader
               OpCapability Float16
               OpCapability VulkanMemoryModel
               OpCapability CooperativeMatrixKHR
               OpExtension "SPV_KHR_cooperative_matrix"
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %local_size_x %uint_1 %uint_1
               OpDecorate %local_size_x SpecId 18
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %half = OpTypeFloat 16
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %uint_32 = OpConstant %uint 32
%local_size_x = OpSpecConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %local_size_x %uint_1 %uint_1
      %mat_a = OpTypeCooperativeMatrixKHR %half %uint_2 %uint_32 %uint_32 %uint_0
      %mat_b = OpTypeCooperativeMatrixKHR %half %uint_2 %uint_32 %uint_32 %uint_1
      %mat_c = OpTypeCooperativeMatrixKHR %half %uint_2 %uint_32 %uint_32 %uint_2
%_ptr_Function_mat_a = OpTypePointer Function %mat_a
%_ptr_Function_mat_b = OpTypePointer Function %mat_b
%_ptr_Function_mat_c = OpTypePointer Function %mat_c
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function_mat_a Function
          %b = OpVariable %_ptr_Function_mat_b Function
          %c = OpVariable %_ptr_Function_mat_c Function
     %load_a = OpLoad %mat_a %a
     %load_b = OpLoad %mat_b %b
     %load_c = OpLoad %mat_c %c
     %result = OpCooperativeMatrixMulAddKHR %mat_c %load_a %load_b %load_c
               OpReturn
               OpFunctionEnd
    )asm";

    const uint32_t spec_data = flexible_prop->workgroupInvocations;
    const VkSpecializationMapEntry entry = {18, 0, sizeof(uint32_t)};
    const VkSpecializationInfo spec_info = {1, &entry, sizeof(spec_data), &spec_data};

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM,
                           &spec_info);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, OpExtractSubArrayQCOM) {
    TEST_DESCRIPTION("Correct OpExtractSubArrayQCOM instruction usage.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_COOPERATIVE_MATRIX_CONVERSION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::storageBuffer16BitAccess);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixConversion);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    struct MatrixTileInfo {
        uint32_t tile_m = 0;
        uint32_t tile_n = 0;
        uint32_t tile_k = 0;
    } tile_info;

    uint32_t props_count = 0;
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(Gpu(), &props_count, nullptr);
    std::vector<VkCooperativeMatrixPropertiesKHR> cooperative_matrix_props{};
    cooperative_matrix_props.resize(props_count, vku::InitStructHelper());
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(Gpu(), &props_count, cooperative_matrix_props.data());

    bool found_target_tile = false;
    for (const auto& cooperative_matrix_prop : cooperative_matrix_props) {
        if (cooperative_matrix_prop.scope == VK_SCOPE_SUBGROUP_KHR && cooperative_matrix_prop.MSize == 64 &&
            cooperative_matrix_prop.NSize == 32 && cooperative_matrix_prop.KSize == 16 &&
            cooperative_matrix_prop.AType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            cooperative_matrix_prop.BType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            cooperative_matrix_prop.CType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            cooperative_matrix_prop.ResultType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
            cooperative_matrix_prop.saturatingAccumulation == VK_FALSE) {
            tile_info = {64, 32, 16};
            found_target_tile = true;
            break;
        }
    }

    if (!found_target_tile) {
        GTEST_SKIP() << "Failed to find the expected TILE from all enumerated VkCooperativeMatrixPropertiesKHR, skipping test.";
    }

    const char* cs_source = R"glsl(
        #version 460 core

        #pragma use_vulkan_memory_model
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_QCOM_cooperative_matrix_conversion : enable

        layout(set = 0, binding = 0) coherent readonly buffer InputBufA { float16_t input_data[]; } input_buf_a;
        layout(set = 0, binding = 1) coherent readonly buffer InputBufB { float16_t input_data[]; } input_buf_b;
        layout(constant_id = 0) const uint TILE_M = 64;
        layout(constant_id = 1) const uint TILE_N = 32;
        layout(constant_id = 2) const uint TILE_K = 16;
        layout(constant_id = 3) const uint START_INDEX = 0;
        layout(constant_id = 4) const uint STRIDE_A = 16;
        layout(constant_id = 5) const uint STRIDE_B = 32;

        layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
        void main() {
            if (gl_SubgroupSize != TILE_M) {
                return;
            }

            coopmat<float16_t, gl_ScopeSubgroup, TILE_M, TILE_K, gl_MatrixUseA> mat_a;
            coopmat<float16_t, gl_ScopeSubgroup, TILE_K, TILE_N, gl_MatrixUseB> mat_b;
            coopmat<float16_t, gl_ScopeSubgroup, TILE_M, TILE_N, gl_MatrixUseAccumulator> mat_c;

            coopmat<float16_t, gl_ScopeSubgroup, TILE_M, TILE_K, gl_MatrixUseA> mat_c2a;

            mat_c = coopmat<float16_t, gl_ScopeSubgroup, TILE_M, TILE_N, gl_MatrixUseAccumulator>(0.0);

            coopMatLoad(mat_a, input_buf_a.input_data, 0, STRIDE_A, gl_CooperativeMatrixLayoutRowMajor);
            coopMatLoad(mat_b, input_buf_b.input_data, 0, STRIDE_B, gl_CooperativeMatrixLayoutRowMajor);
            mat_c = coopMatMulAdd(mat_a, mat_b, mat_c);

            float16_t vec_c[TILE_N];
            coopmatToVectorQCOM(mat_c, vec_c);

            float16_t vec_k[TILE_K];
            extractSubArrayQCOM(vec_c, START_INDEX, vec_k);

            vectorToCoopmatQCOM(vec_k, mat_c2a);
        }
    )glsl";

    constexpr std::array<VkSpecializationMapEntry, 6> entries{
        VkSpecializationMapEntry{0, 0, sizeof(uint32_t)},
        VkSpecializationMapEntry{1, sizeof(uint32_t) * 1, sizeof(uint32_t)},
        VkSpecializationMapEntry{2, sizeof(uint32_t) * 2, sizeof(uint32_t)},
        VkSpecializationMapEntry{3, sizeof(uint32_t) * 3, sizeof(uint32_t)},
        VkSpecializationMapEntry{4, sizeof(uint32_t) * 4, sizeof(uint32_t)},
        VkSpecializationMapEntry{5, sizeof(uint32_t) * 5, sizeof(uint32_t)},
    };
    const std::array<uint32_t, 6> spec_data{
        tile_info.tile_m,
        tile_info.tile_n,
        tile_info.tile_k,
        0,
        tile_info.tile_k,
        tile_info.tile_n,
    };
    const VkSpecializationInfo spec_info{
        entries.size(),
        entries.data(),
        sizeof(spec_data),
        spec_data.data(),
    };

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_GLSL, &spec_info};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    compute_pipe.CreateComputePipeline();
}
