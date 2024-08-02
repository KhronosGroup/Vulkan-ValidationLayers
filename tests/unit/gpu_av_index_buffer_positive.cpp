/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/buffer_helper.h"

class PositiveGpuAVIndexBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVIndexBuffer, BadVertexIndex) {
    TEST_DESCRIPTION("If no vertex buffer is used, all index values are legal");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddDisabledFeature(vkt::Feature::drawIndirectFirstInstance);
    RETURN_IF_SKIP(InitState(nullptr));
    InitRenderTarget();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer indexed_draw_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    {
        auto indexed_draw_ptr = reinterpret_cast<VkDrawIndexedIndirectCommand *>(indexed_draw_buffer.memory().map());
        indexed_draw_ptr->indexCount = 3;
        indexed_draw_ptr->instanceCount = 1;
        indexed_draw_ptr->firstIndex = 0;
        indexed_draw_ptr->vertexOffset = 0;
        indexed_draw_ptr->firstInstance = 0;
        indexed_draw_buffer.memory().unmap();
    }

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, std::numeric_limits<uint32_t>::max(), 42});

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
}
