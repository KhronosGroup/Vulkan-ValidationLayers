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

class NegativeSubgroup : public VkLayerTest {};

TEST_F(NegativeSubgroup, Properties) {
    TEST_DESCRIPTION(
        "Test shader validation support for subgroup VkPhysicalDeviceSubgroupProperties such as supportedStages, and "
        "supportedOperations, quadOperationsInAllStages.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    VkPhysicalDeviceFeatures features{};
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    if (features.vertexPipelineStoresAndAtomics == VK_FALSE) {
        GTEST_SKIP() << "vertexPipelineStoresAndAtomics not supported";
    }

    features = {};
    features.vertexPipelineStoresAndAtomics = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required.";
    }

    // Don't enable the extension on purpose
    const bool extension_support_partitioned =
        DeviceExtensionSupported(gpu(), nullptr, VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Gather all aspects supported
    VkPhysicalDeviceSubgroupProperties subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
    GetPhysicalDeviceProperties2(subgroup_prop);
    VkSubgroupFeatureFlags subgroup_operations = subgroup_prop.supportedOperations;
    const bool feature_support_basic = ((subgroup_operations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0);
    const bool feature_support_vote = ((subgroup_operations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0);
    const bool feature_support_arithmetic = ((subgroup_operations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0);
    const bool feature_support_ballot = ((subgroup_operations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0);
    const bool feature_support_shuffle = ((subgroup_operations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0);
    const bool feature_support_relative = ((subgroup_operations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0);
    const bool feature_support_culstered = ((subgroup_operations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0);
    const bool feature_support_quad = ((subgroup_operations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0);
    const bool feature_support_partitioned = ((subgroup_operations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0);
    const bool vertex_support = ((subgroup_prop.supportedStages & VK_SHADER_STAGE_VERTEX_BIT) != 0);
    const bool vertex_quad_support = (subgroup_prop.quadOperationsInAllStages == VK_TRUE);

    std::string vsSource;
    std::vector<const char *> errors;
    // There is no 'supportedOperations' check due to it would be redundant to the Capability check done first in VUID 01091 since
    // each 'supportedOperations' flag is 1:1 map to a SPIR-V Capability
    const char *operation_vuid = "VUID-VkShaderModuleCreateInfo-pCode-08740";
    const char *stage_vuid = "VUID-RuntimeSpirv-None-06343";
    const char *quad_vuid = "VUID-RuntimeSpirv-None-06342";

    // Same pipeline creation for each subgroup test
    auto info_override = [&](CreatePipelineHelper &info) {
        info.vs_.reset(new VkShaderObj(this, vsSource.c_str(), VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1));
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
        info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    // Basic
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_basic: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               if (subgroupElect()) { ssbo.x += 2.0; }
               gl_Position = vec4(ssbo.x);
            }
        )glsl";
        errors.clear();
        if (feature_support_basic == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Vote
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_vote: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               if (subgroupAll(ssbo.y == 0)) { ssbo.x += 2.0; }
               gl_Position = vec4(ssbo.x);
            }
        )glsl";
        errors.clear();
        if (feature_support_vote == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Arithmetic
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_arithmetic: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupMax(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_arithmetic == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Ballot
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_ballot: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupBroadcastFirst(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_ballot == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Shuffle
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_shuffle: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupShuffle(ssbo.x, 1);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_shuffle == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Shuffle Relative
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_shuffle_relative: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupShuffleUp(ssbo.x, 1);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_relative == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Clustered
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_clustered: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupClusteredAdd(ssbo.x, 2);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_culstered == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Quad
    {
        vsSource = R"glsl(
            #version 450
            #extension GL_KHR_shader_subgroup_quad: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               float z = subgroupQuadSwapHorizontal(ssbo.x);
               gl_Position = vec4(z);
            }
        )glsl";
        errors.clear();
        if (feature_support_quad == false) {
            errors.push_back(operation_vuid);
        }
        if (vertex_quad_support == false) {
            errors.push_back(quad_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }

    // Partitoned
    if (extension_support_partitioned) {
        vsSource = R"glsl(
            #version 450
            #extension GL_NV_shader_subgroup_partitioned: enable
            layout(set = 0, binding = 0) buffer StorageBuffer { float x; uint y; } ssbo;
            void main(){
               uvec4 a = subgroupPartitionNV(ssbo.x); // forces OpGroupNonUniformPartitionNV
               gl_Position = vec4(float(a.x));
            }
        )glsl";
        errors.clear();
        // Extension not enabled on purpose if supported
        errors.push_back("VUID-VkShaderModuleCreateInfo-pCode-08742");
        if (feature_support_partitioned == false) {
            // errors.push_back(operation_vuid);
        }
        if (vertex_support == false) {
            errors.push_back(stage_vuid);
        }
        CreatePipelineHelper::OneshotTest(*this, info_override, kErrorBit, errors);
    }
}

TEST_F(NegativeSubgroup, Features) {
    TEST_DESCRIPTION("Test that the minimum required functionality for subgroups is present.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required.";
    }

    VkPhysicalDeviceSubgroupProperties subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
    GetPhysicalDeviceProperties2(subgroup_prop);

    auto queue_family_properties = m_device->phy().queue_properties();

    bool foundGraphics = false;
    bool foundCompute = false;

    for (auto queue_family : queue_family_properties) {
        if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            foundCompute = true;
            break;
        }

        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            foundGraphics = true;
        }
    }

    if (!(foundGraphics || foundCompute)) return;

    ASSERT_GE(subgroup_prop.subgroupSize, 1u);

    if (foundCompute) {
        ASSERT_TRUE(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT);
    }

    ASSERT_TRUE(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT);
}

TEST_F(NegativeSubgroup, ExtendedTypesEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = LvlInitStruct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_types_features);

    VkPhysicalDeviceSubgroupProperties subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
    GetPhysicalDeviceProperties2(subgroup_prop);
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) || !float16_features.shaderFloat16 ||
        !extended_types_features.shaderSubgroupExtendedTypes) {
        GTEST_SKIP() << "Required features not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    pipe.CreateComputePipeline();
}

TEST_F(NegativeSubgroup, ExtendedTypesDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto extended_types_features = LvlInitStruct<VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR>(&float16_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_types_features);

    VkPhysicalDeviceSubgroupProperties subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
    GetPhysicalDeviceProperties2(subgroup_prop);
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) || !float16_features.shaderFloat16) {
        GTEST_SKIP() << "Required features not supported";
    }

    // Disabled extended types support, and expect an error
    extended_types_features.shaderSubgroupExtendedTypes = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const VkDescriptorSetLayoutObj dsl(m_device, bindings);
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-None-06275");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSubgroup, PipelineSubgroupSizeControl) {
    TEST_DESCRIPTION("Test Subgroub Size Control");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    GetPhysicalDeviceFeatures2(sscf);
    if (sscf.subgroupSizeControl == VK_FALSE || sscf.computeFullSubgroups == VK_FALSE) {
        GTEST_SKIP() << "Required features are not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sscf));

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props11 = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>(&subgroup_properties);
    GetPhysicalDeviceProperties2(props11);

    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "Required shader stage not present in requiredSubgroupSizeStages";
    }

    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    {
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.pNext = &subgroup_size_control;
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02754");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    if (subgroup_properties.maxSubgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x = )glsl";
        csSource << subgroup_properties.maxSubgroupSize + 1;
        csSource << R"glsl() in;
        void main() {}
        )glsl";
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT |
                                         VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02758");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    if (props11.subgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x = )glsl";
        csSource << props11.subgroupSize + 1;
        csSource << R"glsl() in;
        void main() {}
        )glsl";
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02759");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    // if on a device with the min and max the same, there is no way to isolate this VUID
    // Intel integrated GPU normally have a min of 8 and max of 32
    if (subgroup_properties.minSubgroupSize >= 8 && subgroup_properties.minSubgroupSize < 16 &&
        subgroup_properties.maxSubgroupSize >= 16) {
        subgroup_size_control.requiredSubgroupSize = 10;  // non-power of 2
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.pNext = &subgroup_size_control;
        cs_pipeline.cp_ci_.stage.flags = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02760");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    if (subgroup_properties.minSubgroupSize > 1) {
        subgroup_size_control.requiredSubgroupSize = 1;  // below min
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.pNext = &subgroup_size_control;
        cs_pipeline.cp_ci_.stage.flags = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02761");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }

    {
        subgroup_size_control.requiredSubgroupSize = subgroup_properties.maxSubgroupSize * 2;  // above max
        CreateComputePipelineHelper cs_pipeline(*this);
        cs_pipeline.InitInfo();
        cs_pipeline.InitState();
        cs_pipeline.LateBindPipelineInfo();
        cs_pipeline.cp_ci_.stage.pNext = &subgroup_size_control;
        cs_pipeline.cp_ci_.stage.flags = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02762");
        cs_pipeline.CreateComputePipeline(true, false);  // need false to prevent late binding
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSubgroup, SubgroupSizeControlFeaturesNotEnabled) {
    TEST_DESCRIPTION("Use subgroup size control features when they are not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    sscf.subgroupSizeControl = VK_FALSE;
    sscf.computeFullSubgroups = VK_FALSE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&sscf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    VkPhysicalDeviceVulkan11Properties props11 = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>();
    GetPhysicalDeviceProperties2(props11);

    if (props11.subgroupSize == 0) {
        GTEST_SKIP() << "subgroupSize is 0, skipping test.";
    }

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"(
        #version 450
        layout(local_size_x = )";
    csSource << props11.subgroupSize;
    csSource << R"() in;
        void main(){
        }
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02784");
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();

    pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-flags-02785");
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSubgroup, SubgroupUniformControlFlow) {
    TEST_DESCRIPTION("Test SubgroupUniformControlFlow spirv execution mode");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    auto ssucff = LvlInitStruct<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR>();
    ssucff.shaderSubgroupUniformControlFlow = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ssucff));

    const char *source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_subgroup_uniform_control_flow"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main SubgroupUniformControlFlowKHR

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4

               ; Annotations
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSubgroup, ComputeLocalWorkgroupSize) {
    TEST_DESCRIPTION("Test size of local workgroud with requiredSubgroupSize.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto sscf = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(sscf);
    if (sscf.subgroupSizeControl == VK_FALSE || sscf.computeFullSubgroups == VK_FALSE || sscf.subgroupSizeControl == VK_FALSE) {
        GTEST_SKIP() << "Required features are not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&subgroup_properties);
    GetPhysicalDeviceProperties2(props);

    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "Required shader stage not present in requiredSubgroupSizeStages";
    }

    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    uint32_t size = static_cast<uint32_t>(
        std::ceil(std::sqrt(subgroup_size_control.requiredSubgroupSize * subgroup_properties.maxComputeWorkgroupSubgroups)));

    if (size <= 1024) {
        std::stringstream csSource;
        csSource << R"glsl(
        #version 450
        layout(local_size_x=
    )glsl";
        csSource << size;
        csSource << R"glsl(, local_size_y=
    )glsl";
        csSource << size;
        csSource << R"glsl(, local_size_z=2) in;
        void main(){
           if (gl_GlobalInvocationID.x >= 0) { return; }
        }
    )glsl";

        CreateComputePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        pipe.InitState();
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &subgroup_size_control;
        if (size * size * 2 > m_device->props.limits.maxComputeWorkGroupInvocations) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-x-06432");
        }
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02756");
        pipe.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();
    }

    if (subgroup_properties.maxSubgroupSize > 1) {
        std::stringstream csSource;
        csSource << R"glsl(
            #version 450
            layout(local_size_x=
        )glsl";
        csSource << subgroup_properties.maxSubgroupSize + 1;
        csSource << R"glsl(, local_size_y=1, local_size_z=1) in;
            void main(){
            if (gl_GlobalInvocationID.x >= 0) { return; }
            }
        )glsl";

        CreateComputePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
        pipe.InitState();
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &subgroup_size_control;
        pipe.cp_ci_.stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02757");
        pipe.CreateComputePipeline(true, false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSubgroup, SubgroupSizeControlFeature) {
    TEST_DESCRIPTION("Test using subgroupSizeControl feature when it's not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto subgroup_properties = LvlInitStruct<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT>();
    auto props = LvlInitStruct<VkPhysicalDeviceProperties2>(&subgroup_properties);
    GetPhysicalDeviceProperties2(props);
    auto subgroup_size_control = LvlInitStruct<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>();
    subgroup_size_control.requiredSubgroupSize = subgroup_properties.minSubgroupSize;

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.pNext = &subgroup_size_control;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02755");
    if ((subgroup_properties.requiredSubgroupSizeStages & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pNext-02755");
    }
    pipe.CreateComputePipeline(true, false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSubgroup, CooperativeMatrixNV) {
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
    auto features2 = GetPhysicalDeviceFeatures2(memory_model_features);
    if (memory_model_features.vulkanMemoryModel == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModel feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specInfo));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06316");
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddNV-06317");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06719");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}
