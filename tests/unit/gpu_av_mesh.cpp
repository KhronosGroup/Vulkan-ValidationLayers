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
#include "../framework/descriptor_helper.h"

class NegativeGpuAVMesh : public GpuAVMesh {};

TEST_F(NegativeGpuAVMesh, VertexCountConstant) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(triangles, max_vertices = 3, max_primitives = 1) out;
        void main() {
            SetMeshOutputsEXT(6, 1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(0);
            gl_PrimitiveTriangleIndicesEXT[0] =  uvec3(0, 1, 2);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07332");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, PrimitiveCountConstant) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(triangles, max_vertices = 3, max_primitives = 1) out;
        void main() {
            SetMeshOutputsEXT(3, 2);
            gl_MeshVerticesEXT[0].gl_Position = vec4(0);
            gl_PrimitiveTriangleIndicesEXT[0] =  uvec3(0, 1, 2);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07333");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, PrimitiveCount) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout(triangles, max_vertices = 3, max_primitives = 1) out;
        layout(set = 0, binding = 0) buffer SSBO {
            uint v;
            uint p;
        };
        void main() {
            SetMeshOutputsEXT(v, p);
            gl_MeshVerticesEXT[0].gl_Position = vec4(0);
            gl_PrimitiveTriangleIndicesEXT[0] =  uvec3(0, 1, 2);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 60;
    buffer_ptr[1] = 1;

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07332");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
