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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

struct icd_spv_header {
    uint32_t magic = 0x07230203;
    uint32_t version = 99;
    uint32_t gen_magic = 0;  // Generator's magic number
};

TEST_F(NegativeShaderSpirv, CodeSize) {
    TEST_DESCRIPTION("Test that errors are produced for a spirv modules with invalid code sizes");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    {
        VkShaderModule module;
        VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();

        constexpr icd_spv_header spv = {};
        module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
        module_create_info.codeSize = 4;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Invalid SPIR-V header");
        vk::CreateShaderModule(m_device->device(), &module_create_info, nullptr, &module);
        m_errorMonitor->VerifyFound();
    }

    {
        std::vector<uint32_t> shader;
        VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
        VkShaderModule module;
        this->GLSLtoSPV(&m_device->phy().limits_, VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl, shader);
        module_create_info.pCode = shader.data();
        // Introduce failure by making codeSize a non-multiple of 4
        module_create_info.codeSize = shader.size() * sizeof(uint32_t) - 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-codeSize-08735");
        vk::CreateShaderModule(m_device->handle(), &module_create_info, nullptr, &module);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderSpirv, Magic) {
    TEST_DESCRIPTION("Test that an error is produced for a spirv module with a bad magic number");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderModule module;
    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();

    constexpr uint32_t bad_magic = 4175232508U;
    constexpr icd_spv_header spv = {bad_magic};

    module_create_info.pCode = reinterpret_cast<const uint32_t *>(&spv);
    module_create_info.codeSize = sizeof(spv);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Invalid SPIR-V magic number");
    vk::CreateShaderModule(m_device->device(), &module_create_info, nullptr, &module);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderFloatControl) {
    TEST_DESCRIPTION("Test VK_KHR_shader_float_controls");

    // Need 1.1 to get SPIR-V 1.3 since OpExecutionModeId was added in SPIR-V 1.2
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // The issue with revision 4 of this extension should not be an issue with the tests
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);

    if (shader_float_control.denormBehaviorIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "denormBehaviorIndependence is NONE";
    }
    if (shader_float_control.roundingModeIndependence == VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        GTEST_SKIP() << "roundingModeIndependence is NONE";
    }

    // Check for support of 32-bit properties, but only will test if they are not supported
    // in case all 16/32/64 version are not supported will set SetUnexpectedError for capability check
    bool signed_zero_inf_nan_preserve = (shader_float_control.shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE);
    bool denorm_preserve = (shader_float_control.shaderDenormPreserveFloat32 == VK_TRUE);
    bool denorm_flush_to_zero = (shader_float_control.shaderDenormFlushToZeroFloat32 == VK_TRUE);
    bool rounding_mode_rte = (shader_float_control.shaderRoundingModeRTEFloat32 == VK_TRUE);
    bool rounding_mode_rtz = (shader_float_control.shaderRoundingModeRTZFloat32 == VK_TRUE);

    // same body for each shader, only the start is different
    // this is just "float a = 1.0 + 2.0;" in SPIR-V
    const std::string source_body = R"(
             OpExecutionMode %main LocalSize 1 1 1
             OpSource GLSL 450
             OpName %main "main"
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %float = OpTypeFloat 32
%pFunction = OpTypePointer Function %float
  %float_3 = OpConstant %float 3
     %main = OpFunction %void None %3
        %5 = OpLabel
        %6 = OpVariable %pFunction Function
             OpStore %6 %float_3
             OpReturn
             OpFunctionEnd
)";

    if (!signed_zero_inf_nan_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability SignedZeroInfNanPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main SignedZeroInfNanPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294");
    }

    if (!denorm_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297");
    }

    if (!denorm_flush_to_zero) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormFlushToZero
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormFlushToZero 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300");
    }

    if (!rounding_mode_rte) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTE
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTE 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303");
    }

    if (!rounding_mode_rtz) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTZ
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTZ 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                                 "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306");
    }
}

