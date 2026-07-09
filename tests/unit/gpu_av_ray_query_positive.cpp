/* Copyright (c) 2023-2026 The Khronos Group Inc.
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "descriptor_helper.h"
#include "ray_tracing_objects.h"
#include "utils/math_utils.h"

void GpuAVRayQueryTest::InitGpuAVRayQuery(std::vector<VkLayerSettingEXT> layer_settings) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVRayQuery : public GpuAVRayQueryTest {};

TEST_F(PositiveGpuAVRayQuery, ComputeBasic) {
    TEST_DESCRIPTION("Ray query in a compute shader");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    const char* shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_.WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicTminTmax) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set t_min and t_max");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    const char* shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          float t_min;
          float t_max;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(100000),
              trace_rays_params.t_min, vec3(0,1,0), trace_rays_params.t_max);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_.WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_.WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_.UpdateDescriptorSets();

    // Ray query with t_min dynamically set to 0
    auto uniform_buffer_ptr = static_cast<float*>(uniform_buffer.Memory().Map());
    {
        uniform_buffer_ptr[0] = 0.0f;   // t_min
        uniform_buffer_ptr[1] = 42.0f;  // t_max
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Ray query with both t_min and t_max dynamically set to 42
    {
        uniform_buffer_ptr[0] = 42.0f;  // t_min
        uniform_buffer_ptr[1] = 42.0f;  // t_max
    }
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicRayFlags) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags");
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    const char* shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_.WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_.WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_.UpdateDescriptorSets();

    // Ray query with t_min dynamically set to 0
    {
        auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 4u | 16u;  // gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsCullBackFacingTrianglesEXT
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, ComputeDynamicRayFlagsSkipTriangles) {
    TEST_DESCRIPTION("Ray query in a compute shader, with dynamically set ray flags containing gl_RayFlagsSkipTrianglesEXT");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    const char* shader_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
        layout(set = 0, binding = 1) uniform Uniforms {
          uint ray_query_flags;
        } trace_rays_params;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, trace_rays_params.ray_query_flags, 0xff, vec3(0), 0.1, vec3(0,0,1), 42.0);
            rayQueryProceedEXT(query);
        }
    )glsl";

    CreateComputePipelineHelper pipeline(*this);
    pipeline.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipeline.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipeline.CreateComputePipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_.WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Add uniform buffer binding
    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    pipeline.descriptor_set_.WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);

    pipeline.descriptor_set_.UpdateDescriptorSets();

    {
        auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer.Memory().Map());
        uniform_buffer_ptr[0] = 4u | 0x100;  // gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipTrianglesEXT
    }
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, GraphicsBasic) {
    TEST_DESCRIPTION("Ray query in a vertex shader");
    RETURN_IF_SKIP(InitGpuAVRayQuery());
    InitRenderTarget();

    const char* vertex_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : require

        layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

        void main() {
            rayQueryEXT query;
            rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
            rayQueryProceedEXT(query);
            gl_Position = vec4(1);
        }
    )glsl";
    VkShaderObj vs(*m_device, vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipeline(*this);
    pipeline.shader_stages_ = {vs.GetStageCreateInfo(), pipeline.fs_->GetStageCreateInfo()};
    pipeline.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    pipeline.CreateGraphicsPipeline();

    // Add TLAS binding
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer);
    pipeline.descriptor_set_->WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_layout_, 0, 1,
                              &pipeline.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, RayTracingBasic) {
    TEST_DESCRIPTION("Ray query in a ray generation shader");
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set ray gen shader

    const char* ray_gen = R"glsl(
    #version 460

    #extension GL_EXT_ray_query : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    void main() {
      rayQueryEXT query;
      rayQueryInitializeEXT(query, tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 1000.0);
      rayQueryProceedEXT(query);
    }
)glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayQuery, Test) {
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitGpuAVRayQuery());

    const char* rgen_source = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : require
        layout(location = 0) rayPayloadEXT vec3 hitValue;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

        void main() {
          uint rayFlags = 0;
          uint cullMask = 0xFF;
          float tmin = 0.0;
          float tmax = 9.0;
          vec3 origin = vec3((float(gl_LaunchIDEXT.x) + 0.5f) / float(gl_LaunchSizeEXT.x), (float(gl_LaunchIDEXT.y) + 0.5f) / float(gl_LaunchSizeEXT.y), 0.0);
          vec3 direct = vec3(0.0, 0.0, -1.0);
          traceRayEXT(topLevelAS, rayFlags, cullMask, 0, 0, 0, origin, tmin, direct, tmax, 0);
        }
    )glsl";

    const char* ahit_source = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_ray_query : require
        hitAttributeEXT vec3 attribs;
        layout(location = 0) rayPayloadInEXT vec3 hitValue;
        layout(set = 0, binding = 0, r32i) uniform iimage3D result;
        layout(set = 0, binding = 2) uniform accelerationStructureEXT rayQueryTopLevelAccelerationStructure;

        void main() {
          ivec3 pos = ivec3(gl_LaunchIDEXT);
          ivec3 size = ivec3(gl_LaunchSizeEXT);
          uint rayFlags = 0;
          uint cullMask = 0xFF;
          float tmin = 0.0;
          float tmax = 9.0;
          vec3 origin = vec3((float(pos.x) + 0.5f) / float(size.x), (float(pos.y) + 0.5f) / float(size.y), 0.0);
          vec3 direct = vec3(0.0, 0.0, -1.0);
          uint value = 1;
          rayQueryEXT rayQuery;

          rayQueryInitializeEXT(rayQuery, rayQueryTopLevelAccelerationStructure, rayFlags, cullMask, origin, tmin, direct, tmax);

          if (rayQueryProceedEXT(rayQuery)) {
            value++;
            rayQueryTerminateEXT(rayQuery);
          }

          imageStore(result, pos, ivec4(value, 0, 0, 0));
        }
    )glsl";

    const char* chit_source = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : require
        hitAttributeEXT vec3 attribs;
        layout(location = 0) rayPayloadInEXT vec3 hitValue;

        void main() {
        }
    )glsl";

    const char* miss_source = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : require
        layout(location = 0) rayPayloadInEXT vec3 hitValue;

        void main() {
        }
    )glsl";

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R32_SINT;
    image_ci.extent = {8u, 8u, 8u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!IsImageFormatSupported(Gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_R32_SINT not supported";
    }
    vkt::Image result_image(*m_device, image_ci);
    result_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView result_view = result_image.CreateView(VK_IMAGE_VIEW_TYPE_3D, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                         VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::as::GeometryKHR geometry;
    geometry.SetType(vkt::as::GeometryKHR::Type::Triangle);
    float vertices[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, -1.0f,
        +1.0f, -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f,
    };
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags geometry_buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                     VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer vertex_buffer(*m_device, sizeof(vertices), geometry_buffer_usage, kHostVisibleMemProps, &alloc_flags);
    void* vertex_buffer_data = vertex_buffer.Memory().Map();
    memcpy(vertex_buffer_data, vertices, sizeof(vertices));
    vertex_buffer.Memory().Unmap();

    const VkTransformMatrixKHR transform_matrix = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
    }};
    vkt::Buffer transform_buffer(*m_device, sizeof(transform_matrix), geometry_buffer_usage, kHostVisibleMemProps, &alloc_flags);
    void* transform_buffer_data = transform_buffer.Memory().Map();
    memcpy(transform_buffer_data, &transform_matrix, sizeof(transform_matrix));
    transform_buffer.Memory().Unmap();
    geometry.SetPrimitiveCount(2u);
    geometry.SetTrianglesDeviceVertexBuffer(std::move(vertex_buffer), 5u);
    geometry.SetTrianglesIndexType(VK_INDEX_TYPE_NONE_KHR);
    geometry.SetTrianglesTransformBuffer(std::move(transform_buffer));
    geometry.SetFlags(0u);
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(geometry));
    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());
    m_command_buffer.Begin();
    tlas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    const VkShaderStageFlags ray_tracing_stages = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
                                                  VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR |
                                                  VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0u, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1u, ray_tracing_stages, nullptr},
                                        {1u, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1u, ray_tracing_stages, nullptr},
                                        {2u, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1u, ray_tracing_stages, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkShaderObj rgen_shader(*m_device, rgen_source, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj ahit_shader(*m_device, ahit_source, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(*m_device, chit_source, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj miss_shader(*m_device, miss_source, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipelineShaderStageCreateInfo stage_cis[4];
    stage_cis[0] = vku::InitStructHelper();
    stage_cis[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_cis[0].module = rgen_shader;
    stage_cis[0].pName = "main";
    stage_cis[1] = vku::InitStructHelper();
    stage_cis[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    stage_cis[1].module = ahit_shader;
    stage_cis[1].pName = "main";
    stage_cis[2] = vku::InitStructHelper();
    stage_cis[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_cis[2].module = chit_shader;
    stage_cis[2].pName = "main";
    stage_cis[3] = vku::InitStructHelper();
    stage_cis[3].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    stage_cis[3].module = miss_shader;
    stage_cis[3].pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_cis[3];
    group_cis[0] = vku::InitStructHelper();
    group_cis[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_cis[0].generalShader = 0u;
    group_cis[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_cis[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_cis[0].intersectionShader = VK_SHADER_UNUSED_KHR;
    group_cis[1] = vku::InitStructHelper();
    group_cis[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_cis[1].generalShader = 3u;
    group_cis[1].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_cis[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_cis[1].intersectionShader = VK_SHADER_UNUSED_KHR;
    group_cis[2] = vku::InitStructHelper();
    group_cis[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_cis[2].generalShader = VK_SHADER_UNUSED_KHR;
    group_cis[2].closestHitShader = 2u;
    group_cis[2].anyHitShader = 1u;
    group_cis[2].intersectionShader = VK_SHADER_UNUSED_KHR;

    const uint32_t group_count = 3;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.stageCount = 4u;
    pipeline_ci.pStages = stage_cis;
    pipeline_ci.groupCount = group_count;
    pipeline_ci.pGroups = group_cis;
    pipeline_ci.maxPipelineRayRecursionDepth = 1u;
    pipeline_ci.layout = pipeline_layout;
    vkt::Pipeline rt_pipeline(*m_device, pipeline_ci);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rt_props);
    const uint32_t sbt_entry_size = Align(rt_props.shaderGroupHandleSize, rt_props.shaderGroupBaseAlignment);
    std::vector<uint8_t> shader_group_handles(rt_props.shaderGroupHandleSize * group_count);
    vk::GetRayTracingShaderGroupHandlesKHR(device(), rt_pipeline, 0u, group_count, shader_group_handles.size(),
                                           shader_group_handles.data());

    vkt::Buffer sbt_buffer(*m_device, sbt_entry_size * group_count, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                           vkt::device_address);
    uint8_t* sbt_ptr = static_cast<uint8_t*>(sbt_buffer.Memory().Map());
    for (uint32_t i = 0; i < group_count; ++i) {
        memcpy(sbt_ptr + sbt_entry_size * i, shader_group_handles.data() + rt_props.shaderGroupHandleSize * i,
               rt_props.shaderGroupHandleSize);
    }
    sbt_buffer.Memory().Unmap();

    const VkDeviceAddress sbt_address = sbt_buffer.Address();
    const VkStridedDeviceAddressRegionKHR raygen_sbt = {sbt_address, sbt_entry_size, sbt_entry_size};
    const VkStridedDeviceAddressRegionKHR miss_sbt = {sbt_address + sbt_entry_size, sbt_entry_size, sbt_entry_size};
    const VkStridedDeviceAddressRegionKHR hit_sbt = {sbt_address + 2u * sbt_entry_size, sbt_entry_size, sbt_entry_size};
    const VkStridedDeviceAddressRegionKHR callable_sbt = {};

    VkAccelerationStructureKHR tlas_handle = tlas.GetDstAS()->handle();
    VkAccelerationStructureKHR null_as = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorImageInfo(0, result_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorAccelStruct(1, 1, &tlas_handle);
    descriptor_set.WriteDescriptorAccelStruct(2, 1, &null_as);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_layout, 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    vk::CmdTraceRaysKHR(m_command_buffer, &raygen_sbt, &miss_sbt, &hit_sbt, &callable_sbt, 8u, 8u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}
