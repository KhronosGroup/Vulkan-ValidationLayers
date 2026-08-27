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
#include <iterator>
#include <optional>
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

namespace {

struct FloatMatrixConfig {
    uint32_t m;
    uint32_t n;
    uint32_t k;
    VkComponentTypeKHR c_type;
};

template <typename Property>
bool HasFloatTypes(const Property& property, VkComponentTypeKHR c_type) {
    return property.AType == VK_COMPONENT_TYPE_FLOAT16_KHR && property.BType == VK_COMPONENT_TYPE_FLOAT16_KHR &&
           property.CType == c_type && property.ResultType == c_type;
}

bool IsRequiredFloatProperty(const VkCooperativeMatrixProperties2EXT& property) {
    // Every required query has an FP16-input property with matching FP16 or FP32 accumulator/result types.
    return HasFloatTypes(property, property.CType) &&
           (property.CType == VK_COMPONENT_TYPE_FLOAT16_KHR || property.CType == VK_COMPONENT_TYPE_FLOAT32_KHR);
}

FloatMatrixConfig GetFloatMatrixConfig(const VkCooperativeMatrixProperties2EXT& property) {
    return {property.MGranularity, property.NGranularity, property.KGranularity, property.CType};
}

bool HasFixedFloatMulAdd(const CooperativeMatrixHelper& helper, VkScopeKHR scope, const FloatMatrixConfig& config) {
    return std::any_of(helper.coop_matrix_props.begin(), helper.coop_matrix_props.end(), [&](const auto& property) {
        return property.scope == scope && property.MSize == config.m && property.NSize == config.n && property.KSize == config.k &&
               HasFloatTypes(property, config.c_type) && !property.saturatingAccumulation;
    });
}

bool HasProperties2FloatMulAdd(const std::vector<VkCooperativeMatrixProperties2EXT>& properties, const FloatMatrixConfig& config) {
    return std::any_of(properties.begin(), properties.end(), [&](const auto& property) {
        return property.MGranularity == config.m && property.NGranularity == config.n && property.KGranularity == config.k &&
               HasFloatTypes(property, config.c_type);
    });
}

bool HasFlexibleFloatMulAdd(const CooperativeMatrixHelper& helper, VkScopeKHR scope, const FloatMatrixConfig& config,
                            uint32_t workgroup_invocations) {
    return std::any_of(helper.coop_matrix_flex_props.begin(), helper.coop_matrix_flex_props.end(), [&](const auto& property) {
        return property.scope == scope && property.MGranularity != 0 && property.NGranularity != 0 && property.KGranularity != 0 &&
               (config.m % property.MGranularity) == 0 && (config.n % property.NGranularity) == 0 &&
               (config.k % property.KGranularity) == 0 && HasFloatTypes(property, config.c_type) &&
               !property.saturatingAccumulation &&
               (scope != VK_SCOPE_WORKGROUP_KHR || property.workgroupInvocations == workgroup_invocations);
    });
}

std::optional<FloatMatrixConfig> FindExactProperties2Config(const CooperativeMatrixHelper& helper, VkScopeKHR scope,
                                                            const std::vector<VkCooperativeMatrixProperties2EXT>& properties,
                                                            bool require_properties2_only) {
    for (const auto& property : properties) {
        if (!IsRequiredFloatProperty(property) || property.MGranularity == 0 || property.NGranularity == 0 ||
            property.KGranularity == 0) {
            continue;
        }
        const FloatMatrixConfig config = GetFloatMatrixConfig(property);
        if (!require_properties2_only || !HasFixedFloatMulAdd(helper, scope, config)) {
            return config;
        }
    }
    return std::nullopt;
}

std::optional<FloatMatrixConfig> FindFlexibleProperties2Config(const CooperativeMatrixHelper& helper,
                                                               const std::vector<VkCooperativeMatrixProperties2EXT>& properties,
                                                               uint32_t max_dimension, bool require_properties2_only) {
    const auto is_properties2_only = [&helper](const FloatMatrixConfig& config) {
        return !HasFixedFloatMulAdd(helper, VK_SCOPE_SUBGROUP_KHR, config) &&
               !HasFlexibleFloatMulAdd(helper, VK_SCOPE_SUBGROUP_KHR, config, 0);
    };

    for (const auto& property : properties) {
        if (!IsRequiredFloatProperty(property) || property.MGranularity == 0 || property.NGranularity == 0 ||
            property.KGranularity == 0 || property.MGranularity > max_dimension || property.NGranularity > max_dimension ||
            property.KGranularity > max_dimension) {
            continue;
        }

        const FloatMatrixConfig base = GetFloatMatrixConfig(property);
        const uint32_t max_scale =
            std::min(16u, std::max({max_dimension / base.m, max_dimension / base.n, max_dimension / base.k}));
        for (uint32_t scale = 2; scale <= max_scale; ++scale) {
            for (uint32_t dimension = 0; dimension < 3; ++dimension) {
                FloatMatrixConfig config = base;
                uint32_t* value = dimension == 0 ? &config.m : (dimension == 1 ? &config.n : &config.k);
                if (*value > max_dimension / scale) {
                    continue;
                }
                *value *= scale;
                if (!require_properties2_only || is_properties2_only(config)) {
                    return config;
                }
            }
        }
    }
    return std::nullopt;
}

std::string MakeFloatMatrixSource(VkScopeKHR scope, const FloatMatrixConfig& config, uint32_t local_size) {
    const char* glsl_scope = scope == VK_SCOPE_WORKGROUP_KHR ? "gl_ScopeWorkgroup" : "gl_ScopeSubgroup";
    const char* c_type = config.c_type == VK_COMPONENT_TYPE_FLOAT16_KHR ? "float16_t" : "float";
    std::ostringstream source;
    source << R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

        layout(local_size_x = )glsl"
           << local_size << R"glsl() in;
        void main() {
            coopmat<float16_t, )glsl"
           << glsl_scope << ", " << config.m << ", " << config.k << R"glsl(, gl_MatrixUseA> a;
            coopmat<float16_t, )glsl"
           << glsl_scope << ", " << config.k << ", " << config.n << R"glsl(, gl_MatrixUseB> b;
            coopmat<)glsl"
           << c_type << ", " << glsl_scope << ", " << config.m << ", " << config.n << R"glsl(, gl_MatrixUseAccumulator> c;
            coopmat<)glsl"
           << c_type << ", " << glsl_scope << ", " << config.m << ", " << config.n
           << R"glsl(, gl_MatrixUseAccumulator> result = coopMatMulAdd(a, b, c);
        }
    )glsl";
    return source.str();
}

}  // namespace

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

