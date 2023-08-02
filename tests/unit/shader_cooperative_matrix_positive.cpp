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

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrixNV) {
    TEST_DESCRIPTION("Test VK_NV_cooperative_matrix.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto cooperative_matrix_features = LvlInitStruct<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(&float16_features);
    auto memory_model_features = LvlInitStruct<VkPhysicalDeviceVulkanMemoryModelFeaturesKHR>(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &memory_model_features));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_cooperative_matrix : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        layout(constant_id = 0) const uint C0 = 1;
        layout(constant_id = 1) const uint C1 = 1;
        void main() {
           // Bad type
           fcoopmatNV<16, gl_ScopeSubgroup, 3, 5> badSize = fcoopmatNV<16, gl_ScopeSubgroup, 3, 5>(float16_t(0.0));
           // Not a valid multiply when C0 != C1
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> A;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> B;
           fcoopmatNV<16, gl_ScopeSubgroup, C0, C1> C;
           coopMatMulAddNV(A, B, C);
        }
    )glsl";

    const uint32_t specData[] = {
        16,
        8,
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
    pipe.InitInfo();
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo);
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveShaderCooperativeMatrix, CooperativeMatrixKHR) {
    TEST_DESCRIPTION("Test VK_KHR_cooperative_matrix.");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto storage16_features = LvlInitStruct<VkPhysicalDevice16BitStorageFeatures>(&float16_features);
    auto cooperative_matrix_features = LvlInitStruct<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>(&storage16_features);
    auto memory_model_features = LvlInitStruct<VkPhysicalDeviceVulkanMemoryModelFeaturesKHR>(&cooperative_matrix_features);
    GetPhysicalDeviceFeatures2(memory_model_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &memory_model_features));

    VkCooperativeMatrixPropertiesKHR props = LvlInitStruct<VkCooperativeMatrixPropertiesKHR>();
    uint32_t props_count = 1;
    VkResult props_result = vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(gpu(), &props_count, &props);

    if (props_result != VK_SUCCESS && props_result != VK_INCOMPLETE && props_count != 1) {
        GTEST_SKIP() << "GetPhysicalDeviceCooperativeMatrixPropertiesKHR does not report any matrices supported";
    }

    const VkSampler *ptr = nullptr;
    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, ptr},
    };
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

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

    auto replace = [](std::string &str, const std::string &from, const std::string &to) {
        size_t pos;
        while ((pos = str.find(from)) != std::string::npos) str.replace(pos, from.length(), to);
    };
    const auto get_type_name = [](const VkComponentTypeKHR &type) {
        const struct {
            VkComponentTypeKHR type;
            const char *name;
        } cvt[] = {
            {VK_COMPONENT_TYPE_FLOAT16_KHR, "float16_t"},
            {VK_COMPONENT_TYPE_FLOAT32_KHR, "float32_t"},
            {VK_COMPONENT_TYPE_FLOAT64_KHR, "float64_t"},
            {VK_COMPONENT_TYPE_SINT8_KHR,   "int8_t"   },
            {VK_COMPONENT_TYPE_SINT16_KHR,  "int16_t"  },
            {VK_COMPONENT_TYPE_SINT32_KHR,  "int32_t"  },
            {VK_COMPONENT_TYPE_SINT64_KHR,  "int64_t"  },
            {VK_COMPONENT_TYPE_UINT8_KHR,   "uint8_t"  },
            {VK_COMPONENT_TYPE_UINT16_KHR,  "uint16_t" },
            {VK_COMPONENT_TYPE_UINT32_KHR,  "uint32_t" },
            {VK_COMPONENT_TYPE_UINT64_KHR,  "uint64_t" },
        };
        for (const auto &x: cvt)
            if (x.type == type) return x.name;
        return "";
    };
    replace(css, "%M%", std::to_string(props.MSize));
    replace(css, "%N%", std::to_string(props.NSize));
    replace(css, "%K%", std::to_string(props.KSize));
    replace(css, "%type_A%", get_type_name(props.AType));
    replace(css, "%type_B%", get_type_name(props.BType));
    replace(css, "%type_C%", get_type_name(props.CType));
    replace(css, "%type_R%", get_type_name(props.ResultType));

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, css.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr);
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&dsl});
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}