TEST_F(NegativeShaderSpirv, Storage8and16bitCapability) {
    TEST_DESCRIPTION("Test VK_KHR_8bit_storage and VK_KHR_16bit_storage not having feature bits required for SPIR-V capability");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Prevent extra errors for not having the support for the SPV extensions

    // Need to explicitly turn off shaderInt16 as test will try to add and easier if all test have off
    VkPhysicalDeviceFeatures features = {};
    features.shaderInt16 = VK_FALSE;
    RETURN_IF_SKIP(InitState(&features));
    InitRenderTarget();

    // storageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) buffer SSBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-storageBuffer8BitAccess-06328",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",        // Int8
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // StorageBuffer8BitAccess
        }
    }
    // uniformAndStorageBuffer8BitAccess
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(set = 0, binding = 0) uniform UBO { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",                  // Int8
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // UniformAndStorageBuffer8BitAccess
        }
    }

    // storagePushConstant8
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_8bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
            layout(push_constant) uniform PushConstant { int8_t x; } data;
            void main(){
               int8_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vector<string>{"VUID-RuntimeSpirv-storagePushConstant8-06330",  // feature
                                                             "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Int8
                                                             "VUID-VkShaderModuleCreateInfo-pCode-08740"});  // StoragePushConstant8
        }
    }

    // storageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) buffer SSBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",         // Float16
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // StorageBuffer16BitAccess
        }
    }

    // uniformAndStorageBuffer16BitAccess - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(set = 0, binding = 0) uniform UBO { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {"VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332",  // feature
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740",                   // Float16
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740"};
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              vuids);  // UniformAndStorageBuffer16BitAccess
        }
    }

    // storagePushConstant16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(push_constant) uniform PushConstant { float16_t x; } data;
            void main(){
               float16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storagePushConstant16-06333",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",      // Float16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"       // StoragePushConstant16
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageInputOutput16 - Float
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) out float16_t outData;
            void main(){
               outData = float16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
            layout(location = 0) in float16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature frag
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Float16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // StorageInputOutput16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740"      // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) buffer SSBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",         // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"          // StorageBuffer16BitAccess
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // uniformAndStorageBuffer16BitAccess - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(set = 0, binding = 0) uniform UBO { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",                   // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"                    // UniformAndStorageBuffer16BitAccess
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storagePushConstant16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(push_constant) uniform PushConstant { int16_t x; } data;
            void main(){
               int16_t a = data.x + data.x;
               gl_Position = vec4(float(a) * 0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storagePushConstant16-06333",  // feature
                "VUID-VkShaderModuleCreateInfo-pCode-08740",      // Int16
                "VUID-VkShaderModuleCreateInfo-pCode-08740"       // StoragePushConstant16
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // storageInputOutput16 - Int
    {
        char const *vsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) out int16_t outData;
            void main(){
               outData = int16_t(1);
               gl_Position = vec4(0.0);
            }
        )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        // Need to match in/out
        char const *fsSource = R"glsl(
            #version 450
            #extension GL_EXT_shader_16bit_storage: enable
            #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
            layout(location = 0) flat in int16_t x;
            layout(location = 0) out vec4 uFragColor;
            void main(){
               uFragColor = vec4(0,1,0,1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        if ((VK_SUCCESS == vs.InitFromGLSLTry()) && (VK_SUCCESS == fs.InitFromGLSLTry())) {
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // feature frag
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // Int16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740",     // StorageInputOutput16 vert
                "VUID-VkShaderModuleCreateInfo-pCode-08740"      // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }
}

TEST_F(NegativeShaderSpirv, Storage8and16bitFeatures) {
    TEST_DESCRIPTION(
        "Test VK_KHR_8bit_storage and VK_KHR_16bit_storage where the Int8/Int16 capability are only used and since they are "
        "superset of a capabilty");

    // the following [OpCapability UniformAndStorageBuffer8BitAccess] requires the uniformAndStorageBuffer8BitAccess feature bit or
    // the generated capability checking code will catch it
    //
    // But having just [OpCapability Int8] is still a legal SPIR-V shader because the Int8 capabilty allows all storage classes in
    // the SPIR-V spec... but the shaderInt8 feature bit in Vulkan spec explains how you still need the
    // uniformAndStorageBuffer8BitAccess feature bit for Uniform storage class from Vulkan's perspective

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Prevent extra errors for not having the support for the SPV extensions

    VkPhysicalDeviceShaderFloat16Int8Features float16Int8 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(float16Int8);
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    if (float16Int8.shaderInt8 == VK_TRUE) {
        // storageBuffer8BitAccess
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer8BitAccess-06328");
        }

        // uniformAndStorageBuffer8BitAccess
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329");
        }

        // storagePushConstant8
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpExtension "SPV_KHR_8bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
       %Data = OpTypeStruct %int8
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant8-06330");
        }
    }

    if (float16Int8.shaderFloat16 == VK_TRUE) {
        // storageBuffer16BitAccess - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
        }

        // uniformAndStorageBuffer16BitAccess - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
        }

        // storagePushConstant16 - float
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
       %Data = OpTypeStruct %float16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant16-06333");
        }

        // storageInputOutput16 - float
        {
            const char *vs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
        %ptr = OpTypePointer Output %float16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

            const char *fs_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
    %float16 = OpTypeFloat 16
      %inPtr = OpTypePointer Input %float16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto fs = VkShaderObj::CreateFromASM(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // StorageInputOutput16 vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334"   // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    if (features2.features.shaderInt16 == VK_TRUE) {
        // storageBuffer16BitAccess - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331");
        }

        // uniformAndStorageBuffer16BitAccess - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer Uniform %Data
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332");
        }

        // storagePushConstant16 - int
        {
            const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %Data Block
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
       %Data = OpTypeStruct %int16
        %ptr = OpTypePointer PushConstant %Data
        %var = OpVariable %ptr PushConstant
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-storagePushConstant16-06333");
        }

        // storageInputOutput16 - int
        {
            const char *vs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %var
               OpDecorate %var Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
        %ptr = OpTypePointer Output %int16
        %var = OpVariable %ptr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto vs = VkShaderObj::CreateFromASM(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

            const char *fs_source = R"(
               OpCapability Shader
               OpCapability Int16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in %out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %in Location 0
               OpDecorate %in Flat
               OpDecorate %out Location 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %int16 = OpTypeInt 16 0
      %inPtr = OpTypePointer Input %int16
         %in = OpVariable %inPtr Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %outPtr = OpTypePointer Output %v4float
        %out = OpVariable %outPtr Output
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
            auto fs = VkShaderObj::CreateFromASM(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs->GetStageCreateInfo(), fs->GetStageCreateInfo()};
            };
            constexpr std::array vuids = {
                "VUID-RuntimeSpirv-storageInputOutput16-06334",  // StorageInputOutput16 vert
                "VUID-RuntimeSpirv-storageInputOutput16-06334"   // StorageInputOutput16 frag
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
        }
    }

    // tests struct with multiple types
    if (float16Int8.shaderInt8 == VK_TRUE && features2.features.shaderInt16 == VK_TRUE) {
        // struct X {
        //   u16vec2 a;
        // };
        // struct {
        //   uint a;
        //   X b;
        //   uint8_t c;
        // } Data;
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability Int16
               OpExtension "SPV_KHR_8bit_storage"
               OpExtension "SPV_KHR_16bit_storage"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpMemberDecorate %X 0 Offset 0
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 4
               OpMemberDecorate %Data 2 Offset 8
               OpDecorate %Data Block
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %int8 = OpTypeInt 8 0
      %int16 = OpTypeInt 16 0
    %v2int16 = OpTypeVector %int16 2
      %int32 = OpTypeInt 32 0
          %X = OpTypeStruct %v2int16
       %Data = OpTypeStruct %int32 %X %int8
        %ptr = OpTypePointer StorageBuffer %Data
        %var = OpVariable %ptr StorageBuffer
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";
        auto vs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
            helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          vector<string>{"VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",    // 16 bit var
                                                         "VUID-RuntimeSpirv-storageBuffer8BitAccess-06328 "});  // 8 bit var
    }
}