TEST_F(PositiveShaderCooperativeMatrix, Properties2SubgroupQueries) {
    TEST_DESCRIPTION("Valid subgroup vkGetPhysicalDeviceCooperativeMatrixProperties2EXT queries.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
    info.scope = VK_SCOPE_SUBGROUP_KHR;
    const auto varying_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(varying_properties.has_value());
    ASSERT_FALSE(varying_properties->empty());

    VkPhysicalDeviceSubgroupProperties subgroup_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_properties);
    info.subgroupSize = subgroup_properties.subgroupSize;
    const auto default_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(default_properties.has_value());
    ASSERT_FALSE(default_properties->empty());

    VkPhysicalDeviceSubgroupSizeControlFeatures subgroup_size_control_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(subgroup_size_control_features);
    if (subgroup_size_control_features.subgroupSizeControl) {
        VkPhysicalDeviceSubgroupSizeControlProperties subgroup_size_control_props = vku::InitStructHelper();
        GetPhysicalDeviceProperties2(subgroup_size_control_props);
        if (subgroup_size_control_props.minSubgroupSize != subgroup_properties.subgroupSize) {
            info.subgroupSize = subgroup_size_control_props.minSubgroupSize;
            // A valid non-default subgroup size is allowed to return no properties.
            ASSERT_TRUE(QueryCooperativeMatrixProperties2(Gpu(), info).has_value());
        }
    }
}

