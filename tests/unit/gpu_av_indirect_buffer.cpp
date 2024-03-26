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
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"

TEST_F(NegativeGpuAVIndirectBuffer, DrawCountDeviceLimit) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);  // instead of enabling feature
    AddOptionalExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&mesh_shader_features);
    bool mesh_shader_enabled = false;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        GetPhysicalDeviceFeatures2(mesh_shader_features);
        mesh_shader_enabled = IsExtensionsEnabled(VK_EXT_MESH_SHADER_EXTENSION_NAME) && features13.maintenance4;
        if (mesh_shader_enabled) {
            mesh_shader_features.multiviewMeshShader = VK_FALSE;
            mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        }
    }

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState(nullptr, (features13.dynamicRendering || mesh_shader_enabled) ? (void *)&features13 : nullptr));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 2 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndirectCommand));
    draw_buffer.memory().unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.memory().unmap();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                sizeof(VkDrawIndirectCommand));

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();

    if (features13.dynamicRendering) {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
        vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndirectCommand));

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();
    }

    if (mesh_shader_enabled) {
        char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {})glsl";
        VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
        mesh_pipe.CreateGraphicsPipeline();
        vkt::Buffer mesh_draw_buffer(*m_device, 2 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDrawMeshTasksIndirectCommandEXT *mesh_draw_ptr =
            static_cast<VkDrawMeshTasksIndirectCommandEXT *>(mesh_draw_buffer.memory().map());
        mesh_draw_ptr->groupCountX = 0;
        mesh_draw_ptr->groupCountY = 0;
        mesh_draw_ptr->groupCountZ = 0;
        mesh_draw_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02717");
        count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
        *count_ptr = 2;
        count_buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), mesh_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVIndirectBuffer, DrawCountDeviceLimitSubmit2) {
    TEST_DESCRIPTION("GPU validation: Validate maxDrawIndirectCount limit using vkQueueSubmit2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxDrawIndirectCount = 1;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    VkPhysicalDeviceVulkan13Features features_13 = vku::InitStructHelper();
    VkPhysicalDeviceVulkan12Features features_12 = vku::InitStructHelper(&features_13);
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&features_12);
    GetPhysicalDeviceFeatures2(features2);
    if (!features_12.drawIndirectCount) {
        GTEST_SKIP() << "drawIndirectCount not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 2 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *draw_ptr = static_cast<VkDrawIndexedIndirectCommand *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, 2 * sizeof(VkDrawIndexedIndirectCommand));
    draw_buffer.memory().unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;  // Fits in buffer but exceeds (fake) limit
    count_buffer.memory().unmap();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717");
    vk::CmdDrawIndexedIndirectCount(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 2,
                                    sizeof(VkDrawIndexedIndirectCommand));

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vkt::Fence null_fence;
    // use vkQueueSumit2
    m_commandBuffer->QueueCommandBuffer(null_fence, true, true);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DrawCount) {
    TEST_DESCRIPTION("GPU validation: Validate Draw*IndirectCount countBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&mesh_shader_features);
    bool mesh_shader_enabled = false;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        GetPhysicalDeviceFeatures2(mesh_shader_features);
        mesh_shader_enabled = IsExtensionsEnabled(VK_EXT_MESH_SHADER_EXTENSION_NAME) && features13.maintenance4;
        if (mesh_shader_enabled) {
            mesh_shader_features.multiviewMeshShader = VK_FALSE;
            mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, mesh_shader_enabled ? &features13 : nullptr));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    draw_ptr->firstInstance = 0;
    draw_ptr->firstVertex = 0;
    draw_ptr->instanceCount = 1;
    draw_ptr->vertexCount = 3;
    draw_buffer.memory().unmap();

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03122");
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;
    count_buffer.memory().unmap();
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectCount-countBuffer-03121");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Offset of 4 should error
    vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154");
    vkt::Buffer indexed_draw_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.memory().map();
    indexed_draw_ptr->indexCount = 3;
    indexed_draw_ptr->firstIndex = 0;
    indexed_draw_ptr->instanceCount = 1;
    indexed_draw_ptr->firstInstance = 0;
    indexed_draw_ptr->vertexOffset = 0;
    indexed_draw_buffer.memory().unmap();

    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 2;
    count_buffer.memory().unmap();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 1;
    count_buffer.memory().unmap();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153");
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    // Offset of 4 should error
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    if (mesh_shader_enabled) {
        char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {})glsl";
        VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
        mesh_pipe.CreateGraphicsPipeline();
        vkt::Buffer mesh_draw_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDrawMeshTasksIndirectCommandEXT *mesh_draw_ptr =
            +static_cast<VkDrawMeshTasksIndirectCommandEXT *>(mesh_draw_buffer.memory().map());
        mesh_draw_ptr->groupCountX = 0;
        mesh_draw_ptr->groupCountY = 0;
        mesh_draw_ptr->groupCountZ = 0;
        mesh_draw_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-07098");
        count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
        *count_ptr = 1;
        count_buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), mesh_draw_buffer.handle(), 8, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-07099");
        count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
        *count_ptr = 2;
        count_buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), mesh_draw_buffer.handle(), 4, count_buffer.handle(), 0, 1,
                                             sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVIndirectBuffer, Mesh) {
    TEST_DESCRIPTION("GPU validation: Validate DrawMeshTasksIndirect* DrawBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&mesh_shader_features);

    GetPhysicalDeviceFeatures2(features13);
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features13));
    InitRenderTarget();
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_props);

    if (mesh_shader_props.maxMeshWorkGroupTotalCount > 0xfffffffe) {
        GTEST_SKIP() << "MeshWorkGroupTotalCount too high for this test";
    }
    const uint32_t num_commands = 3;
    uint32_t buffer_size = num_commands * (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4);  // 4 byte pad between commands

    vkt::Buffer draw_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *draw_ptr = static_cast<uint32_t *>(draw_buffer.memory().map());
    memset(draw_ptr, 0, buffer_size);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.memory().map());
    *count_ptr = 3;
    count_buffer.memory().unmap();
    char const *mesh_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3, max_primitives = 1) out;
        layout(triangles) out;
        struct Task {
          uint baseID;
        };
        taskPayloadSharedEXT Task IN;
        void main() {})glsl";
    VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper mesh_pipe(*this);
    mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    mesh_pipe.CreateGraphicsPipeline();
    // 012 456 8910
    // Set x in third draw
    draw_ptr[8] = mesh_shader_props.maxMeshWorkGroupCount[0] + 1;
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), draw_buffer.handle(), 0, 3,
                                    (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();

    // Set y in second draw
    draw_ptr[8] = 0;
    draw_ptr[5] = mesh_shader_props.maxMeshWorkGroupCount[1] + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327");
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();

    // Set z in first draw
    draw_ptr[5] = 0;
    draw_ptr[2] = mesh_shader_props.maxMeshWorkGroupCount[2] + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328");
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    draw_ptr[2] = 0;

    uint32_t half_total = (mesh_shader_props.maxMeshWorkGroupTotalCount + 2) / 2;
    if (half_total < mesh_shader_props.maxMeshWorkGroupCount[0]) {
        draw_ptr[2] = 1;
        draw_ptr[1] = 2;
        draw_ptr[0] = (mesh_shader_props.maxMeshWorkGroupTotalCount + 2) / 2;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329");
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();

        draw_ptr[2] = 0;
        draw_ptr[1] = 0;
        draw_ptr[0] = 0;
    }

    char const *task_shader_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
        }
    )glsl";
    VkShaderObj task_shader(this, task_shader_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper task_pipe(*this);
    task_pipe.shader_stages_[0] = task_shader.GetStageCreateInfo();
    task_pipe.shader_stages_[1] = mesh_shader.GetStageCreateInfo();
    task_pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, task_pipe.Handle());
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 3,
                                         (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Set x in second draw
    draw_ptr[4] = mesh_shader_props.maxTaskWorkGroupCount[0] + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322");
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    draw_ptr[4] = 0;

    // Set y in first draw
    draw_ptr[1] = mesh_shader_props.maxTaskWorkGroupCount[0] + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323");
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    draw_ptr[1] = 0;

    // Set z in third draw
    draw_ptr[10] = mesh_shader_props.maxTaskWorkGroupCount[0] + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324");
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
    draw_ptr[10] = 0;

    half_total = (mesh_shader_props.maxTaskWorkGroupTotalCount + 2) / 2;
    if (half_total < mesh_shader_props.maxTaskWorkGroupCount[0]) {
        draw_ptr[2] = 1;
        draw_ptr[1] = 2;
        draw_ptr[0] = (mesh_shader_props.maxTaskWorkGroupTotalCount + 2) / 2;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325");
        m_commandBuffer->QueueCommandBuffer();
        m_default_queue->wait();
        m_errorMonitor->VerifyFound();

        draw_ptr[2] = 0;
        draw_ptr[1] = 0;
        draw_ptr[0] = 0;
    }
    draw_buffer.memory().unmap();
}