TEST_F(NegativeShaderSpirv, ReadShaderClock) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Don't enable either feature bit on purpose
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // Device scope using GL_EXT_shader_realtime_clock
    char const *vsSourceDevice = R"glsl(
        #version 450
        #extension GL_EXT_shader_realtime_clock: enable
        void main(){
           uvec2 a = clockRealtime2x32EXT();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_device(this, vsSourceDevice, VK_SHADER_STAGE_VERTEX_BIT);

    // Subgroup scope using ARB_shader_clock
    char const *vsSourceScope = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_subgroup(this, vsSourceScope, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info_device = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_device.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_device, kErrorBit, "VUID-RuntimeSpirv-shaderDeviceClock-06268");

    const auto set_info_subgroup = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_subgroup.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info_subgroup, kErrorBit, "VUID-RuntimeSpirv-shaderSubgroupClock-06267");
}

TEST_F(NegativeShaderSpirv, SpecializationApplied) {
    TEST_DESCRIPTION(
        "Make sure specialization constants get applied during shader validation by using a value that breaks compilation.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // Size an array using a specialization constant of default value equal to 1.
    const char *fs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %size "size"
               OpName %array "array"
               OpDecorate %size SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
       %size = OpSpecConstant %int 1
%_arr_float_size = OpTypeArray %float %size
%_ptr_Function__arr_float_size = OpTypePointer Function %_arr_float_size
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
       %main = OpFunction %void None %3
          %5 = OpLabel
      %array = OpVariable %_ptr_Function__arr_float_size Function
         %15 = OpAccessChain %_ptr_Function_float %array %int_0
               OpStore %15 %float_0
               OpReturn
               OpFunctionEnd)";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    // Set the specialization constant to 0.
    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint32_t)  // size
    };
    uint32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint32_t),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849");
}

