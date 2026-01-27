/* Copyright (c) 2024-2026 The Khronos Group Inc.
 * Copyright (c) 2024-2026 Valve Corporation
 * Copyright (c) 2024-2026 LunarG, Inc.
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

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeGpuAVRayHitObject : public GpuAVRayHitObjectTest {};

TEST_F(NegativeGpuAVRayHitObject, NegativeTmin) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with negative tmin");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float tmin; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                 vec3(0,0,0), params.tmin, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = -1.0f;  // negative tmin
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, TmaxLessThanTmin) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with tmax < tmin");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float tmin; float tmax; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                 vec3(0,0,0), params.tmin, vec3(0,0,1), params.tmax, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 10.0f;  // tmin
    ptr[1] = 5.0f;   // tmax < tmin
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11880");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, OriginNaN) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with NaN in origin");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float x; float y; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            float nan_val = fract(params.x / params.y);  // 1.0/0.0 then fract = NaN
            hitObjectTraceRayEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                 vec3(nan_val, 0, 0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 1.0f;
    ptr[1] = 0.0f;  // division by zero -> inf, fract(inf) = NaN
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11881");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, OriginNonFinite) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with infinity in origin");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float x; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            float inf_val = 1.0 / params.x;  // 1.0/0.0 = +infinity
            hitObjectTraceRayEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                 vec3(inf_val, 0, 0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 0.0f;  // causes division by zero -> +infinity
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11878");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, BothSkipFlags) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with both SkipTriangles and SkipAABBs flags");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_EXT_ray_flags_primitive_culling : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint flags; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, params.flags, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x100 | 0x200;  // SkipTrianglesKHR | SkipAABBsKHR
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11883");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, OpaqueFlags) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with conflicting opaque flags");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint flags; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, params.flags, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x1 | 0x2;  // OpaqueKHR | NoOpaqueKHR
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11885");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, SkipAndCullFlags) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with SkipTriangles and CullBackFacing flags");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_EXT_ray_flags_primitive_culling : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint flags; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, params.flags, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x100 | 0x10;  // SkipTrianglesKHR | CullBackFacingTrianglesKHR
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11884");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, SkipTrianglesWithPipelineSkipAABBs) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with SkipTriangles flag and pipeline SKIP_AABBS");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_EXT_ray_flags_primitive_culling : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint flags; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, params.flags, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x100;  // SkipTrianglesKHR
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11886");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, SkipAABBsWithPipelineSkipTriangles) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with SkipAABBs flag and pipeline SKIP_TRIANGLES");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_EXT_ray_flags_primitive_culling : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint flags; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, params.flags, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x200;  // SkipAABBsKHR
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11887");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, MotionTimeOutOfRange) {
    TEST_DESCRIPTION("hitObjectTraceRayMotionEXT with time outside [0.0, 1.0]");
    RETURN_IF_SKIP(InitHitObjectMotionTest());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_NV_ray_tracing_motion_blur : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float time; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayMotionEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                       vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, params.time, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 1.5f;  // time > 1.0, invalid
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11882");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, SBTIndexExceedsLimit) {
    TEST_DESCRIPTION("hitObjectSetShaderBindingTableRecordIndexEXT with index exceeding limit");
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitHitObjectTest());

    // Get the max SBT index from device properties
    VkPhysicalDeviceRayTracingInvocationReorderPropertiesEXT reorder_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&reorder_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props2);

    if (reorder_props.rayTracingInvocationReorderReorderingHint == VK_RAY_TRACING_INVOCATION_REORDER_MODE_NONE_EXT) {
        GTEST_SKIP() << "Ray tracing invocation reorder not supported";
    }

    if (reorder_props.maxShaderBindingTableRecordIndex == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "Cannot test exceeding limit when maxShaderBindingTableRecordIndex is UINT32_MAX";
    }

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_ray_query : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { uint sbt_index; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            rayQueryEXT rq;
            rayQueryInitializeEXT(rq, tlas, gl_RayFlagsOpaqueEXT, 0xff, vec3(0), 0.1, vec3(0,0,1), 100.0);
            hitObjectRecordFromQueryEXT(hObj, rq, 0, 0);
            hitObjectSetShaderBindingTableRecordIndexEXT(hObj, params.sbt_index);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = reorder_props.maxShaderBindingTableRecordIndex + 1;  // Exceed the limit
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-maxShaderBindingTableRecordIndex-11888");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, TraceReorderExecuteNegativeTmin) {
    TEST_DESCRIPTION("hitObjectTraceReorderExecuteEXT with negative tmin");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float tmin; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceReorderExecuteEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                            vec3(0,0,0), params.tmin, vec3(0,0,1), 100.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = -1.0f;  // negative tmin
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeGpuAVRayHitObject, TraceMotionReorderExecuteNegativeTmin) {
    TEST_DESCRIPTION("hitObjectTraceMotionReorderExecuteEXT with negative tmin");
    RETURN_IF_SKIP(InitHitObjectMotionTest());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_NV_ray_tracing_motion_blur : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float tmin; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceMotionReorderExecuteEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                                  vec3(0,0,0), params.tmin, vec3(0,0,1), 100.0, 0.5, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = -1.0f;  // negative tmin
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11879");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVRayHitObject, TraceMotionReorderExecuteTimeOutOfRange) {
    TEST_DESCRIPTION("hitObjectTraceMotionReorderExecuteEXT with time outside [0.0, 1.0]");
    RETURN_IF_SKIP(InitHitObjectMotionTest());

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        #extension GL_NV_ray_tracing_motion_blur : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform Params { float time; } params;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceMotionReorderExecuteEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                                  vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, params.time, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    vkt::Buffer uniform_buffer(*m_device, 4096, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 1.5f;  // time > 1.0, invalid
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpHitObjectTraceRayEXT-11882");
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}
