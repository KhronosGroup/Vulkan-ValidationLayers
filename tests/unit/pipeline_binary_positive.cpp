/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
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

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    CreateComputePipelineHelper pipe(*this, &flags2);
    pipe.CreateComputePipeline(true, true);

    VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
    binary_create_info.pipeline = pipe;

    VkPipelineBinaryKHR pipeline_binary;
    VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
    handles_info.pipelineBinaryCount = 1;
    handles_info.pPipelineBinaries = &pipeline_binary;

    vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
    vk::DestroyPipelineBinaryKHR(device(), pipeline_binary, nullptr);
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

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    std::vector<uint8_t> binary_data;
    size_t data_size;
    VkPipelineBinaryKeyKHR binary_key = vku::InitStructHelper();

    // create binary from pipeline
    {
        CreateComputePipelineHelper pipe(*this, &flags2);
        pipe.CreateComputePipeline(true, true);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = pipe;

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pPipelineBinaries = nullptr;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        if (handles_info.pipelineBinaryCount != 1) {
            for (uint32_t i = 0; i < handles_info.pipelineBinaryCount; i++) {
                vk::DestroyPipelineBinaryKHR(device(), handles_info.pPipelineBinaries[i], nullptr);
            }

            GTEST_SKIP() << "Test doesn't support multiple binaries";
        }

        VkPipelineBinaryKHR pipeline_binary1;
        handles_info.pPipelineBinaries = &pipeline_binary1;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        pipe.Destroy();

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

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = 1;
        handles_info.pPipelineBinaries = &pipeline_binary2;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();

        binary_info.binaryCount = 1;
        binary_info.pPipelineBinaries = &pipeline_binary2;

        flags2.pNext = &binary_info;

        CreateComputePipelineHelper pipe2(*this, &flags2);
        pipe2.CreateComputePipeline(true, true);

        vk::DestroyPipelineBinaryKHR(device(), pipeline_binary2, nullptr);
    }
}

TEST_F(PositivePipelineBinary, GetPipelineKey) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8540");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());

    VkShaderObj cs(*m_device, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, bindings);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});

    VkComputePipelineCreateInfo compute_create_info = vku::InitStructHelper();
    compute_create_info.stage = cs.GetStageCreateInfo();
    compute_create_info.layout = pipeline_layout;

    VkPipelineBinaryInfoKHR pipeline_binary_info = vku::InitStructHelper();
    pipeline_binary_info.binaryCount = 0;

    compute_create_info.pNext = &pipeline_binary_info;

    VkPipelineCreateInfoKHR pipeline_create_info = vku::InitStructHelper(&compute_create_info);

    VkPipelineBinaryKeyKHR pipeline_key = vku::InitStructHelper();

    ASSERT_EQ(VK_SUCCESS, vk::GetPipelineKeyKHR(device(), &pipeline_create_info, &pipeline_key));
}

TEST_F(PositivePipelineBinary, GetPipelineKeyGlobal) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8544");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());
    VkPipelineBinaryKeyKHR pipeline_key = vku::InitStructHelper();
    ASSERT_EQ(VK_SUCCESS, vk::GetPipelineKeyKHR(device(), nullptr, &pipeline_key));
}

TEST_F(PositivePipelineBinary, Draw) {
    TEST_DESCRIPTION("Create pipeline binary from data blob");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Pipeline binaries not supported on MockICD";
    }

    VkResult err;

    VkPipelineCreateFlags2CreateInfo flags2 = vku::InitStructHelper();
    flags2.flags = VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR;

    uint32_t binary_count = 0u;
    std::vector<uint8_t> binary_data[2];
    size_t data_size[2];
    VkPipelineBinaryKeyKHR binary_key[2];
    binary_key[0] = vku::InitStructHelper();
    binary_key[1] = vku::InitStructHelper();

    // create binary from pipeline
    {
        CreatePipelineHelper pipe(*this, &flags2);
        pipe.CreateGraphicsPipeline(true, true);

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pipeline = pipe;

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pPipelineBinaries = nullptr;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);
        std::vector<VkPipelineBinaryKHR> pipeline_binaries(handles_info.pipelineBinaryCount);
        handles_info.pPipelineBinaries = pipeline_binaries.data();
        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        pipe.Destroy();
        binary_count = handles_info.pipelineBinaryCount;

        for (uint32_t i = 0; i < binary_count; i++) {
            VkPipelineBinaryDataInfoKHR data_info = vku::InitStructHelper();
            data_info.pipelineBinary = handles_info.pPipelineBinaries[i];

            err = vk::GetPipelineBinaryDataKHR(device(), &data_info, &binary_key[i], &data_size[i], nullptr);
            ASSERT_EQ(VK_SUCCESS, err);

            binary_data[i].resize(data_size[i]);

            err = vk::GetPipelineBinaryDataKHR(device(), &data_info, &binary_key[i], &data_size[i], binary_data[i].data());
            ASSERT_EQ(VK_SUCCESS, err);

            vk::DestroyPipelineBinaryKHR(device(), handles_info.pPipelineBinaries[i], nullptr);
        }
    }

    // create binary from data, then create pipeline from binary
    {
        VkPipelineBinaryKHR pipeline_binaries[2];

        VkPipelineBinaryDataKHR data[2];
        for (uint32_t i = 0; i < binary_count; i++) {
            data[i].dataSize = data_size[i];
            data[i].pData = binary_data[i].data();
        }

        VkPipelineBinaryKeysAndDataKHR keys_data_info;
        keys_data_info.binaryCount = binary_count;
        keys_data_info.pPipelineBinaryKeys = binary_key;
        keys_data_info.pPipelineBinaryData = data;

        VkPipelineBinaryCreateInfoKHR binary_create_info = vku::InitStructHelper();
        binary_create_info.pKeysAndDataInfo = &keys_data_info;

        VkPipelineBinaryHandlesInfoKHR handles_info = vku::InitStructHelper();
        handles_info.pipelineBinaryCount = binary_count;
        handles_info.pPipelineBinaries = pipeline_binaries;

        err = vk::CreatePipelineBinariesKHR(device(), &binary_create_info, nullptr, &handles_info);
        ASSERT_EQ(VK_SUCCESS, err);

        VkPipelineBinaryInfoKHR binary_info = vku::InitStructHelper();

        binary_info.binaryCount = binary_count;
        binary_info.pPipelineBinaries = pipeline_binaries;

        flags2.pNext = &binary_info;

        CreatePipelineHelper pipe2(*this, &flags2);
        pipe2.shader_stages_[0].module = VK_NULL_HANDLE;
        pipe2.shader_stages_[1].module = VK_NULL_HANDLE;
        pipe2.CreateGraphicsPipeline(true, true);

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2);
        vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        for (uint32_t i = 0; i < binary_count; i++) {
            vk::DestroyPipelineBinaryKHR(device(), pipeline_binaries[i], nullptr);
        }
    }
}
