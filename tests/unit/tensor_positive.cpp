/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "data_graph_objects.h"
#include "generated/pnext_chain_extraction.h"
#include <vector>

class PositiveTensor : public TensorTest {};

void TensorTest::InitBasicTensor() {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(Init());
}

void TensorTest::InitTensorControls() {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_ARM_TENSOR_CONTROLS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(Init());
};

// Trivial rank 1 tensor
VkTensorDescriptionARM TensorTest::DefaultDesc() {
    static std::vector<int64_t> dimensions{2};
    static std::vector<int64_t> strides{1};
    static VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
    desc.format = VK_FORMAT_R8_SINT;
    desc.dimensionCount = dimensions.size();
    desc.pDimensions = dimensions.data();
    desc.pStrides = strides.data();
    desc.usage = VK_TENSOR_USAGE_SHADER_BIT_ARM;

    return desc;
}

// Tensor matching kMinimalTensorGlsl and GetSpirvBasicShader
VkTensorDescriptionARM TensorTest::TensorShaderDesc() {
    static std::vector<int64_t> dimensions{2};
    static VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
    desc.format = VK_FORMAT_R32_SINT;
    desc.dimensionCount = dimensions.size();
    desc.pDimensions = dimensions.data();
    desc.pStrides = nullptr;
    desc.usage = VK_TENSOR_USAGE_SHADER_BIT_ARM;

    return desc;
}

VkTensorCreateInfoARM TensorTest::DefaultCreateInfo(VkTensorDescriptionARM* desc) {
    static VkTensorCreateInfoARM info = vku::InitStructHelper();
    info.pDescription = desc;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return info;
}

TEST_F(PositiveTensor, CreateTensor) {
    TEST_DESCRIPTION("Create a tensor");
    RETURN_IF_SKIP(InitBasicTensor());

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);

    vkt::Tensor tensor(*m_device, info);
}

TEST_F(PositiveTensor, ProtectedMemory) {
    TEST_DESCRIPTION("Create a protected tensor");
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitBasicTensor());

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    info.flags = VK_TENSOR_CREATE_PROTECTED_BIT_ARM;

    vkt::Tensor tensor(*m_device, info);

    tensor.BindToMem(VK_MEMORY_PROPERTY_PROTECTED_BIT);
}

TEST_F(PositiveTensor, DescriptorBuffer) {
    TEST_DESCRIPTION("Create a tensor with replay capability");
    AddRequiredFeature(vkt::Feature::descriptorBufferTensorDescriptors);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicTensor());

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    info.flags = VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;

    vkt::Tensor tensor(*m_device, info);

    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor;
    tensor_view_create_info.format = tensor.Format();
    tensor_view_create_info.flags = VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;

    vkt::TensorView view(*m_device, tensor_view_create_info);

    VkTensorViewCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensorView = view;

    uint32_t data = 0;
    vk::GetTensorViewOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
}

