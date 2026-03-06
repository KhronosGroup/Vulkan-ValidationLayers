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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"

class NegativeGpuAVMesh : public GpuAVMesh {};

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

TEST_F(NegativeGpuAVMesh, DISABLED_TaskPayloadSharedMissing) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char* task_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        void main() {
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint payload;
        void main() {
            uint x = payload;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ts(*m_device, task_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, TaskPayloadSharedMissingNoTask) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint payload;
        void main() {
            uint x = payload;
            SetMeshOutputsEXT(3,1);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, TaskPayloadSharedMissingNoTaskStruct) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        struct S {
            uint x;
            uint y[4];
        };
        taskPayloadSharedEXT S payload;
        void main() {
            uint a = payload.y[2];
            SetMeshOutputsEXT(3,1);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, TaskPayloadSharedMissingNoTaskSlang) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    RETURN_IF_SKIP(CheckSlangSupport());
    InitRenderTarget();

    const char* mesh_source = R"glsl(
        struct S {
            uint x;
            uint y[4];
        };
        struct VertexOut {
            float4 position : SV_Position;
        };

        [shader("mesh")]
        [numthreads(1, 1, 1)]
        [outputtopology("triangle")]
        void main(
            in payload S payloadData,
            OutputVertices<VertexOut, 32> vertices,
            OutputIndices<uint3, 32> indices
        ) {
            uint a = payloadData.y[2];

            SetMeshOutputCounts(3, 1);

            VertexOut v;
            v.position = float4(0.0, 0.0, 0.0, 0.0);
            vertices[0] = v;

            if (a > 2) {
                a = 2; // prevent dead code
            }
            indices[0] = uint3(0, 1, a);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_SLANG);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, DISABLED_TaskPayloadSharedDifferent) {
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitRenderTarget();

    const char* task_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        struct Foo {
            uint a[3];
            uint b;
        };
        taskPayloadSharedEXT Foo payload;
        void main() {
            payload.b = 4;
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint payload;
        void main() {
            uint x = payload;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ts(*m_device, task_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVMesh, DISABLED_TaskPayloadSharedMissingShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicMeshAndTask());
    InitDynamicRenderTarget();

    const char* task_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        void main() {
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint mesh_payload;
        void main() {
            uint x = mesh_payload;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    const vkt::Shader task_shader(*m_device, VK_SHADER_STAGE_TASK_BIT_EXT, task_source);
    const vkt::Shader mesh_shader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT, mesh_source);
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    m_command_buffer.BindMeshShaders(task_shader, mesh_shader, frag_shader);
    SetDefaultDynamicStatesExclude();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-12380");
    vk::CmdDrawMeshTasksEXT(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}