TEST_F(NegativeShaderSpirv, SpecializationOffsetOutOfBounds) {
    TEST_DESCRIPTION("Validate VkSpecializationInfo offset.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry offset is greater than dataSize.
    const VkSpecializationMapEntry entry = {0, 5, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-offset-00773");
}

TEST_F(NegativeShaderSpirv, SpecializationOffsetOutOfBoundsWithIdentifier) {
    TEST_DESCRIPTION("Validate VkSpecializationInfo offset using a shader module identifier.");

    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePipelineCreationCacheControlFeatures shader_cache_control_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT shader_module_id_features =
        vku::InitStructHelper(&shader_cache_control_features);
    GetPhysicalDeviceFeatures2(shader_module_id_features);
    RETURN_IF_SKIP(InitState(nullptr, &shader_module_id_features));
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        layout (constant_id = 0) const float x = 0.0f;
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageModuleIdentifierCreateInfoEXT sm_id_create_info = vku::InitStructHelper();
    VkShaderModuleIdentifierEXT get_identifier = vku::InitStructHelper();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    sm_id_create_info.pIdentifier = get_identifier.identifier;

    // Entry offset is greater than dataSize.
    const VkSpecializationMapEntry entry = {0, 5, sizeof(uint32_t)};
    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&sm_id_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";
    stage_ci.pSpecializationInfo = &specialization_info;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    pipe.gp_ci_.flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationInfo-offset-00773");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, SpecializationSizeOutOfBounds) {
    TEST_DESCRIPTION("Challenge core_validation with shader validation issues related to vkCreateGraphicsPipelines.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Entry size is greater than dataSize minus offset.
    const VkSpecializationMapEntry entry = {0, 3, sizeof(uint32_t)};

    uint32_t data = 1;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(float),
        &data,
    };

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-pMapEntries-00774");
}

TEST_F(NegativeShaderSpirv, SpecializationSizeZero) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is 0");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const char *cs_src = R"glsl(
        #version 450
        layout (constant_id = 0) const int c = 3;
        layout (local_size_x = 1) in;
        void main() {
            if (gl_GlobalInvocationID.x >= c) { return; }
        }
    )glsl";

    // Set the specialization constant size to 0 (anything other than 1, 2, 4, or 8 will produce the expected error).
    VkSpecializationMapEntry entry = {
        0,  // id
        0,  // offset
        0,  // size
    };
    int32_t data = 0;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(decltype(data)),
        &data,
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    entry.size = sizeof(decltype(data));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.CreateComputePipeline();
}

TEST_F(NegativeShaderSpirv, SpecializationSizeMismatch) {
    TEST_DESCRIPTION("Make sure an error is logged when a specialization map entry's size is not correct with type");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    bool int8_support = false;
    bool float64_support = false;

    // require to make enable logic simpler
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.shaderInt8 = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.shaderInt8 == VK_TRUE) {
        int8_support = true;
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    if (m_device->phy().features().shaderFloat64) {
        float64_support = true;
    }

    // layout (constant_id = 0) const int a = 3;
    // layout (constant_id = 1) const uint b = 3;
    // layout (constant_id = 2) const float c = 3.0f;
    // layout (constant_id = 3) const bool d = true;
    // layout (constant_id = 4) const bool f = false;
    const char *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 2
               OpDecorate %d SpecId 3
               OpDecorate %f SpecId 4
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
       %bool = OpTypeBool
          %a = OpSpecConstant %int 3
          %b = OpSpecConstant %uint 3
          %c = OpSpecConstant %float 3
          %d = OpSpecConstantTrue %bool
          %f = OpSpecConstantFalse %bool
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    // use same offset to keep simple since unused data being read
    VkSpecializationMapEntry entries[5] = {
        {0, 0, 4},                 // OpTypeInt 32
        {1, 0, 4},                 // OpTypeInt 32
        {2, 0, 4},                 // OpTypeFloat 32
        {3, 0, sizeof(VkBool32)},  // OpTypeBool
        {4, 0, sizeof(VkBool32)}   // OpTypeBool
    };

    std::array<int32_t, 4> data;  // enough garbage data to grab from
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        data.size() * sizeof(decltype(data)::value_type),
        data.data(),
    };

    std::unique_ptr<VkShaderObj> cs;
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };

    // Sanity check
    cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
    if (cs) {
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

        // signed int mismatch
        entries[0].size = 0;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 2;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 8;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[0].size = 4;  // reset

        // unsigned int mismatch
        entries[1].size = 1;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 8;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 3;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[1].size = 4;  // reset

        // float mismatch
        entries[2].size = 0;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 8;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 7;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[2].size = 4;  // reset

        // bool mismatch
        entries[3].size = sizeof(VkBool32) / 2;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        entries[3].size = sizeof(VkBool32) + 1;
        cs = VkShaderObj::CreateFromASM(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
    }

    if (int8_support == true) {
        // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
        // layout (constant_id = 0) const int8_t a = int8_t(3);
        // layout (constant_id = 1) const uint8_t b = uint8_t(3);
        const char *cs_int8 = R"(
               OpCapability Shader
               OpCapability Int8
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %char = OpTypeInt 8 1
      %uchar = OpTypeInt 8 0
          %a = OpSpecConstant %char 3
          %b = OpSpecConstant %uchar 3
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
            )";

        specialization_info.mapEntryCount = 2;
        entries[0] = {0, 0, 1};  // OpTypeInt 8
        entries[1] = {1, 0, 1};  // OpTypeInt 8

        cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        if (cs) {
            // Sanity check
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

            // signed int 8 mismatch
            entries[0].size = 0;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 2;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 4;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 1;  // reset

            // unsigned int 8 mismatch
            entries[1].size = 0;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[1].size = 2;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[1].size = 4;
            cs = VkShaderObj::CreateFromASM(this, cs_int8, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        }
    }

    if (float64_support == true) {
        // #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        // layout (constant_id = 0) const float64_t a = 3.0f;
        const char *cs_float64 = R"(
               OpCapability Shader
               OpCapability Float64
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float64"
               OpDecorate %a SpecId 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
     %double = OpTypeFloat 64
          %a = OpSpecConstant %double 3
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
            )";

        specialization_info.mapEntryCount = 1;
        entries[0] = {0, 0, 8};  // OpTypeFloat 64

        cs = VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
        if (cs) {
            // Sanity check
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);

            // float 64 mismatch
            entries[0].size = 1;
            cs =
                VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 2;
            cs =
                VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 4;
            cs =
                VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
            entries[0].size = 16;
            cs =
                VkShaderObj::CreateFromASM(this, cs_float64, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, &specialization_info);
            CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationMapEntry-constantID-00776");
        }
    }
}

