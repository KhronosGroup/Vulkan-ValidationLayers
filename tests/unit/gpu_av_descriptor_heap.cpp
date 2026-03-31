/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/buffer_helper.h"
#include "../utils/math_utils.h"

void GpuAVDescriptorHeap::InitGpuAVDescriptorHeap(bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
    RETURN_IF_SKIP(InitState());
    GetPhysicalDeviceProperties2(heap_props);
}

void GpuAVDescriptorHeap::CreateResourceHeap(VkDeviceSize app_size) {
    const VkDeviceSize heap_size = AlignResource(app_size) + heap_props.minResourceHeapReservedRange;
    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    resource_heap_.Init(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    resource_heap_data_ = static_cast<uint8_t*>(resource_heap_.Memory().Map());
}

void GpuAVDescriptorHeap::CreateSamplerHeap(VkDeviceSize app_size, bool use_embedded_samplers) {
    embedded_samplers = use_embedded_samplers;
    const VkDeviceSize reserved_range =
        (embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange);
    const VkDeviceSize heap_size = AlignSampler(app_size + reserved_range);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    sampler_heap_.Init(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    sampler_heap_data_ = static_cast<uint8_t*>(sampler_heap_.Memory().Map());
}

void GpuAVDescriptorHeap::BindResourceHeap() {
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
}

void GpuAVDescriptorHeap::BindSamplerHeap() {
    const VkDeviceSize min_reserved_range =
        embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange;
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = sampler_heap_.Address();
    bind_resource_info.heapRange.size = sampler_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = sampler_heap_.CreateInfo().size - min_reserved_range;
    bind_resource_info.reservedRangeSize = min_reserved_range;
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);
}

VkDeviceSize GpuAVDescriptorHeap::AlignResource(VkDeviceSize offset) {
    return Align(Align(offset, heap_props.bufferDescriptorAlignment), heap_props.imageDescriptorAlignment);
}

VkDeviceSize GpuAVDescriptorHeap::AlignSampler(VkDeviceSize offset) { return Align(offset, heap_props.samplerDescriptorAlignment); }

class NegativeGpuAVDescriptorHeap : public GpuAVDescriptorHeap {};

TEST_F(NegativeGpuAVDescriptorHeap, IndexBufferOOB) {
    TEST_DESCRIPTION("Validate overruning the index buffer");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 1;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {1, 2, 3});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    m_errorMonitor->SetDesiredErrorRegex("VUID-VkDrawIndexedIndirectCommand-robustBufferAccess2-08798",
                                         "Index 4 is not within the bound index buffer.");
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, NoMappings) {
    TEST_DESCRIPTION("Validate illegal index buffer values with no VkShaderDescriptorSetAndBindingMappingInfoEXT provided");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, NoHeapBound) {
    TEST_DESCRIPTION("Validate illegal index buffer values when no descriptor heap is bound");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.shader_stages_[0].pNext = &mapping_info;
    pipe.shader_stages_[1].pNext = &mapping_info;
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, HeapBoundBeforePipeline) {
    TEST_DESCRIPTION("Validate illegal index buffer values when descriptor heap is bound before the pipeline");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.shader_stages_[0].pNext = &mapping_info;
    pipe.shader_stages_[1].pNext = &mapping_info;
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, HeapBoundAfterPipeline) {
    TEST_DESCRIPTION("Validate illegal index buffer values when descriptor heap is bound after pipeline");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.shader_stages_[0].pNext = &mapping_info;
    pipe.shader_stages_[1].pNext = &mapping_info;
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, SamplerHeapBound) {
    TEST_DESCRIPTION("Validate illegal index buffer values when sampler heap is bound");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize descriptor_size = AlignSampler(heap_props.samplerDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minSamplerHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.shader_stages_[0].pNext = &mapping_info;
    pipe.shader_stages_[1].pNext = &mapping_info;
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, MappingsUsed) {
    TEST_DESCRIPTION("Validate illegal index buffer values when custom mappings are used");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    uint8_t* heap_data = static_cast<uint8_t*>(heap.Memory().Map());

    vkt::Buffer buffer1(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer2(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer3(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDeviceAddressRangeEXT address_ranges[3];
    address_ranges[0].address = buffer1.Address();
    address_ranges[0].size = 256u;
    address_ranges[1].address = buffer2.Address();
    address_ranges[1].size = 256u;
    address_ranges[2].address = buffer3.Address();
    address_ranges[2].size = 256u;

    VkResourceDescriptorInfoEXT resource_infos[3];
    resource_infos[0] = vku::InitStructHelper();
    resource_infos[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    resource_infos[0].data.pAddressRange = &address_ranges[0];
    resource_infos[1] = vku::InitStructHelper();
    resource_infos[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    resource_infos[1].data.pAddressRange = &address_ranges[1];
    resource_infos[2] = vku::InitStructHelper();
    resource_infos[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    resource_infos[2].data.pAddressRange = &address_ranges[2];

    VkHostAddressRangeEXT host_addresses[3];
    host_addresses[0].address = heap_data;
    host_addresses[0].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    host_addresses[1].address = heap_data + heap_props.bufferDescriptorAlignment;
    host_addresses[1].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    host_addresses[2].address = heap_data + heap_props.bufferDescriptorAlignment * 2u;
    host_addresses[2].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    vk::WriteResourceDescriptorsEXT(*m_device, 3u, resource_infos, host_addresses);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        layout(set=0, binding=0) buffer Buf1 { uint data1[]; } buf1;
        layout(set=0, binding=1) readonly buffer Buf2 { uint data2[]; } buf2;
        layout(set=1, binding=2) buffer Buf3 { uint data3[]; } buf3;
        void main() {
            buf3.data3[gl_VertexIndex] = buf1.data1[gl_VertexIndex] + buf2.data2[gl_VertexIndex];
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = vku::InitStructHelper();
    mappings[0].descriptorSet = 0u;
    mappings[0].firstBinding = 0u;
    mappings[0].bindingCount = 1u;
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = vku::InitStructHelper();
    mappings[1].descriptorSet = 0u;
    mappings[1].firstBinding = 1u;
    mappings[1].bindingCount = 1u;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT;
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);
    mappings[2] = vku::InitStructHelper();
    mappings[2].descriptorSet = 1u;
    mappings[2].firstBinding = 2u;
    mappings[2].bindingCount = 1u;
    mappings[2].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment * 2u);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.shader_stages_[0].pNext = &mapping_info;
    pipe.shader_stages_[1].pNext = &mapping_info;
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, DispatchWorkgroupSize) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand with descriptor heap");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());
    GetPhysicalDeviceProperties2(heap_props);

    vkt::Buffer indirect_buffer(*m_device, 5 * sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                kHostVisibleMemProps);
    VkDispatchIndirectCommand* ptr = static_cast<VkDispatchIndirectCommand*>(indirect_buffer.Memory().Map());
    // VkDispatchIndirectCommand[0]
    ptr->x = 4;  // over
    ptr->y = 2;
    ptr->z = 1;
    // VkDispatchIndirectCommand[1]
    ptr++;
    ptr->x = 2;
    ptr->y = 3;  // over
    ptr->z = 1;
    // VkDispatchIndirectCommand[2] - valid in between
    ptr++;
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;
    // VkDispatchIndirectCommand[3]
    ptr++;
    ptr->x = 0;  // allowed
    ptr->y = 2;
    ptr->z = 3;  // over
    // VkDispatchIndirectCommand[4]
    ptr++;
    ptr->x = 3;  // over
    ptr->y = 2;
    ptr->z = 3;  // over

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = pipe.cs_.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 4 * sizeof(VkDispatchIndirectCommand));

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Check again in a 2nd submitted command buffer
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);

    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, HeapRebound) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap1(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    vkt::Buffer heap2(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;

    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.stageCount = pipe.shader_stages_.size();
    pipe.gp_ci_.pStages = pipe.shader_stages_.data();
    pipe.CreateGraphicsPipeline(false);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap1.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    bind_resource_info.heapRange.address = heap2.Address();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ShaderObjects) {
    TEST_DESCRIPTION("Validate illegal index buffer values with shader objects");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitDynamicRenderTarget();

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorAlignment * 4u);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    const char* vsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    const auto vspv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource);
    VkShaderCreateInfoEXT vert_ci = vku::InitStructHelper();
    vert_ci.flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    vert_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_ci.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vert_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vert_ci.codeSize = vspv.size() * sizeof(vspv[0]);
    vert_ci.pCode = vspv.data();
    vert_ci.pName = "main";

    const auto fspv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT frag_ci = vku::InitStructHelper();
    frag_ci.flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    frag_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    frag_ci.codeSize = fspv.size() * sizeof(fspv[0]);
    frag_ci.pCode = fspv.data();
    frag_ci.pName = "main";

    const vkt::Shader vert_shader(*m_device, vert_ci);
    const vkt::Shader frag_shader(*m_device, frag_ci);

    VkDrawIndexedIndirectCommand draw_params{};
    draw_params.indexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstIndex = 0;
    draw_params.vertexOffset = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    VkShaderStageFlagBits stages[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkShaderEXT shaders[2] = {vert_shader.handle(), frag_shader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer, 2, stages, shaders);

    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer, VK_FALSE);
    vk::CmdSetCullModeEXT(m_command_buffer, VK_CULL_MODE_NONE);
    vk::CmdSetDepthTestEnableEXT(m_command_buffer, VK_FALSE);
    vk::CmdSetStencilTestEnableEXT(m_command_buffer, VK_FALSE);
    vk::CmdSetPolygonModeEXT(m_command_buffer, VK_POLYGON_MODE_FILL);
    vk::CmdSetPrimitiveTopology(m_command_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer, VK_FALSE);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(m_command_buffer, 0, 1u, &colorBlendEquation);
    VkViewport viewport = {0.0f, 0.0f, 32.0f, 32.0f, 0.0f, 1.0f};
    vk::CmdSetViewportWithCountEXT(m_command_buffer, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {32, 32}};
    vk::CmdSetScissorWithCountEXT(m_command_buffer, 1, &scissor);
    vk::CmdSetDepthBiasEnableEXT(m_command_buffer, VK_FALSE);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    vk::CmdSetSampleMaskEXT(m_command_buffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vk::CmdSetAlphaToCoverageEnableEXT(m_command_buffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer, 0u, 1u, &colorBlendEnable);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(m_command_buffer, 0u, 1u, &colorWriteMask);

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0u;
    binding.stride = 3 * sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1u;

    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0u;
    attribute.binding = 0u;
    attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute.offset = 0u;
    vk::CmdSetVertexInputEXT(m_command_buffer, 1u, &binding, 1u, &attribute);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721", "Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, PushAddressUniformBufferUsage) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer read_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    uint32_t* read_data = static_cast<uint32_t*>(read_buffer.Memory().Map());
    for (uint32_t i = 0; i < 4; ++i) {
        read_data[i] = i + 1;
    }
    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const uint32_t read_offset = 48u;
    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorAlignment * 7u, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize descriptor_size = AlignResource(write_offset + resource_stride);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    uint8_t* heap_data = static_cast<uint8_t*>(heap.Memory().Map());

    VkHostAddressRangeEXT descriptor_host;
    descriptor_host.address = heap_data + write_offset;
    descriptor_host.size = resource_stride;

    VkDeviceAddressRangeEXT device_range;
    device_range.address = write_buffer.Address();
    device_range.size = write_buffer.CreateInfo().size;

    VkResourceDescriptorInfoEXT descriptor_info;
    descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = vku::InitStructHelper();
    mappings[0].descriptorSet = 2u;
    mappings[0].firstBinding = 3u;
    mappings[0].bindingCount = 1u;
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT;
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = static_cast<uint32_t>(read_offset);
    mappings[1] = vku::InitStructHelper();
    mappings[1].descriptorSet = 1u;
    mappings[1].firstBinding = 0u;
    mappings[1].bindingCount = 1u;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 2, binding = 3) uniform ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[3];
            }
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkDeviceAddress read_address = read_buffer.Address();

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = read_offset;
    push_data.data.address = &read_address;
    push_data.data.size = sizeof(read_address);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-11438");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
