/*
 * Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/ray_tracing_nv.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeGpuAssistedRayTracing, ArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders using gpu assited validation.");
    OOBRayTracingShadersTestBody(true);
}

TEST_F(NegativeGpuAssistedRayTracingNV, BuildAccelerationStructureValidationInvalidHandle) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with an invalid handle for a bottom level acceleration structure.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false, nullptr, &validation_features))

    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::CommandPool command_pool(*m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        1234567890,  // invalid
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    vkt::Buffer instance_buffer(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    const vkt::Buffer top_level_as_scratch = top_level_as.create_scratch_buffer(*m_device);

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vk::CmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAssistedRayTracingNV, BuildAccelerationStructureValidationBottomLevelNotYetBuilt) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with a handle for a bottom level acceleration structure that has not yet been built.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false, nullptr, &validation_features))

    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::CommandPool command_pool(*m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    vkt::AccelerationStructure bot_level_as_never_built(*m_device, bot_level_as_create_info);

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        bot_level_as_never_built.opaque_handle(),
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    vkt::Buffer instance_buffer(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    const vkt::Buffer top_level_as_scratch = top_level_as.create_scratch_buffer(*m_device);

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vk::CmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAssistedRayTracingNV, BuildAccelerationStructureValidationBottomLevelDestroyed) {
    TEST_DESCRIPTION(
        "Acceleration structure gpu validation should report an invalid handle when trying to build a top level "
        "acceleration structure with a handle for a destroyed bottom level acceleration structure.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false, nullptr, &validation_features))

    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::CommandPool command_pool(*m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    uint64_t destroyed_bot_level_as_handle = 0;
    {
        vkt::AccelerationStructure destroyed_bot_level_as(*m_device, bot_level_as_create_info);

        destroyed_bot_level_as_handle = destroyed_bot_level_as.opaque_handle();

        const vkt::Buffer bot_level_as_scratch = destroyed_bot_level_as.create_scratch_buffer(*m_device);

        vkt::CommandBuffer command_buffer(m_device, &command_pool);
        command_buffer.begin();
        vk::CmdBuildAccelerationStructureNV(command_buffer.handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                            destroyed_bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
        command_buffer.end();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer.handle();
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);

        // vk::DestroyAccelerationStructureNV called on destroyed_bot_level_as during destruction.
    }

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        destroyed_bot_level_as_handle,
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    vkt::Buffer instance_buffer(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    const vkt::Buffer top_level_as_scratch = top_level_as.create_scratch_buffer(*m_device);

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vk::CmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAssistedRayTracingNV, BuildAccelerationStructureValidationRestoresState) {
    TEST_DESCRIPTION("Validate that acceleration structure gpu validation correctly restores compute state.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false, nullptr, &validation_features))

    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::CommandPool command_pool(*m_device, 0, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    VkGeometryInstanceNV instance = {
        {
            // clang-format off
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            // clang-format on
        },
        0,
        0xFF,
        0,
        VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
        1234567,
    };

    VkDeviceSize instance_buffer_size = sizeof(VkGeometryInstanceNV);
    vkt::Buffer instance_buffer(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)&instance, static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    const vkt::Buffer top_level_as_scratch = top_level_as.create_scratch_buffer(*m_device);

    struct ComputeOutput {
        uint32_t push_constant_value;
        uint32_t push_descriptor_value;
        uint32_t normal_descriptor_value;
    };

    vkt::Buffer push_descriptor_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer normal_descriptor_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceSize output_descriptor_buffer_size = static_cast<VkDeviceSize>(sizeof(ComputeOutput));
    vkt::Buffer output_descriptor_buffer(*m_device, output_descriptor_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    const char *cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        layout(push_constant) uniform PushConstants { uint value; } push_constant;
        layout(set = 0, binding = 0, std430) buffer PushDescriptorBuffer { uint value; } push_descriptor;
        layout(set = 1, binding = 0, std430) buffer NormalDescriptorBuffer { uint value; } normal_descriptor;

        layout(set = 2, binding = 0, std430) buffer ComputeOutputBuffer {
            uint push_constant_value;
            uint push_descriptor_value;
            uint normal_descriptor_value;
        } compute_output;

        void main() {
            compute_output.push_constant_value = push_constant.value;
            compute_output.push_descriptor_value = push_descriptor.value;
            compute_output.normal_descriptor_value = normal_descriptor.value;
        }
    )glsl";
    VkShaderObj cs(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet push_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                            },
                                            VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              });
    OneOffDescriptorSet output_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              });

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.size = 4;
    push_constant_range.offset = 0;

    const vkt::PipelineLayout compute_pipeline_layout(*m_device,
                                                      {
                                                          &push_descriptor_set.layout_,
                                                          &normal_descriptor_set.layout_,
                                                          &output_descriptor_set.layout_,
                                                      },
                                                      {push_constant_range});

    VkComputePipelineCreateInfo compute_pipeline_ci = vku::InitStructHelper();
    compute_pipeline_ci.layout = compute_pipeline_layout.handle();
    compute_pipeline_ci.stage = cs.GetStageCreateInfo();

    VkPipeline compute_pipeline;
    ASSERT_EQ(VK_SUCCESS,
        vk::CreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 1, &compute_pipeline_ci, nullptr, &compute_pipeline));

    normal_descriptor_set.WriteDescriptorBufferInfo(0, normal_descriptor_buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    normal_descriptor_set.UpdateDescriptorSets();

    output_descriptor_set.WriteDescriptorBufferInfo(0, output_descriptor_buffer.handle(), 0, output_descriptor_buffer_size,
                                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    output_descriptor_set.UpdateDescriptorSets();

    // Set input data
    const uint32_t push_constant_value = 1234567890;
    const uint32_t push_descriptor_value = 98765432;
    const uint32_t normal_descriptor_value = 1111111;

    uint32_t *mapped_push_descriptor_buffer_data = (uint32_t *)push_descriptor_buffer.memory().map();
    *mapped_push_descriptor_buffer_data = push_descriptor_value;
    push_descriptor_buffer.memory().unmap();

    uint32_t *mapped_normal_descriptor_buffer_data = (uint32_t *)normal_descriptor_buffer.memory().map();
    *mapped_normal_descriptor_buffer_data = normal_descriptor_value;
    normal_descriptor_buffer.memory().unmap();

    ComputeOutput *mapped_output_buffer_data = (ComputeOutput *)output_descriptor_buffer.memory().map();
    mapped_output_buffer_data->push_constant_value = 0;
    mapped_output_buffer_data->push_descriptor_value = 0;
    mapped_output_buffer_data->normal_descriptor_value = 0;
    output_descriptor_buffer.memory().unmap();

    VkDescriptorBufferInfo push_descriptor_buffer_info = {};
    push_descriptor_buffer_info.buffer = push_descriptor_buffer.handle();
    push_descriptor_buffer_info.offset = 0;
    push_descriptor_buffer_info.range = 4;
    VkWriteDescriptorSet push_descriptor_set_write = vku::InitStructHelper();
    push_descriptor_set_write.descriptorCount = 1;
    push_descriptor_set_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    push_descriptor_set_write.dstBinding = 0;
    push_descriptor_set_write.pBufferInfo = &push_descriptor_buffer_info;

    vkt::CommandBuffer command_buffer(m_device, &command_pool);
    command_buffer.begin();
    vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
    vk::CmdPushConstants(command_buffer.handle(), compute_pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, 4,
                         &push_constant_value);
    vk::CmdPushDescriptorSetKHR(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 0, 1,
                                &push_descriptor_set_write);
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 1, 1,
                              &normal_descriptor_set.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout.handle(), 2, 1,
                              &output_descriptor_set.set_, 0, nullptr);

    vk::CmdBuildAccelerationStructureNV(command_buffer.handle(), &top_level_as_create_info.info, instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, top_level_as_scratch.handle(), 0);

    vk::CmdDispatch(command_buffer.handle(), 1, 1, 1);
    command_buffer.end();

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "Attempted to build top level acceleration structure using invalid bottom level acceleration structure handle");

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    mapped_output_buffer_data = (ComputeOutput *)output_descriptor_buffer.memory().map();
    EXPECT_EQ(mapped_output_buffer_data->push_constant_value, push_constant_value);
    EXPECT_EQ(mapped_output_buffer_data->push_descriptor_value, push_descriptor_value);
    EXPECT_EQ(mapped_output_buffer_data->normal_descriptor_value, normal_descriptor_value);
    output_descriptor_buffer.memory().unmap();

    // Clean up
    vk::DestroyPipeline(m_device->device(), compute_pipeline, nullptr);
}