TEST_F(NegativeShaderSpirv, DuplicatedSpecializationConstantID) {
    TEST_DESCRIPTION("Create a pipeline with non unique constantID in specialization pMapEntries.");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const float r = 0.0f;
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(r,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkSpecializationMapEntry entries[2];
    entries[0].constantID = 0;
    entries[0].offset = 0;
    entries[0].size = sizeof(uint32_t);
    entries[1].constantID = 0;
    entries[1].offset = 0;
    entries[1].size = sizeof(uint32_t);

    uint32_t data = 1;
    VkSpecializationInfo specialization_info;
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.shader_stages_[1].pSpecializationInfo = &specialization_info;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkSpecializationInfo-constantID-04911");
}

TEST_F(NegativeShaderSpirv, ShaderModuleCheckCapability) {
    TEST_DESCRIPTION("Create a shader in which a capability declared by the shader is not supported.");
    // Note that this failure message comes from spirv-tools, specifically the validator.

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const char *spv_source = R"(
                  OpCapability ImageRect
                  OpEntryPoint Vertex %main "main"
          %main = OpFunction %void None %3
                  OpReturn
                  OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Capability ImageRect is not allowed by Vulkan");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderNotEnabled) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a capability declared by the shader requires a feature not enabled on the device.");

    RETURN_IF_SKIP(InitFramework())

    // Some awkward steps are required to test with custom device features.
    VkPhysicalDeviceFeatures device_features = {};
    // Disable support for 64 bit floats
    device_features.shaderFloat64 = false;
    // The sacrificial device object
    RETURN_IF_SKIP(InitState(&device_features));
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main(){
           dvec4 green = vec4(0.0, 1.0, 0.0, 1.0);
           color = vec4(green);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NonSemanticInfoEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_non_semantic_info.");

    RETURN_IF_SKIP(InitFramework())
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_shader_non_semantic_info not supported";
    }
    RETURN_IF_SKIP(InitState())

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    const char *source = R"(
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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ShaderImageFootprintEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the shader image footprint feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the image footprint feature.
    VkPhysicalDeviceShaderImageFootprintFeaturesNV image_footprint_features = vku::InitStructHelper();
    image_footprint_features.imageFootprint = VK_FALSE;

    vkt::Device test_device(gpu(), device_extension_names, &features, &image_footprint_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_shader_texture_footprint  : require
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 color;
        void main(){
          gl_TextureFootprint2DNV footprint;
          if (textureFootprintNV(s, vec2(1.0), 5, false, footprint)) {
            color = vec4(0.0, 1.0, 0.0, 1.0);
          } else {
            color = vec4(vec2(footprint.anchor), vec2(footprint.offset));
          }
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);

    CreatePipelineHelper pipe(*this);
    pipe.device_ = &test_device;
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout ds_layout(test_device, {binding});
    ASSERT_TRUE(ds_layout.initialized());

    const vkt::PipelineLayout pipeline_layout(test_device, {&ds_layout});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, FragmentShaderBarycentricEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader barycentric feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    auto features = m_device->phy().features();

    // Disable the fragment shader barycentric feature.
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features = vku::InitStructHelper();
    fragment_shader_barycentric_features.fragmentShaderBarycentric = VK_FALSE;

    vkt::Device test_device(gpu(), m_device_extension_names, &features, &fragment_shader_barycentric_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_NV_fragment_shader_barycentric : require
        layout(location=0) out float value;
        void main(){
          value = gl_BaryCoordNV.x;
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);
    const vkt::PipelineLayout pipeline_layout(test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    CreatePipelineHelper pipe(*this);
    pipe.device_ = &test_device;
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, ComputeShaderDerivativesEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the compute shader derivatives feature which has not enabled on the device.");

    AddRequiredExtensions(VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Disable the compute shader derivatives features.
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivatives_features = vku::InitStructHelper();
    compute_shader_derivatives_features.computeDerivativeGroupLinear = VK_FALSE;
    compute_shader_derivatives_features.computeDerivativeGroupQuads = VK_FALSE;

    vkt::Device test_device(gpu(), device_extension_names, &features, &compute_shader_derivatives_features);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(test_device, {binding});
    const vkt::PipelineLayout pl(test_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        layout(local_size_x=2, local_size_y=4) in;
        layout(derivative_group_quadsNV) in;
        layout(set=0, binding=0) buffer InputOutputBuffer {
          float values[];
        };
        void main(){
           values[gl_LocalInvocationIndex] = dFdx(values[gl_LocalInvocationIndex]);
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(false, &test_device);

    VkComputePipelineCreateInfo cpci = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                        nullptr,
                                        0,
                                        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                                         VK_SHADER_STAGE_COMPUTE_BIT, cs.handle(), "main", nullptr},
                                        pl.handle(),
                                        VK_NULL_HANDLE,
                                        -1};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    VkPipeline pipe = VK_NULL_HANDLE;
    vk::CreateComputePipelines(test_device.device(), VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipeline(test_device.device(), pipe, nullptr);
}

TEST_F(NegativeShaderSpirv, FragmentShaderInterlockEnabled) {
    TEST_DESCRIPTION("Create a pipeline requiring the fragment shader interlock feature which has not enabled on the device.");

    RETURN_IF_SKIP(Init())

    std::vector<const char *> device_extension_names;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME)) {
        // Note: we intentionally do not add the required extension to the device extension list.
        //       in order to create the error below
    } else {
        // We skip this test if the extension is not supported by the driver as in some cases this will cause
        // the vk::CreateShaderModule to fail without generating an error message
        printf("Extension %s is not supported.\n", VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);
        return;
    }

    auto features = m_device->phy().features();

    // Disable the fragment shader interlock feature.
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_interlock_features = vku::InitStructHelper();
    fragment_shader_interlock_features.fragmentShaderSampleInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderPixelInterlock = VK_FALSE;
    fragment_shader_interlock_features.fragmentShaderShadingRateInterlock = VK_FALSE;

    vkt::Device test_device(gpu(), device_extension_names, &features, &fragment_shader_interlock_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_ARB_fragment_shader_interlock : require
        layout(sample_interlock_ordered) in;
        void main(){
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);
    const vkt::PipelineLayout pipeline_layout(test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08742");
    CreatePipelineHelper pipe(*this);
    pipe.device_ = &test_device;
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DemoteToHelperInvocation) {
    TEST_DESCRIPTION("Create a pipeline requiring the demote to helper invocation feature which has not enabled on the device.");

    AddRequiredExtensions(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    auto features = m_device->phy().features();

    // Disable the demote to helper invocation feature.
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_features = vku::InitStructHelper();
    demote_features.shaderDemoteToHelperInvocation = VK_FALSE;

    vkt::Device test_device(gpu(), m_device_extension_names, &features, &demote_features);

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_demote_to_helper_invocation : require
        void main(){
            demote;
        }
    )glsl";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);
    const vkt::PipelineLayout pipeline_layout(test_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    CreatePipelineHelper pipe(*this);
    pipe.device_ = &test_device;
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoUniformBufferStandardLayout10) {
    TEST_DESCRIPTION("Don't enable uniformBufferStandardLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init())
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoUniformBufferStandardLayout12) {
    TEST_DESCRIPTION(
        "Don't enable uniformBufferStandardLayout in Vulkan1.2 when VK_KHR_uniform_buffer_standard_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())

    // layout(std430, set = 0, binding = 0) uniform ubo430 {
    //     float floatArray430[8];
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %ubo430 0 Offset 0
               OpDecorate %ubo430 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
     %ubo430 = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_ubo430 = OpTypePointer Uniform %ubo430
          %_ = OpVariable %_ptr_Uniform_ubo430 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoScalarBlockLayout10) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan 1.0 and have spirv-val catch invalid shader");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init())
    if (DeviceValidationVersion() > VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 4) vec3 x;
    // };
    //
    // Note: using BufferBlock for Vulkan 1.0
    // Note: Relaxed Block Layout would also make this valid if enabled
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 4
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
       %ssbo = OpTypeStruct %v3float
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, NoScalarBlockLayout12) {
    TEST_DESCRIPTION("Don't enable scalarBlockLayout in Vulkan1.2 when VK_EXT_scalar_block_layout was promoted");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())

    // layout (scalar, set = 0, binding = 0) buffer ssbo {
    //     layout(offset = 0) vec3 a;
    //     layout(offset = 12) vec2 b;
    // };
    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %ssbo 0 Offset 0
               OpMemberDecorate %ssbo 1 Offset 12
               OpDecorate %ssbo Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
       %ssbo = OpTypeStruct %v3float %v2float
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08737");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderSpirv, DeviceMemoryScope) {
    TEST_DESCRIPTION("Validate using Device memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    features12.vulkanMemoryModelDeviceScope = VK_FALSE;
    if (features12.vulkanMemoryModel == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModel feature is not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
	   }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-vulkanMemoryModel-06265");
}

TEST_F(NegativeShaderSpirv, QueueFamilyMemoryScope) {
    TEST_DESCRIPTION("Validate using QueueFamily memory scope in spirv.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    features12.vulkanMemoryModel = VK_FALSE;
    if (features12.vulkanMemoryModelDeviceScope == VK_FALSE) {
        GTEST_SKIP() << "vulkanMemoryModelDeviceScope feature is not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint y; };
        void main() {
            atomicStore(y, 1u, gl_ScopeQueueFamily, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
	   }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-RuntimeSpirv-vulkanMemoryModel-06266", "VUID-VkShaderModuleCreateInfo-pCode-08740"});
}

TEST_F(NegativeShaderSpirv, ConservativeRasterizationPostDepthCoverage) {
    TEST_DESCRIPTION("Make sure conservativeRasterizationPostDepthCoverage is set if needed.");

    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);
    if (conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        GTEST_SKIP() << "need conservativeRasterizationPostDepthCoverage to not be supported";
    }
    InitRenderTarget();

    std::string const source{R"(
               OpCapability Shader
               OpCapability SampleMaskPostDepthCoverage
               OpCapability FragmentFullyCoveredEXT
               OpExtension "SPV_EXT_fragment_fully_covered"
               OpExtension "SPV_KHR_post_depth_coverage"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12
               OpExecutionMode %4 OriginUpperLeft
               OpExecutionMode %4 EarlyFragmentTests
               OpExecutionMode %4 PostDepthCoverage
               OpDecorate %12 BuiltIn FullyCoveredEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
         %12 = OpVariable %_ptr_Input_bool Input
          %4 = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    VkShaderObj fs(this, source.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    auto set_info = [&](CreatePipelineHelper &info) {
        info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235");
}

TEST_F(NegativeShaderSpirv, DynamicUniformIndex) {
    TEST_DESCRIPTION("Check for the array dynamic array index features when the SPIR-V capabilities are requested.");

    VkPhysicalDeviceFeatures features{};
    features.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
    RETURN_IF_SKIP(Init(&features));

    InitRenderTarget();

    std::string const source{R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)"};

    {
        std::string const capability{"OpCapability UniformBufferArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability SampledImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability StorageBufferArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }

    {
        std::string const capability{"OpCapability StorageImageArrayDynamicIndexing"};

        VkShaderObj fs(this, (capability + source).c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-VkShaderModuleCreateInfo-pCode-08740");
    }
}
