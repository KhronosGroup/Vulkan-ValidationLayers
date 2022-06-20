/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

//
// POSITIVE VALIDATION TESTS
//
// These tests do not expect to encounter ANY validation errors pass only if this is true

TEST_F(VkPositiveLayerTest, ShaderRelaxedBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to relax block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires relaxed block layout.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // The Relaxed Block Layout extension was promoted to core in 1.1.
    // Go ahead and check for it and turn it on in case a 1.0 device has it.
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix, VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring relaxed layout.
    // Without relaxed layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";
    m_errorMonitor->ExpectSuccess();
    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderUboStd430Layout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires UBO std430 layout.");
    // Enable req'd extensions
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for the UBO standard block layout extension and turn it on if it's available
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto uniform_buffer_standard_layout_features = LvlInitStruct<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR>(NULL);
    uniform_buffer_standard_layout_features.uniformBufferStandardLayout = VK_TRUE;
    auto query_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    auto set_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring std430 in a uniform buffer.
    // Without uniform buffer standard layout, we would expect a message like:
    // "Structure id 3 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 0 is an array
    // with stride 4 not satisfying alignment to 16"

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 460
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %foo 0 Offset 0
               OpDecorate %foo Block
               OpDecorate %b DescriptorSet 0
               OpDecorate %b Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
        %foo = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
          %b = OpVariable %_ptr_Uniform_foo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->ExpectSuccess();
    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderScalarBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires scalar block layout.");
    // Enable req'd extensions
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for the Scalar Block Layout extension and turn it on if it's available
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto scalar_block_features = LvlInitStruct<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT>(NULL);
    auto query_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&scalar_block_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    if (scalar_block_features.scalarBlockLayout != VK_TRUE) {
        printf("%s scalarBlockLayout feature not supported\n", kSkipPrefix);
        return;
    }

    auto set_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&scalar_block_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring scalar layout.
    // Without scalar layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpMemberDecorate %S 2 Offset 8
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->ExpectSuccess();
    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // More background: When workgroupMemoryExplicitLayout is enabled and there are 2 or more structs, the
    // maxComputeSharedMemorySize is the MAX of the structs since they share the same WorkGroup memory. Test makes sure validation
    // is not doing an ADD and correctly doing a MAX operation in this case.

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto explicit_layout_features = LvlInitStruct<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&explicit_layout_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &explicit_layout_features));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        printf("%s workgroupMemoryExplicitLayout feature not supported.\n", kSkipPrefix);
        return;
    }

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[)glsl";
    csSource << (max_shared_vec4 - 2);
    csSource << R"glsl(];
            vec4 x2;
        };

        shared Y {
            int y1[)glsl";
    csSource << (max_shared_ints - 8);
    csSource << R"glsl(];
            int y2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2));
    pipe.InitState();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizePrecedenceOverLocalSize) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->props.limits.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->props.limits.maxComputeWorkGroupSize[2];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize )";
    spv_source << std::to_string(x_size_limit + 1) << " " << std::to_string(y_size_limit + 1) << " "
               << std::to_string(z_size_limit + 1);
    spv_source << R"(
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeSpecConstantUnder) {
    TEST_DESCRIPTION("Make sure spec constants get applied to to be under maxComputeWorkGroupSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %7 SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %7 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                         SPV_SOURCE_ASM, &specialization_info));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeLocalSizeIdSpecConstant) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit with spec constants");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint 32
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = x_size_limit - 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 18;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                         SPV_SOURCE_ASM, &specialization_info));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizePrecedenceOverLocalSizeId) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSizeId");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %uint_1
               OpSource GLSL 450
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
               OpDecorate %spec_x SpecId 18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
}

