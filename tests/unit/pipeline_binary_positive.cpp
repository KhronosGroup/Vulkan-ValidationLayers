/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositivePipelineBinary : public VkLayerTest {};

TEST_F(PositivePipelineBinary, CreateBinaryFromPipeline) {
    TEST_DESCRIPTION("Create pipeline binaries from a pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkResult err;

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineCreateFlags2CreateInfoKHR flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
    compute_create_info.pNext = &flags2;

    VkPipeline test_pipeline1;

    err = vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &test_pipeline1);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = test_pipeline1;

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handlesInfo = vku::InitStructHelper();
    handlesInfo.pipelineBinaryCount = 1;
    handlesInfo.pPipelineBinaries = &pipeline_binary;

    err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handlesInfo);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary, nullptr);
    vk::DestroyPipeline(device(), test_pipeline1, nullptr);
}

TEST_F(PositivePipelineBinary, CreateBinaryFromData) {
    TEST_DESCRIPTION("Create pipeline binary from data blob");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkResult err;

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout.handle();

    VkPipelineCreateFlags2CreateInfoKHR flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;
    compute_create_info.pNext = &flags2;

    std::vector<uint8_t> binary_data;
    size_t data_size;
    VkPipelineBinaryKeyKHR binary_key = vku::InitStructHelper();

    // create binary from pipeline
    {
        VkPipeline test_pipeline1;

        err = vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &test_pipeline1);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = test_pipeline1;

        VkPipelineBinaryHandlesInfoKHR handlesInfo = vku::InitStructHelper();
        handlesInfo.pPipelineBinaries = nullptr;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handlesInfo);
        ASSERT_EQ(VK_SUCCESS, err);

        if (handlesInfo.pipelineBinaryCount != 1) {
            for (uint32_t i = 0; i < handlesInfo.pipelineBinaryCount; i++) {
                vk::DestroyPipelineBinaryKHR(device(), handlesInfo.pPipelineBinaries[i], nullptr);
            }

            vk::DestroyPipeline(device(), test_pipeline1, nullptr);

            GTEST_SKIP() << "Test doesn't support multiple binaries";
        }

        VkPipelineBinaryKHR pipeline_binary1;
        handlesInfo.pPipelineBinaries = &pipeline_binary1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handlesInfo);
        ASSERT_EQ(VK_SUCCESS, err);

        vk::DestroyPipeline(device(), test_pipeline1, nullptr);

        VkPipelineBinaryDataInfoKHR data_info = vku::InitStructHelper();
        data_info.pipelineBinary = pipeline_binary1;

        err = vk::GetPipelineBinaryDataKHR(device(), &data_info, &binary_key, &data_size, nullptr);
        ASSERT_EQ(VK_SUCCESS, err);

        binary_data.resize(data_size);

        err = vk::GetPipelineBinaryDataKHR(device(), &data_info, &binary_key, &data_size, binary_data.data());
        ASSERT_EQ(VK_SUCCESS, err);

        vk::DestroyPipelineBinaryKHR(device(), pipeline_binary1, nullptr);
    }

    // create binary from data, then create pipeline from binary
    {
        VkPipelineBinaryKHR pipeline_binary2;

        VkPipelineBinaryDataKHR data;
        data.dataSize = data_size;
        data.pData = binary_data.data();

        VkPipelineBinaryKeysAndDataKHR keys_data_info;
        keys_data_info.binaryCount = 1;
        keys_data_info.pPipelineBinaryKeys = &binary_key;
        keys_data_info.pPipelineBinaryData = &data;

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pKeysAndDataInfo = &keys_data_info;

        VkPipelineBinaryHandlesInfoKHR handlesInfo = vku::InitStructHelper();
        handlesInfo.pipelineBinaryCount = 1;
        handlesInfo.pPipelineBinaries = &pipeline_binary2;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handlesInfo);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipeline test_pipeline2;
        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();

        binary_info.binaryCount = 1;
        binary_info.pPipelineBinaries = &pipeline_binary2;

        flags2.pNext = &binary_info;

        err = vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &compute_create_info, nullptr, &test_pipeline2);
        ASSERT_EQ(VK_SUCCESS, err);

        vk::DestroyPipelineBinaryKHR(device(), pipeline_binary2, nullptr);

        vk::DestroyPipeline(device(), test_pipeline2, nullptr);
    }
}
