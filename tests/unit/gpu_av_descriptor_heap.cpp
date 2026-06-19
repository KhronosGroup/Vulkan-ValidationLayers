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
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/buffer_helper.h"
#include "../utils/math_utils.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "shader_helper.h"
#include "shader_templates.h"
#include "test_framework.h"

void GpuAVDescriptorHeap::CreateResourceHeap(VkDeviceSize app_size, bool reserved_range_in_front) {
    resource_reserved_range_in_front_ = reserved_range_in_front;
    const VkDeviceSize heap_size = AlignResource(app_size) + heap_props.minResourceHeapReservedRange;
    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    resource_heap_.Init(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    resource_heap_data_ = static_cast<uint8_t*>(resource_heap_.Memory().Map());
    if (resource_reserved_range_in_front_) {
        resource_heap_data_ += heap_props.minResourceHeapReservedRange;
    }
}

void GpuAVDescriptorHeap::CreateSamplerHeap(VkDeviceSize app_size, bool reserved_range_in_front, bool use_embedded_samplers) {
    embedded_samplers = use_embedded_samplers;
    sampler_reserved_range_in_front_ = reserved_range_in_front;
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
    if (sampler_reserved_range_in_front_) {
        sampler_heap_data_ += reserved_range;
    }
}

void GpuAVDescriptorHeap::BindResourceHeap() {
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = resource_heap_.AddressRange();
    if (resource_reserved_range_in_front_) {
        bind_resource_info.reservedRangeOffset = 0;
    } else {
        bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    }

    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
}

void GpuAVDescriptorHeap::BindSamplerHeap() {
    const VkDeviceSize min_reserved_range =
        embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange;
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = sampler_heap_.AddressRange();
    if (resource_reserved_range_in_front_) {
        bind_resource_info.reservedRangeOffset = 0;
    } else {
        bind_resource_info.reservedRangeOffset = sampler_heap_.CreateInfo().size - min_reserved_range;
    }
    bind_resource_info.reservedRangeSize = min_reserved_range;
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);
}

void GpuAVDescriptorHeap::WriteBufferToHeap(const vkt::Buffer& buffer, uint32_t stride, VkDescriptorType type) {
    VkHostAddressRangeEXT descriptor_host{resource_heap_data_ + (heap_props.bufferDescriptorSize * stride),
                                          static_cast<size_t>(heap_props.bufferDescriptorSize)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    assert(type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_info.type = type;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);
}

// Currently used for simple Sampled Images
void GpuAVDescriptorHeap::WriteImageToHeap(const vkt::Image& image, uint32_t stride, VkDescriptorType type) {
    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);
    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkHostAddressRangeEXT descriptor_host{resource_heap_data_ + (heap_props.imageDescriptorSize * stride),
                                          static_cast<size_t>(heap_props.imageDescriptorSize)};
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = type;
    descriptor_info.data.pImage = &image_info;
    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);
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

