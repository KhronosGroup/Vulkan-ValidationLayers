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
#include "data_graph_model_pipeline_cache.h"
#include "containers/container_utils.h"
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan/vulkan_android.h"
#endif  // VK_USE_PLATFORM_ANDROID_KHR

void DataGraphModelTest::InitBasicDataGraphModel() {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_ARM_DATA_GRAPH_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_DATA_GRAPH_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    AddRequiredFeature(vkt::Feature::dataGraph);
    AddRequiredFeature(vkt::Feature::dataGraphModel);
    AddRequiredFeature(vkt::Feature::pipelineCreationCacheControl);
}

uint32_t DataGraphModelTest::FindMemoryType(const VkPhysicalDevice& physical_device, uint32_t type_bits,
                                            VkMemoryPropertyFlags property_flags) {
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vk::GetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
        if (((mem_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) && (type_bits & (1 << i))) {
            return i;
        }
    }

    return 0U;
}

DataGraphModelTest::DGModelTensorInfo DataGraphModelTest::BuildTensor(const VkLayerTest& layer_test,
                                                                      const VkTensorDescriptionARM& tensor_description) {
    // Create tensor
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VkExternalMemoryTensorCreateInfoARM external_memory_tensor_create_info{
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_TENSOR_CREATE_INFO_ARM,
        nullptr,
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    VkTensorCreateInfoARM tensor_info{
        VK_STRUCTURE_TYPE_TENSOR_CREATE_INFO_ARM,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        &external_memory_tensor_create_info,
#else
        nullptr,
#endif  // VK_USE_PLATFORM_ANDROID_KHR
        0,
        &tensor_description,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VkExportMemoryAllocateInfo export_memory_allocate_info{
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        nullptr,
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID
    };
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    const auto* device = layer_test.DeviceObj();
    auto tensor_arm = std::make_shared<vkt::Tensor>();
    tensor_arm->InitNoMem(*device, tensor_info);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    tensor_arm->BindToMem(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, &export_memory_allocate_info);
#else
    tensor_arm->BindToMem(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    // Create tensor view
    VkTensorViewCreateInfoARM tensor_view_info{
        VK_STRUCTURE_TYPE_TENSOR_VIEW_CREATE_INFO_ARM,
        nullptr,
        0,
        tensor_arm->handle(),
        tensor_description.format
    };
    auto tensor_view_arm = std::make_shared<vkt::TensorView>();
    tensor_view_arm->Init(*device, tensor_view_info);

    DGModelTensorInfo dg_model_tensor_info{
        tensor_arm,
        tensor_view_arm
    };

    return dg_model_tensor_info;
}

DataGraphModelTest::DGModelPipelineLayoutInfo DataGraphModelTest::BuildGeneralPipelineLayout(const VkLayerTest& layer_test) {
    const auto* device = layer_test.DeviceObj();
    // Create data graph pipeline layout
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci = vku::InitStructHelper();
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
    descriptor_set_layout_ci.bindingCount = bindings.size();
    descriptor_set_layout_ci.pBindings = bindings.data();
    auto descriptor_set_layout = std::make_shared<vkt::DescriptorSetLayout>(*device, descriptor_set_layout_ci);
    auto descriptor_set_layout_handle = descriptor_set_layout->handle();

    // Create data graph pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &descriptor_set_layout_handle;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;
    auto pipeline_layout = std::make_shared<vkt::PipelineLayout>(*device, pipeline_layout_ci);

    DGModelPipelineLayoutInfo dg_model_pipeline_layout_info{
        descriptor_set_layout,
        pipeline_layout
    };

    return dg_model_pipeline_layout_info;
}

void DataGraphModelTest::TestDataGraphPipelineCreationOnce(const VkLayerTest& layer_test, const vkt::PipelineCache& pipeline_cache) {
    const auto dg_queue_family_index = layer_test.DeviceObj()->QueueFamily(VK_QUEUE_DATA_GRAPH_BIT_ARM);
    assert(dg_queue_family_index.has_value());
    // Query and fill VkPhysicalDeviceDataGraphProcessingEngineARM and VkPhysicalDeviceDataGraphOperationSupportARM structures
    uint32_t dg_properties_count = 0;
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(layer_test.Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, nullptr);
    assert(dg_properties_count >= 1);
    std::vector<VkQueueFamilyDataGraphPropertiesARM> dg_queue_family_properties(dg_properties_count);
    for (uint32_t index = 0; index < dg_properties_count; ++index) {
        dg_queue_family_properties[index].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM;
    }
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(layer_test.Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, dg_queue_family_properties.data());
    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_queue_family_properties[0].engine;

    // Data graph pipeline identifier create information, QCOM do not care identifier currently
    constexpr std::array<uint8_t, 4> qnn_graph_id{ 0, 0, 0, 0 };
    VkDataGraphPipelineIdentifierCreateInfoARM identifier_ci = vku::InitStructHelper();
    identifier_ci.pNext = &dg_processing_engine_ci;
    identifier_ci.identifierSize = qnn_graph_id.size();
    identifier_ci.pIdentifier = qnn_graph_id.data();

    // Create data graph pipeline layout
    DGModelPipelineLayoutInfo dg_pipeline_layout_info = BuildGeneralPipelineLayout(layer_test);

    // Data graph pipeline create information
    VkDataGraphPipelineCreateInfoARM data_graph_pipeline_ci = vku::InitStructHelper();
    data_graph_pipeline_ci.pNext = &identifier_ci;
    data_graph_pipeline_ci.flags = VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    data_graph_pipeline_ci.layout = dg_pipeline_layout_info.pipeline_layout->handle();
    data_graph_pipeline_ci.resourceInfoCount = 0;
    data_graph_pipeline_ci.pResourceInfos = nullptr;

    // Build data graph pipeline
    VkPipeline data_graph_pipeline = nullptr;
    vk::CreateDataGraphPipelinesARM(layer_test.device(), nullptr, pipeline_cache, 1,
                                    &data_graph_pipeline_ci, nullptr, &data_graph_pipeline);

    vk::DestroyPipeline(layer_test.device(), data_graph_pipeline, nullptr);
}

void DataGraphModelTest::TestDataGraphPipelineCreationOnce(const VkLayerTest& layer_test,
                                                           const VkDataGraphPipelineBuiltinModelCreateInfoQCOM& builtin_model_ci) {
    // Create data graph pipeline layout
    DGModelPipelineLayoutInfo dg_pipeline_layout_info = BuildGeneralPipelineLayout(layer_test);

    // Create data graph pipeline with builtin model information
    VkDataGraphPipelineCreateInfoARM data_graph_pipeline_ci = vku::InitStructHelper();
    data_graph_pipeline_ci.pNext = &builtin_model_ci;
    data_graph_pipeline_ci.flags = VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    data_graph_pipeline_ci.layout = dg_pipeline_layout_info.pipeline_layout->handle();
    data_graph_pipeline_ci.resourceInfoCount = 0;
    data_graph_pipeline_ci.pResourceInfos = nullptr;

    VkPipeline data_graph_pipeline = nullptr;
    vk::CreateDataGraphPipelinesARM(layer_test.device(), nullptr, nullptr, 1,
                                    &data_graph_pipeline_ci, nullptr, &data_graph_pipeline);

    vk::DestroyPipeline(layer_test.device(), data_graph_pipeline, nullptr);
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildValidPipelineCache(const VkLayerTest& layer_test) {
    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = kDGValidPipelineCacheData.size();
    pipeline_cache_ci.pInitialData = kDGValidPipelineCacheData.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildPipelineCacheWithInvalidHeaderSize(const VkLayerTest& layer_test) {
    VkPipelineCacheHeaderVersionDataGraphQCOM dg_header_version{};
    std::vector<uint8_t> dg_pipeline_cache_data{};
    dg_pipeline_cache_data.reserve(kDGValidPipelineCacheData.size());
    std::copy(kDGValidPipelineCacheData.begin(), kDGValidPipelineCacheData.end(), std::back_inserter(dg_pipeline_cache_data));
    memcpy(&dg_header_version, kDGValidPipelineCacheData.data(), sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    // Modify header size
    dg_header_version.headerSize = sizeof(VkPipelineCacheHeaderVersionOne);
    memcpy(dg_pipeline_cache_data.data(), &dg_header_version, sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = dg_pipeline_cache_data.size();
    pipeline_cache_ci.pInitialData = dg_pipeline_cache_data.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildPipelineCacheWithInvalidGreaterHeaderSize(const VkLayerTest& layer_test) {
    VkPipelineCacheHeaderVersionDataGraphQCOM dg_header_version{};
    std::vector<uint8_t> dg_pipeline_cache_data{};
    dg_pipeline_cache_data.reserve(kDGValidPipelineCacheData.size());
    std::copy(kDGValidPipelineCacheData.begin(), kDGValidPipelineCacheData.end(), std::back_inserter(dg_pipeline_cache_data));
    memcpy(&dg_header_version, kDGValidPipelineCacheData.data(), sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    // Modify header size
    dg_header_version.headerSize = 2 * kDGValidPipelineCacheData.size();
    memcpy(dg_pipeline_cache_data.data(), &dg_header_version, sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = dg_pipeline_cache_data.size();
    pipeline_cache_ci.pInitialData = dg_pipeline_cache_data.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildPipelineCacheWithUnsupportedHeaderVersion(const VkLayerTest& layer_test) {
    VkPipelineCacheHeaderVersionDataGraphQCOM dg_header_version{};
    std::vector<uint8_t> dg_pipeline_cache_data{};
    dg_pipeline_cache_data.reserve(kDGValidPipelineCacheData.size());
    std::copy(kDGValidPipelineCacheData.begin(), kDGValidPipelineCacheData.end(), std::back_inserter(dg_pipeline_cache_data));
    memcpy(&dg_header_version, kDGValidPipelineCacheData.data(), sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    // Modify header version
    dg_header_version.headerVersion = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
    memcpy(dg_pipeline_cache_data.data(), &dg_header_version, sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = dg_pipeline_cache_data.size();
    pipeline_cache_ci.pInitialData = dg_pipeline_cache_data.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildPipelineCacheWithInvalidHeaderVersion(const VkLayerTest& layer_test) {
    VkPipelineCacheHeaderVersionDataGraphQCOM dg_header_version{};
    std::vector<uint8_t> dg_pipeline_cache_data{};
    dg_pipeline_cache_data.reserve(kDGValidPipelineCacheData.size());
    std::copy(kDGValidPipelineCacheData.begin(), kDGValidPipelineCacheData.end(), std::back_inserter(dg_pipeline_cache_data));
    memcpy(&dg_header_version, kDGValidPipelineCacheData.data(), sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    // Modify header version
    dg_header_version.headerVersion = VK_PIPELINE_CACHE_HEADER_VERSION_MAX_ENUM;
    memcpy(dg_pipeline_cache_data.data(), &dg_header_version, sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = dg_pipeline_cache_data.size();
    pipeline_cache_ci.pInitialData = dg_pipeline_cache_data.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

std::shared_ptr<vkt::PipelineCache> DataGraphModelTest::BuildPipelineCacheWithInvalidCacheType(const VkLayerTest& layer_test) {
    VkPipelineCacheHeaderVersionDataGraphQCOM dg_header_version{};
    std::vector<uint8_t> dg_pipeline_cache_data{};
    dg_pipeline_cache_data.reserve(kDGValidPipelineCacheData.size());
    std::copy(kDGValidPipelineCacheData.begin(), kDGValidPipelineCacheData.end(), std::back_inserter(dg_pipeline_cache_data));
    memcpy(&dg_header_version, kDGValidPipelineCacheData.data(), sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    // Modify cache type
    dg_header_version.cacheType = VK_DATA_GRAPH_MODEL_CACHE_TYPE_MAX_ENUM_QCOM;
    memcpy(dg_pipeline_cache_data.data(), &dg_header_version, sizeof(VkPipelineCacheHeaderVersionDataGraphQCOM));

    VkPipelineCacheCreateInfo pipeline_cache_ci = vku::InitStructHelper();
    pipeline_cache_ci.initialDataSize = dg_pipeline_cache_data.size();
    pipeline_cache_ci.pInitialData = dg_pipeline_cache_data.data();
    const auto* device = layer_test.DeviceObj();
    auto pipeline_cache = std::make_shared<vkt::PipelineCache>(*device, pipeline_cache_ci);

    return pipeline_cache;
}

class PositiveDataGraphModel : public DataGraphModelTest {};

TEST_F(PositiveDataGraphModel, CreateDataGraphPipelineWithPipelineCache) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via a valid pipeline cache.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    auto pipeline_cache = BuildValidPipelineCache(*this);

    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    TestDataGraphPipelineCreationOnce(*this, *pipeline_cache);
}

TEST_F(PositiveDataGraphModel, CreateDataGraphPipelineWithBuiltinModelInfo) {
    TEST_DESCRIPTION("Try to create a data graph pipeline cache via builtin model information.");
    InitBasicDataGraphModel();
    RETURN_IF_SKIP(Init());

    const auto dg_queue_family_index = m_device->QueueFamily(VK_QUEUE_DATA_GRAPH_BIT_ARM);
    assert(dg_queue_family_index.has_value());

    // Query and fill VkPhysicalDeviceDataGraphProcessingEngineARM and VkPhysicalDeviceDataGraphOperationSupportARM structures
    uint32_t dg_properties_count = 0;
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, nullptr);
    assert(dg_properties_count >= 1);
    std::vector<VkQueueFamilyDataGraphPropertiesARM> dg_queue_family_properties(dg_properties_count);
    for (uint32_t index = 0; index < dg_properties_count; ++index) {
        dg_queue_family_properties[index].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM;
    }
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, dg_queue_family_properties.data());

    // Data graph processing engine create information
    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_queue_family_properties[0].engine;

    // Data graph pipeline builtin model creation information
    VkDataGraphPipelineBuiltinModelCreateInfoQCOM builtin_model_ci = vku::InitStructHelper();
    builtin_model_ci.pOperation = &dg_queue_family_properties[0].operation;
    builtin_model_ci.pNext = &dg_processing_engine_ci;

    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");
    TestDataGraphPipelineCreationOnce(*this, builtin_model_ci);
}

TEST_F(PositiveDataGraphModel, ExecuteDataGraphModel) {
    TEST_DESCRIPTION("Try to create a data graph model and execute it.");
    InitBasicDataGraphModel();
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    AddOptionalExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    RETURN_IF_SKIP(Init());

    // Note: According to spec, when include VkDataGraphPipelineIdentifierCreateInfoARM structure in the pNext chain of
    //       VkDataGraphPipelineCreateInfoARM structure, resourceInfoCount must be 0 and pResourceInfos must be NULL.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDataGraphPipelineCreateInfoARM-resourceInfoCount-arraylength");

    // Query and fill VkPhysicalDeviceDataGraphProcessingEngineARM and VkPhysicalDeviceDataGraphOperationSupportARM structures
    const auto dg_queue_family_index = m_device->QueueFamily(VK_QUEUE_DATA_GRAPH_BIT_ARM);
    assert(dg_queue_family_index.has_value());
    uint32_t dg_properties_count = 0;
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, nullptr);
    assert(dg_properties_count >= 1);
    std::vector<VkQueueFamilyDataGraphPropertiesARM> dg_queue_family_properties(dg_properties_count);
    for (uint32_t index = 0; index < dg_properties_count; ++index) {
        dg_queue_family_properties[index].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM;
    }
    vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(Gpu(), dg_queue_family_index.value(),
                                                           &dg_properties_count, dg_queue_family_properties.data());
    VkDataGraphProcessingEngineCreateInfoARM dg_processing_engine_ci = vku::InitStructHelper();
    dg_processing_engine_ci.processingEngineCount = 1;
    dg_processing_engine_ci.pProcessingEngines = &dg_queue_family_properties[0].engine;

    // Create two general tensors
    constexpr std::array<int64_t, 3> input_dimensions{ 960, 540, 4 };
    constexpr std::array<int64_t, 3> output_dimensions{ 1920, 1080, 4 };
    VkTensorDescriptionARM input_tensor_description{
        VK_STRUCTURE_TYPE_TENSOR_DESCRIPTION_ARM,
        nullptr,
        VK_TENSOR_TILING_LINEAR_ARM,
        VK_FORMAT_R8_UNORM,
        input_dimensions.size(),
        input_dimensions.data(),
        nullptr,
        VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM
    };
    VkTensorDescriptionARM output_tensor_description = input_tensor_description;
    output_tensor_description.pDimensions = output_dimensions.data();
    DGModelTensorInfo input_tensor_info = BuildTensor(*this, input_tensor_description);
    DGModelTensorInfo output_tensor_info = BuildTensor(*this, output_tensor_description);

    // Create command pool
    VkCommandPoolCreateInfo command_pool_ci = vku::InitStructHelper();
    command_pool_ci.queueFamilyIndex = dg_queue_family_index.value();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.pNext = &dg_processing_engine_ci;
    vkt::CommandPool command_pool{ *m_device, command_pool_ci };

    // Allocate command buffer
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkt::CommandBuffer command_buffer{ *m_device, command_buffer_allocate_info };

    // Create data graph pipeline layout
    DGModelPipelineLayoutInfo dg_pipeline_layout_info = BuildGeneralPipelineLayout(*this);

    // Create descriptor pool
    VkDescriptorPoolSize descriptor_pool_size{ VK_DESCRIPTOR_TYPE_TENSOR_ARM, 2 };
    VkDescriptorPoolCreateInfo descriptor_pool_ci = vku::InitStructHelper();
    descriptor_pool_ci.pNext = &dg_processing_engine_ci;
    descriptor_pool_ci.maxSets = 1;
    descriptor_pool_ci.poolSizeCount = 1;
    descriptor_pool_ci.pPoolSizes = &descriptor_pool_size;
    vkt::DescriptorPool descriptor_pool{ *m_device, descriptor_pool_ci };

    // Allocate descriptor set
    auto descriptor_set_layout_handle = dg_pipeline_layout_info.descriptor_set_layout->handle();
    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = vku::InitStructHelper();
    descriptor_set_alloc_info.descriptorPool = descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = 1;
    descriptor_set_alloc_info.pSetLayouts = &descriptor_set_layout_handle;
    VkDescriptorSet descriptor_set_handle = nullptr;
    vk::AllocateDescriptorSets(device(), &descriptor_set_alloc_info, &descriptor_set_handle);
    vkt::DescriptorSet descriptor_set{ *m_device, &descriptor_pool, descriptor_set_handle };

    // Update descriptor set
    auto input_tensor_view_handle = input_tensor_info.tensor_view_arm->handle();
    auto output_tensor_view_handle = output_tensor_info.tensor_view_arm->handle();
    std::array<VkWriteDescriptorSetTensorARM, 2> write_descriptor_set_tensors = {};
    write_descriptor_set_tensors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM;
    write_descriptor_set_tensors[0].pNext = nullptr;
    write_descriptor_set_tensors[0].tensorViewCount = 1;
    write_descriptor_set_tensors[0].pTensorViews = &input_tensor_view_handle;
    write_descriptor_set_tensors[1] = write_descriptor_set_tensors[0];
    write_descriptor_set_tensors[1].pTensorViews = &output_tensor_view_handle;
    std::array<VkWriteDescriptorSet, 2> write_descriptor_sets = {};
    write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_sets[0].pNext = &write_descriptor_set_tensors[0];
    write_descriptor_sets[0].dstBinding = kInputBindingPoint;
    write_descriptor_sets[0].descriptorCount = 1;
    write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;
    write_descriptor_sets[0].dstSet = descriptor_set;
    write_descriptor_sets[0].dstArrayElement = 0;
    write_descriptor_sets[0].pBufferInfo = nullptr;
    write_descriptor_sets[0].pImageInfo = nullptr;
    write_descriptor_sets[0].pTexelBufferView = nullptr;
    write_descriptor_sets[1] = write_descriptor_sets[0];
    write_descriptor_sets[1].pNext = &write_descriptor_set_tensors[1];
    write_descriptor_sets[1].dstBinding = kOutputBindingPoint;
    vk::UpdateDescriptorSets(device(), 2, write_descriptor_sets.data(), 0, nullptr);

    // Create data graph pipeline cache
    auto pipeline_cache = BuildValidPipelineCache(*this);

    // Create data graph pipeline - do not care identifier currently
    constexpr std::array<uint8_t, 4> qnn_graph_id{ 0, 0, 0, 0 };
    VkDataGraphPipelineIdentifierCreateInfoARM identifier_ci = vku::InitStructHelper();
    identifier_ci.pNext = &dg_processing_engine_ci;
    identifier_ci.identifierSize = qnn_graph_id.size();
    identifier_ci.pIdentifier = qnn_graph_id.data();
    VkDataGraphPipelineCreateInfoARM data_graph_pipeline_ci = vku::InitStructHelper();
    data_graph_pipeline_ci.pNext = &identifier_ci;
    data_graph_pipeline_ci.flags = VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    data_graph_pipeline_ci.layout = dg_pipeline_layout_info.pipeline_layout->handle();
    data_graph_pipeline_ci.resourceInfoCount = 0;
    data_graph_pipeline_ci.pResourceInfos = nullptr;
    VkPipeline data_graph_pipeline = nullptr;
    vk::CreateDataGraphPipelinesARM(device(), nullptr, *pipeline_cache, 1,
                                    &data_graph_pipeline_ci, nullptr, &data_graph_pipeline);
    assert(data_graph_pipeline != nullptr);

    // Create data graph pipeline session
    VkDataGraphPipelineSessionCreateInfoARM session_ci{
        VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_CREATE_INFO_ARM,
        nullptr,
        0,
        data_graph_pipeline
    };
    VkDataGraphPipelineSessionARM data_graph_pipeline_session = nullptr;
    vk::CreateDataGraphPipelineSessionARM(device(), &session_ci, nullptr, &data_graph_pipeline_session);
    assert(data_graph_pipeline_session != nullptr);

    // Allocate and bind data graph session memory objects
    uint32_t session_bind_point_req_count = 0;
    VkDataGraphPipelineSessionBindPointRequirementsInfoARM session_bind_point_req_info{
        VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_REQUIREMENTS_INFO_ARM,
        nullptr,
        data_graph_pipeline_session
    };
    vk::GetDataGraphPipelineSessionBindPointRequirementsARM(device(), &session_bind_point_req_info,
                                                            &session_bind_point_req_count, nullptr);
    std::vector<VkDataGraphPipelineSessionBindPointRequirementARM> session_bind_point_reqs(session_bind_point_req_count);
    std::vector<VkDeviceMemory> session_memory_objects{};
    std::vector<VkBindDataGraphPipelineSessionMemoryInfoARM> session_memory_infos{};
    for (uint32_t index = 0; index < session_bind_point_req_count; ++index) {
        session_bind_point_reqs[index].sType = VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_REQUIREMENT_ARM;
    }
    vk::GetDataGraphPipelineSessionBindPointRequirementsARM(device(), &session_bind_point_req_info,
                                                            &session_bind_point_req_count, session_bind_point_reqs.data());

    uint32_t session_mem_count = 0;
    for (uint32_t req_index = 0; req_index < session_bind_point_req_count; ++req_index) {
        const auto& cur_session_bind_point_req = session_bind_point_reqs[req_index];
        if (cur_session_bind_point_req.bindPointType != VK_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_TYPE_MEMORY_ARM) {
            continue;
        }

        session_memory_objects.resize(session_memory_objects.size() + cur_session_bind_point_req.numObjects);
        session_memory_infos.resize(session_memory_infos.size() + cur_session_bind_point_req.numObjects);
        for (uint32_t obj_index = 0; obj_index < cur_session_bind_point_req.numObjects; ++obj_index) {
            VkDataGraphPipelineSessionMemoryRequirementsInfoARM mem_req_info{
                VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_MEMORY_REQUIREMENTS_INFO_ARM,
                nullptr,
                data_graph_pipeline_session,
                cur_session_bind_point_req.bindPoint,
                obj_index
            };
            VkMemoryRequirements2 memory_requirements2 = vku::InitStructHelper();
            vk::GetDataGraphPipelineSessionMemoryRequirementsARM(device(), &mem_req_info, &memory_requirements2);
            uint32_t mem_index = FindMemoryType(Gpu(), memory_requirements2.memoryRequirements.memoryTypeBits,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VkMemoryAllocateInfo mem_alloc_info{
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                memory_requirements2.memoryRequirements.size,
                mem_index
            };
            vk::AllocateMemory(device(), &mem_alloc_info, nullptr, &session_memory_objects[session_mem_count]);
            session_memory_infos[session_mem_count].sType = VK_STRUCTURE_TYPE_BIND_DATA_GRAPH_PIPELINE_SESSION_MEMORY_INFO_ARM;
            session_memory_infos[session_mem_count].pNext = nullptr;
            session_memory_infos[session_mem_count].session = data_graph_pipeline_session;
            session_memory_infos[session_mem_count].bindPoint = cur_session_bind_point_req.bindPoint;
            session_memory_infos[session_mem_count].objectIndex = obj_index;
            session_memory_infos[session_mem_count].memory = session_memory_objects[session_mem_count];
            session_memory_infos[session_mem_count].memoryOffset = 0;
            ++session_mem_count;
        }
    }

    // Note: Currently QCOM data graph pipeline session doesn't have any memory requirements, so the bindInfoCount will be 0,
    //       it will violate the specific Vulkan spec item.
    if (!session_memory_infos.empty()) {
        vk::BindDataGraphPipelineSessionMemoryARM(*m_device, session_memory_infos.size(), session_memory_infos.data());
    }

    // Record and execute command packets
    VkDataGraphPipelineDispatchInfoARM data_graph_pipeline_dispatch_info{
        VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_DISPATCH_INFO_ARM,
        nullptr,
        0
    };
    VkQueue data_graph_queue_handle = nullptr;
    vk::GetDeviceQueue(device(), dg_queue_family_index.value(), 0, &data_graph_queue_handle);
    assert(data_graph_queue_handle != nullptr);
    vkt::Queue data_graph_queue{ data_graph_queue_handle, 0 };
    command_buffer.Begin();
    vk::CmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, data_graph_pipeline);
    vk::CmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM,
                        dg_pipeline_layout_info.pipeline_layout->handle(), 0, 1,
                              &descriptor_set_handle, 0, nullptr);
    vk::CmdDispatchDataGraphARM(command_buffer, data_graph_pipeline_session, &data_graph_pipeline_dispatch_info);
    command_buffer.End();
    data_graph_queue.SubmitAndWait(command_buffer);

    // Destroy resources
    vk::DestroyPipeline(device(), data_graph_pipeline, nullptr);
    vk::DestroyDataGraphPipelineSessionARM(device(), data_graph_pipeline_session, nullptr);
    for (size_t index = 0; index < session_memory_objects.size(); ++index) {
        vk::FreeMemory(device(), session_memory_objects[index], nullptr);
    }
}