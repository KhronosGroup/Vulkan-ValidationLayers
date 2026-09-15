/*
 * Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
 * Copyright (c) 2020-2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "buffer_helper.h"
#include "descriptor_heap_object.h"

class NegativeGpuAVIndexBuffer : public GpuAVTest {};

TEST_F(NegativeGpuAVIndexBuffer, IndexBufferOOB) {
    TEST_DESCRIPTION("Validate overruning the index buffer");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

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

TEST_F(NegativeGpuAVIndexBuffer, IndexBufferOOBDescriptorHeap) {
    TEST_DESCRIPTION("Validate overruning the index buffer");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    vkt::DescriptorHeap::AddDescriptorHeapRequirements(*this);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize heap_size = desc_heap.heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(heap_size);

    vkt::Buffer buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    *((float*)buffer.Memory().Map()) = 0.1f;
    const VkDeviceSize buffer_heap_offset = desc_heap.WriteBufferDescriptor(buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    const char* vsSource = R"glsl(
        #version 450

        layout(set = 0, binding = 0) buffer ssbo { float x; };

        void main() {
            gl_Position = vec4(vec3(x), gl_VertexIndex);
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex.heapOffset = (uint32_t)buffer_heap_offset;
    mapping.sourceData.heapData.pushOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;
    VkShaderObj vs(*m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL, nullptr, "main", nullptr,
                   &mapping_info);
    VkShaderObj fs = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo stages[2] = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    VkPipelineCreateFlags2CreateInfoKHR pipe_flags = vku::InitStructHelper();
    pipe_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 2;
    pipe.gp_ci_.pStages = stages;
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
    desc_heap.BindResourceHeap(m_command_buffer);
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

TEST_F(NegativeGpuAVIndexBuffer, IndexBufferOOB2) {
    TEST_DESCRIPTION("Validate overruning the index buffer - large indirect draw");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    std::vector<VkDrawIndexedIndirectCommand> draw_params(64 * 64 + 1);
    for (size_t i = 0; i < draw_params.size(); ++i) {
        draw_params[i].indexCount = 3;
        draw_params[i].instanceCount = 1;
        draw_params[i].vertexOffset = 0;
        draw_params[i].firstInstance = 0;

        // What really needs to be tested is ability to catch an error in the last
        // indirect draw, if validation dispatch size is computed incorrectly
        // it will be missed
        if (i == (draw_params.size() - 1)) {
            draw_params[i].firstIndex = 1;
        } else {
            draw_params[i].firstIndex = 0;
        }
    }
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndexedIndirectCommand>(*m_device, draw_params);

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {1, 2, 3});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    m_errorMonitor->SetDesiredErrorRegex(
        "VUID-VkDrawIndexedIndirectCommand-robustBufferAccess2-08798",
        "Index 4 is not within the bound index buffer. Computed from VkDrawIndexedIndirectCommand\\[4096\\]");
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer, draw_params_buffer, 0, size32(draw_params), sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, BindIndexBuffer3StorageUsage) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    {
        vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                 vkt::device_address);

        VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
        index_buffer_info.addressRange = index_buffer.AddressRange();
        index_buffer_info.addressFlags = 0u;
        index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-addressRange-13122");
        vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    {
        vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

        VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
        index_buffer_info.addressRange = index_buffer.AddressRange();
        index_buffer_info.addressFlags = VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR;
        index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-addressRange-13123");
        vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}