TEST_F(NegativeGpuAVDescriptorHeap, HeapBoundBeforePipeline) {
    TEST_DESCRIPTION("Validate illegal index buffer values when descriptor heap is bound before the pipeline");
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

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = heap.AddressRange();
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
    address_ranges[0] = buffer1.AddressRange();
    address_ranges[1] = buffer2.AddressRange();
    address_ranges[2] = buffer3.AddressRange();

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
    mappings[0] = MakeSetAndBindingMapping(0u, 0u, 1u, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0u, 1u, 1u, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);
    mappings[2] = MakeSetAndBindingMapping(1u, 2u, 1u, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
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
    pipe.shader_stages_ = {vs.GetStageCreateInfo(&mapping_info), pipe.fs_->GetStageCreateInfo(&mapping_info)};
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

    vkt::HeapComputePipeline pipe(*m_device, kMinimalShaderGlsl, SPV_ENV_VULKAN_1_0);

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
    VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoHeap(vspv, VK_SHADER_STAGE_VERTEX_BIT);
    const vkt::Shader vert_shader(*m_device, vert_ci);

    const auto fspv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT frag_ci = ShaderCreateInfoHeap(fspv, VK_SHADER_STAGE_FRAGMENT_BIT);
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

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBConstantOffset) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(1, 3);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    // 1 resource stride OOB
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 1, binding = 3) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBConstantOffsetDynamicIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    auto ssbo_memory = (uint32_t*)ssbo_buffer.Memory().Map();
    ssbo_memory[0] = 3;
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 1);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        // only [0] is valid, [1] is uninitialized, [2] will go OOB
        layout(set = 0, binding = 1) buffer A { uint index; uint result; } ssbo[4];
        void main() {
            uint index = ssbo[0].index;
            ssbo[index].result = 99;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBMultipleBinding) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    // Goal is to mix-match the set/binding and mapping index
    uint32_t oob_offset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    VkDescriptorSetAndBindingMappingEXT mappings[4];
    mappings[0] = MakeSetAndBindingMapping(1, 2);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = oob_offset;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    // This mapping is valid
    mappings[1] = MakeSetAndBindingMapping(0, 2);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = oob_offset + (uint32_t)resource_stride;
    mappings[2].sourceData.constantOffset.heapArrayStride = 0;
    mappings[3] = MakeSetAndBindingMapping(0, 1);
    mappings[3].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = oob_offset;
    mappings[3].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 4;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 1) buffer A { uint a; };
        layout(set = 0, binding = 2) buffer B { uint b; };
        layout(set = 1, binding = 0) buffer C { uint c; };
        layout(set = 1, binding = 2) buffer D { uint d; };
        void main() {
            a = 1;
            b = 2; // valid
            c = 3;
            d = 4;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBPushIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.pushIndex.heapArrayStride = 0;
    mapping.sourceData.pushIndex.pushOffset = 8;
    mapping.sourceData.pushIndex.heapIndexStride = 2;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();

    uint32_t push_index = 512;
    m_command_buffer.PushData(8, sizeof(uint32_t), &push_index);

    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBIndirectIndex) {
    TEST_DESCRIPTION("Also tests having different bindings and calling things twice");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4, true);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mappings[0].sourceData.indirectIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[0].sourceData.indirectIndex.pushOffset = 16;
    mappings[0].sourceData.indirectIndex.addressOffset = 4;
    mappings[0].sourceData.indirectIndex.heapIndexStride = 1;
    mappings[0].sourceData.indirectIndex.heapArrayStride = (uint32_t)resource_stride;
    mappings[1] = MakeSetAndBindingMapping(0, 2);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
    mappings[1].sourceData.pushDataOffset = 0;
    mappings[2] = MakeSetAndBindingMapping(0, 1);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mappings[2].sourceData.indirectIndexArray.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[2].sourceData.indirectIndexArray.pushOffset = 16;
    mappings[2].sourceData.indirectIndexArray.addressOffset = 4;
    mappings[2].sourceData.indirectIndexArray.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        // x[3] and y[3] is where it goes OOB
        layout(set = 0, binding = 0) buffer A { uint a; } x[4];
        layout(set = 0, binding = 1) buffer B { uint b; } y[4];
        layout(set = 0, binding = 2) uniform C { uint a_index; uint b_index; };
        void main() {
            x[a_index].a = 1;
            y[b_index].b = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    uint32_t* indirect_data = (uint32_t*)indirect_buffer.Memory().Map();
    for (uint32_t i = 0; i < 8; i++) {
        indirect_data[i] = (uint32_t)resource_stride * i;
    }

    VkDeviceAddress indirect_address = indirect_buffer.Address();
    m_command_buffer.PushData(16, sizeof(VkDeviceAddress), &indirect_address);

    BindResourceHeap();

    // first pass good
    uint32_t a_index = 0;
    uint32_t b_index = 0;
    m_command_buffer.PushData(0, sizeof(uint32_t), &a_index);
    m_command_buffer.PushData(4, sizeof(uint32_t), &b_index);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // second pass
    a_index = 3;
    m_command_buffer.PushData(0, sizeof(uint32_t), &a_index);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // third pass
    a_index = 0;
    m_command_buffer.PushData(0, sizeof(uint32_t), &a_index);
    b_index = 3;
    m_command_buffer.PushData(4, sizeof(uint32_t), &b_index);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    // one for each indirect mapping
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBShaderObjects) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitDynamicRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    const char* fs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { vec4 a; } x[4];
        layout(location = 0) out vec4 uFragColor;
        void main(){
            uFragColor = x[1].a;
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl);
    VkShaderCreateInfoEXT vert_shader_ci = ShaderCreateInfoHeap(vert_spv, VK_SHADER_STAGE_VERTEX_BIT);
    const vkt::Shader vert_shader(*m_device, vert_shader_ci);

    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);
    VkShaderCreateInfoEXT frag_shader_ci = ShaderCreateInfoHeap(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, &mapping_info);
    const vkt::Shader frag_shader(*m_device, frag_shader_ci);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    BindResourceHeap();

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    VkShaderStageFlagBits stages[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkShaderEXT shaders[2] = {vert_shader.handle(), frag_shader.handle()};
    vk::CmdBindShadersEXT(m_command_buffer, 2, stages, shaders);

    SetDefaultDynamicStatesExclude();
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-11309", gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBRebindHeap) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer, 1);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();  // normal, will bind full heap
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // Bind 1 less resource, which causes the OOB
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = resource_heap_.AddressRange();
    bind_resource_info.heapRange.size -= resource_stride;
    bind_resource_info.reservedRangeOffset = 0;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    BindResourceHeap();  // back to full heap
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, SamplerOOB) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    CreateResourceHeap(resource_stride, true);
    CreateSamplerHeap(sampler_stride, true);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    WriteImageToHeap(image);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(sampler_stride)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minSamplerHeapReservedRange;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D kTextures2D;
        layout(set = 0, binding = 1) uniform sampler kSampler;

        void main() {
            vec4 out_color = texture(sampler2D(kTextures2D, kSampler), vec2(0));
        }
    )glsl";
    // bad image mappings
    vkt::HeapComputePipeline pipe_image(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minSamplerHeapReservedRange + sampler_stride);
    // bad sampler mappings
    vkt::HeapComputePipeline pipe_sampler(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_image);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_sampler);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, SamplerOOBCombinedImageSampler) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    CreateResourceHeap(resource_stride, true);
    CreateSamplerHeap(sampler_stride, true);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    WriteImageToHeap(image);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(sampler_stride)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + resource_stride);
    mapping.sourceData.constantOffset.heapArrayStride = 0;
    mapping.sourceData.constantOffset.samplerHeapOffset = (uint32_t)heap_props.minSamplerHeapReservedRange;
    mapping.sourceData.constantOffset.samplerHeapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler2D tex;

        void main() {
            vec4 out_color = texture(tex, vec2(0.5f));
        }
    )glsl";
    // bad image mappings
    vkt::HeapComputePipeline pipe_image(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    mapping.sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.constantOffset.samplerHeapOffset = (uint32_t)(heap_props.minSamplerHeapReservedRange + sampler_stride);
    // bad sampler mappings
    vkt::HeapComputePipeline pipe_sampler(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_image);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_sampler);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBStorageImage) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    WriteImageToHeap(image, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 2);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32ui) uniform uimage2D good_si;
        layout(set = 0, binding = 1, r32ui) uniform uimage2D bad_si;
        void main() {
            uvec4 texel = imageLoad(bad_si, ivec2(0, 0));
            imageStore(good_si, ivec2(1, 1), texel * 2);
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    // VUID-vkCmdDispatch-None-11309
    m_errorMonitor->SetDesiredError("bad_si");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersBuffer) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minResourceHeapReservedRange / heap_props.bufferDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout (descriptor_heap) buffer SSBO_0 {
            uint data;
        } heapBuffer[];

        void main() {
            // Something large enough to get pass the reserved range
            heapBuffer[10000].data = 0;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersBufferOldGlsl) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minResourceHeapReservedRange / heap_props.bufferDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    // Same shader above in ResourceOOBUntypedPointers, but using the old (still valid) GLSL output
    const char* cs_source = R"asm(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberName %SSBO_0 0 "data"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO_0 Block
               OpMemberDecorate %SSBO_0 0 Offset 0
               OpDecorateId %_runtimearr_16 ArrayStrideIdEXT %17
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
  %int_10000 = OpConstant %int 10000
       %uint = OpTypeInt 32 0
     %SSBO_0 = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %16 = OpTypeBufferEXT StorageBuffer
         %17 = OpConstantSizeOfEXT %int %16
