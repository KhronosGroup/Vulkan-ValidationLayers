/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/data_graph_objects.h"
#include <vector>

class PositiveTensor : public TensorTest {};

void TensorTest::InitBasicTensor() {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(Init());
}

VkTensorDescriptionARM TensorTest::DefaultDesc() {
    static std::vector<int64_t> dimensions{2ul};
    static std::vector<int64_t> strides{1l};
    static VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
    desc.format = VK_FORMAT_R8_SINT;
    desc.dimensionCount = 1;
    desc.pDimensions = dimensions.data();
    desc.pStrides = strides.data();
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
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    tensor_view_create_info.flags = VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;

    vkt::TensorView view(*m_device, tensor_view_create_info);

    VkTensorViewCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensorView = view.handle();

    uint32_t data = 0;
    vk::GetTensorViewOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
}

TEST_F(PositiveTensor, DispatchShaderGLSL) {
    TEST_DESCRIPTION("Use a tensor in a GLSL shader");
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    vkt::TensorView view(*m_device, tensor_view_create_info);

    vkt::Buffer buffer(*m_device, tensor.GetMemoryReqs().memoryRequirements.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    CreateComputePipelineHelper pipe(*m_device);
    pipe.cs_ = VkShaderObj::CreateFromGLSL(this, tensor_shader_source, VK_SHADER_STAGE_COMPUTE_BIT);

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
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveTensor, DispatchShaderSpirv) {
    TEST_DESCRIPTION("Use a tensor in a Spir-V shader");
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    vkt::TensorView view(*m_device, tensor_view_create_info);

    vkt::Buffer buffer(*m_device, tensor.GetMemoryReqs().memoryRequirements.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    CreateComputePipelineHelper pipe(*m_device);
    const std::string spirv_source = vkt::dg::DataGraphPipelineHelper::GetSpirvBasicShader();
    pipe.cs_ = VkShaderObj(this, spirv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_4, SPV_SOURCE_ASM);

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
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
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