TEST_F(PositiveTensor, DispatchShaderGLSL) {
    TEST_DESCRIPTION("Use a tensor in a GLSL shader");
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = TensorShaderDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor;
    tensor_view_create_info.format = tensor.Format();
    vkt::TensorView view(*m_device, tensor_view_create_info);

    vkt::Buffer buffer(*m_device, tensor.GetMemoryReqs().memoryRequirements.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    CreateComputePipelineHelper pipe(*m_device);
    pipe.cs_ = VkShaderObj::CreateFromGLSL(this, kMinimalTensorGlsl, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.CreateComputePipeline();
    pipe.descriptor_set_.WriteDescriptorTensorInfo(0, &view.handle());
    pipe.descriptor_set_.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveTensor, DispatchShaderSpirv) {
    TEST_DESCRIPTION("Use a tensor in a Spir-V shader");
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = TensorShaderDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor;
    tensor_view_create_info.format = tensor.Format();
    vkt::TensorView view(*m_device, tensor_view_create_info);

    vkt::Buffer buffer(*m_device, tensor.GetMemoryReqs().memoryRequirements.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    CreateComputePipelineHelper pipe(*m_device);
    const std::string spirv_source = vkt::dg::DataGraphPipelineHelper::GetSpirvBasicShader();
    pipe.cs_ = VkShaderObj(*m_device, spirv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_4, SPV_SOURCE_ASM);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.CreateComputePipeline();
    pipe.descriptor_set_.WriteDescriptorTensorInfo(0, &view.handle());
    pipe.descriptor_set_.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveTensor, DescriptorBindingUpdateAfterBindTensor) {
    TEST_DESCRIPTION("Call UpdateAfterBind on tensors.");

    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageTensorUpdateAfterBind);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, 1, VK_SHADER_STAGE_ALL, nullptr};

    constexpr VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    vkt::DescriptorSetLayout(*m_device, create_info);
}

TEST_F(PositiveTensor, WriteDescriptorSetTensorInfoNullViewsNullDescriptor) {
    TEST_DESCRIPTION("Test writing a tensor descriptor with null tensor views");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor;
    tensor_view_create_info.format = tensor.Format();

    vkt::TensorView view(*m_device, tensor_view_create_info);

    constexpr uint32_t tensor_binding_count = 1;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, tensor_binding_count, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    std::vector<VkTensorViewARM> views = {VK_NULL_HANDLE};
    VkWriteDescriptorSetTensorARM tensor_descriptor_write = vku::InitStructHelper();
    tensor_descriptor_write.tensorViewCount = views.size();
    tensor_descriptor_write.pTensorViews = views.data();

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper(&tensor_descriptor_write);
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = tensor_binding_count;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(PositiveTensor, DescriptorTensorViewNull) {
    TEST_DESCRIPTION("Descriptor buffer with null tensor views.");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(InitBasicTensor());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    uint8_t buffer[128];

    VkDescriptorGetTensorInfoARM tensor_info = vku::InitStructHelper();
    tensor_info.tensorView = VK_NULL_HANDLE;

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper(&tensor_info);
    dgi.type = VK_DESCRIPTOR_TYPE_TENSOR_ARM;

    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
}

/* VK_ARM_tensor_controls */

TEST_F(PositiveTensor, Tilings) {
    TEST_DESCRIPTION("Create tensors with BLOCK_U and BRICK tilings");
    RETURN_IF_SKIP(InitTensorControls());

    const std::vector<int64_t> dimensions{ 1, 2, 3, 4 };
    VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.pStrides = nullptr;  // required
    desc.format = VK_FORMAT_R16_SINT;
    desc.usage = VK_TENSOR_USAGE_SHADER_BIT_ARM;

    for (auto tiling : { VK_TENSOR_TILING_BLOCK_U_INTERLEAVED_ARM,
                         VK_TENSOR_TILING_BLOCK_U_INTERLEAVED_64K_ARM,
                         VK_TENSOR_TILING_BRICK_16_WIDE_ARM,
                         VK_TENSOR_TILING_BRICK_8_WIDE_ARM,
                         VK_TENSOR_TILING_BRICK_4_WIDE_ARM }) {
        desc.tiling = tiling;
        VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

        vkt::Tensor tensor(*m_device, info);
    }
}

TEST_F(PositiveTensor, Rolling) {
    TEST_DESCRIPTION("Create rolling tensors with all valid tilings");
    RETURN_IF_SKIP(InitTensorControls());

    // same as in layers/core_checks/cc_tensor.cpp
    const std::vector<VkTensorTilingARM> valid_rolling_tilings = {
        VK_TENSOR_TILING_LINEAR_ARM,
        VK_TENSOR_TILING_BRICK_16_WIDE_ARM,
        VK_TENSOR_TILING_BRICK_8_WIDE_ARM,
        VK_TENSOR_TILING_BRICK_4_WIDE_ARM
    };

    VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.format = VK_FORMAT_R64_SINT;
    const std::vector<int64_t> dimensions{ 1, 32, 64, 4 };
    desc.dimensionCount = dimensions.size();
    desc.pDimensions = dimensions.data();
    desc.pStrides = nullptr;
    desc.usage = VK_TENSOR_USAGE_SHADER_BIT_ARM;

    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    VkTensorRollingBackingCreateInfoARM rolling_info {
        VK_STRUCTURE_TYPE_TENSOR_ROLLING_BACKING_CREATE_INFO_ARM,
        nullptr,
        { 1, 16, 16, 4 }
    };
    vvl::PnextChainAdd(&info, &rolling_info);

    for (auto tiling : valid_rolling_tilings) {
        desc.tiling = tiling;
        vkt::Tensor tensor(*m_device, info);
    }
}
