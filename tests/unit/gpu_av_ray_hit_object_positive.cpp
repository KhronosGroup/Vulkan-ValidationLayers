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

// Positive tests for VK_EXT_ray_tracing_invocation_reorder hit object operations

void GpuAVRayHitObjectTest::InitHitObjectTest(std::vector<VkLayerSettingEXT> layer_settings) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTracingInvocationReorder);
    AddRequiredFeature(vkt::Feature::rayTraversalPrimitiveCulling);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
}

void GpuAVRayHitObjectTest::InitHitObjectMotionTest(std::vector<VkLayerSettingEXT> layer_settings) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayTracingInvocationReorder);
    AddRequiredFeature(vkt::Feature::rayTracingMotionBlur);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVRayHitObject : public GpuAVRayHitObjectTest {};

TEST_F(PositiveGpuAVRayHitObject, TraceRayBasic) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with valid parameters");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceRayEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                 vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
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

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

// TODO: Test causes GPU fault - needs investigation
TEST_F(PositiveGpuAVRayHitObject, DynamicTminTmax) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with dynamically set valid tmin and tmax");
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
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 0.1f;    // tmin
    ptr[1] = 100.0f;  // tmax

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

TEST_F(PositiveGpuAVRayHitObject, DynamicRayFlags) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with dynamically set valid ray flags");
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
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // Test with valid flag combination: OpaqueKHR | CullBackFacingTrianglesKHR
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x1 | 0x10;  // OpaqueKHR | CullBackFacingTrianglesKHR

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

TEST_F(PositiveGpuAVRayHitObject, DynamicRayFlagsSkipTriangles) {
    TEST_DESCRIPTION("hitObjectTraceRayEXT with SkipTriangles flag (without conflicting pipeline flag)");
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
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // SkipTrianglesKHR alone is valid (no pipeline SKIP_AABBS flag)
    auto *ptr = static_cast<uint32_t *>(uniform_buffer.Memory().Map());
    ptr[0] = 0x100;  // SkipTrianglesKHR

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

TEST_F(PositiveGpuAVRayHitObject, TraceReorderExecuteBasic) {
    TEST_DESCRIPTION("hitObjectTraceReorderExecuteEXT with valid parameters");
    RETURN_IF_SKIP(InitHitObjectTest());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_shader_invocation_reorder : require
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 payload;
        layout(location = 0) hitObjectAttributeEXT vec2 attr;

        void main() {
            hitObjectEXT hObj;
            hitObjectTraceReorderExecuteEXT(hObj, tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0,
                                            vec3(0,0,0), 0.1, vec3(0,0,1), 100.0, 0);
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

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayHitObject, TraceRayMotionValidTime) {
    TEST_DESCRIPTION("hitObjectTraceRayMotionEXT with valid time in [0.0, 1.0]");
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
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // Test with time = 0.5 (valid)
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 0.5f;

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

    // Test edge case: time = 0.0
    ptr[0] = 0.0f;
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Test edge case: time = 1.0
    ptr[0] = 1.0f;
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveGpuAVRayHitObject, TraceMotionReorderExecuteValidTime) {
    TEST_DESCRIPTION("hitObjectTraceMotionReorderExecuteEXT with valid time in [0.0, 1.0]");
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
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer, 0, VK_WHOLE_SIZE);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    // Test with time = 0.5 (valid)
    auto *ptr = static_cast<float *>(uniform_buffer.Memory().Map());
    ptr[0] = 0.5f;

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

TEST_F(PositiveGpuAVRayHitObject, SBTIndexWithinLimit) {
    TEST_DESCRIPTION("hitObjectSetShaderBindingTableRecordIndexEXT with valid index within limit");
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
    // Use 0 which is always valid
    ptr[0] = 0;
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

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}
