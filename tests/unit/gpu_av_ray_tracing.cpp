/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/shader_helper.h"

class NegativeGpuAVRayTracing : public GpuAVRayTracingTest {};

TEST_F(NegativeGpuAVRayTracing, DISABLED_CmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Invalid parameters used in vkCmdTraceRaysIndirectKHR");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    // Create ray tracing pipeline
    std::vector<VkDescriptorSetLayoutBinding> bindings(1);
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    const vkt::DescriptorSetLayout desc_set_layout(*m_device, bindings);
    OneOffDescriptorSet desc_set(m_device, bindings);

    const vkt::PipelineLayout rt_pipeline_layout(*m_device, {&desc_set_layout});

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
    shader_stages[0] = vku::InitStructHelper();
    shader_stages[0].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    shader_stages[0].module = chit_shader.handle();
    shader_stages[0].pName = "main";

    shader_stages[1] = vku::InitStructHelper();
    shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    shader_stages[1].module = rgen_shader.handle();
    shader_stages[1].pName = "main";

    std::array<VkRayTracingShaderGroupCreateInfoKHR, 1> shader_groups;
    shader_groups[0] = vku::InitStructHelper();
    shader_groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shader_groups[0].generalShader = 1;
    shader_groups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_ci = vku::InitStructHelper();
    raytracing_pipeline_ci.flags = 0;
    raytracing_pipeline_ci.stageCount = static_cast<uint32_t>(shader_stages.size());
    raytracing_pipeline_ci.pStages = shader_stages.data();
    raytracing_pipeline_ci.pGroups = shader_groups.data();
    raytracing_pipeline_ci.groupCount = shader_groups.size();
    raytracing_pipeline_ci.layout = rt_pipeline_layout;

    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;

    const VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                             &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
    ASSERT_EQ(VK_SUCCESS, result);

    // Create dummy shader binding table (SBT)
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    vkt::Buffer sbt_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), sbt_buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), sbt_buffer.handle(), mem.handle(), 0);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress sbt_address = sbt_buffer.address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = sbt_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    // Create and fill buffers storing indirect data (ray query dimensions)
    vkt::Buffer ray_query_dimensions_buffer_1(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    VkTraceRaysIndirectCommandKHR ray_query_dimensions{vvl::kU32Max, 1, 1};

    uint8_t *ray_query_dimensions_buffer_1_ptr = (uint8_t *)ray_query_dimensions_buffer_1.memory().map();
    std::memcpy(ray_query_dimensions_buffer_1_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_1.memory().unmap();

    ray_query_dimensions = {1, vvl::kU32Max, 1};

    vkt::Buffer ray_query_dimensions_buffer_2(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    uint8_t *ray_query_dimensions_buffer_2_ptr = (uint8_t *)ray_query_dimensions_buffer_2.memory().map();
    std::memcpy(ray_query_dimensions_buffer_2_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_2.memory().unmap();

    ray_query_dimensions = {1, 1, vvl::kU32Max};

    vkt::Buffer ray_query_dimensions_buffer_3(
        *m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);

    uint8_t *ray_query_dimensions_buffer_3_ptr = (uint8_t *)ray_query_dimensions_buffer_3.memory().map();
    std::memcpy(ray_query_dimensions_buffer_3_ptr, &ray_query_dimensions, sizeof(ray_query_dimensions));
    ray_query_dimensions_buffer_3.memory().unmap();

    // Trace rays
    m_command_buffer.begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline_layout.handle(), 0, 1,
                              &desc_set.set_, 0, nullptr);

    vk::CmdTraceRaysIndirectKHR(m_command_buffer.handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_1.address());

    vk::CmdTraceRaysIndirectKHR(m_command_buffer.handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_2.address());

    vk::CmdTraceRaysIndirectKHR(m_command_buffer.handle(), &stridebufregion, &stridebufregion, &stridebufregion, &stridebufregion,
                                ray_query_dimensions_buffer_3.address());

    m_command_buffer.end();

    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[0]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[0]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-width-03638");
    }
    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[1]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[1]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-height-03639");
    }
    if (uint64_t(physDevProps().limits.maxComputeWorkGroupCount[2]) * uint64_t(physDevProps().limits.maxComputeWorkGroupSize[2]) <
        uint64_t(vvl::kU32Max)) {
        m_errorMonitor->SetDesiredError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640");
    }
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8545
TEST_F(NegativeGpuAVRayTracing, DISABLED_BasicTraceRaysDeferredBuild) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders, and deferred build) and acceleration "
        "structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(&validation_features));
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char *ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) readonly buffer RayTracingParams {
            vec4 nothing;
            float Tmin;
            float Tmax;
        };

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) uniform uniform_buffer {
            RayTracingParams rt_params;
        };

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), rt_params.Tmin, vec3(0,0,1), rt_params.Tmax, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char *miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char *closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pipeline.CreateDescriptorSet();

    // Create TLAS
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());

    // Create uniform_buffer
    vkt::Buffer rt_params_buffer(*m_device, 4 * sizeof(float), 0, vkt::device_address);  // missing space for Tmin and Tmax
    vkt::Buffer uniform_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(uniform_buffer.memory().map());
    data[0] = rt_params_buffer.address();
    uniform_buffer.memory().unmap();
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, uniform_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Add one to use the descriptor slot GPU-AV tried to reserve
    const uint32_t max_bound_desc_sets = m_device->phy().limits_.maxBoundDescriptorSets + 1;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> desc_set_layouts(max_bound_desc_sets);
        for (uint32_t i = 0; i < max_bound_desc_sets; i++) {
            desc_set_layouts[i] = &pipeline.GetDescriptorSet().layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, desc_set_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Then use the maximum allowed number of sets
    std::vector<const vkt::DescriptorSetLayout *> des_set_layouts(max_bound_desc_sets - 1);
    for (uint32_t i = 0; i < max_bound_desc_sets - 1; i++) {
        des_set_layouts[i] = &pipeline.GetDescriptorSet().layout_;
    }
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();

    pipeline.GetPipelineLayout().init(*m_device, pipe_layout_ci, des_set_layouts);

    // Deferred pipeline build
    RETURN_IF_SKIP(pipeline.DeferBuild());
    RETURN_IF_SKIP(pipeline.Build());

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.end();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}