TEST_F(PositiveShaderCooperativeMatrix, Properties2WorkgroupQuery) {
    TEST_DESCRIPTION("Valid workgroup vkGetPhysicalDeviceCooperativeMatrixProperties2EXT query.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceSubgroupProperties subgroup_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_props);

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV cooperative_matrix2_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(cooperative_matrix2_props);
    if (subgroup_props.subgroupSize > cooperative_matrix2_props.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize) {
        GTEST_SKIP() << "Default subgroup size exceeds cooperativeMatrixWorkgroupScopeMaxWorkgroupSize";
    }

    VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
    info.scope = VK_SCOPE_WORKGROUP_KHR;
    info.invocations = subgroup_props.subgroupSize;
    const auto varying_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(varying_properties.has_value());
    ASSERT_FALSE(varying_properties->empty());

    info.subgroupSize = subgroup_props.subgroupSize;
    const auto default_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(default_properties.has_value());
    ASSERT_FALSE(default_properties->empty());
}

TEST_F(PositiveShaderCooperativeMatrix, Properties2WorkgroupMatrix) {
    TEST_DESCRIPTION("Validate a floating-point workgroup matrix advertised by the Properties2EXT query.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixWorkgroupScope);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    CooperativeMatrixHelper helper(*this);
    VkPhysicalDeviceVulkan11Properties properties11 = vku::InitStructHelper();
    VkPhysicalDeviceCooperativeMatrix2PropertiesNV cooperative_matrix2_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(properties11);
    GetPhysicalDeviceProperties2(cooperative_matrix2_properties);

    const uint32_t max_invocations = std::min({cooperative_matrix2_properties.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize,
                                               m_device->Physical().limits_.maxComputeWorkGroupInvocations,
                                               m_device->Physical().limits_.maxComputeWorkGroupSize[0]});
    std::vector<std::pair<uint32_t, std::vector<VkCooperativeMatrixProperties2EXT>>> property_tables;
    for (uint32_t invocations = properties11.subgroupSize; invocations <= max_invocations;) {
        VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
        info.scope = VK_SCOPE_WORKGROUP_KHR;
        info.invocations = invocations;
        const auto properties = QueryCooperativeMatrixProperties2(Gpu(), info);
        ASSERT_TRUE(properties.has_value());
        property_tables.emplace_back(invocations, *properties);
        if (invocations > max_invocations / 2) {
            break;
        }
        invocations *= 2;
    }

    std::optional<FloatMatrixConfig> config;
    uint32_t workgroup_invocations = 0;
    // First look for a tile advertised for only one invocation count, which verifies the runtime selector in the mock ICD.
    for (size_t table_index = 0; table_index < property_tables.size() && !config; ++table_index) {
        for (const auto& property : property_tables[table_index].second) {
            if (!IsRequiredFloatProperty(property) || property.MGranularity == 0 || property.NGranularity == 0 ||
                property.KGranularity == 0) {
                continue;
            }
            const FloatMatrixConfig candidate = GetFloatMatrixConfig(property);
            if (HasFixedFloatMulAdd(helper, VK_SCOPE_WORKGROUP_KHR, candidate)) {
                continue;
            }
            bool advertised_for_another_selector = false;
            for (size_t other_index = 0; other_index < property_tables.size(); ++other_index) {
                if (other_index != table_index && HasProperties2FloatMulAdd(property_tables[other_index].second, candidate)) {
                    advertised_for_another_selector = true;
                    break;
                }
            }
            if (!advertised_for_another_selector) {
                config = candidate;
                workgroup_invocations = property_tables[table_index].first;
                break;
            }
        }
    }

    // Real drivers may advertise the same tiles for every selector or overlap the fixed KHR table.
    for (const bool require_properties2_only : {true, false}) {
        if (config) {
            break;
        }
        for (const auto& [invocations, properties] : property_tables) {
            config = FindExactProperties2Config(helper, VK_SCOPE_WORKGROUP_KHR, properties, require_properties2_only);
            if (config) {
                workgroup_invocations = invocations;
                break;
            }
        }
    }
    if (!config) {
        GTEST_SKIP() << "No FP16-input workgroup tile is advertised by Properties2EXT";
    }

    const std::string cs_source = MakeFloatMatrixSource(VK_SCOPE_WORKGROUP_KHR, *config, workgroup_invocations);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, Properties2RuntimeQuerySelectors) {
    TEST_DESCRIPTION("Validate every shader-derived Properties2EXT subgroupSize selector for pipelines and shader objects.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::subgroupSizeControl);
    AddRequiredFeature(vkt::Feature::computeFullSubgroups);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    VkPhysicalDeviceSubgroupSizeControlProperties subgroup_size_control_props = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper(&subgroup_size_control_props);
    GetPhysicalDeviceProperties2(props11);
    if (!(subgroup_size_control_props.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT)) {
        GTEST_SKIP() << "Compute shaders do not support a required subgroup size";
    }

    CooperativeMatrixHelper helper(*this);
    VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
    info.scope = VK_SCOPE_SUBGROUP_KHR;
    const auto varying_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(varying_properties.has_value());
    const auto find_config = [&helper](const std::vector<VkCooperativeMatrixProperties2EXT>& properties) {
        auto config = FindExactProperties2Config(helper, VK_SCOPE_SUBGROUP_KHR, properties, true);
        if (!config) {
            // The fixed KHR table may overlap Properties2EXT; the mock's unique tiles still test selector choice in CI.
            config = FindExactProperties2Config(helper, VK_SCOPE_SUBGROUP_KHR, properties, false);
        }
        return config;
    };
    const auto varying_config = find_config(*varying_properties);
    if (!varying_config) {
        GTEST_SKIP() << "The subgroupSize 0 query advertises no FP16-input tile";
    }

    info.subgroupSize = props11.subgroupSize;
    const auto default_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(default_properties.has_value());
    const auto default_config = find_config(*default_properties);
    if (!default_config) {
        GTEST_SKIP() << "The default subgroup-size query advertises no FP16-input tile";
    }

    std::optional<FloatMatrixConfig> required_config;
    uint32_t required_subgroup_size = 0;
    for (uint32_t subgroup_size = subgroup_size_control_props.minSubgroupSize;
         subgroup_size <= subgroup_size_control_props.maxSubgroupSize;) {
        if (subgroup_size != props11.subgroupSize) {
            info.subgroupSize = subgroup_size;
            const auto required_properties = QueryCooperativeMatrixProperties2(Gpu(), info);
            ASSERT_TRUE(required_properties.has_value());
            required_config = find_config(*required_properties);
            if (required_config) {
                required_subgroup_size = subgroup_size;
                break;
            }
        }
        if (subgroup_size > subgroup_size_control_props.maxSubgroupSize / 2) {
            break;
        }
        subgroup_size *= 2;
    }
    if (!required_config) {
        GTEST_SKIP() << "No non-default required subgroup-size query advertises an FP16-input tile";
    }

    const auto create_pipeline = [this](spv_target_env target_env, const FloatMatrixConfig& config, uint32_t local_size,
                                        VkPipelineShaderStageCreateFlags flags, const void* p_next = nullptr) {
        const std::string source = MakeFloatMatrixSource(VK_SCOPE_SUBGROUP_KHR, config, local_size);
        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = VkShaderObj(*m_device, source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, target_env);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.flags = flags;
        pipe.cp_ci_.stage.pNext = p_next;
        pipe.CreateComputePipeline(false);
    };

    // SPIR-V 1.6 with no required subgroup size queries subgroupSize == 0.
    create_pipeline(SPV_ENV_VULKAN_1_3, *varying_config, props11.subgroupSize, 0);

    // Pre-1.6 SPIR-V with a fixed subgroup size queries the device's default subgroup size.
    create_pipeline(SPV_ENV_VULKAN_1_2, *default_config, props11.subgroupSize,
                    VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT);

    // An explicit pipeline required subgroup size takes precedence over the SPIR-V 1.6 rule.
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfo pipeline_required_subgroup_size = vku::InitStructHelper();
    pipeline_required_subgroup_size.requiredSubgroupSize = required_subgroup_size;
    create_pipeline(SPV_ENV_VULKAN_1_3, *required_config, required_subgroup_size, 0, &pipeline_required_subgroup_size);

    // The varying-subgroup flag queries subgroupSize == 0 for pre-1.6 SPIR-V.
    create_pipeline(SPV_ENV_VULKAN_1_2, *varying_config, subgroup_size_control_props.maxSubgroupSize,
                    VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT |
                        VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT);

    const auto create_shader_object = [this](const FloatMatrixConfig& config, uint32_t local_size, VkShaderCreateFlagsEXT flags,
                                             const void* p_next = nullptr) {
        const std::string source = MakeFloatMatrixSource(VK_SCOPE_SUBGROUP_KHR, config, local_size);
        const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, source.c_str(), SPV_ENV_VULKAN_1_2);
        auto create_info = ShaderCreateInfoNoNextStage(spv, VK_SHADER_STAGE_COMPUTE_BIT);
        create_info.flags = flags;
        create_info.pNext = p_next;
        const vkt::Shader shader(*m_device, create_info);
    };

    create_shader_object(*default_config, props11.subgroupSize, 0);

    create_shader_object(*varying_config, subgroup_size_control_props.maxSubgroupSize,
                         VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT | VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);

    VkShaderRequiredSubgroupSizeCreateInfoEXT shader_required_subgroup_size = vku::InitStructHelper();
    shader_required_subgroup_size.requiredSubgroupSize = required_subgroup_size;
    create_shader_object(*required_config, required_subgroup_size, VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT,
                         &shader_required_subgroup_size);
}

TEST_F(PositiveShaderCooperativeMatrix, Properties2FlexibleDimensions) {
    TEST_DESCRIPTION("Validate Properties2EXT granularity matching when cooperativeMatrixFlexibleDimensions is enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixFlexibleDimensions);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    CooperativeMatrixHelper helper(*this);
    VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
    info.scope = VK_SCOPE_SUBGROUP_KHR;
    const auto properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(properties.has_value());

    VkPhysicalDeviceCooperativeMatrix2PropertiesNV cooperative_matrix2_properties = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Properties properties11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(cooperative_matrix2_properties);
    GetPhysicalDeviceProperties2(properties11);
    auto config = FindFlexibleProperties2Config(
        helper, *properties, cooperative_matrix2_properties.cooperativeMatrixFlexibleDimensionsMaxDimension, true);
    if (!config) {
        // The legacy flexible table is allowed to overlap Properties2EXT, so an exclusive tile may not exist.
        config = FindFlexibleProperties2Config(
            helper, *properties, cooperative_matrix2_properties.cooperativeMatrixFlexibleDimensionsMaxDimension, false);
    }
    if (!config) {
        GTEST_SKIP() << "No scalable FP16-input subgroup tile is advertised by Properties2EXT";
    }

    const std::string cs_source = MakeFloatMatrixSource(VK_SCOPE_SUBGROUP_KHR, *config, properties11.subgroupSize);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrix2Capabilities) {
    TEST_DESCRIPTION("Enable and accept the SPV_NV_cooperative_matrix2 capabilities that alias maintenance1 capabilities.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixReductions);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixConversions);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixPerElementOperations);
    RETURN_IF_SKIP(Init());

    const char* capabilities[] = {
        "CooperativeMatrixReductionsNV",
        "CooperativeMatrixConversionsNV",
        "CooperativeMatrixPerElementOperationsNV",
    };
    for (const char* capability : capabilities) {
        const std::string source = MakeCooperativeMatrixCapabilitySpirv(capability, "SPV_NV_cooperative_matrix2");
        const VkShaderObj shader(*m_device, source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    }
}

TEST_F(PositiveShaderCooperativeMatrix, Maintenance1Capabilities) {
    TEST_DESCRIPTION("Enable and accept all SPV_EXT_cooperative_matrix_maintenance1 capabilities.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceCooperativeMatrixMaintenance1FeaturesEXT maintenance1_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance1_features);
    const std::pair<const char*, VkBool32> capabilities[] = {
        {"CooperativeMatrixReductionsEXT", maintenance1_features.cooperativeMatrixReductions},
        {"CooperativeMatrixConversionsEXT", maintenance1_features.cooperativeMatrixConversions},
        {"CooperativeMatrixPerElementOperationsEXT", maintenance1_features.cooperativeMatrixPerElementOperations},
        {"CooperativeMatrixGetCoordinateEXT", maintenance1_features.cooperativeMatrixGetCoordinate},
    };
    if (std::none_of(std::begin(capabilities), std::end(capabilities), [](const auto& capability) { return capability.second; })) {
        GTEST_SKIP() << "No cooperative matrix maintenance1 operation features are supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &maintenance1_features));

    for (const auto& [capability, supported] : capabilities) {
        if (!supported) {
            continue;
        }
        const std::string source = MakeCooperativeMatrixCapabilitySpirv(capability, "SPV_EXT_cooperative_matrix_maintenance1");
        const VkShaderObj shader(*m_device, source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    }
}

TEST_F(PositiveShaderCooperativeMatrix, Maintenance1CapabilityAliasesEnabledByCooperativeMatrix2Features) {
    TEST_DESCRIPTION("The promoted EXT capability aliases can be enabled by the corresponding NV feature bits.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixReductions);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixPerElementOperations);
    RETURN_IF_SKIP(Init());

    const char* capabilities[] = {
        "CooperativeMatrixReductionsEXT",
        "CooperativeMatrixPerElementOperationsEXT",
    };
    for (const char* capability : capabilities) {
        const std::string source = MakeCooperativeMatrixCapabilitySpirv(capability, "SPV_EXT_cooperative_matrix_maintenance1");
        const VkShaderObj shader(*m_device, source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    }
}

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrix2CapabilityAliasesEnabledByMaintenance1Features) {
    TEST_DESCRIPTION("The NV spellings of promoted capability aliases can be enabled by the corresponding EXT feature bits.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceCooperativeMatrixMaintenance1FeaturesEXT maintenance1_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance1_features);
    if (!maintenance1_features.cooperativeMatrixReductions || !maintenance1_features.cooperativeMatrixPerElementOperations) {
        GTEST_SKIP() << "The maintenance1 reduction and per-element features are not both supported";
    }
    maintenance1_features.cooperativeMatrixProperties2 = VK_FALSE;
    maintenance1_features.cooperativeMatrixConversions = VK_FALSE;
    maintenance1_features.cooperativeMatrixGetCoordinate = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &maintenance1_features));

    const char* capabilities[] = {
        "CooperativeMatrixReductionsNV",
        "CooperativeMatrixPerElementOperationsNV",
    };
    for (const char* capability : capabilities) {
        const std::string source = MakeCooperativeMatrixCapabilitySpirv(capability, "SPV_NV_cooperative_matrix2");
        const VkShaderObj shader(*m_device, source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    }
}

TEST_F(PositiveShaderCooperativeMatrix, Maintenance1Glsl) {
    TEST_DESCRIPTION("Compile and validate GL_EXT_cooperative_matrix_maintenance1 GLSL.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_COOPERATIVE_MATRIX_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixProperties2);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixGetCoordinate);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR());

    CooperativeMatrixHelper helper(*this);
    VkPhysicalDeviceCooperativeMatrixInfo2EXT info = vku::InitStructHelper();
    info.scope = VK_SCOPE_SUBGROUP_KHR;
    const auto properties = QueryCooperativeMatrixProperties2(Gpu(), info);
    ASSERT_TRUE(properties.has_value());
    const auto config = FindExactProperties2Config(helper, VK_SCOPE_SUBGROUP_KHR, *properties, false);
    if (!config) {
        GTEST_SKIP() << "The subgroupSize 0 query advertises no FP16-input tile";
    }

    VkPhysicalDeviceVulkan11Properties properties11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(properties11);
    std::ostringstream source;
    source << R"glsl(
        #version 450
        #pragma use_vulkan_memory_model
        #extension GL_KHR_cooperative_matrix : require
        #extension GL_KHR_shader_subgroup_basic : require
        #extension GL_KHR_memory_scope_semantics : require
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
        #extension GL_EXT_cooperative_matrix_maintenance1 : require

        layout(local_size_x = )glsl"
           << properties11.subgroupSize << R"glsl() in;

        void main() {
            coopmat<float16_t, gl_ScopeSubgroup, )glsl"
           << config->m << ", " << config->k << R"glsl(, gl_MatrixUseA> matrix_a;
            uvec2 coordinate = coopMatGetCoordinateEXT(matrix_a, 0);
        }
    )glsl";

    const std::string cs_source = source.str();
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.CreateComputePipeline();
}