TEST_F(VkPositiveLayerTest, ShaderNonSemanticInfo) {
    // This is a positive test, no errors expected
    // Verifies the ability to use non-semantic extended instruction sets when the extension is enabled
    TEST_DESCRIPTION("Create a shader that uses SPV_KHR_non_semantic_info.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for the extension and turn it on if it's available
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // compute shader using a non-semantic extended instruction set.

    const char *spv_source = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

    m_errorMonitor->ExpectSuccess();
    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SpirvGroupDecorations) {
    TEST_DESCRIPTION("Test shader validation support for group decorations.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string spv_source = R"(
              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpName %main "main"
               OpName %gl_GlobalInvocationID "gl_GlobalInvocationID"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %_runtimearr_float ArrayStride 4
               OpDecorate %4 BufferBlock
               OpDecorate %5 Offset 0
          %4 = OpDecorationGroup
          %5 = OpDecorationGroup
               OpGroupDecorate %4 %_struct_6 %_struct_7 %_struct_8 %_struct_9 %_struct_10 %_struct_11
               OpGroupMemberDecorate %5 %_struct_6 0 %_struct_7 0 %_struct_8 0 %_struct_9 0 %_struct_10 0 %_struct_11 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 NonWritable
               OpDecorate %13 Restrict
         %14 = OpDecorationGroup
         %12 = OpDecorationGroup
         %13 = OpDecorationGroup
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpDecorate %15 DescriptorSet 0
               OpDecorate %15 Binding 5
               OpGroupDecorate %14 %16
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpGroupDecorate %12 %17
               OpDecorate %17 Binding 1
               OpGroupDecorate %13 %18 %19
               OpDecorate %18 Binding 2
               OpDecorate %19 Binding 3
               OpGroupDecorate %14 %20
               OpGroupDecorate %12 %20
               OpGroupDecorate %13 %20
               OpDecorate %20 Binding 4
       %bool = OpTypeBool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %v3uint = OpTypeVector %uint 3
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_runtimearr_int = OpTypeRuntimeArray %int
%_runtimearr_float = OpTypeRuntimeArray %float
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
      %int_0 = OpConstant %int 0
  %_struct_6 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
         %15 = OpVariable %_ptr_Uniform__struct_6 Uniform
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
         %16 = OpVariable %_ptr_Uniform__struct_7 Uniform
  %_struct_8 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_8 = OpTypePointer Uniform %_struct_8
         %17 = OpVariable %_ptr_Uniform__struct_8 Uniform
  %_struct_9 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
         %18 = OpVariable %_ptr_Uniform__struct_9 Uniform
 %_struct_10 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %19 = OpVariable %_ptr_Uniform__struct_10 Uniform
 %_struct_11 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_11 = OpTypePointer Uniform %_struct_11
         %20 = OpVariable %_ptr_Uniform__struct_11 Uniform
       %main = OpFunction %void None %23
         %40 = OpLabel
         %41 = OpLoad %v3uint %gl_GlobalInvocationID
         %42 = OpCompositeExtract %uint %41 0
         %43 = OpAccessChain %_ptr_Uniform_float %16 %int_0 %42
         %44 = OpAccessChain %_ptr_Uniform_float %17 %int_0 %42
         %45 = OpAccessChain %_ptr_Uniform_float %18 %int_0 %42
         %46 = OpAccessChain %_ptr_Uniform_float %19 %int_0 %42
         %47 = OpAccessChain %_ptr_Uniform_float %20 %int_0 %42
         %48 = OpAccessChain %_ptr_Uniform_float %15 %int_0 %42
         %49 = OpLoad %float %43
         %50 = OpLoad %float %44
         %51 = OpLoad %float %45
         %52 = OpLoad %float %46
         %53 = OpLoad %float %47
         %54 = OpFAdd %float %49 %50
         %55 = OpFAdd %float %54 %51
         %56 = OpFAdd %float %55 %52
         %57 = OpFAdd %float %56 %53
               OpStore %48 %57
               OpReturn
               OpFunctionEnd
)";

    // CreateDescriptorSetLayout
    VkDescriptorSetLayoutBinding dslb[6] = {};
    size_t dslb_size = size(dslb);
    for (size_t i = 0; i < dslb_size; i++) {
        dslb[i].binding = i;
        dslb[i].descriptorCount = 1;
        dslb[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[i].pImmutableSamplers = NULL;
        dslb[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ALL;
    }
    if (m_device->props.limits.maxPerStageDescriptorStorageBuffers < dslb_size) {
        printf("%sNeeded storage buffer bindings exceeds this devices limit.  Skipping tests.\n", kSkipPrefix);
        return;
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(dslb_size);
    memcpy(pipe.dsl_bindings_.data(), dslb, dslb_size * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(this, bindStateMinimalShaderText, VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension1of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 1 of 2");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        printf("%s Device does not support multiViewport, test skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension2of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 2 of 2");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    // Need to use SPV_EXT_shader_viewport_index_layer
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
        printf("%s Extension %s not supported, skipping this pass. \n", kSkipPrefix,
               VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        printf("%s Device does not support multiViewport, test skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CreatePipelineFragmentOutputNotWrittenButMasked) {
    TEST_DESCRIPTION(
        "Test that no error is produced when the fragment shader fails to declare an output, but the corresponding attachment's "
        "write mask is 0.");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *fsSource = R"glsl(
        #version 450
        void main() {}
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    /* set up CB 0, not written, but also masked */
    pipe.AddDefaultColorAttachment(0);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, PointSizeWriteInFunction) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize in vertex shader function.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, PointSizeGeomShaderSuccess) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, and write in the final geometry stage.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        printf("%s Device does not support the required geometry shader features; skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and writing to it
    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, bindStateGeomPointSizeShaderText, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
    // Set Input Assembly to TOPOLOGY POINT LIST
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, LoosePointSizeWrite) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize outside of a structure.");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    const char *LoosePointSizeWrite = R"(
                                       OpCapability Shader
                                  %1 = OpExtInstImport "GLSL.std.450"
                                       OpMemoryModel Logical GLSL450
                                       OpEntryPoint Vertex %main "main" %glposition %glpointsize %gl_VertexIndex
                                       OpSource GLSL 450
                                       OpName %main "main"
                                       OpName %vertices "vertices"
                                       OpName %glposition "glposition"
                                       OpName %glpointsize "glpointsize"
                                       OpName %gl_VertexIndex "gl_VertexIndex"
                                       OpDecorate %glposition BuiltIn Position
                                       OpDecorate %glpointsize BuiltIn PointSize
                                       OpDecorate %gl_VertexIndex BuiltIn VertexIndex
                               %void = OpTypeVoid
                                  %3 = OpTypeFunction %void
                              %float = OpTypeFloat 32
                            %v2float = OpTypeVector %float 2
                               %uint = OpTypeInt 32 0
                             %uint_3 = OpConstant %uint 3
                %_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %_ptr_Private__arr_v2float_uint_3 = OpTypePointer Private %_arr_v2float_uint_3
                           %vertices = OpVariable %_ptr_Private__arr_v2float_uint_3 Private
                                %int = OpTypeInt 32 1
                              %int_0 = OpConstant %int 0
                           %float_n1 = OpConstant %float -1
                                 %16 = OpConstantComposite %v2float %float_n1 %float_n1
               %_ptr_Private_v2float = OpTypePointer Private %v2float
                              %int_1 = OpConstant %int 1
                            %float_1 = OpConstant %float 1
                                 %21 = OpConstantComposite %v2float %float_1 %float_n1
                              %int_2 = OpConstant %int 2
                            %float_0 = OpConstant %float 0
                                 %25 = OpConstantComposite %v2float %float_0 %float_1
                            %v4float = OpTypeVector %float 4
            %_ptr_Output_gl_Position = OpTypePointer Output %v4float
                         %glposition = OpVariable %_ptr_Output_gl_Position Output
           %_ptr_Output_gl_PointSize = OpTypePointer Output %float
                        %glpointsize = OpVariable %_ptr_Output_gl_PointSize Output
                     %_ptr_Input_int = OpTypePointer Input %int
                     %gl_VertexIndex = OpVariable %_ptr_Input_int Input
                              %int_3 = OpConstant %int 3
                %_ptr_Output_v4float = OpTypePointer Output %v4float
                  %_ptr_Output_float = OpTypePointer Output %float
                               %main = OpFunction %void None %3
                                  %5 = OpLabel
                                 %18 = OpAccessChain %_ptr_Private_v2float %vertices %int_0
                                       OpStore %18 %16
                                 %22 = OpAccessChain %_ptr_Private_v2float %vertices %int_1
                                       OpStore %22 %21
                                 %26 = OpAccessChain %_ptr_Private_v2float %vertices %int_2
                                       OpStore %26 %25
                                 %33 = OpLoad %int %gl_VertexIndex
                                 %35 = OpSMod %int %33 %int_3
                                 %36 = OpAccessChain %_ptr_Private_v2float %vertices %35
                                 %37 = OpLoad %v2float %36
                                 %38 = OpCompositeExtract %float %37 0
                                 %39 = OpCompositeExtract %float %37 1
                                 %40 = OpCompositeConstruct %v4float %38 %39 %float_0 %float_1
                                 %42 = OpAccessChain %_ptr_Output_v4float %glposition
                                       OpStore %42 %40
                                       OpStore %glpointsize %float_1
                                       OpReturn
                                       OpFunctionEnd
        )";

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, LoosePointSizeWrite, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        // Set Input Assembly to TOPOLOGY POINT LIST
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.0 before shaderDrawParameters feature was added");
    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_0);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "requires Vulkan 1.0 exactly";
    }

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
    }

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithoutFeature11) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.1 using the extension");
    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry(false)) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
    }

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.2 with feature bit enabled");
    m_errorMonitor->ExpectSuccess();

    // use 1.2 to get the feature bit in VkPhysicalDeviceVulkan11Features
    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Devsim won't read in values like maxDescriptorSetUpdateAfterBindUniformBuffers which cause OneshotTest to fail pipeline
    // layout creation if using 1.2 devsim as it enables VK_EXT_descriptor_indexing
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%sNot suppored by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    features11.shaderDrawParameters = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);

    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (features11.shaderDrawParameters != VK_TRUE) {
        printf("shaderDrawParameters not supported, skipping test\n");
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry(false)) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit, "", true);
    }

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ShaderImageAtomicInt64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto image_atomic_int64_features = lvl_init_struct<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&image_atomic_int64_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (features2.features.shaderInt64 == VK_FALSE) {
        printf("%s shaderInt64 feature not supported, skipping tests\n", kSkipPrefix);
        return;
    } else if (image_atomic_int64_features.shaderImageInt64Atomics == VK_FALSE) {
        printf("%s shaderImageInt64Atomics feature not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required for SPIR-V 1.3, skipping test.\n", kSkipPrefix);
        return;
    }

    // clang-format off
    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_.reset(new VkShaderObj(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    //  shaderImageInt64Atomics
    current_shader = cs_image_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    current_shader = cs_image_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    current_shader = cs_image_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

    current_shader = cs_image_add.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
}

TEST_F(VkPositiveLayerTest, ShaderAtomicFloat) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto atomic_float_features = lvl_init_struct<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&atomic_float_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required for SPIR-V 1.3, skipping test.\n", kSkipPrefix);
        return;
    }

    // clang-format off
    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_add = cs_32_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_store = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_32_add = cs_32_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_store = cs_32_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_add = cs_64_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_store = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_64_add = cs_64_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_store = cs_64_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_.reset(new VkShaderObj(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1));
        helper.dsl_bindings_ = current_bindings;
    };

    if (atomic_float_features.shaderBufferFloat32Atomics == VK_TRUE) {
        current_shader = cs_buffer_float_32_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_buffer_float_32_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_buffer_float_32_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (atomic_float_features.shaderBufferFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_buffer_float_32_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float_features.shaderBufferFloat64Atomics == VK_TRUE) {
            current_shader = cs_buffer_float_64_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_64_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_64_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float_features.shaderBufferFloat64AtomicAdd == VK_TRUE) {
            current_shader = cs_buffer_float_64_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }
    }

    if (atomic_float_features.shaderSharedFloat32Atomics == VK_TRUE) {
        current_shader = cs_shared_float_32_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_shared_float_32_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_shared_float_32_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (atomic_float_features.shaderSharedFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_shared_float_32_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float_features.shaderSharedFloat64Atomics == VK_TRUE) {
            current_shader = cs_shared_float_64_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_64_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_64_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float_features.shaderSharedFloat64AtomicAdd == VK_TRUE) {
            current_shader = cs_shared_float_64_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    if (atomic_float_features.shaderImageFloat32Atomics == VK_TRUE) {
        current_shader = cs_image_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_image_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_image_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (atomic_float_features.shaderImageFloat32AtomicAdd == VK_TRUE) {
        current_shader = cs_image_add.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }
}

TEST_F(VkPositiveLayerTest, ShaderAtomicFloat2) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
        return;
    }

    auto atomic_float_features = lvl_init_struct<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>();
    auto atomic_float2_features = lvl_init_struct<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(&atomic_float_features);
    auto float16int8_features = LvlInitStruct<VkPhysicalDeviceShaderFloat16Int8Features>(&atomic_float2_features);
    auto storage_16_bit_features = LvlInitStruct<VkPhysicalDevice16BitStorageFeatures>(&float16int8_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&storage_16_bit_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // clang-format off
    std::string cs_16_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
    )glsl";

     std::string cs_buffer_float_16_add = cs_16_base + R"glsl(
           atomicAdd(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_store = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_buffer_float_16_min = cs_16_base + R"glsl(
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_max = cs_16_base + R"glsl(
           atomicMax(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_add = cs_16_base + R"glsl(
           y = atomicAdd(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_store = cs_16_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_shared_float_16_min = cs_16_base + R"glsl(
           y = atomicMin(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_max = cs_16_base + R"glsl(
           y = atomicMax(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_min = cs_32_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_max = cs_32_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_32_min = cs_32_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_max = cs_32_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_min = cs_64_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_max = cs_64_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_64_min = cs_64_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_max = cs_64_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_image_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_32_min = cs_image_32_base + R"glsl(
           y = imageAtomicMin(z, ivec2(1, 1), y);
        }
    )glsl";

    std::string cs_image_32_max = cs_image_32_base + R"glsl(
           y = imageAtomicMax(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // This could get triggered in the event that the shader fails to compile
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01091");
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ =
            VkShaderObj::CreateFromGLSL(*this, VK_SHADER_STAGE_COMPUTE_BIT, current_shader, "main", nullptr, SPV_ENV_VULKAN_1_1);
        // Skip the test if shader failed to compile
        helper.override_skip_ = !static_cast<bool>(helper.cs_);
        helper.dsl_bindings_ = current_bindings;
    };

    if (float16int8_features.shaderFloat16 == VK_TRUE && storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE) {
        if (atomic_float2_features.shaderBufferFloat16Atomics == VK_TRUE) {
            current_shader = cs_buffer_float_16_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_16_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_16_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderBufferFloat16AtomicAdd == VK_TRUE) {
            current_shader = cs_buffer_float_16_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) {
            current_shader = cs_buffer_float_16_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_16_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderSharedFloat16Atomics == VK_TRUE) {
            current_shader = cs_shared_float_16_load.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_16_store.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_16_exchange.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderSharedFloat16AtomicAdd == VK_TRUE) {
            current_shader = cs_shared_float_16_add.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderSharedFloat16AtomicMinMax == VK_TRUE) {
            current_shader = cs_shared_float_16_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_16_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }
    }

    if (atomic_float2_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_buffer_float_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_buffer_float_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (atomic_float2_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_shared_float_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_shared_float_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }

    if (features2.features.shaderFloat64 == VK_TRUE) {
        if (atomic_float2_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) {
            current_shader = cs_buffer_float_64_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_buffer_float_64_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }

        if (atomic_float2_features.shaderSharedFloat64AtomicMinMax == VK_TRUE) {
            current_shader = cs_shared_float_64_min.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

            current_shader = cs_shared_float_64_max.c_str();
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
        }
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    if (atomic_float2_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) {
        current_shader = cs_image_32_min.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);

        current_shader = cs_image_32_max.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "", true);
    }
}

TEST_F(VkPositiveLayerTest, ShaderAtomicFromPhysicalPointer) {
    TEST_DESCRIPTION("Make sure atomic validation handles if from a OpConvertUToPtr (physical pointer)");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!features12.bufferDeviceAddress) {
        printf("%s VkPhysicalDeviceVulkan12Features::bufferDeviceAddress not supported and is required. Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const char *spv_source = R"(
               OpCapability Int64
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpExtension "SPV_EXT_descriptor_indexing"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %tex_ref 0 Offset 0
               OpMemberDecorate %tex_ref 1 Offset 4
               OpDecorate %_runtimearr_tex_ref ArrayStride 8
               OpMemberDecorate %outbuftype 0 Offset 0
               OpDecorate %outbuftype BufferBlock
               OpDecorate %outbuf DescriptorSet 0
               OpDecorate %outbuf Binding 0
               OpMemberDecorate %__rd_feedbackStruct 0 Offset 0
               OpDecorate %__rd_feedbackStruct Block
       %void = OpTypeVoid
      %voidf = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
    %tex_ref = OpTypeStruct %uint %uint
%_runtimearr_tex_ref = OpTypeRuntimeArray %tex_ref
 %outbuftype = OpTypeStruct %_runtimearr_tex_ref
%_runtimearr_outbuftype = OpTypeRuntimeArray %outbuftype
%_ptr_Uniform__runtimearr_outbuftype = OpTypePointer Uniform %_runtimearr_outbuftype
     %outbuf = OpVariable %_ptr_Uniform__runtimearr_outbuftype Uniform
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
     %v3uint = OpTypeVector %uint 3
      %ulong = OpTypeInt 64 0
  %ulong_2 = OpConstant %ulong 2
  %ulong_1 = OpConstant %ulong 1
%__rd_feedbackStruct = OpTypeStruct %uint
%__feedbackOffset_set0_bind0 = OpConstant %ulong 0
%__rd_feedbackAddress = OpConstant %ulong 260636672
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
%uint_4294967295 = OpConstant %uint 4294967295
     %uint_4 = OpConstant %uint 4
   %uint_0_0 = OpConstant %uint 0
       %main = OpFunction %void None %voidf
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_uint %outbuf %int_0 %int_0 %int_0 %int_0
         %65 = OpExtInst %ulong %1 UMin %ulong_1 %ulong_2
         %66 = OpIAdd %ulong %__rd_feedbackAddress %__feedbackOffset_set0_bind0
         %67 = OpShiftLeftLogical %ulong %65 %uint_4
         %68 = OpIAdd %ulong %66 %67
         %69 = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_uint %68
         %70 = OpAtomicUMax %uint %69 %uint_4 %uint_0_0 %uint_4294967295
               OpStore %63 %uint_0
               OpReturn
               OpFunctionEnd
        )";
    m_errorMonitor->ExpectSuccess();
    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, ValidateComputeShaderSharedMemory) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        shared uint a;
        shared float b;
        shared vec2 c;
        shared mat3 d;
        shared mat4 e[3];
        struct A {
            int f;
            float g;
            uint h;
        };
        shared A f;
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, TestShaderInputAndOutputComponents) {
    TEST_DESCRIPTION("Test shader layout in and out with different components.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out vec2 rg;
                layout(location = 0, component = 2) out float b;

                layout(location = 1, component = 0) out float r;
                layout(location = 1, component = 1) out vec3 gba;

                layout(location = 2) out vec4 out_color_0;
                layout(location = 3) out vec4 out_color_1;

                layout(location = 4, component = 0) out float x;
                layout(location = 4, component = 1) out vec2 yz;
                layout(location = 4, component = 3) out float w;

                layout(location = 5, component = 0) out vec3 stp;
                layout(location = 5, component = 3) out float q;

                layout(location = 6, component = 0) out vec2 cd;
                layout(location = 6, component = 2) out float e;
                layout(location = 6, component = 3) out float f;

                layout(location = 7, component = 0) out float ar1;
                layout(location = 7, component = 1) out float ar2[2];
                layout(location = 7, component = 3) out float ar3;

                void main() {
	                    vec2 xy = vec2((gl_VertexIndex >> 1u) & 1u, gl_VertexIndex & 1u);
                        gl_Position = vec4(xy, 0.0f, 1.0f);
                        out_color_0 = vec4(1.0f, 0.0f, 1.0f, 0.0f);
                        out_color_1 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
                        rg = vec2(0.25f, 0.75f);
                        b = 0.5f;
                        r = 0.75f;
                        gba = vec3(1.0f);
                        x = 1.0f;
                        yz = vec2(0.25f);
                        w = 0.5f;
                        stp = vec3(1.0f);
                        q = 0.1f;
                        ar1 = 1.0f;
                        ar2[0] = 0.5f;
                        ar2[1] = 0.75f;
                        ar3 = 1.0f;
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float r;
                layout(location = 0, component = 1) in vec2 gb;

                layout(location = 1, component = 0) in float r1;
                layout(location = 1, component = 1) in float g1;
                layout(location = 1, component = 2) in float b1;
                layout(location = 1, component = 3) in float a1;

                layout(location = 2) in InputBlock {
                    layout(location = 3, component = 3) float one_alpha;
                    layout(location = 2, component = 3) float zero_alpha;
                    layout(location = 3, component = 2) float one_blue;
                    layout(location = 2, component = 2) float zero_blue;
                    layout(location = 3, component = 1) float one_green;
                    layout(location = 2, component = 1) float zero_green;
                    layout(location = 3, component = 0) float one_red;
                    layout(location = 2, component = 0) float zero_red;
                } inBlock;

                layout(location = 4, component = 0) in vec2 xy;
                layout(location = 4, component = 2) in vec2 zw;

                layout(location = 5, component = 0) in vec2 st;
                layout(location = 5, component = 2) in vec2 pq;

                layout(location = 6, component = 0) in vec4 cdef;

                layout(location = 7, component = 0) in float ar1;
                layout(location = 7, component = 1) in float ar2;
                layout(location = 8, component = 1) in float ar3;
                layout(location = 7, component = 3) in float ar4;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(r, gb, 1.0f) *
                            vec4(r1, g1, 1.0f, a1) *
                            vec4(inBlock.zero_red, inBlock.zero_green, inBlock.zero_blue, inBlock.zero_alpha) *
                            vec4(inBlock.one_red, inBlock.one_green, inBlock.one_blue, inBlock.one_alpha) *
                            vec4(xy, zw) * vec4(st, pq) * cdef * vec4(ar1, ar2, ar3, ar4);
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit, "", true);
}

TEST_F(VkPositiveLayerTest, MeshShaderPointSize) {
    TEST_DESCRIPTION("Test writing point size in a mesh shader.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    std::array<const char *, 2> required_device_extensions = {
        {VK_NV_MESH_SHADER_EXTENSION_NAME, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%sNot suppored by MockICD or devsim, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (mesh_shader_features.meshShader != VK_TRUE) {
        printf("%s Mesh shader feature not supported\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char meshShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable
        layout (local_size_x=1) in;
        layout (points) out;
        layout (max_vertices=1, max_primitives=1) out;
        void main ()
        {
            gl_PrimitiveCountNV = 1u;
            gl_PrimitiveIndicesNV[0] = 0;
            gl_MeshVerticesNV[0].gl_Position = vec4(-0.5, -0.5, 0.0, 1.0);
            gl_MeshVerticesNV[0].gl_PointSize = 4;
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    helper.gp_ci_.pVertexInputState = nullptr;
    helper.gp_ci_.pInputAssemblyState = nullptr;

    helper.InitState();

    m_errorMonitor->ExpectSuccess();
    helper.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, TestShaderInputAndOutputStructComponents) {
    TEST_DESCRIPTION("Test shader interface with structs.");

    ASSERT_NO_FATAL_FAILURE(Init());

    // There is a crash inside the driver on S10
    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test does not currently run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) out R color[3];

                void main() {
                    color[0].rgba = vec4(1.0f);
                    color[1].rgba = vec4(0.5f);
                    color[2].rgba = vec4(0.75f);
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) in R inColor[3];

                layout (location = 0) out vec4 color;

                void main() {
                    color = inColor[0].rgba * inColor[1].rgba * inColor[2].rgba;
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit, "", true);
}

TEST_F(VkPositiveLayerTest, TaskAndMeshShader) {
    TEST_DESCRIPTION("Test task and mesh shader");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mesh_shader_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        printf("%s Test requires (unsupported) meshShader and taskShader features, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    VkPhysicalDeviceVulkan11Properties vulkan11_props = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&vulkan11_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    if ((vulkan11_props.subgroupSupportedStages & VK_SHADER_STAGE_TASK_BIT_NV) == 0) {
        printf(
            "%s VkPhysicalDeviceVulkan11Properties::subgroupSupportedStages does not include VK_SHADER_STAGE_TASK_BIT_NV, skipping "
            "test.\n",
            kSkipPrefix);
        return;
    }

    static const char taskShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require
        #extension GL_KHR_shader_subgroup_ballot : require

        #define GROUP_SIZE 32

        layout(local_size_x = 32) in;

        taskNV out Task {
          uint baseID;
          uint subIDs[GROUP_SIZE];
        } OUT;

        void main() {
            uvec4 desc = uvec4(gl_GlobalInvocationID.x);

            // implement some early culling function
            bool render = gl_GlobalInvocationID.x < 32;

            uvec4 vote  = subgroupBallot(render);
            uint tasks = subgroupBallotBitCount(vote);

            if (gl_LocalInvocationID.x == 0) {
                // write the number of surviving meshlets, i.e.
                // mesh workgroups to spawn
                gl_TaskCountNV = tasks;

                // where the meshletIDs started from for this task workgroup
                OUT.baseID = gl_WorkGroupID.x * GROUP_SIZE;
            }
        }
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
          uint subIDs[32];
        } IN;

        void main() {
            uint meshletID = IN.baseID + IN.subIDs[gl_WorkGroupID.x];
            uvec4 desc = uvec4(meshletID);
        }
    )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_2);

    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, "", true);
}

TEST_F(VkPositiveLayerTest, ShaderPointSizeStructMemeberWritten) {
    TEST_DESCRIPTION("Write built-in PointSize within a struct");
    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_1); // At least 1.1 is required for maintenance4
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " required but not supported";
    }
    auto maint4features = LvlInitStruct<VkPhysicalDeviceMaintenance4FeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&maint4features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!maint4features.maintenance4) {
        GTEST_SKIP() << "VkPhysicalDeviceMaintenance4FeaturesKHR::maintenance4 is required but not enabled.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string vs_src = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %14 %25 %47 %52
               OpSource GLSL 450
               OpMemberDecorate %12 0 BuiltIn Position
               OpMemberDecorate %12 1 BuiltIn PointSize
               OpMemberDecorate %12 2 BuiltIn ClipDistance
               OpMemberDecorate %12 3 BuiltIn CullDistance
               OpDecorate %12 Block
               OpMemberDecorate %18 0 ColMajor
               OpMemberDecorate %18 0 Offset 0
               OpMemberDecorate %18 0 MatrixStride 16
               OpMemberDecorate %18 1 Offset 64
               OpMemberDecorate %18 2 Offset 80
               OpDecorate %18 Block
               OpDecorate %25 Location 0
               OpDecorate %47 Location 1
               OpDecorate %52 Location 0
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 4
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 1
         %11 = OpTypeArray %7 %10
         %12 = OpTypeStruct %8 %7 %11 %11
         %13 = OpTypePointer Output %12
         %14 = OpVariable %13 Output
         %15 = OpTypeInt 32 1
         %16 = OpConstant %15 0
         %17 = OpTypeMatrix %8 4
         %18 = OpTypeStruct %17 %7 %8
         %19 = OpTypePointer PushConstant %18
         %20 = OpVariable %19 PushConstant
         %21 = OpTypePointer PushConstant %17
         %24 = OpTypePointer Input %8
         %25 = OpVariable %24 Input
         %28 = OpTypePointer Output %8
         %30 = OpConstant %7 0.5
         %31 = OpConstant %9 2
         %32 = OpTypePointer Output %7
         %36 = OpConstant %9 3
         %46 = OpConstant %15 1
         %47 = OpVariable %24 Input
         %48 = OpTypePointer Input %7
         %52 = OpVariable %28 Output
         %53 = OpTypeVector %7 3
         %56 = OpConstant %7 1
          %main = OpFunction %3 None %4
          %6 = OpLabel

               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
               ; For the following, only the _first_ index of the access chain
               ; should be used for output validation, as subsequent indices refer
               ; to individual components within the output variable of interest.
               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
         %22 = OpAccessChain %21 %20 %16
         %23 = OpLoad %17 %22
         %26 = OpLoad %8 %25
         %27 = OpMatrixTimesVector %8 %23 %26
         %29 = OpAccessChain %28 %14 %16
               OpStore %29 %27
         %33 = OpAccessChain %32 %14 %16 %31
         %34 = OpLoad %7 %33
         %35 = OpFMul %7 %30 %34
         %37 = OpAccessChain %32 %14 %16 %36
         %38 = OpLoad %7 %37
         %39 = OpFMul %7 %30 %38
         %40 = OpFAdd %7 %35 %39
         %41 = OpAccessChain %32 %14 %16 %31
               OpStore %41 %40
         %42 = OpAccessChain %32 %14 %16 %10
         %43 = OpLoad %7 %42
         %44 = OpFNegate %7 %43
         %45 = OpAccessChain %32 %14 %16 %10
               OpStore %45 %44
         %49 = OpAccessChain %48 %47 %36
         %50 = OpLoad %7 %49
         %51 = OpAccessChain %32 %14 %46
               OpStore %51 %50

         %54 = OpLoad %8 %47
         %55 = OpVectorShuffle %53 %54 %54 0 1 2
         %57 = OpCompositeExtract %7 %55 0
         %58 = OpCompositeExtract %7 %55 1
         %59 = OpCompositeExtract %7 %55 2
         %60 = OpCompositeConstruct %8 %57 %58 %59 %56
               OpStore %52 %60
               OpReturn
               OpFunctionEnd
    )asm";
    auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, vs_src, "main");

    if (vs) {
        VkPushConstantRange push_constant_ranges[1]{{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * (16 + 4 + 1)}};

        VkPipelineLayoutCreateInfo const pipeline_layout_info{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, push_constant_ranges};

        VkVertexInputBindingDescription input_binding[2] = {
            {0, 16, VK_VERTEX_INPUT_RATE_VERTEX},
            {1, 16, VK_VERTEX_INPUT_RATE_VERTEX},
        };
        VkVertexInputAttributeDescription input_attribs[2] = {
            {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        };

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.vi_ci_.pVertexBindingDescriptions = input_binding;
        pipe.vi_ci_.vertexBindingDescriptionCount = 2;
        pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    } else {
        printf("%s Error creating shader from assembly\n", kSkipPrefix);
    }
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, Std430SpirvOptFlags10) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.0");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto uniform_buffer_standard_layout_features = LvlInitStruct<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>();
    auto scalar_block_layout_features =
        LvlInitStruct<VkPhysicalDeviceScalarBlockLayoutFeatures>(&uniform_buffer_standard_layout_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&scalar_block_layout_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &features2);

    if (scalar_block_layout_features.scalarBlockLayout == VK_FALSE ||
        uniform_buffer_standard_layout_features.uniformBufferStandardLayout == VK_FALSE) {
        printf("%s scalarBlockLayout and uniformBufferStandardLayout are not supported Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, Std430SpirvOptFlags12) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.2");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    m_errorMonitor->ExpectSuccess();
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features12);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (features12.scalarBlockLayout == VK_FALSE || features12.uniformBufferStandardLayout == VK_FALSE) {
        printf("%s scalarBlockLayout and uniformBufferStandardLayout are not supported Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, SpecializationWordBoundryOffset) {
    TEST_DESCRIPTION("Make sure a specialization constant entry can stide over a word boundry");

    // require to make enable logic simpler
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16int8_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&float16int8_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (float16int8_features.shaderInt8 == VK_FALSE) {
        printf("%s shaderInt8 feature not supported; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // need real device to produce output to check
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    // glslang currenlty turned the GLSL to
    //      %19 = OpSpecConstantOp %uint UConvert %a
    // which causes issue (to be fixed outside scope of this test)
    // but move the UConvert to inside the function as
    //      %19 = OpUConvert %uint %a
    //
    // #version 450
    // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
    // layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // // All spec constants will write zero by default
    // layout (constant_id = 0) const uint8_t a = uint8_t(0);
    // layout (constant_id = 1) const uint b = 0;
    // layout (constant_id = 3) const uint c = 0;
    // layout (constant_id = 4) const uint d = 0;
    // layout (constant_id = 5) const uint8_t e = uint8_t(0);
    //
    // layout(set = 0, binding = 0) buffer ssbo {
    //     uint data[5];
    // };
    //
    // void main() {
    //     data[0] = 0; // clear full word
    //     data[0] = uint(a);
    //     data[1] = b;
    //     data[2] = c;
    //     data[3] = d;
    //     data[4] = 0; // clear full word
    //     data[4] = uint(e);
    // }
    std::string cs_src = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
               OpDecorate %_arr_uint_uint_5 ArrayStride 4
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 3
               OpDecorate %d SpecId 4
               OpDecorate %e SpecId 5
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
       %ssbo = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %uchar = OpTypeInt 8 0
          %a = OpSpecConstant %uchar 0
      %int_1 = OpConstant %int 1
          %b = OpSpecConstant %uint 0
      %int_2 = OpConstant %int 2
          %c = OpSpecConstant %uint 0
      %int_3 = OpConstant %int 3
          %d = OpSpecConstant %uint 0
      %int_4 = OpConstant %int 4
          %e = OpSpecConstant %uchar 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUConvert %uint %a
         %33 = OpUConvert %uint %e
         %16 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %16 %uint_0
         %20 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %20 %19
         %23 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_1
               OpStore %23 %b
         %26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_2
               OpStore %26 %c
         %29 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_3
               OpStore %29 %d
         %31 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %31 %uint_0
         %34 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %34 %33
               OpReturn
               OpFunctionEnd
    )";

    // Use strange combinations of size and offsets around word boundry
    VkSpecializationMapEntry entries[5] = {
        {0, 1, 1},  // OpTypeInt 8
        {1, 1, 4},  // OpTypeInt 32
        {3, 2, 4},  // OpTypeInt 32
        {4, 3, 4},  // OpTypeInt 32
        {5, 3, 1},  // OpTypeInt 8
    };

    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        sizeof(uint8_t) * 8,
        reinterpret_cast<void *>(data),
    };

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(this, cs_src.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM,
                                   &specialization_info));
    pipe.InitState();
    m_errorMonitor->ExpectSuccess();
    pipe.CreateComputePipeline();

    // Submit shader to see SSBO output
    VkBufferObj buffer;
    auto bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.init(*m_device, bci, mem_props);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    // Make sure spec constants were updated correctly
    void *pData;
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), buffer.memory().handle(), 0, VK_WHOLE_SIZE, 0, &pData));
    uint32_t *ssbo_data = reinterpret_cast<uint32_t *>(pData);
    ASSERT_EQ(ssbo_data[0], 0x02);
    ASSERT_EQ(ssbo_data[1], 0x05040302);
    ASSERT_EQ(ssbo_data[2], 0x06050403);
    ASSERT_EQ(ssbo_data[3], 0x07060504);
    ASSERT_EQ(ssbo_data[4], 0x04);
    vk::UnmapMemory(m_device->device(), buffer.memory().handle());
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, WriteDescriptorSetAccelerationStructureNVNullDescriptor) {
    TEST_DESCRIPTION("Validate using NV acceleration structure descriptor writing with null descriptor.");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&robustness2_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (robustness2_features.nullDescriptor != VK_TRUE) {
        printf("%s nullDescriptor feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->ExpectSuccess();

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_MISS_BIT_NV, nullptr},
                                     });

    VkAccelerationStructureNV top_level_as = VK_NULL_HANDLE;

    VkWriteDescriptorSetAccelerationStructureNV acc = LvlInitStruct<VkWriteDescriptorSetAccelerationStructureNV>();
    acc.accelerationStructureCount = 1;
    acc.pAccelerationStructures = &top_level_as;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>(&acc);
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, Spirv16Vulkan13) {
    TEST_DESCRIPTION("Create a shader using 1.3 spirv environment");
    m_errorMonitor->ExpectSuccess();
    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3);
    m_errorMonitor->VerifyNotFound();
}
