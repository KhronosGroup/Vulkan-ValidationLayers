/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
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

    RETURN_IF_SKIP(InitState(nullptr));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

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
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, std::numeric_limits<uint32_t>::max(), 42});

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, VertexIndex) {
    TEST_DESCRIPTION("Validate index buffer values");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    constexpr uint32_t num_vertices = 12;
    std::vector<uint32_t> indicies(num_vertices);
    for (uint32_t i = 0; i < num_vertices; i++) {
        indicies[i] = num_vertices - 1 - i;
    }
    vkt::Buffer index_buffer = vkt::IndexBuffer(*m_device, std::move(indicies));

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
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVIndexBuffer, DrawIndexedDynamicStates) {
    TEST_DESCRIPTION("vkCmdDrawIndexed - Set dynamic states");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3PolygonMode);
    // AddRequiredFeature(vkt::Feature::depthBiasClamp);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450

        layout(location=0) in vec3 pos;

        void main() {
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_CULL_MODE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_FRONT_FACE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 1, 2});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetRasterizerDiscardEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetCullModeEXT(m_command_buffer.handle(), VK_CULL_MODE_NONE);
    vk::CmdSetFrontFaceEXT(m_command_buffer.handle(), VK_FRONT_FACE_CLOCKWISE);
    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdSetDepthBias(m_command_buffer.handle(), 0.0f, 0.0f, 1.0f);
    vk::CmdSetLineWidth(m_command_buffer.handle(), 1.0f);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
