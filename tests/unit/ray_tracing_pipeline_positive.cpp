/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"

class PositiveRayTracingPipeline : public RayTracingTest {};

TEST_F(PositiveRayTracingPipeline, ShaderGroupsKHR) {
    TEST_DESCRIPTION("Test that no warning is produced when a library is referenced in the raytracing shader groups.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader;
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout;
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    VkPipelineShaderStageCreateInfo stage_create_infos[2] = {};
    stage_create_infos[0] = vku::InitStructHelper();
    stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_infos[0].module = rgen_shader;
    stage_create_infos[0].pName = "main";

    stage_create_infos[1] = vku::InitStructHelper();
    stage_create_infos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_infos[1].module = chit_shader;
    stage_create_infos[1].pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_infos[2] = {};
    group_create_infos[0] = vku::InitStructHelper();
    group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_infos[0].generalShader = 0;
    group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    group_create_infos[1] = vku::InitStructHelper();
    group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_infos[1].generalShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].closestHitShader = 1;  // Index 1 corresponds to the closest hit shader from the library
    group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 2;
    pipeline_ci.pStages = stage_create_infos;
    pipeline_ci.groupCount = 2;
    pipeline_ci.pGroups = group_create_infos;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkResult err = vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_NE(pipeline, VK_NULL_HANDLE);

    vk::DestroyPipeline(*m_device, pipeline, nullptr);
    vk::DestroyPipeline(*m_device, library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, CacheControl) {
    TEST_DESCRIPTION("Create ray tracing pipeline with VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::pipelineCreationCacheControl);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader;
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout;
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);
    vk::DestroyPipeline(device(), library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, GetCaptureReplayShaderGroupHandlesKHR) {
    TEST_DESCRIPTION(
        "Regression test for issue 6282: make sure that when validating vkGetRayTracingCaptureReplayShaderGroupHandlesKHR on a "
        "pipeline created using pipeline libraries, the total shader group count is computed using info from the libraries.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::pipelineLibraryGroupHandles);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineShaderGroupHandleCaptureReplay);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe_lib(*this, m_device);
    rt_pipe_lib.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe_lib.InitLibraryInfo(sizeof(float), false);
    rt_pipe_lib.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.AddGlslMissShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.Build();

    vkt::rt::Pipeline rt_pipe(*this, m_device);
    rt_pipe.AddCreateInfoFlags(VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR);
    rt_pipe.InitLibraryInfo(sizeof(float), true);
    rt_pipe.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipe.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipe.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipe.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipe.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe.AddLibrary(rt_pipe_lib);
    rt_pipe.Build();

    // dataSize must be at least groupCount * VkPhysicalDeviceRayTracingPropertiesKHR::shaderGroupHandleCaptureReplaySize
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    const size_t buffer_size = (3 * ray_tracing_properties.shaderGroupHandleCaptureReplaySize);
    void* out_buffer = malloc(buffer_size);
    vk::GetRayTracingCaptureReplayShaderGroupHandlesKHR(*m_device, rt_pipe, 0, 3, buffer_size, out_buffer);
    free(out_buffer);
}

TEST_F(PositiveRayTracingPipeline, GetRayTracingShaderGroupStackSizeKHR) {
    TEST_DESCRIPTION("Iterating over a ray tracing pipeline's shader groups should take into account associated libraries");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::pipelineLibraryGroupHandles);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe_lib(*this, m_device);
    rt_pipe_lib.InitLibraryInfo(sizeof(float), false);
    rt_pipe_lib.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.AddGlslMissShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.Build();

    vkt::rt::Pipeline rt_pipe(*this, m_device);
    rt_pipe.InitLibraryInfo(sizeof(float), true);
    rt_pipe.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipe.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipe.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipe.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipe.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe.AddLibrary(rt_pipe_lib);
    rt_pipe.Build();

    const VkDeviceSize stack_size =
        vk::GetRayTracingShaderGroupStackSizeKHR(device(), rt_pipe, 1, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
    (void)stack_size;
}

TEST_F(PositiveRayTracingPipeline, GetRayTracingShaderGroupStackSizeUnusedGroupPipelineLibraries) {
    TEST_DESCRIPTION(
        "Call vkGetRayTracingShaderGroupStackSizeKHR on a shader group coming from a RT pipeline created from libraries");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::pipelineLibraryGroupHandles);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipe_lib(*this, m_device);
    rt_pipe_lib.InitLibraryInfo(sizeof(float), false);
    rt_pipe_lib.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.AddGlslMissShader(kRayTracingMinimalGlsl);
    rt_pipe_lib.Build();

    vkt::rt::Pipeline rt_pipe(*this, m_device);
    rt_pipe.InitLibraryInfo(sizeof(float), true);
    rt_pipe.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipe.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipe.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipe.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipe.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    rt_pipe.AddLibrary(rt_pipe_lib);
    rt_pipe.Build();

    vk::GetRayTracingShaderGroupStackSizeKHR(*m_device, rt_pipe, 1, VK_SHADER_GROUP_SHADER_GENERAL_KHR);
}

TEST_F(PositiveRayTracingPipeline, ClusterAccelerationStructureFeatureEnabled) {
    TEST_DESCRIPTION("Test that ray tracing pipeline creation succeeds when cluster acceleration structure feature is enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::clusterAccelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader;
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkPipelineLibraryCreateInfoKHR library_info = vku::InitStructHelper();
    library_info.libraryCount = 0;

    VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV cluster_info = vku::InitStructHelper();
    cluster_info.allowClusterAccelerationStructure = VK_TRUE;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper(&cluster_info);
    pipeline_ci.pLibraryInfo = &library_info;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = empty_pipeline_layout;

    // Should succeed because clusterAccelerationStructure feature is enabled
    VkResult result =
        vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(VK_SUCCESS, result);

    if (pipeline != VK_NULL_HANDLE) {
        vk::DestroyPipeline(*m_device, pipeline, nullptr);
    }
}
TEST_F(PositiveRayTracingPipeline, DescriptorBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_ALL,
                                                  nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ds_layout_ci.bindingCount = 1u;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));

    vkt::Buffer descriptor_buffer(*m_device, 4096, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    uint8_t *descriptor_data = reinterpret_cast<uint8_t *>(descriptor_buffer.Memory().Map());
    VkDeviceSize buffer_offset = ds_layout.GetDescriptorBufferBindingOffset(0);

    vkt::DescriptorGetInfo buffer_get_info(tlas.GetDstAS()->GetBufferDeviceAddress());
    vk::GetDescriptorEXT(*m_device, buffer_get_info, descriptor_buffer_properties.accelerationStructureDescriptorSize,
                         descriptor_data + buffer_offset);

    vkt::rt::Pipeline pipeline(*this, m_device);
    pipeline.AddCreateInfoFlags(VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);
    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);
    pipeline.SetPipelineSetLayouts(1, &ds_layout.handle());
    pipeline.Build();
    vkt::rt::TraceRaysSbt sbt = pipeline.GetTraceRaysSbt();

    VkDescriptorBufferBindingInfoEXT buffer_binding_info = vku::InitStructHelper();
    buffer_binding_info.address = descriptor_buffer.Address();
    buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &buffer_binding_info);
    uint32_t buffer_index = 0u;
    VkDeviceSize offset = 0u;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                                         &buffer_index, &offset);

    vk::CmdTraceRaysKHR(m_command_buffer, &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 32u, 32u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveRayTracingPipeline, DeferredOp) {
    TEST_DESCRIPTION(
        "Test that objects created with deferred operations are recorded once the operation has successfully completed.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkGetDeferredOperationResultKHR not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    const char* chit_src = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        layout(location = 0) rayPayloadEXT uvec4 hitValue;
        layout(r32ui, set = 0, binding = 0) uniform uimage2D result;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

        void main()
        {
          float tmin     = 0.0;
          float tmax     = 1.0;
          vec3  origin   = vec3(float(gl_LaunchIDEXT.x) + 0.5f, float(gl_LaunchIDEXT.y) + 0.5f, float(gl_LaunchIDEXT.z + 0.5f));
          vec3  direct   = vec3(0.0, 0.0, -1.0);
          hitValue       = uvec4(1,0,0,0);
          traceRayEXT(topLevelAS, 0, 0xFF, 0, 0, 0, origin, tmin, direct, tmax, 0);
          imageStore(result, ivec2(gl_LaunchIDEXT.xy), hitValue);
        }
    )glsl";

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, chit_src, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::DescriptorSetLayout ds_layout(
        *m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr},
                    {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader;
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout;
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader;
    stage_create_info.pName = "main";

    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkDeferredOperationKHR deferredOperation = VK_NULL_HANDLE;
    vk::CreateDeferredOperationKHR(*m_device, 0, &deferredOperation);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result =
        vk::CreateRayTracingPipelinesKHR(*m_device, deferredOperation, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);

    m_command_buffer.Begin();
    if (result == VK_OPERATION_DEFERRED_KHR) {
        result = vk::DeferredOperationJoinKHR(*m_device, deferredOperation);
        ASSERT_EQ(result, VK_SUCCESS);

        // VVL implicity calls vkGetDeferredOperationResultKHR in vkDeferredOperationJoinKHR,
        // because some applications never call this function.
        // If successful (here always yes), pipeline will be state tracked, so the following error
        // won't be emitted.
        // m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipeline-parameter");
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
        // m_errorMonitor->VerifyFound();
    }

    result = vk::GetDeferredOperationResultKHR(*m_device, deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_command_buffer.End();

    vk::DestroyPipeline(*m_device, pipeline, nullptr);
    vk::DestroyDeferredOperationKHR(*m_device, deferredOperation, nullptr);
    vk::DestroyPipeline(*m_device, library, nullptr);
}

TEST_F(PositiveRayTracingPipeline, DeferredOpNoGetResult) {
    TEST_DESCRIPTION(
        "Test that objects created with deferred operations are recorded once the operation has successfully completed. Don't call "
        "vkGetDeferredOperationResultKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkGetDeferredOperationResultKHR not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState());

    const char* chit_src = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        layout(location = 0) rayPayloadEXT uvec4 hitValue;
        layout(r32ui, set = 0, binding = 0) uniform uimage2D result;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

        void main()
        {
          float tmin     = 0.0;
          float tmax     = 1.0;
          vec3  origin   = vec3(float(gl_LaunchIDEXT.x) + 0.5f, float(gl_LaunchIDEXT.y) + 0.5f, float(gl_LaunchIDEXT.z + 0.5f));
          vec3  direct   = vec3(0.0, 0.0, -1.0);
          hitValue       = uvec4(1,0,0,0);
          traceRayEXT(topLevelAS, 0, 0xFF, 0, 0, 0, origin, tmin, direct, tmax, 0);
          imageStore(result, ivec2(gl_LaunchIDEXT.xy), hitValue);
        }
    )glsl";

    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, chit_src, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::DescriptorSetLayout ds_layout(
        *m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr},
                    {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkPipelineShaderStageCreateInfo stage_create_info = vku::InitStructHelper();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader;
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = vku::InitStructHelper();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = vku::InitStructHelper();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = vku::InitStructHelper();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout;
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = vku::InitStructHelper();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader;
    stage_create_info.pName = "main";

    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = vku::InitStructHelper();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkDeferredOperationKHR deferredOperation = VK_NULL_HANDLE;
    vk::CreateDeferredOperationKHR(*m_device, 0, &deferredOperation);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result =
        vk::CreateRayTracingPipelinesKHR(*m_device, deferredOperation, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);

    m_command_buffer.Begin();
    if (result == VK_OPERATION_DEFERRED_KHR) {
        result = vk::DeferredOperationJoinKHR(*m_device, deferredOperation);
        ASSERT_EQ(result, VK_SUCCESS);
    }

    // VVL implicity calls vkGetDeferredOperationResultKHR in vkDeferredOperationJoinKHR,
    // because some applications never call this function.
    // If successful (here always yes), pipeline will be state tracked, so the following error
    // won't be emitted.
    // m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipeline-parameter");
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    // m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    vk::DestroyPipeline(*m_device, pipeline, nullptr);
    vk::DestroyDeferredOperationKHR(*m_device, deferredOperation, nullptr);
    vk::DestroyPipeline(*m_device, library, nullptr);
}