%_runtimearr_16 = OpTypeRuntimeArray %16
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_16 %resource_heap %int_10000
         %19 = OpBufferPointerEXT %_ptr_StorageBuffer %15
         %20 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO_0 %19 %int_0
               OpStore %20 %uint_0
               OpReturn
               OpFunctionEnd
    )asm";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersBufferAtomics) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minResourceHeapReservedRange / heap_props.bufferDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : enable
        #extension GL_KHR_memory_scope_semantics : enable

        layout (descriptor_heap) buffer heap {
            uint a;
        } heapBuffer[];

        void main() {
            atomicStore(heapBuffer[10000].a, 0u, gl_ScopeDevice, 0, 0);
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersStorageImage) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minResourceHeapReservedRange / heap_props.imageDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer ssbo_buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    // Note - this looks the same in the "old" glslang code as well
    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap, rgba8i) uniform iimage2D heapImages[];
        layout(set = 0, binding = 0) buffer SSBO { ivec4 result; };
        void main() {
            ivec4 data = imageLoad(heapImages[10000], ivec2(0));
            result = data;
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersSampledImage) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minResourceHeapReservedRange / heap_props.imageDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    CreateResourceHeap(resource_stride);
    CreateSamplerHeap(sampler_stride);

    vkt::Buffer ssbo_buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    // Note - this looks the same in the "old" glslang code as well
    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) uniform texture2D heapTextures[];
        layout(descriptor_heap) uniform sampler heapSamplers[];
        layout(set = 0, binding = 0) buffer SSBO { vec4 result; };
        void main() {
            vec4 data = texture(sampler2D(heapTextures[10000], heapSamplers[0]), vec2(0.5f));
            result = data;
        }
    )glsl";
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, SamplerOOBUntypedPointers) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if ((heap_props.minSamplerHeapReservedRange / heap_props.samplerDescriptorSize) >= 10000) {
        GTEST_SKIP() << "reserved range is too large, access will not be OOB";
    }
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    CreateResourceHeap(resource_stride);
    CreateSamplerHeap(heap_props.samplerDescriptorSize);

    // Note - this looks the same in the "old" glslang code as well
    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) uniform texture2D heapTextures[];
        layout(descriptor_heap) uniform sampler heapSamplers[];
        void main() {
            vec4 data = texture(sampler2D(heapTextures[0], heapSamplers[10000]), vec2(0.5f));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceOOBUntypedPointersOffsetId) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Buffer buffer_0(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(buffer_0, 0);

    // layout(storage_buffer) SSBO {
    //     uint a;
    // };
    // layout(offset = buffer_size * 10000) heap {
    //     SSBO runtime_buffer[];
    // } heap_layout;
    //
    // heap_layout.runtime_buffer[0].a = 42;
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %heap_layout Block
               OpMemberDecorateIdEXT %heap_layout 0 OffsetIdEXT %crazy_offset
               OpDecorateId %runtime_buffer ArrayStrideIdEXT %buf_size
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
    %uint_42 = OpConstant %uint 42
    %int_10k = OpConstant %int 10000
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%crazy_offset = OpSpecConstantOp %int IMul %buf_size %int_10k
%runtime_buffer = OpTypeRuntimeArray %type_buffer

 %heap_layout = OpTypeStruct %runtime_buffer

       %main = OpFunction %void None %void_fn
          %5 = OpLabel

%heap_index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_0 %int_0
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_0
               OpStore %member_0 %uint_42

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, BufferDescriptorAlignmentMapping) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    // [SSBO, SSBO, UBO, UBO]
    CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);
    WriteBufferToHeap(ssbo_buffer, 1);
    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ubo_buffer, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    WriteBufferToHeap(ubo_buffer, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    const uint32_t push_data = (uint32_t)resource_stride;
    const uint32_t bad_push_data = (uint32_t)resource_stride + 1u;
    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mappings[0].sourceData.pushIndex.heapOffset = 0;
    mappings[0].sourceData.pushIndex.heapIndexStride = 1;
    mappings[0].sourceData.pushIndex.pushOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mappings[1].sourceData.pushIndex.heapOffset = (uint32_t)resource_stride;
    mappings[1].sourceData.pushIndex.heapIndexStride = 1;
    mappings[1].sourceData.pushIndex.pushOffset = 4;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO { uint a; };
        layout(set = 0, binding = 1) uniform UBO { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    // bad SSBO
    m_command_buffer.PushData(0, sizeof(uint32_t), &bad_push_data);
    m_command_buffer.PushData(4, sizeof(uint32_t), &push_data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // bad UBO
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_data);
    m_command_buffer.PushData(4, sizeof(uint32_t), &bad_push_data);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11297", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ImageSamplerDescriptorAlignment) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    CreateResourceHeap(resource_stride * 2);
    CreateSamplerHeap(sampler_stride * 2);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    WriteImageToHeap(image);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(sampler_stride)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mappings[0].sourceData.indirectIndex.pushOffset = 0;
    mappings[0].sourceData.indirectIndex.heapIndexStride = 1;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mappings[1].sourceData.indirectIndex.pushOffset = 8;
    mappings[1].sourceData.indirectIndex.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D kTextures2D;
        layout(set = 0, binding = 1) uniform sampler kSampler;

        void main() {
            vec4 out_color = texture(sampler2D(kTextures2D, kSampler), vec2(0));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* indirect_data = (uint32_t*)indirect_buffer.Memory().Map();
    indirect_data[0] = 0;  // valid
    indirect_data[1] = 1;  // invalid
    VkDeviceAddress indirect_address = indirect_buffer.Address();
    VkDeviceAddress bad_indirect_address = indirect_address + 4;

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &bad_indirect_address);
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &bad_indirect_address);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11298");  // image
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11299");  // sampler
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, BufferDescriptorAlignmentUntypedPointers) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Buffer buffer_0(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(buffer_0, 0);
    WriteBufferToHeap(buffer_0, 1);

    // layout(storage_buffer) SSBO {
    //     uint a;
    // };
    // layout(offset = 1) heap {
    //     SSBO runtime_buffer[];
    // } heap_layout;
    //
    // heap_layout.runtime_buffer[0].a = 42;
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %heap_layout Block
               OpMemberDecorateIdEXT %heap_layout 0 OffsetIdEXT %uint_1
               OpDecorateId %runtime_buffer ArrayStrideIdEXT %buf_size
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
    %uint_42 = OpConstant %uint 42
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%runtime_buffer = OpTypeRuntimeArray %type_buffer

 %heap_layout = OpTypeStruct %runtime_buffer

       %main = OpFunction %void None %void_fn
          %5 = OpLabel

%heap_index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_0 %int_0
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_0
               OpStore %member_0 %uint_42

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-bufferDescriptorAlignment-11384");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, StorageImageDescriptorAlignmentUntypedPointers) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    WriteImageToHeap(image, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    WriteImageToHeap(image, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    // [stride: spec_constant_id = 1]
    // layout(descriptor_heap, R32ui) uniform uimage2D si[];
    //
    // imageStore(si[1], ivec2(1), uvec4(1)); // bad
    // imageStore(si[0], ivec2(1), uvec4(1)); // good
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %runtime_image ArrayStrideIdEXT %bad_stride
               OpDecorate %bad_stride SpecId 1
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
         %12 = OpTypeImage %uint 2D 0 0 0 2 R32ui
 %bad_stride = OpSpecConstant %int 0
%runtime_image = OpTypeRuntimeArray %12
      %v2int = OpTypeVector %int 2
         %18 = OpConstantComposite %v2int %int_1 %int_1
     %v4uint = OpTypeVector %uint 4
     %uint_1 = OpConstant %uint 1
         %21 = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %4
          %6 = OpLabel
    %one_ac = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_image %resource_heap %int_1
  %si1_load = OpLoad %12 %one_ac
               OpImageWrite %si1_load %18 %21 ZeroExtend
    %zero_ac = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_image %resource_heap
   %si0_load = OpLoad %12 %zero_ac
               OpImageWrite %si0_load %18 %21 ZeroExtend
               OpReturn
               OpFunctionEnd
    )";
    const uint32_t data = (uint32_t)heap_props.imageDescriptorSize - 1;
    const VkSpecializationMapEntry entry = {1, 0, sizeof(uint32_t)};
    const VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM, &specialization_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11349");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, StorageImageAtomicDescriptorAlignmentUntypedPointers) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8_SINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    WriteImageToHeap(image, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    WriteImageToHeap(image, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    // [stride: spec_constant_id = 1]
    // layout(descriptor_heap, r32i) uniform iimage1D si1[];
    //
    // imageAtomicAdd(si1[1], 1, 1); // bad
    // imageAtomicAdd(si1[0], 1, 1); // good
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability Image1D
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorateId %runtime_image ArrayStrideIdEXT %bad_stride
               OpDecorate %bad_stride SpecId 1
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %11 = OpTypeImage %int 1D 0 0 0 2 R32i
%_runtimearr_11 = OpTypeRuntimeArray %11
%_ptr_Image_int = OpTypePointer Image %int
 %_ptr_Image = OpTypeUntypedPointerKHR Image
 %bad_stride = OpSpecConstant %uint 0
