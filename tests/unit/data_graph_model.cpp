/*
* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (C) 2026 Qualcomm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "layer_validation_tests.h"

class NegativeDataGraphModel : public DataGraphModelTest {};

TEST_F(NegativeDataGraphModel, CreateDataGraphPipelineCacheWithInvalidHeaderSize) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via a VkDevice with dataGraphModel feature enabled, "
                     "but provide a invalid header size.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Validation header size - 11836
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerSize-11836");
    {
        auto pipeline_cache = BuildPipelineCacheWithInvalidHeaderSize(*this);
        TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    }
    m_errorMonitor->VerifyFound();

    // Validation header size - 11836 and 11838
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerSize-11836");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerSize-11838");
    {
        auto pipeline_cache = BuildPipelineCacheWithInvalidGreaterHeaderSize(*this);
        TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    }
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, CreateDataGraphPipelineCacheWithInvalidHeaderVersion) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via a VkDevice with dataGraphModel feature enabled, "
                     "but provide a invalid header version.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Validation header version - 11837
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerVersion-11837");
    {
        auto pipeline_cache = BuildPipelineCacheWithUnsupportedHeaderVersion(*this);
        TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    }
    m_errorMonitor->VerifyFound();

    // Validation header version - 11837 and parameter
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerVersion-11837");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-headerVersion-parameter");
    {
        auto pipeline_cache = BuildPipelineCacheWithInvalidHeaderVersion(*this);
        TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    }
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, CreateDataGraphPipelineCacheWithInvalidCacheType) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via VkDevice with dataGraphModel feature enabled, "
                     "but provide a invalid cache type.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Validation cache type
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-cacheType-parameter");
    auto pipeline_cache = BuildPipelineCacheWithInvalidCacheType(*this);
    TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, CreateDataGraphPipelineCacheButDataGraphModelFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via a VkDevice with dataGraphModel feature disabled.");
    // add all the requirements of InitBasicDataGraphModel except dataGraphModel
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_ARM_DATA_GRAPH_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_DATA_GRAPH_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    AddRequiredFeature(vkt::Feature::dataGraph);
    AddRequiredFeature(vkt::Feature::pipelineCreationCacheControl);

    // Note: Avoid crash due to device creation failure
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDeviceCreateInfo-queueFamilyIndex-11831");
    RETURN_IF_SKIP(Init());

    // Validation dataGraphModel feature
    // Note: Firstly, according to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    //       Secondly, avoid VkDataGraphProcessingEngineCreateInfoARM check failure since we always check pProcessingEngines member.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-11844");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineCacheHeaderVersionDataGraphQCOM-None-11835");
    auto pipeline_cache = BuildValidPipelineCache(*this);
    TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, CreateDataGraphPipelineBuiltinModelButInconsistentOperation) {
    TEST_DESCRIPTION("Try to create a data graph pipeline via a VkDevice with dataGraphModel feature enabled, "
                     "but provide a invalid VkDataGraphPipelineBuiltinModelCreateInfoQCOM::pOperation parameter.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Data graph processing engine create information
    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    VkPhysicalDeviceDataGraphProcessingEngineARM dg_processing_engine{};
    dg_processing_engine.type = VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_NEURAL_QCOM;
    dg_processing_engine.isForeign = VK_TRUE;
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_processing_engine;

    // Data graph pipeline builtin model creation information
    VkDataGraphPipelineBuiltinModelCreateInfoQCOM builtin_model_ci = vku::InitStructHelper();
    VkPhysicalDeviceDataGraphOperationSupportARM data_graph_operation_support{};
    data_graph_operation_support.operationType = VK_PHYSICAL_DEVICE_DATA_GRAPH_OPERATION_TYPE_BUILTIN_MODEL_QCOM;
    builtin_model_ci.pOperation = &data_graph_operation_support;
    builtin_model_ci.pNext = &dg_processing_engine_ci;

    // Validation built-in model operation - 11842
    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkDataGraphPipelineBuiltinModelCreateInfoQCOM-pOperation-11842");
    TestDataGraphPipelineCreationOnce(*this, builtin_model_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, QueryPipelineCacheDataButHasDataGraphHeaderVersion) {
    TEST_DESCRIPTION("Try to query a pipeline cache data that uses data graph header version.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Validation pipelineCache - 11834
    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineCacheData-pipelineCache-11834");
    size_t data_size = 0;
    auto pipeline_cache = BuildValidPipelineCache(*this);
    vk::GetPipelineCacheData(*m_device, *pipeline_cache, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, MergePipelineCacheButHasDataGraphHeaderVersion) {
    TEST_DESCRIPTION("Try to merge two pipeline caches that use data graph header version.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    // Create a data graph pipeline cache
    auto data_graph_pipeline_cache = BuildValidPipelineCache(*this);

    // Create a general pipeline cache
    VkPipelineCacheCreateInfo general_pipeline_cache_ci = vku::InitStructHelper();
    general_pipeline_cache_ci.initialDataSize = 0;
    general_pipeline_cache_ci.pInitialData = nullptr;
    vkt::PipelineCache general_pipeline_cache{ *m_device, general_pipeline_cache_ci };

    // Create another general pipeline cache
    std::vector<uint8_t> general_cache_blob(sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM) * 4, 0);
    general_pipeline_cache_ci.initialDataSize = general_cache_blob.size();
    general_pipeline_cache_ci.pInitialData = general_cache_blob.data();
    vkt::PipelineCache general_pipeline_cache2{ *m_device, general_pipeline_cache_ci };

    // Validation dstCache - 11832
    auto general_pipeline_cache_handle = general_pipeline_cache.handle();
    m_errorMonitor->SetDesiredError("VUID-vkMergePipelineCaches-dstCache-11832");
    vk::MergePipelineCaches(device(), *data_graph_pipeline_cache, 1, &general_pipeline_cache_handle);
    m_errorMonitor->VerifyFound();

    // Validation headerVersion - 11833
    auto data_graph_pipeline_cache_handle = data_graph_pipeline_cache->handle();
    m_errorMonitor->SetDesiredError("VUID-vkMergePipelineCaches-headerVersion-11833");
    vk::MergePipelineCaches(device(), general_pipeline_cache2, 1, &data_graph_pipeline_cache_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDataGraphModel, CreateDataGraphModelCommandPoolWithoutProcessingEngineInfo) {
    TEST_DESCRIPTION("Try to create a VkCommandPool with queueFamilyIndex supports QCOM data graph processing engine, "
                     "but do not include VkDataGraphProcessingEngineCreateInfoARM structure in the pNext chain.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    const auto dg_queue_family_index = m_device->QueueFamily(VK_QUEUE_DATA_GRAPH_BIT_ARM);
    assert(dg_queue_family_index.has_value());

    VkCommandPoolCreateInfo command_pool_ci = vku::InitStructHelper();
    command_pool_ci.pNext = nullptr;
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = dg_queue_family_index.value();

    // Validation queueFamilyIndex - 11830
    m_errorMonitor->SetDesiredError("VUID-VkCommandPoolCreateInfo-queueFamilyIndex-11830");
    VkCommandPool command_pool = nullptr;
    vk::CreateCommandPool(device(), &command_pool_ci, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();
    vk::DestroyCommandPool(device(), command_pool, nullptr);
}

TEST_F(NegativeDataGraphModel, CreateDataGraphModelCommandPoolWithInvalidProcessingEngineInfoParameter) {
    TEST_DESCRIPTION("Try to create a VkCommandPool with queueFamilyIndex that supports QCOM data graph processing engine, "
                     "but include VkDataGraphProcessingEngineCreateInfoARM structure with invalid isForeign parameter "
                     "in the pNext chain.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    const auto dg_queue_family_index = m_device->QueueFamily(VK_QUEUE_DATA_GRAPH_BIT_ARM);
    assert(dg_queue_family_index.has_value());

    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    VkPhysicalDeviceDataGraphProcessingEngineARM dg_processing_engine{};
    dg_processing_engine.type = VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_NEURAL_QCOM;
    dg_processing_engine.isForeign = VK_FALSE;
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_processing_engine;

    VkCommandPoolCreateInfo command_pool_ci = vku::InitStructHelper();
    command_pool_ci.queueFamilyIndex = dg_queue_family_index.value();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.pNext = &dg_processing_engine_ci;

    // Validation queueFamilyIndex - 11830
    m_errorMonitor->SetDesiredError("VUID-VkCommandPoolCreateInfo-queueFamilyIndex-11830");
    VkCommandPool command_pool = nullptr;
    vk::CreateCommandPool(device(), &command_pool_ci, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();
    vk::DestroyCommandPool(device(), command_pool, nullptr);
}

TEST_F(NegativeDataGraphModel, CreateDataGraphModelDescriptorPoolWithInvalidProcessingEngineInfo) {
    TEST_DESCRIPTION("Try to create a VkDescriptorPool with a VkDevice supports data graph model, but include a invalid "
                     "VkDataGraphProcessingEngineCreateInfoARM structure in the pNext chain of VkDescriptorPoolCreateInfo structure.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    std::array<VkPhysicalDeviceDataGraphProcessingEngineARM, 2> dg_processing_engines{};
    dg_processing_engines[0].type = VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_COMPUTE_QCOM;
    dg_processing_engines[0].isForeign = VK_TRUE;
    dg_processing_engines[1].type = VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_NEURAL_QCOM;
    dg_processing_engines[1].isForeign = VK_TRUE;
    dg_processing_engine_ci.processingEngineCount = dg_processing_engines.size();
    dg_processing_engine_ci.pProcessingEngines = dg_processing_engines.data();

    VkDescriptorPoolSize descriptor_pool_size{ VK_DESCRIPTOR_TYPE_TENSOR_ARM, 2 };
    VkDescriptorPoolCreateInfo descriptor_pool_ci = vku::InitStructHelper();
    descriptor_pool_ci.pNext = &dg_processing_engine_ci;
    descriptor_pool_ci.maxSets = 1;
    descriptor_pool_ci.poolSizeCount = 1;
    descriptor_pool_ci.pPoolSizes = &descriptor_pool_size;

    // Some real qualcomm devices don't support compute engine, will lead to 09946 error, skip 09946 error
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDescriptorPoolCreateInfo-pNext-09946");
    m_errorMonitor->SetDesiredError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-11843", 2);
    VkDescriptorPool descriptor_pool = nullptr;
    vk::CreateDescriptorPool(device(), &descriptor_pool_ci, nullptr, &descriptor_pool);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorPool(device(), descriptor_pool, nullptr);
}

TEST_F(NegativeDataGraphModel, CreateDataGraphModelDescriptorPoolButDataGraphModelFeatureNotEnabled) {
    TEST_DESCRIPTION("Try to create a VkDescriptorPool with a VkDevice supports data graph model and include a valid "
                     "VkDataGraphProcessingEngineCreateInfoARM structure in the pNext chain of VkDescriptorPoolCreateInfo, "
                     "but dataGraphModel is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_ARM_DATA_GRAPH_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_DATA_GRAPH_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    AddRequiredFeature(vkt::Feature::dataGraph);

    // Note: Avoid crash due to device creation failure
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDeviceCreateInfo-queueFamilyIndex-11831");
    RETURN_IF_SKIP(Init());

    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    VkPhysicalDeviceDataGraphProcessingEngineARM dg_processing_engine{};
    dg_processing_engine.type = VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_NEURAL_QCOM;
    dg_processing_engine.isForeign = VK_TRUE;
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_processing_engine;

    VkDescriptorPoolSize descriptor_pool_size{ VK_DESCRIPTOR_TYPE_TENSOR_ARM, 2 };
    VkDescriptorPoolCreateInfo descriptor_pool_ci = vku::InitStructHelper();
    descriptor_pool_ci.pNext = &dg_processing_engine_ci;
    descriptor_pool_ci.maxSets = 1;
    descriptor_pool_ci.poolSizeCount = 1;
    descriptor_pool_ci.pPoolSizes = &descriptor_pool_size;

    m_errorMonitor->SetDesiredError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-11844");
    VkDescriptorPool descriptor_pool = nullptr;
    vk::CreateDescriptorPool(device(), &descriptor_pool_ci, nullptr, &descriptor_pool);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorPool(device(), descriptor_pool, nullptr);
}