/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveShaderCooperativeVector : public VkLayerTest {};

TEST_F(PositiveShaderCooperativeVector, ConvertCooperativeVectorMatrixNV) {
    TEST_DESCRIPTION("Validate using vkConvertCooperativeVectorMatrixNV.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    RETURN_IF_SKIP(Init());

    VkConvertCooperativeVectorMatrixInfoNV info = vku::InitStructHelper();
    size_t dstSize = 16 * 32 * 2;
    info.srcSize = 16 * 32 * 2;
    info.srcData.hostAddress = nullptr;
    info.pDstSize = &dstSize;
    info.dstData.hostAddress = nullptr;
    info.srcComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.numRows = 16;
    info.numColumns = 32;
    info.srcLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.srcStride = 64;
    info.dstLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.dstStride = 64;

    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);

    uint8_t src[16 * 32 * 2], dst[16 * 32 * 2];
    info.srcData.hostAddress = src;
    info.dstData.hostAddress = dst;

    vk::ConvertCooperativeVectorMatrixNV(*m_device, &info);
}

TEST_F(PositiveShaderCooperativeVector, CmdConvertCooperativeVectorMatrixNV) {
    TEST_DESCRIPTION("Validate using vkCmdConvertCooperativeVectorMatrixNV.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkConvertCooperativeVectorMatrixInfoNV info = vku::InitStructHelper();
    size_t dstSize = 16 * 32 * 2;
    info.srcSize = 16 * 32 * 2;
    info.pDstSize = &dstSize;
    info.srcComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.dstComponentType = VK_COMPONENT_TYPE_FLOAT16_KHR;
    info.numRows = 16;
    info.numColumns = 32;
    info.srcLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.srcStride = 64;
    info.dstLayout = VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV;
    info.dstStride = 64;

    vkt::Buffer src_buffer(*m_device, 16 * 32 * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 16 * 32 * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);
    const VkDeviceAddress src_address = src_buffer.Address();
    const VkDeviceAddress dst_address = dst_buffer.Address();
    info.srcData.deviceAddress = src_address;
    info.dstData.deviceAddress = dst_address;

    m_command_buffer.Begin();
    vk::CmdConvertCooperativeVectorMatrixNV(m_command_buffer, 1, &info);
    m_command_buffer.End();
}

TEST_F(PositiveShaderCooperativeVector, CooperativeVectorSPIRV) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceCooperativeVectorPropertiesNV props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);
    if (!props.cooperativeVectorTrainingFloat16Accumulation) {
        GTEST_SKIP() << "cooperativeVectorTrainingFloat16Accumulation not supported";
    }

    char const *vt_source = R"glsl(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(set=0, binding=0) buffer B { uint8_t x[]; } b;
        void main() {
            coopvecNV<float16_t, 16> A;
            coopvecNV<float16_t, 32> R;
            coopVecMatMulNV(R, A, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, 32, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
            coopVecMatMulAddNV(R, A, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, b.x, 0, gl_ComponentTypeFloat16NV, 32, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);

            coopvecNV<int8_t, 16> A2;
            coopvecNV<int32_t, 32> R2;
            coopVecMatMulNV(R2, A2, gl_ComponentTypeSignedInt8NV, b.x, 0, gl_ComponentTypeSignedInt8NV, 32, 16, gl_CooperativeVectorMatrixLayoutInferencingOptimalNV, false, 0);
       }
    )glsl";

    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, vt_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&dsl});
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCooperativeVector, CooperativeVectorTraingingSPIRV) {
    TEST_DESCRIPTION("Validate Cooperative Vector SPIR-V environment rules.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeVector);
    AddRequiredFeature(vkt::Feature::cooperativeVectorTraining);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceCooperativeVectorPropertiesNV props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);
    if (!props.cooperativeVectorTrainingFloat16Accumulation) {
        GTEST_SKIP() << "cooperativeVectorTrainingFloat16Accumulation not supported";
    }

    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    char const *vt_source = R"glsl(
        #version 450
        #extension GL_NV_cooperative_vector : enable
        #extension GL_KHR_shader_subgroup_basic : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types : enable
        layout(set=0, binding=0) buffer B { uint8_t x[]; } b;
        void main() {
            coopvecNV<float16_t, 16> A;
            coopVecReduceSumAccumulateNV(A, b.x, 0);
            coopVecOuterProductAccumulateNV(A, A, b.x, 0, 0, gl_CooperativeVectorMatrixLayoutTrainingOptimalNV, gl_ComponentTypeFloat16NV);
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, vt_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&dsl});
    pipe.CreateComputePipeline();
}
