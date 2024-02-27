/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_helper.h"
#include "../framework/gpu_av_helper.h"

TEST_F(PositiveGpuAVRayTracing, BasicTraceRays) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;

    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysMultipleStages) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
        traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);

    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveGpuAVRayTracing, DynamicTminTmax) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic t_min and t_max");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      float t_min;
      float t_max;
    } trace_rays_params;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      float t_min;
      float t_max;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
        traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max, 0);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      float t_min;
      float t_max;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;
      traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), trace_rays_params.t_min, vec3(0,0,1), trace_rays_params.t_max, 0);

    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Add uniform buffer binding
    auto uniform_buffer = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto uniform_buffer_ptr = static_cast<float*>(uniform_buffer->memory().map());
    uniform_buffer_ptr[0] = 0.1f;   // t_min
    uniform_buffer_ptr[1] = 42.0f;  // t_max
    uniform_buffer->memory().unmap();

    pipeline.SetUniformBufferBinding(uniform_buffer, 1);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveGpuAVRayTracing, BasicTraceRaysDynamicRayFlags) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic ray flags mask set to gl_RayFlagsCullBackFacingTrianglesEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
        traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;
      traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Add uniform buffer binding
    auto uniform_buffer = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer->memory().map());
    uniform_buffer_ptr[0] = 16;  // gl_RayFlagsCullBackFacingTrianglesEXT
    uniform_buffer->memory().unmap();

    pipeline.SetUniformBufferBinding(uniform_buffer, 1);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}

TEST_F(PositiveGpuAVRayTracing, DynamicRayFlagsSkipTriangle) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray, with dynamic ray flags mask set to gl_RayFlagsSkipTrianglesEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    #extension GL_EXT_ray_flags_primitive_culling : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadEXT vec3 hit;

    void main() {
      traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.SetRayGenShader(ray_gen);

    const char* miss = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require
    #extension GL_EXT_ray_flags_primitive_culling : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;

    void main() {
        hit = vec3(0.1, 0.2, 0.3);
        traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.AddMissShader(miss);

    const char* closest_hit = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require
    #extension GL_EXT_ray_flags_primitive_culling : require

    layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
    layout(binding = 1, set = 0) uniform Uniforms {
      uint ray_flags;
    } trace_rays_params;

    layout(location = 0) rayPayloadInEXT vec3 hit;
    hitAttributeEXT vec2 baryCoord;

    void main() {
      const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
      hit = barycentricCoords;
      traceRayEXT(tlas, trace_rays_params.ray_flags, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1.0, 0);
    }
)glsl";
    pipeline.AddClosestHitShader(closest_hit);

    // Add TLAS binding
    auto tlas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_commandBuffer));
    pipeline.AddTopLevelAccelStructBinding(std::move(tlas), 0);

    // Add uniform buffer binding
    auto uniform_buffer = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto uniform_buffer_ptr = static_cast<uint32_t*>(uniform_buffer->memory().map());
    uniform_buffer_ptr[0] = 0x100;  // gl_RayFlagsSkipTrianglesEXT, or RayFlagsSkipTrianglesKHRMask in SPIR-V
    uniform_buffer->memory().unmap();

    pipeline.SetUniformBufferBinding(uniform_buffer, 1);

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet()->set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(*m_commandBuffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    m_device->wait();
}
