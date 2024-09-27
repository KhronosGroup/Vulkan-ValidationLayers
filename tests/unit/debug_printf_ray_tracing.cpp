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

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/buffer_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeDebugPrintfRayTracing : public DebugPrintfTests {};

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

    m_command_buffer.begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.end();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "In Raygen");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfRayTracing, RaygenOneMissShaderOneClosestHitShader) {
    TEST_DESCRIPTION("Test debug printf in raygen shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitDebugPrintfFramework());

    RETURN_IF_SKIP(InitState());

    // #ARNO_TODO: For clarity, here geometry should be set explicitly, as of now the ray hitting or not
    // implicitly depends on the default triangle position.
    auto blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
        vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Triangle));

    // Build Bottom Level Acceleration Structure
    m_command_buffer.begin();
    blas->BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    m_command_buffer.begin();
    tlas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Buffer used to count invocations for the 3 shader types
    vkt::Buffer debug_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_command_buffer.begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.end();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

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
            debugPrintfEXT("In Raygen\n");
            atomicAdd(debug_buffer[0], 1);

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
            debugPrintfEXT("In Miss\n");
            atomicAdd(debug_buffer[1], 1);
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
            debugPrintfEXT("In Closest Hit\n");
            atomicAdd(debug_buffer[2], 1);
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    uint32_t frames_count = 250;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
        vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt,
                            &trace_rays_sbt.hit_sbt, &trace_rays_sbt.callable_sbt, 1, 1, 1);

        m_command_buffer.end();
        m_errorMonitor->SetDesiredInfo("In Raygen");
        m_errorMonitor->SetDesiredInfo("In Miss", 3);
        m_errorMonitor->SetDesiredInfo("In Closest Hit", 2);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }
    m_errorMonitor->VerifyFound();

    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.memory().map());
    ASSERT_EQ(debug_buffer_ptr[0], frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 3 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], 2 * frames_count);
    debug_buffer.memory().unmap();
}