%runtime_image = OpTypeRuntimeArray %11
       %main = OpFunction %void None %4
          %6 = OpLabel
         %12 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_image %resource_heap %uint_1
         %19 = OpUntypedImageTexelPointerEXT %_ptr_Image %11 %12 %int_1 %uint_0
         %21 = OpAtomicIAdd %int %19 %uint_1 %uint_0 %int_1
         %23 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_image %resource_heap %uint_0
         %26 = OpUntypedImageTexelPointerEXT %_ptr_Image %11 %23 %int_1 %uint_0
         %27 = OpAtomicIAdd %int %26 %uint_1 %uint_0 %int_1
               OpReturn
               OpFunctionEnd
    )";
    const uint32_t data = (uint32_t)heap_props.imageDescriptorSize - 1;
    const VkSpecializationMapEntry entry = {1, 0, sizeof(uint32_t)};
    const VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM, &specialization_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11349");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceReservedRange) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if (heap_props.minResourceHeapReservedRange == 0) {
        GTEST_SKIP() << "minResourceHeapReservedRange is zero";
    }
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;  // in reserved range

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, SamplerReservedRange) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    if (heap_props.minSamplerHeapReservedRange == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRange is zero";
    }
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    CreateResourceHeap(resource_stride, true);
    CreateSamplerHeap(sampler_stride, true);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    WriteImageToHeap(image);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(sampler_stride)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D kTextures2D;
        layout(set = 0, binding = 1) uniform sampler kSampler;

        void main() {
            vec4 out_color = texture(sampler2D(kTextures2D, kSampler), vec2(0));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, IndirectIndexPushDataAlignment) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.indirectIndex.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress indirect_address = indirect_buffer.Address() + 1;

    m_command_buffer.Begin();
    BindResourceHeap();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11300");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, IndirectAddressPushDataAlignment) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.indirectAddress.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress indirect_address = indirect_buffer.Address() + 1;

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11304");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, MappingAddressBufferAlignment) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);
    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ubo_buffer, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[1].sourceData.pushAddressOffset = 8;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress ssbo_address = ssbo_buffer.Address() + 1;
    VkDeviceAddress ubo_address = ubo_buffer.Address() + 0;

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &ssbo_address);  // bad
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &ubo_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    ssbo_address = ssbo_buffer.Address() + 0;
    ubo_address = ubo_buffer.Address() + 1;
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &ssbo_address);
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &ubo_address);  // bad
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11441");  // ubo
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11442");  // ssbo
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, MappingAddressBufferAlignmentHeap) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    CreateResourceHeap(heap_props.bufferDescriptorSize * 2);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 8;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mappings[1].sourceData.heapData.heapOffset = 0;
    mappings[1].sourceData.heapData.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t bad_offset = 1;
    VkDeviceAddress ssbo_address = ssbo_buffer.Address();
    m_command_buffer.PushData(0, sizeof(uint32_t), &bad_offset);
    m_command_buffer.PushData(8, sizeof(VkDeviceAddress), &ssbo_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11441");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, MappingIndirectAddressBufferAlignment) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);
    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ubo_buffer, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mappings[0].sourceData.indirectAddress.pushOffset = 0;
    mappings[0].sourceData.indirectAddress.addressOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mappings[1].sourceData.indirectAddress.pushOffset = 0;
    mappings[1].sourceData.indirectAddress.addressOffset = 8;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress* indirect_data = (VkDeviceAddress*)indirect_buffer.Memory().Map();
    indirect_data[0] = ssbo_buffer.Address() + 1;
    indirect_data[1] = ubo_buffer.Address();
    indirect_data[2] = ssbo_buffer.Address();
    indirect_data[3] = ubo_buffer.Address() + 1;
    VkDeviceAddress indirect_address = indirect_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    indirect_address += (sizeof(VkDeviceAddress) * 2);
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11441");  // ubo
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11442");  // ssbo
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, IndirectIndexNullIndirect) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    CreateResourceHeap(heap_props.bufferDescriptorSize);
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress null_address = 0;

    m_command_buffer.Begin();
    BindResourceHeap();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &null_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11301");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, PushAddressNullIndirect) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress null_address = 0;

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &null_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11302");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, IndirectAddressNullIndirect) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.indirectAddress.pushOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress null_address = 0;

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress* indirect_data = (VkDeviceAddress*)indirect_buffer.Memory().Map();
    indirect_data[0] = null_address;
    VkDeviceAddress indirect_address = indirect_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &null_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.PushData(0, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11305");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11306");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, MappingBindingCountAlias) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4, true);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 5);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.constantOffset.heapArrayStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; } x[2];
        layout(set = 0, binding = 2) buffer B { uint b; } y[3];
        void main() {
            x[0].a = 2;
            y[2].b = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    char const* cs_source2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer X { uint d; } x;
        layout(set = 0, binding = 1) buffer Y { uint d; } y;
        layout(set = 0, binding = 2) buffer Z { uint d; } z;
        layout(set = 0, binding = 4) buffer W { uint d; } w;
        void main() {
            x.d = y.d + z.d + w.d;
        }
    )glsl";
    vkt::HeapComputePipeline pipe2(*m_device, cs_source2, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Same things, just a different mapping
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mapping.sourceData.indirectIndexArray.heapIndexStride = (uint32_t)resource_stride;
    mapping.sourceData.indirectIndexArray.pushOffset = 0;
    mapping.sourceData.indirectIndexArray.addressOffset = 4;

    vkt::HeapComputePipeline pipe3(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
    vkt::HeapComputePipeline pipe4(*m_device, cs_source2, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer ubo_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* ubo_data = (uint32_t*)ubo_buffer.Memory().Map();
    ubo_data[1] = 0;
    ubo_data[2] = 1;
    ubo_data[3] = 2;
    ubo_data[4] = 3;
    ubo_data[5] = 4;  // OOB
    VkDeviceAddress indirect_ubo_address = ubo_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, 8, &indirect_ubo_address);
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe3);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe4);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309", 2);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, DifferentMappingResourceMask) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2, true);
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)(heap_props.minResourceHeapReservedRange + (resource_stride * 10));

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) readonly buffer A { uint a; } a;
        layout(set = 0, binding = 0) writeonly buffer B { uint b; } b;
        void main() {
            b.b = a.a;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, PushDataPushIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mappings[0].sourceData.pushIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[0].sourceData.pushIndex.heapArrayStride = 0;
    mappings[0].sourceData.pushIndex.pushOffset = 80;
    mappings[0].sourceData.pushIndex.heapIndexStride = 2;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mappings[1].sourceData.pushIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1].sourceData.pushIndex.heapArrayStride = 0;
    mappings[1].sourceData.pushIndex.pushOffset = 128;
    mappings[1].sourceData.pushIndex.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) buffer B { uint b; };
        void main() {
            a = 2;
            b = 4;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    uint32_t bad_push_index = 512;
    m_command_buffer.PushData(80, sizeof(uint32_t), &bad_push_index);
    uint32_t good_push_index = 512;
    m_command_buffer.PushData(128, sizeof(uint32_t), &good_push_index);

    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // now again with the user making using of the push constant
    char const* cs_source2 = R"glsl(
        #version 450
        layout(push_constant) uniform PC {
            uint data[20];
        };
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) buffer B { uint b; };
        void main() {
            a = data[3];
            b = data[19];
        }
    )glsl";
    vkt::HeapComputePipeline pipe2(*m_device, cs_source2, SPV_ENV_VULKAN_1_0, &mapping_info);
    m_command_buffer.Begin();
    uint32_t garbage[20];
    m_command_buffer.PushData(0, 80, garbage);
    m_command_buffer.PushData(80, sizeof(uint32_t), &bad_push_index);
    m_command_buffer.PushData(128, sizeof(uint32_t), &good_push_index);
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, PushDataIndirectIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mappings[0].sourceData.indirectIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[0].sourceData.indirectIndex.pushOffset = 80;
    mappings[0].sourceData.indirectIndex.heapIndexStride = 2;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mappings[1].sourceData.indirectIndex.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1].sourceData.indirectIndex.pushOffset = 128;
    mappings[1].sourceData.indirectIndex.heapIndexStride = 1;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(push_constant) uniform PC {
            uint data[20];
        };
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) buffer B { uint b; };
        void main() {
            a = data[3];
            b = data[19];
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* indirect_data = (uint32_t*)indirect_buffer.Memory().Map();
    indirect_data[0] = 5000;
    indirect_data[16] = 0;
    VkDeviceAddress indirect_address_bad = indirect_buffer.Address();
    VkDeviceAddress indirect_address_good = indirect_buffer.Address() + (16 * sizeof(uint32_t));

    m_command_buffer.Begin();
    uint32_t garbage[20];
    m_command_buffer.PushData(0, 80, garbage);
    m_command_buffer.PushData(80, sizeof(VkDeviceAddress), &indirect_address_good);
    m_command_buffer.PushData(128, sizeof(VkDeviceAddress), &indirect_address_bad);

    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorHeap, ResourceHeapDataOOB) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/issues/4861");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    CreateResourceHeap(heap_props.bufferDescriptorSize + 256, true);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.minResourceHeapReservedRange;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mappings[1].sourceData.heapData.heapOffset = 0;
    mappings[1].sourceData.heapData.pushOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    uint32_t push_offset = (uint32_t)resource_heap_.CreateInfo().size;
    m_command_buffer.Begin();
    BindResourceHeap();
    m_command_buffer.PushData(0, sizeof(uint32_t), &push_offset);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11309");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}