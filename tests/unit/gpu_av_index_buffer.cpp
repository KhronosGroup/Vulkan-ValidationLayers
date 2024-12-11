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
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredErrorRegex(
        "VUID-VkDrawIndexedIndirectCommand-robustBufferAccess2-08798",
        "Index 4 is not within the bound index buffer. Computed from VkDrawIndexedIndirectCommand\\[0\\]");
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, IndirectDrawBadVertexIndex32) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint32_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 1 \\] \\(666\\) \\+ vertexOffset \\(0\\) = Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, IndirectDrawBadVertexIndex16) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint16_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    // Two OOB indices
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 42, 128});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 2 \\] \\(128\\) \\+ vertexOffset \\(0\\) = Vertex index 128");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 1 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, IndirectDrawBadVertexIndex8) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint8_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::indexTypeUint8);

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

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint8_t>(*m_device, {0, 128, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT8_KHR);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 1 \\] \\(128\\) \\+ vertexOffset \\(0\\) = Vertex index 128");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexedIndirect-None-02721",
                                         "index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DrawBadVertexIndex32) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint32_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 1 \\] \\(666\\) \\+ vertexOffset \\(0\\) = Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    // vertexOffset = 3
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 0 \\] \\(0\\) \\+ vertexOffset \\(3\\) = Vertex index 3");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 1 \\] \\(666\\) \\+ vertexOffset \\(3\\) = Vertex index 669");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(3\\) = Vertex index 45");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 3, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DISABLED_DrawInSecondaryCmdBufferBadVertexIndex32) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint32_t index. Draw recorded in secondary command buffer.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    pipe.CreateGraphicsPipeline();

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

    std::vector<vkt::CommandBuffer> secondary_cmd_buffers;
    std::vector<VkCommandBuffer> secondary_cmd_buffers_handles;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();
    inheritance_info.renderPass = m_renderPass;
    inheritance_info.subpass = 0;
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo secondary_begin_info = vku::InitStructHelper();
    secondary_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_begin_info.pInheritanceInfo = &inheritance_info;

    constexpr uint32_t secondary_cmd_buffer_executes_count = 3;
    for (uint32_t i = 0; i < secondary_cmd_buffer_executes_count; ++i) {
        vkt::CommandBuffer &secondary_cmd_buffer =
            secondary_cmd_buffers.emplace_back(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        secondary_cmd_buffers_handles.push_back(secondary_cmd_buffer.handle());

        secondary_cmd_buffer.begin(&secondary_begin_info);
        vk::CmdBindPipeline(secondary_cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        VkDeviceSize vertex_buffer_offset = 0;
        vk::CmdBindIndexBuffer(secondary_cmd_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
        vk::CmdBindVertexBuffers(secondary_cmd_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

        m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                             "index_buffer\\[1\\] \\(666\\) \\+ vertexOffset \\(0\\) = Vertex index 666");
        m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                             "index_buffer\\[2\\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
        vk::CmdDrawIndexed(secondary_cmd_buffer.handle(), 3, 1, 0, 0, 0);

        // vertexOffset = 3
        m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                             "index_buffer\\[0\\] \\(0\\) \\+ vertexOffset \\(3\\) = Vertex index 3");
        m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                             "index_buffer\\[1\\] \\(666\\) \\+ vertexOffset \\(3\\) = Vertex index 669");
        m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                             "index_buffer\\[2\\] \\(42\\) \\+ vertexOffset \\(3\\) = Vertex index 45");
        vk::CmdDrawIndexed(secondary_cmd_buffer.handle(), 3, 1, 0, 3, 0);

        secondary_cmd_buffer.end();
    }

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vk::CmdExecuteCommands(m_command_buffer.handle(), size32(secondary_cmd_buffers_handles), secondary_cmd_buffers_handles.data());

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DrawBadVertexIndex16) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint16_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Two OOB indices
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 3, 666});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 2 \\] \\(666\\) \\+ vertexOffset \\(0\\) = Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 1 \\] \\(3\\) \\+ vertexOffset \\(0\\) = Vertex index 3");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DrawBadVertexIndex16_2) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint16_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450
        
        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;
        layout(location=2) in vec3 normal;
        
        void main() {
        gl_Position = vec4(pos + uv.xyx + normal, 1.0);
        }
        )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    // "Array of structs" style vertices
    VkVertexInputBindingDescription input_binding = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputAttributeDescription, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    // UV
    vertex_attributes[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float)};
    // Normal
    vertex_attributes[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, (3 + 2) * sizeof(float)};

    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_attributes.data();
    pipe.vi_ci_.vertexAttributeDescriptionCount = size32(vertex_attributes);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first Vertex can correctly be fetched
    VkDeviceSize vertex_buffer_offset = 2 * sizeof(Vertex);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    // Two OOB indices
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 1 \\] \\(1\\) \\+ vertexOffset \\(0\\) = Vertex index 1");

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DrawBadVertexIndex8) {
    TEST_DESCRIPTION("Validate illegal index buffer values - uint8_t index");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::indexTypeUint8);
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

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer index_buffer = vkt::IndexBuffer<uint8_t>(*m_device, {12, 66, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT8_KHR);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 0 \\] \\(12\\) \\+ vertexOffset \\(0\\) = Vertex index 12");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 1 \\] \\(66\\) \\+ vertexOffset \\(0\\) = Vertex index 66");
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkCmdDrawIndexed-None-02721",
                                         "index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, DrawBadVertexIndex16DebugLabel) {
    TEST_DESCRIPTION(
        "Validate illegal index buffer values - uint16_t index. Also make sure debug label regions are properly accounted for.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 2> uv;
        std::array<float, 3> normal;
    };

    char const *vsSource = R"glsl(
        #version 450
        
        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;
        layout(location=2) in vec3 normal;
        
        void main() {
        gl_Position = vec4(pos + uv.xyx + normal, 1.0);
        }
        )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    // "Array of structs" style vertices
    VkVertexInputBindingDescription input_binding = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputAttributeDescription, 3> vertex_attributes = {};
    // Position
    vertex_attributes[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    // UV
    vertex_attributes[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float)};
    // Normal
    vertex_attributes[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, (3 + 2) * sizeof(float)};

    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_attributes.data();
    pipe.vi_ci_.vertexAttributeDescriptionCount = size32(vertex_attributes);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "my_pipeline";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    label.pLabelName = "my_draw";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    std::vector<Vertex> vertices;
    for (int i = 0; i < 3; ++i) {
        const Vertex vertex = {{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f, 7.0f}};
        vertices.emplace_back(vertex);
    }
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<Vertex>(*m_device, vertices);
    // Offset vertex buffer so that only first Vertex can correctly be fetched
    VkDeviceSize vertex_buffer_offset = 2 * sizeof(Vertex);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    // Two OOB indices
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint16_t>(*m_device, {0, 1, 0});
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    m_errorMonitor->SetDesiredErrorRegex(
        "VUID-vkCmdDrawIndexed-None-02721",
        "my_pipeline::my_draw([\\s\\S]*)index_buffer\\[ 1 \\] \\(1\\) \\+ vertexOffset \\(0\\) = Vertex index 1");

    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndexBuffer, IndirectDrawBadVertexIndex32DebugLabel) {
    TEST_DESCRIPTION(
        "Validate illegal index buffer values - uint32_t index. Also make sure debug label regions are properly accounted for.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

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

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "my_pipeline";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    label.pLabelName = "my_draw";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vkt::Buffer index_buffer = vkt::IndexBuffer<uint32_t>(*m_device, {0, 666, 42});
    vkt::Buffer vertex_buffer = vkt::VertexBuffer<float>(*m_device, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
    VkDeviceSize vertex_buffer_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &vertex_buffer_offset);

    m_errorMonitor->SetDesiredErrorRegex(
        "VUID-vkCmdDrawIndexedIndirect-None-02721",
        "my_pipeline::my_draw([\\s\\S]*)index_buffer\\[ 1 \\] \\(666\\) \\+ vertexOffset \\(0\\) = Vertex index 666");
    m_errorMonitor->SetDesiredErrorRegex(
        "VUID-vkCmdDrawIndexedIndirect-None-02721",
        "my_pipeline::my_draw([\\s\\S]*)index_buffer\\[ 2 \\] \\(42\\) \\+ vertexOffset \\(0\\) = Vertex index 42");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_params_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