TEST_F(NegativeGpuAVIndirectBuffer, FirstInstance) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    features2.features.drawIndirectFirstInstance = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    vkt::Buffer draw_buffer(*m_device, 4 * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndirectCommand *draw_ptr = static_cast<VkDrawIndirectCommand *>(draw_buffer.memory().map());
    for (uint32_t i = 0; i < 4; i++) {
        draw_ptr->vertexCount = 3;
        draw_ptr->instanceCount = 1;
        draw_ptr->firstVertex = 0;
        draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        draw_ptr++;
    }
    draw_buffer.memory().unmap();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vku::InitStructHelper();
    vkt::PipelineLayout pipeline_layout(*m_device, pipelineLayoutCreateInfo);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawIndirectCommand-firstInstance-00501");
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirect(m_commandBuffer->handle(), draw_buffer.handle(), 0, 4, sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();

    // Now with an offset and indexed draw
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554");
    vkt::Buffer indexed_draw_buffer(*m_device, 4 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDrawIndexedIndirectCommand *indexed_draw_ptr = (VkDrawIndexedIndirectCommand *)indexed_draw_buffer.memory().map();
    for (uint32_t i = 0; i < 4; i++) {
        indexed_draw_ptr->indexCount = 3;
        indexed_draw_ptr->instanceCount = 1;
        indexed_draw_ptr->firstIndex = 0;
        indexed_draw_ptr->vertexOffset = 0;
        indexed_draw_ptr->firstInstance = (i == 3) ? 1 : 0;
        indexed_draw_ptr++;
    }
    indexed_draw_buffer.memory().unmap();

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vkt::Buffer index_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), indexed_draw_buffer.handle(), sizeof(VkDrawIndexedIndirectCommand), 3,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DispatchWorkgroupSize) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());

    vkt::Buffer indirect_buffer(*m_device, 5 * sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.memory().map());
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
    indirect_buffer.memory().unmap();

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();

    // Check again in a 2nd submitted command buffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVIndirectBuffer, DispatchWorkgroupSizeShaderObjects) {
    TEST_DESCRIPTION("GPU validation: Validate VkDispatchIndirectCommand");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxComputeWorkGroupCount[0] = 2;
    props.limits.maxComputeWorkGroupCount[1] = 2;
    props.limits.maxComputeWorkGroupCount[2] = 2;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(shaderObjectFeatures);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    vkt::Buffer indirect_buffer(*m_device, 5 * sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.memory().map());
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
    indirect_buffer.memory().unmap();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
    vkt::Shader shader(*m_device, stage, GLSLToSPV(stage, kMinimalShaderGlsl));

    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-y-00418");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), sizeof(VkDispatchIndirectCommand));

    // valid
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-z-00419");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 3 * sizeof(VkDispatchIndirectCommand));

    // Only expect to have the first error return
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 4 * sizeof(VkDispatchIndirectCommand));

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();

    // Check again in a 2nd submitted command buffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 1u, &stage, &shader.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 2 * sizeof(VkDispatchIndirectCommand));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDispatchIndirectCommand-x-00417");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), indirect_buffer.handle(), 0);

    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}
