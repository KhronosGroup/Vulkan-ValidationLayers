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

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeDebugPrintfRayTracing : public DebugPrintfTests {
  public:
    void InitFrameworkWithPrintfBufferSize(uint32_t printf_buffer_size) {
        SetTargetApiVersion(VK_API_VERSION_1_1);
        AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

        VkBool32 printf_value = VK_TRUE;
        VkLayerSettingEXT printf_enable_setting = {OBJECT_LAYER_NAME, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                   &printf_value};

        VkLayerSettingEXT printf_buffer_size_setting = {OBJECT_LAYER_NAME, "printf_buffer_size", VK_LAYER_SETTING_TYPE_UINT32_EXT,
                                                        1, &printf_buffer_size};

        std::array<VkLayerSettingEXT, 2> layer_settings = {printf_enable_setting, printf_buffer_size_setting};
        VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
        layer_settings_create_info.settingCount = static_cast<uint32_t>(layer_settings.size());
        layer_settings_create_info.pSettings = layer_settings.data();
        RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
        if (!CanEnableGpuAV(*this)) {
            GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
        }
        if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
            GTEST_SKIP() << "Currently disabled for Portability";
        }
    }
};

TEST_F(NegativeDebugPrintfRayTracing, Raygen) {
    TEST_DESCRIPTION("Test debug printf in raygen shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            debugPrintfEXT("In Raygen\n");
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredInfo("In Raygen");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfRayTracing, RaygenOneMissShaderOneClosestHitShader) {
    TEST_DESCRIPTION("Test debug printf in raygen, miss and closest hit shaders.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkWithPrintfBufferSize(1024 * 1024));
    RETURN_IF_SKIP(InitState());

    // #ARNO_TODO: For clarity, here geometry should be set explicitly, as of now the ray hitting or not
    // implicitly depends on the default triangle position.
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    // Build Bottom Level Acceleration Structure
    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());
    m_command_buffer.Begin();
    tlas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Buffer used to count invocations for the 3 shader types
    vkt::Buffer debug_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, debug_buffer, 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            uint last = atomicAdd(debug_buffer[0], 1);
            debugPrintfEXT("In Raygen %u", last);

            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            uint last = atomicAdd(debug_buffer[1], 1);
            debugPrintfEXT("In Miss %u", last);
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };


        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            uint last = atomicAdd(debug_buffer[2], 1);
            debugPrintfEXT("In Closest Hit %u", last);
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    constexpr uint32_t frames_count = 14;
    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;
    const uint32_t ray_gen_rays_count = ray_gen_width * ray_gen_height * ray_gen_depth;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                                  &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
        vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

        vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                            &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);

        m_command_buffer.End();
        for (uint32_t i = 0; i < ray_gen_rays_count; ++i) {
            std::string msg = "In Raygen " + std::to_string(frame * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }
        for (uint32_t i = 0; i < 3 * ray_gen_rays_count; ++i) {
            std::string msg = "In Miss " + std::to_string(frame * 3 * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }
        for (uint32_t i = 0; i < 2 * ray_gen_rays_count; ++i) {
            std::string msg = "In Closest Hit " + std::to_string(frame * 2 * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }

        m_default_queue->SubmitAndWait(m_command_buffer);

        m_errorMonitor->VerifyFound();
    }

    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], ray_gen_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 3 * ray_gen_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], 2 * ray_gen_rays_count * frames_count);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9559
TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 1 ray generation shader, 1 miss shader, 1 closest hit shader");

    RETURN_IF_SKIP(CheckSlangSupport());

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitDebugPrintfFramework());

    RETURN_IF_SKIP(InitState());

    // #ARNO_TODO: For clarity, here geometry should be set explicitly, as of now the ray hitting or not
    // implicitly depends on the default triangle position.
    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    // Build Bottom Level Acceleration Structure
    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, *blas.GetDstAS());
    m_command_buffer.Begin();
    tlas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Buffer used to count invocations for the 3 shader types
    vkt::Buffer debug_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, debug_buffer, 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
        [[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

        struct RayPayload {
            float3 hit;
        };

        [shader("raygeneration")]
        void rayGenShader()
        {
            printf("In Raygen");
            InterlockedAdd(debug_buffer[0], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will hit
            ray.Origin = float3(0,0,-50);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,-50);
            ray.Direction = float3(0,0,-1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,50);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,50);
            ray.Direction = float3(0,0,-1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

        }

        [shader("miss")]
        void missShader(inout RayPayload payload)
        {
            printf("In Miss");
            InterlockedAdd(debug_buffer[1], 1);
            payload.hit = float3(0.1, 0.2, 0.3);
        }

        [shader("closesthit")]
        void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit");
            InterlockedAdd(debug_buffer[2], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
        attr.barycentrics.y); payload.hit = barycentric_coords;
        }
    )slang";

    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader");
    pipeline.AddSlangMissShader(slang_shader, "missShader");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                                  &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
        vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

        vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                            &trace_rays_sbt.callable_sbt, 1, 1, 1);

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen");
        m_errorMonitor->SetDesiredInfo("In Miss", 3);
        m_errorMonitor->SetDesiredInfo("In Closest Hit", 2);
        m_default_queue->SubmitAndWait(m_command_buffer);
    }
    m_errorMonitor->VerifyFound();

    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 3 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], 2 * frames_count);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9559
TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRays) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysKHR");

    RETURN_IF_SKIP(CheckSlangSupport());

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::GetCubesTLAS(*m_device, m_command_buffer, *m_default_queue, cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, debug_buffer, 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
        [[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

        struct RayPayload {
            float3 hit;
        };

        [shader("raygeneration")]
        void rayGenShader()
        {
            printf("In Raygen 1");
            InterlockedAdd(debug_buffer[0], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,-1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 1
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(-1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("raygeneration")]
        void rayGenShader2()
        {
            printf("In Raygen 2");
            InterlockedAdd(debug_buffer[1], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will hit cube 1
            ray.Origin = float3(-50,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("miss")]
        void missShader(inout RayPayload payload)
        {
            printf("In Miss 1");
            InterlockedAdd(debug_buffer[2], 1);
            payload.hit = float3(0.1, 0.2, 0.3);
        }

        [shader("miss")]
        void missShader2(inout RayPayload payload)
        {
            printf("In Miss 2");
            InterlockedAdd(debug_buffer[3], 1);
            payload.hit = float3(42, 42, 42);
        }

        [shader("closesthit")]
        void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 1");
            InterlockedAdd(debug_buffer[4], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
                attr.barycentrics.y); payload.hit = barycentric_coords;

            RayPayload ray_payload = { float3(0) };
            RayDesc ray;

            ray.TMin = 0.01;
            ray.TMax = 1000.0;
            const uint32_t miss_shader_i = 1;

            // Supposed to hit cube 2, and invoke closestHitShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);

            // Supposed to miss, and call missShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,1,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);
        }

        [shader("closesthit")]
        void closestHitShader2(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 2");
            InterlockedAdd(debug_buffer[5], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
        attr.barycentrics.y); payload.hit = 999 * barycentric_coords;
        }
    )slang";

    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader");
    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader2");
    pipeline.AddSlangMissShader(slang_shader, "missShader");
    pipeline.AddSlangMissShader(slang_shader, "missShader2");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                                  &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

        // Invoke ray gen shader 1
        vkt::rt::TraceRaysSbt sbt_ray_gen_1 = pipeline.GetTraceRaysSbt(0);
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt_ray_gen_1.ray_gen_sbt, &sbt_ray_gen_1.miss_sbt, &sbt_ray_gen_1.hit_sbt,
                            &sbt_ray_gen_1.callable_sbt, 1, 1, 1);

        // Invoke ray gen shader 2
        vkt::rt::TraceRaysSbt sbt_ray_gen_2 = pipeline.GetTraceRaysSbt(1);
        vk::CmdTraceRaysKHR(m_command_buffer, &sbt_ray_gen_2.ray_gen_sbt, &sbt_ray_gen_2.miss_sbt, &sbt_ray_gen_2.hit_sbt,
                            &sbt_ray_gen_2.callable_sbt, 1, 1, 1);

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1");
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 3);
        m_default_queue->SubmitAndWait(m_command_buffer);
    }
    m_errorMonitor->VerifyFound();

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 + 2) * frames_count);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9559
TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRaysIndirect) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysIndirect2KHR");

    RETURN_IF_SKIP(CheckSlangSupport());

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::GetCubesTLAS(*m_device, m_command_buffer, *m_default_queue, cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, debug_buffer, 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
        [[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

        struct RayPayload {
            float3 hit;
        };

        [shader("raygeneration")]
        void rayGenShader()
        {
            printf("In Raygen 1");
            InterlockedAdd(debug_buffer[0], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,-1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 1
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(-1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("raygeneration")]
        void rayGenShader2()
        {
            printf("In Raygen 2");
            InterlockedAdd(debug_buffer[1], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will hit cube 1
            ray.Origin = float3(-50,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("miss")]
        void missShader(inout RayPayload payload)
        {
            printf("In Miss 1");
            InterlockedAdd(debug_buffer[2], 1);
            payload.hit = float3(0.1, 0.2, 0.3);
        }

        [shader("miss")]
        void missShader2(inout RayPayload payload)
        {
            printf("In Miss 2");
            InterlockedAdd(debug_buffer[3], 1);
            payload.hit = float3(42, 42, 42);
        }

        [shader("closesthit")]
        void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 1");
            InterlockedAdd(debug_buffer[4], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
                attr.barycentrics.y);
            payload.hit = barycentric_coords;

            RayPayload ray_payload = { float3(0) };
            RayDesc ray;

            ray.TMin = 0.01;
            ray.TMax = 1000.0;
            const uint32_t miss_shader_i = 1;

            // Supposed to hit cube 2, and invoke closestHitShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);

            // Supposed to miss, and call missShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,1,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);
        }

        [shader("closesthit")]
        void closestHitShader2(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 2");
            InterlockedAdd(debug_buffer[5], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
        attr.barycentrics.y); payload.hit = 999 * barycentric_coords;
        }
    )slang";

    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader");
    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader2");
    pipeline.AddSlangMissShader(slang_shader, "missShader");
    pipeline.AddSlangMissShader(slang_shader, "missShader2");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    vkt::Buffer sbt_ray_gen_1 = pipeline.GetTraceRaysSbtIndirectBuffer(0, 1, 1, 1);
    vkt::Buffer sbt_ray_gen_2 = pipeline.GetTraceRaysSbtIndirectBuffer(1, 1, 1, 1);

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                                  &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

        // Invoke ray gen shader 1
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer, sbt_ray_gen_1.Address());

        // Invoke ray gen shader 2
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer, sbt_ray_gen_2.Address());

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1");
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 3);
        m_default_queue->SubmitAndWait(m_command_buffer);
        m_errorMonitor->VerifyFound();
    }

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 + 2) * frames_count);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9559
TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRaysIndirectDeferredBuild) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysIndirect2KHR");

    RETURN_IF_SKIP(CheckSlangSupport());

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkWithPrintfBufferSize(1024 * 1024));
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::GetCubesTLAS(*m_device, m_command_buffer, *m_default_queue, cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, debug_buffer, 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
        [[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

        struct RayPayload {
            float3 hit;
        };

        [shader("raygeneration")]
        void rayGenShader()
        {
            printf("In Raygen 1");
            InterlockedAdd(debug_buffer[0], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,-1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 1
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will miss
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(-1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("raygeneration")]
        void rayGenShader2()
        {
            printf("In Raygen 2");
            InterlockedAdd(debug_buffer[1], 1);
            RayPayload ray_payload = { float3(0) };
            RayDesc ray;
            ray.TMin = 0.01;
            ray.TMax = 1000.0;

            // Will hit cube 1
            ray.Origin = float3(-50,0,0);
            ray.Direction = float3(1,0,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

            // Will hit cube 2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
        }

        [shader("miss")]
        void missShader(inout RayPayload payload)
        {
            printf("In Miss 1");
            InterlockedAdd(debug_buffer[2], 1);
            payload.hit = float3(0.1, 0.2, 0.3);
        }

        [shader("miss")]
        void missShader2(inout RayPayload payload)
        {
            printf("In Miss 2");
            InterlockedAdd(debug_buffer[3], 1);
            payload.hit = float3(42, 42, 42);
        }

        [shader("closesthit")]
        void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 1");
            InterlockedAdd(debug_buffer[4], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
                attr.barycentrics.y); payload.hit = barycentric_coords;

            RayPayload ray_payload = { float3(0) };
            RayDesc ray;

            ray.TMin = 0.01;
            ray.TMax = 1000.0;
            const uint32_t miss_shader_i = 1;

            // Supposed to hit cube 2, and invoke closestHitShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,0,1);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);

            // Supposed to miss, and call missShader2
            ray.Origin = float3(0,0,0);
            ray.Direction = float3(0,1,0);
            TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);
        }

        [shader("closesthit")]
        void closestHitShader2(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
        {
            printf("In Closest Hit 2");
            InterlockedAdd(debug_buffer[5], 1);
            const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
        attr.barycentrics.y); payload.hit = 999 * barycentric_coords;
        }
    )slang";

    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader");
    pipeline.AddSlangRayGenShader(slang_shader, "rayGenShader2");
    pipeline.AddSlangMissShader(slang_shader, "missShader");
    pipeline.AddSlangMissShader(slang_shader, "missShader2");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader");
    pipeline.AddSlangClosestHitShader(slang_shader, "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.DeferBuild();
    pipeline.Build();

    const uint32_t ray_gen_1_width = 2;
    const uint32_t ray_gen_1_height = 2;
    const uint32_t ray_gen_1_depth = 1;
    const uint32_t ray_gen_1_rays_count = ray_gen_1_width * ray_gen_1_height * ray_gen_1_depth;
    vkt::Buffer sbt_ray_gen_1 = pipeline.GetTraceRaysSbtIndirectBuffer(0, ray_gen_1_width, ray_gen_1_height, ray_gen_1_depth);
    vkt::Buffer sbt_ray_gen_2 = pipeline.GetTraceRaysSbtIndirectBuffer(1, 1, 1, 1);

    uint32_t frames_count = 1;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                                  &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

        // Invoke ray gen shader 1
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer, sbt_ray_gen_1.Address());

        // Invoke ray gen shader 2
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer, sbt_ray_gen_2.Address());

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1", ray_gen_1_rays_count);
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2 * ray_gen_1_rays_count + 0);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 1 * ray_gen_1_rays_count + 1);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 1 * ray_gen_1_rays_count + 1);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 1 * ray_gen_1_rays_count + 2);
        m_default_queue->SubmitAndWait(m_command_buffer);
    }
    m_errorMonitor->VerifyFound();

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * ray_gen_1_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 * ray_gen_1_rays_count + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 * ray_gen_1_rays_count + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 * ray_gen_1_rays_count + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 * ray_gen_1_rays_count + 2) * frames_count);
}
