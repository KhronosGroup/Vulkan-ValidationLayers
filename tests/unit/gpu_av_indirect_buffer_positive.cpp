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
#include "../framework/ray_tracing_objects.h"
#include "../framework/shader_object_helper.h"

class PositiveGpuAVIndirectBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVIndirectBuffer, BasicTraceRaysMultipleStages) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    // TODO: Refacto ray tracing extensions listing.
    // Originally from RayTracingTest::InitFrameworkForRayTracingTest
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char *miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char *closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    // Descriptor set
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVIndirectBuffer, Mesh) {
    TEST_DESCRIPTION("GPU validation: Validate DrawMeshTasksIndirect* DrawBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_props);

    if (mesh_shader_props.maxMeshWorkGroupTotalCount > 0xfffffffe) {
        GTEST_SKIP() << "MeshWorkGroupTotalCount too high for this test";
    }
    const uint32_t mesh_commands = 3;
    uint32_t buffer_size = mesh_commands * (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4);  // 4 byte pad between commands

    vkt::Buffer draw_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *draw_ptr = static_cast<uint32_t *>(draw_buffer.Memory().Map());
    // Set all mesh group counts to 1
    for (uint32_t i = 0; i < mesh_commands * 4; ++i) {
        draw_ptr[i] = 1;
    }

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 3;

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
        void main() {}
    )glsl";
    VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper mesh_pipe(*this);
    mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    mesh_pipe.CreateGraphicsPipeline();

    // Set x in third draw
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());

    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer, draw_buffer, 0, 3, (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVIndirectBuffer, MeshSingleCommand) {
    TEST_DESCRIPTION("GPU validation: Validate DrawMeshTasksIndirect* DrawBuffer contents");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_props);

    if (mesh_shader_props.maxMeshWorkGroupTotalCount > 0xfffffffe) {
        GTEST_SKIP() << "MeshWorkGroupTotalCount too high for this test";
    }
    const uint32_t mesh_commands = 1;
    uint32_t buffer_size = mesh_commands * (sizeof(VkDrawMeshTasksIndirectCommandEXT) + 4);  // 4 byte pad between commands

    vkt::Buffer draw_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *draw_ptr = static_cast<uint32_t *>(draw_buffer.Memory().Map());
    // Set all mesh group counts to 1
    for (uint32_t i = 0; i < mesh_commands * 4; ++i) {
        draw_ptr[i] = 1;
    }

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *count_ptr = static_cast<uint32_t *>(count_buffer.Memory().Map());
    *count_ptr = 3;
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
        void main() {}
    )glsl";
    VkShaderObj mesh_shader(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    CreatePipelineHelper mesh_pipe(*this);
    mesh_pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    mesh_pipe.CreateGraphicsPipeline();

    // Set x in third draw
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());

    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer, draw_buffer, 0, 1, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVIndirectBuffer, FirstInstanceSingleDrawIndirectCommand) {
    TEST_DESCRIPTION("Validate illegal firstInstance values");
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    // silence MacOS issue
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.lineWidth = 1.0f;
    pipe.CreateGraphicsPipeline();

    VkDrawIndirectCommand draw_params{};
    draw_params.vertexCount = 3;
    draw_params.instanceCount = 1;
    draw_params.firstVertex = 0;
    draw_params.firstInstance = 0;
    vkt::Buffer draw_params_buffer = vkt::IndirectBuffer<VkDrawIndirectCommand>(*m_device, {draw_params});

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdDrawIndirect(m_command_buffer, draw_params_buffer, 0, 1, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9917
TEST_F(PositiveGpuAVIndirectBuffer, PipelineAndShaderObjectComputeDispatchIndirect) {
    TEST_DESCRIPTION("Mix usage of pipeline and shader object");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    vkt::Buffer dispatch_params_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                       kHostVisibleMemProps);
    auto &indirect_dispatch_parameters = *static_cast<VkDispatchIndirectCommand *>(dispatch_params_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1u;
    indirect_dispatch_parameters.y = 1u;
    indirect_dispatch_parameters.z = 1u;

    const VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
    const auto comp_spv = GLSLToSPV(stage, kMinimalShaderGlsl);
    VkShaderCreateInfoEXT comp_create_info = ShaderCreateInfo(comp_spv, stage);
    const vkt::Shader comp_shader(*m_device, comp_create_info);

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer.handle(), 0u);

    vk::CmdBindShadersEXT(m_command_buffer, 1u, &stage, &comp_shader.handle());
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer.handle(), 0u);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVIndirectBuffer, RestoreStress) {
    TEST_DESCRIPTION("Try and use crazy combo of pipeline and shader object to make sure the RestorePipeline logic works");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    vkt::Buffer dispatch_params_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                       kHostVisibleMemProps);
    auto &indirect_dispatch_parameters = *static_cast<VkDispatchIndirectCommand *>(dispatch_params_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1u;
    indirect_dispatch_parameters.y = 1u;
    indirect_dispatch_parameters.z = 1u;

    // used for all stage types
    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Input {
            uint x;
        };
        void main() {
           x = 0;
        }
    )glsl";

    OneOffDescriptorSet c_descriptor_set(m_device,
                                         {
                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                         });
    OneOffDescriptorSet g_descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    const vkt::PipelineLayout g_pipeline_layout(*m_device, {&g_descriptor_set.layout_});
    const vkt::PipelineLayout c_pipeline_layout(*m_device, {&c_descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    c_descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    c_descriptor_set.UpdateDescriptorSets();
    g_descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    g_descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper c_pipe(*this);
    c_pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    c_pipe.cp_ci_.layout = c_pipeline_layout;
    c_pipe.CreateComputePipeline();

    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkFormat color_formats = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper g_pipe(*this, &pipeline_rendering_info);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.gp_ci_.layout = g_pipeline_layout;
    g_pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    g_pipe.CreateGraphicsPipeline();

    const vkt::Shader cs_shader_object(*m_device, VK_SHADER_STAGE_COMPUTE_BIT,
                                       GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source), &c_descriptor_set.layout_.handle());
    const vkt::Shader vs_shader_object(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source),
                                       &g_descriptor_set.layout_.handle());
    const vkt::Shader fs_shader_object(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                       GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, shader_source), &g_descriptor_set.layout_.handle());

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1, &g_descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, c_pipeline_layout, 0, 1, &c_descriptor_set.set_, 0,
                              nullptr);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    m_command_buffer.BindShaders(vs_shader_object, fs_shader_object);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, c_pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);

    m_command_buffer.BindCompShader(cs_shader_object);
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRendering();

    m_command_buffer.BindCompShader(cs_shader_object);
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);
    m_command_buffer.BindShaders(vs_shader_object, fs_shader_object);
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, c_pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, dispatch_params_buffer, 0u);

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRendering();

    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVIndirectBuffer, BufferUsageFlags2) {
    TEST_DESCRIPTION("test VkBufferUsageFlags2CreateInfo");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = sizeof(VkDispatchIndirectCommand);
    vkt::Buffer indirect_buffer(*m_device, buffer_ci, kHostVisibleMemProps);
    VkDispatchIndirectCommand *ptr = static_cast<VkDispatchIndirectCommand *>(indirect_buffer.Memory().Map());
    ptr->x = 1;
    ptr->y = 1;
    ptr->z = 1;

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, indirect_buffer, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}
