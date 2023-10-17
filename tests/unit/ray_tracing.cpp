/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "../layers/utils/vk_layer_utils.h"

void RayTracingTest::OOBRayTracingShadersTestBody(bool gpu_assisted) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    VkValidationFeatureEnableEXT validation_feature_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeatureDisableEXT validation_feature_disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT validation_features = {};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures = validation_feature_enables;
    validation_features.disabledValidationFeatureCount = 4;
    validation_features.pDisabledValidationFeatures = validation_feature_disables;
    RETURN_IF_SKIP(InitFramework(gpu_assisted ? &validation_features : nullptr));
    bool descriptor_indexing = IsExtensionsEnabled(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    if (gpu_assisted && IsPlatformMockICD()) {
        GTEST_SKIP() << "GPU-AV can't run on MockICD";
    }

    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper();
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    if (descriptor_indexing) {
        features2 = GetPhysicalDeviceFeatures2(indexing_features);
        if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingPartiallyBound ||
            !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
            !indexing_features.descriptorBindingVariableDescriptorCount) {
            printf("Not all descriptor indexing features supported, skipping descriptor indexing tests\n");
            descriptor_indexing = false;
        }
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);
    if (ray_tracing_properties.maxTriangleCount == 0) {
        GTEST_SKIP() << "Did not find required ray tracing properties";
    }

    VkQueue ray_tracing_queue = m_default_queue;
    uint32_t ray_tracing_queue_family_index = 0;

    // If supported, run on the compute only queue.
    const std::optional<uint32_t> compute_only_queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (compute_only_queue_family_index) {
        const auto &compute_only_queues = m_device->queue_family_queues(compute_only_queue_family_index.value());
        if (!compute_only_queues.empty()) {
            ray_tracing_queue = compute_only_queues[0]->handle();
            ray_tracing_queue_family_index = compute_only_queue_family_index.value();
        }
    }

    vkt::CommandPool ray_tracing_command_pool(*m_device, ray_tracing_queue_family_index,
                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer ray_tracing_command_buffer(m_device, &ray_tracing_command_pool);

    constexpr std::array<VkAabbPositionsKHR, 1> aabbs = {{{-1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f}}};

    struct VkGeometryInstanceNV {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    const VkDeviceSize aabb_buffer_size = sizeof(VkAabbPositionsKHR) * aabbs.size();
    vkt::Buffer aabb_buffer;
    aabb_buffer.init(*m_device, aabb_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr,
                     {ray_tracing_queue_family_index});

    uint8_t *mapped_aabb_buffer_data = (uint8_t *)aabb_buffer.memory().map();
    std::memcpy(mapped_aabb_buffer_data, (uint8_t *)aabbs.data(), static_cast<std::size_t>(aabb_buffer_size));
    aabb_buffer.memory().unmap();

    VkGeometryNV geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    geometry.geometry.triangles = {};
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry.geometry.aabbs = {};
    geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    geometry.geometry.aabbs.aabbData = aabb_buffer.handle();
    geometry.geometry.aabbs.numAABBs = static_cast<uint32_t>(aabbs.size());
    geometry.geometry.aabbs.offset = 0;
    geometry.geometry.aabbs.stride = static_cast<VkDeviceSize>(sizeof(VkAabbPositionsKHR));
    geometry.flags = 0;

    VkAccelerationStructureInfoNV bot_level_as_info = vku::InitStructHelper();
    bot_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_info.instanceCount = 0;
    bot_level_as_info.geometryCount = 1;
    bot_level_as_info.pGeometries = &geometry;

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = bot_level_as_info;

    vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

    const std::array instances = {
        VkGeometryInstanceNV{
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
            bot_level_as.opaque_handle(),
        },
    };

    VkDeviceSize instance_buffer_size = sizeof(instances[0]) * instances.size();
    vkt::Buffer instance_buffer;
    instance_buffer.init(*m_device, instance_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr,
                         {ray_tracing_queue_family_index});

    uint8_t *mapped_instance_buffer_data = (uint8_t *)instance_buffer.memory().map();
    std::memcpy(mapped_instance_buffer_data, (uint8_t *)instances.data(), static_cast<std::size_t>(instance_buffer_size));
    instance_buffer.memory().unmap();

    VkAccelerationStructureInfoNV top_level_as_info = vku::InitStructHelper();
    top_level_as_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_info.instanceCount = 1;
    top_level_as_info.geometryCount = 0;

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = top_level_as_info;

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    VkDeviceSize scratch_buffer_size = std::max(bot_level_as.build_scratch_memory_requirements().memoryRequirements.size,
                                                top_level_as.build_scratch_memory_requirements().memoryRequirements.size);
    vkt::Buffer scratch_buffer(*m_device, scratch_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    ray_tracing_command_buffer.begin();

    // Build bot level acceleration structure
    vk::CmdBuildAccelerationStructureNV(ray_tracing_command_buffer.handle(), &bot_level_as.info(), VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, scratch_buffer.handle(), 0);

    // Barrier to prevent using scratch buffer for top level build before bottom level build finishes
    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    vk::CmdPipelineBarrier(ray_tracing_command_buffer.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
                           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);

    // Build top level acceleration structure
    vk::CmdBuildAccelerationStructureNV(ray_tracing_command_buffer.handle(), &top_level_as.info(), instance_buffer.handle(), 0,
                                        VK_FALSE, top_level_as.handle(), VK_NULL_HANDLE, scratch_buffer.handle(), 0);

    ray_tracing_command_buffer.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &ray_tracing_command_buffer.handle();
    vk::QueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(ray_tracing_queue);

    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDeviceSize storage_buffer_size = 1024;
    vkt::Buffer storage_buffer;
    storage_buffer.init(*m_device, storage_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr,
                        {ray_tracing_queue_family_index});

    VkDeviceSize shader_binding_table_buffer_size = ray_tracing_properties.shaderGroupBaseAlignment * 4ull;
    vkt::Buffer shader_binding_table_buffer;
    shader_binding_table_buffer.init(*m_device, shader_binding_table_buffer_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr,
                                     {ray_tracing_queue_family_index});

    // Setup descriptors!
    const VkShaderStageFlags kAllRayTracingStages = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV |
                                                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV |
                                                    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV;

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    VkDescriptorPoolCreateFlags pool_create_flags = 0;
    VkDescriptorSetLayoutCreateFlags layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[3] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    if (descriptor_indexing) {
        ds_binding_flags[0] = 0;
        ds_binding_flags[1] = 0;
        ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

        layout_createinfo_binding_flags[0] = vku::InitStructHelper();
        layout_createinfo_binding_flags[0].bindingCount = 3;
        layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
        layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        layout_pnext = layout_createinfo_binding_flags;
    }

    // Prepare descriptors
    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, kAllRayTracingStages, nullptr},
                           },
                           layout_create_flags, layout_pnext, pool_create_flags);

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count =
        vku::InitStructHelper();
    uint32_t desc_counts;
    if (descriptor_indexing) {
        layout_create_flags = 0;
        pool_create_flags = 0;
        ds_binding_flags[2] =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
        variable_count.descriptorSetCount = 1;
        variable_count.pDescriptorCounts = &desc_counts;
        allocate_pnext = &variable_count;
    }

    OneOffDescriptorSet ds_variable(m_device,
                                    {
                                        {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, kAllRayTracingStages, nullptr},
                                        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, kAllRayTracingStages, nullptr},
                                        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, kAllRayTracingStages, nullptr},
                                    },
                                    layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    VkAccelerationStructureNV top_level_as_handle = top_level_as.handle();
    VkWriteDescriptorSetAccelerationStructureNV write_descript_set_as =
        vku::InitStructHelper();
    write_descript_set_as.accelerationStructureCount = 1;
    write_descript_set_as.pAccelerationStructures = &top_level_as_handle;

    VkDescriptorBufferInfo descriptor_buffer_info = {};
    descriptor_buffer_info.buffer = storage_buffer.handle();
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = storage_buffer_size;

    VkDescriptorImageInfo descriptor_image_infos[6] = {};
    for (int i = 0; i < 6; i++) {
        descriptor_image_infos[i] = {sampler.handle(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[3] = {};
    descriptor_writes[0] = vku::InitStructHelper(&write_descript_set_as);
    descriptor_writes[0].dstSet = ds.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = ds.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &descriptor_buffer_info;

    descriptor_writes[2] = vku::InitStructHelper();
    descriptor_writes[2].dstSet = ds.set_;
    descriptor_writes[2].dstBinding = 2;
    if (descriptor_indexing) {
        descriptor_writes[2].descriptorCount = 5;  // Intentionally don't write index 5
    } else {
        descriptor_writes[2].descriptorCount = 6;
    }
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[2].pImageInfo = descriptor_image_infos;
    vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    if (descriptor_indexing) {
        descriptor_writes[0].dstSet = ds_variable.set_;
        descriptor_writes[1].dstSet = ds_variable.set_;
        descriptor_writes[2].dstSet = ds_variable.set_;
        vk::UpdateDescriptorSets(m_device->device(), 3, descriptor_writes, 0, NULL);
    }

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});
    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&ds_variable.layout_});

    const auto SetImagesArrayLength = [](const std::string &shader_template, const std::string &length_str) {
        const std::string to_replace = "IMAGES_ARRAY_LENGTH";

        std::string result = shader_template;
        auto position = result.find(to_replace);
        assert(position != std::string::npos);
        result.replace(position, to_replace.length(), length_str);
        return result;
    };

    const std::string rgen_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 0) uniform accelerationStructureNV topLevelAS;
        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadNV vec3 payload;
        layout(location = 3) callableDataNV vec3 callableData;

        void main() {
            sbo.rgen_ran = 1;

	        executeCallableNV(0, 3);
	        sbo.result1 = callableData.x;

	        vec3 origin = vec3(0.0f, 0.0f, -2.0f);
	        vec3 direction = vec3(0.0f, 0.0f, 1.0f);

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, direction, 10000.0, 0);
	        sbo.result2 = payload.x;

	        traceNV(topLevelAS, gl_RayFlagsNoneNV, 0xFF, 0, 1, 0, origin, 0.001, -direction, 10000.0, 0);
	        sbo.result3 = payload.x;

            if (sbo.rgen_index > 0) {
                // OOB here:
                sbo.result3 = texelFetch(textures[sbo.rgen_index], ivec2(0, 0), 0).x;
            }
        }
        )";

    const std::string rgen_source = SetImagesArrayLength(rgen_source_template, "6");
    const std::string rgen_source_runtime = SetImagesArrayLength(rgen_source_template, "");

    const std::string ahit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
	        sbo.ahit_ran = 2;

	        payload = vec3(0.1234f);

            if (sbo.ahit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.ahit_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string ahit_source = SetImagesArrayLength(ahit_source_template, "6");
    const std::string ahit_source_runtime = SetImagesArrayLength(ahit_source_template, "");

    const std::string chit_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        hitAttributeNV vec3 attribs;

        void main() {
            sbo.chit_ran = 3;

            payload = attribs;
            if (sbo.chit_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.chit_index], ivec2(0, 0), 0).x;
            }
        }
        )";
    const std::string chit_source = SetImagesArrayLength(chit_source_template, "6");
    const std::string chit_source_runtime = SetImagesArrayLength(chit_source_template, "");

    const std::string miss_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer RayTracingSbo {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 0) rayPayloadInNV vec3 payload;

        void main() {
            sbo.miss_ran = 4;

            payload = vec3(1.0, 0.0, 0.0);

            if (sbo.miss_index > 0) {
                // OOB here:
                payload.x = texelFetch(textures[sbo.miss_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string miss_source = SetImagesArrayLength(miss_source_template, "6");
    const std::string miss_source_runtime = SetImagesArrayLength(miss_source_template, "");

    const std::string intr_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        hitAttributeNV vec3 hitValue;

        void main() {
	        sbo.intr_ran = 5;

	        hitValue = vec3(0.0f, 0.5f, 0.0f);

	        reportIntersectionNV(1.0f, 0);

            if (sbo.intr_index > 0) {
                // OOB here:
                hitValue.x = texelFetch(textures[sbo.intr_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string intr_source = SetImagesArrayLength(intr_source_template, "6");
    const std::string intr_source_runtime = SetImagesArrayLength(intr_source_template, "");

    const std::string call_source_template = R"(#version 460
        #extension GL_EXT_nonuniform_qualifier : require
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_NV_ray_tracing : require

        layout(set = 0, binding = 1, std430) buffer StorageBuffer {
	        uint rgen_index;
	        uint ahit_index;
	        uint chit_index;
	        uint miss_index;
	        uint intr_index;
	        uint call_index;

	        uint rgen_ran;
	        uint ahit_ran;
	        uint chit_ran;
	        uint miss_ran;
	        uint intr_ran;
	        uint call_ran;

	        float result1;
	        float result2;
	        float result3;
        } sbo;
        layout(set = 0, binding = 2) uniform texture2D textures[IMAGES_ARRAY_LENGTH];

        layout(location = 3) callableDataInNV vec3 callableData;

        void main() {
	        sbo.call_ran = 6;

	        callableData = vec3(0.1234f);

            if (sbo.call_index > 0) {
                // OOB here:
                callableData.x = texelFetch(textures[sbo.call_index], ivec2(0, 0), 0).x;
            }
        }
    )";
    const std::string call_source = SetImagesArrayLength(call_source_template, "6");
    const std::string call_source_runtime = SetImagesArrayLength(call_source_template, "");

    struct TestCase {
        const std::string &rgen_shader_source;
        const std::string &ahit_shader_source;
        const std::string &chit_shader_source;
        const std::string &miss_shader_source;
        const std::string &intr_shader_source;
        const std::string &call_shader_source;
        bool variable_length;
        uint32_t rgen_index;
        uint32_t ahit_index;
        uint32_t chit_index;
        uint32_t miss_index;
        uint32_t intr_index;
        uint32_t call_index;
        const char *expected_error;
    };

    std::vector<TestCase> tests;
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 25, 0, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 25, 0, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 25, 0, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 25, 0, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 25, 0,
                     "Index of 25 used to index descriptor array of length 6."});
    tests.push_back({rgen_source, ahit_source, chit_source, miss_source, intr_source, call_source, false, 0, 0, 0, 0, 0, 25,
                     "Index of 25 used to index descriptor array of length 6."});

    if (descriptor_indexing) {
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 25, 0, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 25, 0, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 25, 0, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 25, 0, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 25, 0, "Index of 25 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 25, "Index of 25 used to index descriptor array of length 6."});

        // For this group, 6 is less than max specified (max specified is 8) but more than actual specified (actual specified is 5)
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 6, 0, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 6, 0, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 6, 0, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 6, 0, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 6, 0, "Index of 6 used to index descriptor array of length 6."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 6, "Index of 6 used to index descriptor array of length 6."});

        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 5, 0, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 5, 0, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 5, 0, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 5, 0, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 5, 0, "Descriptor index 5 is uninitialized."});
        tests.push_back({rgen_source_runtime, ahit_source_runtime, chit_source_runtime, miss_source_runtime, intr_source_runtime,
                         call_source_runtime, true, 0, 0, 0, 0, 0, 5, "Descriptor index 5 is uninitialized."});
    }

    // Iteration 0 tests with no descriptor set bound (to sanity test "draw" validation). Iteration 1
    // tests what's in the test case vector.
    for (const auto &test : tests) {
        if (gpu_assisted) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }

        VkShaderObj rgen_shader(this, test.rgen_shader_source.c_str(), VK_SHADER_STAGE_RAYGEN_BIT_NV);
        VkShaderObj ahit_shader(this, test.ahit_shader_source.c_str(), VK_SHADER_STAGE_ANY_HIT_BIT_NV);
        VkShaderObj chit_shader(this, test.chit_shader_source.c_str(), VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
        VkShaderObj miss_shader(this, test.miss_shader_source.c_str(), VK_SHADER_STAGE_MISS_BIT_NV);
        VkShaderObj intr_shader(this, test.intr_shader_source.c_str(), VK_SHADER_STAGE_INTERSECTION_BIT_NV);
        VkShaderObj call_shader(this, test.call_shader_source.c_str(), VK_SHADER_STAGE_CALLABLE_BIT_NV);

        VkPipelineShaderStageCreateInfo stage_create_infos[6] = {};
        stage_create_infos[0] = vku::InitStructHelper();
        stage_create_infos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
        stage_create_infos[0].module = rgen_shader.handle();
        stage_create_infos[0].pName = "main";

        stage_create_infos[1] = vku::InitStructHelper();
        stage_create_infos[1].stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        stage_create_infos[1].module = ahit_shader.handle();
        stage_create_infos[1].pName = "main";

        stage_create_infos[2] = vku::InitStructHelper();
        stage_create_infos[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        stage_create_infos[2].module = chit_shader.handle();
        stage_create_infos[2].pName = "main";

        stage_create_infos[3] = vku::InitStructHelper();
        stage_create_infos[3].stage = VK_SHADER_STAGE_MISS_BIT_NV;
        stage_create_infos[3].module = miss_shader.handle();
        stage_create_infos[3].pName = "main";

        stage_create_infos[4] = vku::InitStructHelper();
        stage_create_infos[4].stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        stage_create_infos[4].module = intr_shader.handle();
        stage_create_infos[4].pName = "main";

        stage_create_infos[5] = vku::InitStructHelper();
        stage_create_infos[5].stage = VK_SHADER_STAGE_CALLABLE_BIT_NV;
        stage_create_infos[5].module = call_shader.handle();
        stage_create_infos[5].pName = "main";

        VkRayTracingShaderGroupCreateInfoNV group_create_infos[4] = {};
        group_create_infos[0] = vku::InitStructHelper();
        group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[0].generalShader = 0;  // rgen
        group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[1] = vku::InitStructHelper();
        group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[1].generalShader = 3;  // miss
        group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_NV;

        group_create_infos[2] = vku::InitStructHelper();
        group_create_infos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
        group_create_infos[2].generalShader = VK_SHADER_UNUSED_NV;
        group_create_infos[2].closestHitShader = 2;
        group_create_infos[2].anyHitShader = 1;
        group_create_infos[2].intersectionShader = 4;

        group_create_infos[3] = vku::InitStructHelper();
        group_create_infos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group_create_infos[3].generalShader = 5;  // call
        group_create_infos[3].closestHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].anyHitShader = VK_SHADER_UNUSED_NV;
        group_create_infos[3].intersectionShader = VK_SHADER_UNUSED_NV;

        VkRayTracingPipelineCreateInfoNV pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stageCount = 6;
        pipeline_ci.pStages = stage_create_infos;
        pipeline_ci.groupCount = 4;
        pipeline_ci.pGroups = group_create_infos;
        pipeline_ci.maxRecursionDepth = 2;
        pipeline_ci.layout = test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle();

        VkPipeline pipeline = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS,
                  vk::CreateRayTracingPipelinesNV(m_device->handle(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline));

        std::vector<uint8_t> shader_binding_table_data;
        shader_binding_table_data.resize(static_cast<std::size_t>(shader_binding_table_buffer_size), 0);
        ASSERT_EQ(VK_SUCCESS, vk::GetRayTracingShaderGroupHandlesNV(m_device->handle(), pipeline, 0, 4,
                                                                    static_cast<std::size_t>(shader_binding_table_buffer_size),
                                                                    shader_binding_table_data.data()));

        uint8_t *mapped_shader_binding_table_data = (uint8_t *)shader_binding_table_buffer.memory().map();
        std::memcpy(mapped_shader_binding_table_data, shader_binding_table_data.data(), shader_binding_table_data.size());
        shader_binding_table_buffer.memory().unmap();

        ray_tracing_command_buffer.begin();

        vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);

        if (gpu_assisted) {
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-None-08600");
        }

        if (gpu_assisted) {
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458");
            // Need these values to pass mapped storage buffer checks
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupHandleSize * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupHandleSize * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
        } else {
            // offset shall be multiple of shaderGroupBaseAlignment and stride of shaderGroupHandleSize
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 3ull, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
        }

        ray_tracing_command_buffer.end();
        // Update the index of the texture that the shaders should read
        uint32_t *mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
        mapped_storage_buffer_data[0] = test.rgen_index;
        mapped_storage_buffer_data[1] = test.ahit_index;
        mapped_storage_buffer_data[2] = test.chit_index;
        mapped_storage_buffer_data[3] = test.miss_index;
        mapped_storage_buffer_data[4] = test.intr_index;
        mapped_storage_buffer_data[5] = test.call_index;
        mapped_storage_buffer_data[6] = 0;
        mapped_storage_buffer_data[7] = 0;
        mapped_storage_buffer_data[8] = 0;
        mapped_storage_buffer_data[9] = 0;
        mapped_storage_buffer_data[10] = 0;
        mapped_storage_buffer_data[11] = 0;
        storage_buffer.memory().unmap();

        vk::QueueSubmit(ray_tracing_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(ray_tracing_queue);
        m_errorMonitor->VerifyFound();

        if (gpu_assisted) {
            mapped_storage_buffer_data = (uint32_t *)storage_buffer.memory().map();
            ASSERT_TRUE(mapped_storage_buffer_data[6] == 1);
            ASSERT_TRUE(mapped_storage_buffer_data[7] == 2);
            ASSERT_TRUE(mapped_storage_buffer_data[8] == 3);
            ASSERT_TRUE(mapped_storage_buffer_data[9] == 4);
            ASSERT_TRUE(mapped_storage_buffer_data[10] == 5);
            ASSERT_TRUE(mapped_storage_buffer_data[11] == 6);
            storage_buffer.memory().unmap();
        } else {
            ray_tracing_command_buffer.begin();
            vk::CmdBindPipeline(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);
            vk::CmdBindDescriptorSets(ray_tracing_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
                                      test.variable_length ? pipeline_layout_variable.handle() : pipeline_layout.handle(), 0, 1,
                                      test.variable_length ? &ds_variable.set_ : &ds.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462");
            VkDeviceSize stride_align = ray_tracing_properties.shaderGroupHandleSize;
            VkDeviceSize invalid_max_stride = ray_tracing_properties.maxShaderGroupStride +
                                              (stride_align - (ray_tracing_properties.maxShaderGroupStride %
                                                               stride_align));  // should be less than maxShaderGroupStride
            VkDeviceSize invalid_stride =
                ray_tracing_properties.shaderGroupHandleSize >> 1;  // should  be multiple of shaderGroupHandleSize
            VkDeviceSize invalid_offset =
                ray_tracing_properties.shaderGroupBaseAlignment >> 1;  // should be multiple of shaderGroupBaseAlignment

            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(), invalid_offset,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, invalid_stride,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, invalid_max_stride,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // hit shader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), invalid_offset, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               invalid_stride, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               invalid_max_stride, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // miss shader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               invalid_offset, ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_stride,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, invalid_max_stride,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            // raygenshader
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456");
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(), invalid_offset,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 1ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 2ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment,
                               ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/1);

            m_errorMonitor->VerifyFound();
            const auto &limits = m_device->phy().limits_;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-width-02469");
            uint32_t invalid_width = limits.maxComputeWorkGroupCount[0] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/invalid_width, /*height=*/1, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-height-02470");
            uint32_t invalid_height = limits.maxComputeWorkGroupCount[1] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/invalid_height, /*depth=*/1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysNV-depth-02471");
            uint32_t invalid_depth = limits.maxComputeWorkGroupCount[2] + 1;
            vk::CmdTraceRaysNV(ray_tracing_command_buffer.handle(), shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 0ull, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment * 1ull, ray_tracing_properties.shaderGroupHandleSize,
                               shader_binding_table_buffer.handle(), ray_tracing_properties.shaderGroupBaseAlignment * 2ull,
                               ray_tracing_properties.shaderGroupHandleSize, shader_binding_table_buffer.handle(),
                               ray_tracing_properties.shaderGroupBaseAlignment, ray_tracing_properties.shaderGroupHandleSize,
                               /*width=*/1, /*height=*/1, /*depth=*/invalid_depth);
            m_errorMonitor->VerifyFound();

            ray_tracing_command_buffer.end();
        }
        vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    }
}

TEST_F(NegativeRayTracing, BarrierAccessAccelerationStructure) {
    TEST_DESCRIPTION("Test barrier with access ACCELERATION_STRUCTURE bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_commandBuffer->begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03927");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03928");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03927");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03928");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierSync2AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync2 barrier with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    vkt::Buffer buffer(*m_device, 32);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_commandBuffer->begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-dstAccessMask-06256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-dstAccessMask-06256");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BarrierSync1AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync1 barrier with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer buffer(*m_device, 32);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    // memory barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // buffer barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");
    // image barrier
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstAccessMask-06257");

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 0, 1, &memory_barrier, 1, &buffer_barrier, 1, &image_barrier);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, EventSync1AccessAccelerationStructureRayQueryDisabled) {
    TEST_DESCRIPTION(
        "Test sync1 event wait with ACCELERATION_STRUCTURE_READ memory access."
        "Ray query feature is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    const vkt::Event event(*m_device);

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkt::Buffer buffer(*m_device, 32);
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    // memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");
    // image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcAccessMask-06257");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstAccessMask-06257");

    m_commandBuffer->WaitEvents(1, &event.handle(), VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                1, &memory_barrier, 1, &buffer_barrier, 1, &image_barrier);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, DescriptorBindingUpdateAfterBindWithAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState())

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570");
    vk::CreateDescriptorSetLayout(device(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_struct_features = vku::InitStructHelper();
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features = vku::InitStructHelper(&accel_struct_features);
    GetPhysicalDeviceFeatures2(ray_tracing_pipeline_features);

    VkPhysicalDeviceAccelerationStructurePropertiesKHR accel_struct_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(accel_struct_props);

    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_pipeline_features));

    // Create one descriptor set layout holding (maxPerStageDescriptorAccelerationStructures + 1) bindings
    // for the same shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03571 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test\n");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03571 requires "
                "maxPerStageDescriptorAccelerationStructures >= 1, skipping test\n");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxPerStageDescriptorUpdateAfterBindAccelerationStructures + 1) bindings for the same shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03572 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test\n");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03572 requires "
                "maxPerStageDescriptorUpdateAfterBindAccelerationStructures >= 1, skipping test\n");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout holding (maxDescriptorSetAccelerationStructures + 1) bindings
    // in total for two different shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03573 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test\n");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03573 requires "
                "maxDescriptorSetAccelerationStructures >= 1, skipping test\n");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = (i % 2) ? VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR : VK_SHADER_STAGE_CALLABLE_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03573");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }

    // Create one descriptor set layout with flag VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT holding
    // (maxDescriptorSetUpdateAfterBindAccelerationStructures + 1) bindings in total for two different shader stage
    {
        const uint32_t max_accel_structs = accel_struct_props.maxDescriptorSetUpdateAfterBindAccelerationStructures;
        if (max_accel_structs > 4096) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03574 requires a small maximum number of per stage "
                "descriptor update after bind for acceleration structures, "
                "skipping test\n");
        } else if (max_accel_structs < 1) {
            printf(
                "Testing VUID-VkPipelineLayoutCreateInfo-descriptorType-03574 requires "
                "maxDescriptorSetUpdateAfterBindAccelerationStructures >= 1, skipping test\n");
        } else {
            std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
            dslb_vec.reserve(max_accel_structs);

            for (uint32_t i = 0; i < max_accel_structs + 1; ++i) {
                VkDescriptorSetLayoutBinding dslb = {};
                dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                dslb.descriptorCount = 1;
                dslb.stageFlags = (i % 2) ? VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR : VK_SHADER_STAGE_CALLABLE_BIT_KHR;
                dslb.binding = i;
                dslb_vec.push_back(dslb);
            }

            VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
            ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            ds_layout_ci.bindingCount = dslb_vec.size();
            ds_layout_ci.pBindings = dslb_vec.data();

            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
            ASSERT_TRUE(ds_layout.initialized());

            VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
            pipeline_layout_ci.setLayoutCount = 1;
            pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03574");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
            vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeRayTracing, BeginQueryQueryPoolType) {
    TEST_DESCRIPTION("Test CmdBeginQuery with invalid queryPool queryType");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    const bool khr_acceleration_structure = IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    const bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);
    const bool ext_transform_feedback = IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    const bool rt_maintenance_1 = IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    if (!khr_acceleration_structure && !nv_ray_tracing) {
        GTEST_SKIP() << "Extensions " << VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME << " and " << VK_NV_RAY_TRACING_EXTENSION_NAME
                     << " are not supported.";
    }
    RETURN_IF_SKIP(InitState());

    if (khr_acceleration_structure) {
        auto cmd_begin_query = [this, ext_transform_feedback](VkQueryType query_type, auto vuid_begin_query,
                                                              auto vuid_begin_query_indexed) {
            VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
            query_pool_ci.queryCount = 1;

            query_pool_ci.queryType = query_type;
            vkt::QueryPool query_pool(*m_device, query_pool_ci);
            ASSERT_TRUE(query_pool.initialized());

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid_begin_query);
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid_begin_query_indexed);
                vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        };

        cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-04728",
                        "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
        cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-04728",
                        "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");

        if (rt_maintenance_1) {
            cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, "VUID-vkCmdBeginQuery-queryType-06741",
                            "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741");
            cmd_begin_query(VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
                            "VUID-vkCmdBeginQuery-queryType-06741", "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741");
        }
    }
    if (nv_ray_tracing) {
        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;
        query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04729");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        if (ext_transform_feedback) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729");
            vk::CmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracing, CopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyQueryPoolResults with unsupported query type");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&as_features);
    GetPhysicalDeviceFeatures2(bda_features);
    RETURN_IF_SKIP(InitState(nullptr, &bda_features));

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build(*m_device);

    auto valid_blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096);
    valid_blas->Build(*m_device);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = valid_blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = valid_blas->handle();
    copy_info.dst = blas_no_mem->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceBufferDeviceAddressFeatures bda_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper(&bda_features);
    auto features2 = GetPhysicalDeviceFeatures2(accel_features);

    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    // Init a non host visible buffer
    vkt::Buffer buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), device_memory.handle(), 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096);
    blas->SetDeviceBuffer(std::move(buffer));
    blas->Build(*m_device);

    auto blas_no_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096);
    blas_no_mem->SetDeviceBufferInitNoMem(true);
    blas_no_mem->Build(*m_device);

    auto blas_host_mem = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096);
    blas_host_mem->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    blas_host_mem->Build(*m_device);

    VkCopyAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = blas_no_mem->handle();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = blas->handle();
    copy_info.dst = blas_no_mem->handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vk::CmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = blas->handle();
    copy_info.dst = blas_host_mem->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03727");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = blas_host_mem->handle();
    copy_info.dst = blas->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03728");
    vk::CopyAccelerationStructureKHR(device(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdCopyMemoryToAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate CmdCopyMemoryToAccelerationStructureKHR with dst buffer not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer src_buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    vkt::Buffer dst_buffer;
    dst_buffer.init_no_mem(*m_device, buffer_ci);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(0);
    blas->SetDeviceBuffer(std::move(dst_buffer));
    blas->Build(*m_device);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src.deviceAddress = src_buffer.address();
    copy_info.dst = blas->handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // Acceleration structure buffer is not bound to memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    m_commandBuffer->begin();
    vk::CmdCopyMemoryToAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, BuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate buffers used in vkBuildAccelerationStructureKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_structure_features = vku::InitStructHelper(&ray_tracing_features);
    auto features2 = GetPhysicalDeviceFeatures2(acc_structure_features);
    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    vkt::Buffer host_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    auto bot_level_as = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    bot_level_as.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    bot_level_as.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();

    auto host_cached_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);

    host_cached_blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    host_cached_blas.SetSrcAS(vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(4096));
    host_cached_blas.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // .mode is UPDATE and .srcAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
    host_cached_blas.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, WriteAccelerationStructureMemory) {
    TEST_DESCRIPTION("Test memory in vkWriteAccelerationStructuresPropertiesKHR is host visible");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features = vku::InitStructHelper(&ray_query_features);
    GetPhysicalDeviceFeatures2(as_features);

    if (as_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &as_features));

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
    build_geometry_info.GetDstAS()->SetDeviceBuffer(std::move(non_host_visible_buffer));

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    build_geometry_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    build_geometry_info.BuildHost(instance(), *m_device);
    m_errorMonitor->VerifyFound();

    std::vector<uint32_t> data(4096);
    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
    vk::WriteAccelerationStructuresPropertiesKHR(device(), 1, &build_geometry_info.GetDstAS()->handle(),
                                                 VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data.size(), data.data(),
                                                 data.size());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CopyMemoryToAsBuffer) {
    TEST_DESCRIPTION("Test invalid buffer used in vkCopyMemoryToAccelerationStructureKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(accel_features);
    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructureHostCommands feature is not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    // Init a non host visible buffer
    vkt::Buffer non_host_visible_buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    non_host_visible_buffer.init_no_mem(*m_device, buffer_ci);
    VkMemoryRequirements memory_requirements = non_host_visible_buffer.memory_requirements();
    VkMemoryAllocateInfo memory_alloc = vku::InitStructHelper();
    memory_alloc.allocationSize = memory_requirements.size;
    ASSERT_TRUE(
        m_device->phy().set_memory_type(memory_requirements.memoryTypeBits, &memory_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory device_memory(*m_device, memory_alloc);
    ASSERT_TRUE(device_memory.initialized());
    vk::BindBufferMemory(m_device->handle(), non_host_visible_buffer.handle(), device_memory.handle(), 0);

    auto blas = vkt::as::blueprint::AccelStructSimpleOnHostBottomLevel(buffer_ci.size);
    blas->SetDeviceBuffer(std::move(non_host_visible_buffer));
    blas->Build(*m_device);

    uint8_t output[4096];
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(output);

    VkCopyMemoryToAccelerationStructureInfoKHR info = vku::InitStructHelper();
    info.dst = blas->handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    // .dstAccelerationStructure buffer is not bound to host visible memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vk::CopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, ArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "Core validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");

    OOBRayTracingShadersTestBody(false);
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_struct_features = vku::InitStructHelper(&ray_query_features);
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&acc_struct_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer.handle()};
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);
    // invalid buffer;
    {
        vkt::Buffer invalid_buffer(*m_device, 4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.buffer = invalid_buffer.handle();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid deviceAddress and flag;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.deviceAddress = device_address;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();

        invalid_as_create_info.deviceAddress = 0;
        invalid_as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid size and offset;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.size = 4097;  // buffer size is 4096
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-offset-03616");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid sType;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-sType-sType");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid type;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-type-parameter");
        vk::CreateAccelerationStructureKHR(device(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CreateAccelerationStructureKHRReplayFeature) {
    TEST_DESCRIPTION("Validate acceleration structure creation replay feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_struct_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&acc_struct_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))

    acc_struct_features.accelerationStructureCaptureReplay = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = vku::InitStructHelper();
    device_address_info.buffer = buffer.handle();
    VkDeviceAddress device_address = vk::GetBufferDeviceAddressKHR(device(), &device_address_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = device_address;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488");
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracing, CmdTraceRaysKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper();
    bda_features.bufferDeviceAddress = VK_TRUE;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper(&bda_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&ray_tracing_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    // Create ray tracing pipeline
    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    {
        const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
        VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
        VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

        const vkt::PipelineLayout pipeline_layout(*m_device, {});

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
        raytracing_pipeline_ci.layout = pipeline_layout.handle();

        const VkResult result = vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                                 &raytracing_pipeline_ci, nullptr, &raytracing_pipeline);
        ASSERT_EQ(VK_SUCCESS, result);
    }

    vkt::Buffer buffer;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    buffer_ci.size = 4096;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress device_address = buffer.address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();

    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03694");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03690");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03686");
            vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                100, 100, 1);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride = static_cast<VkDeviceSize>(ray_tracing_properties.maxShaderGroupStride) +
                                (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04041");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04035");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetUnexpectedError("VUID-vkCmdTraceRaysKHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04029");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    vkt::Buffer buffer_missing_flag;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_missing_flag.init_no_mem(*m_device, buffer_ci);
    vk::BindBufferMemory(device(), buffer_missing_flag.handle(), mem.handle(), 0);
    const VkDeviceAddress device_address_missing_flag = buffer_missing_flag.address();

    // buffer is missing flag VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        // This address is the same as the one from the first (valid) buffer, so no validation error
        invalid_stride.deviceAddress = device_address_missing_flag;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
    }

    // pRayGenShaderBindingTable address range and stride are invalid
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 8128;
        invalid_stride.size = 8128;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkStridedDeviceAddressRegionKHR-size-04631");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkStridedDeviceAddressRegionKHR-size-04632");
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_stride, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }

    // pMissShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                            100, 1);
        m_errorMonitor->VerifyFound();
    }
    // pHitShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                            100, 1);
    }
    // pCallableShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                            100, 1);
    }

    m_commandBuffer->end();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirectKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))

    if (ray_tracing_features.rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        GTEST_SKIP() << "rayTracingIndirectTraceRays not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buf_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    const VkDeviceAddress device_address = buffer.address();

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            // TODO - Create minimal ray tracing pipeline helper - remove all extra `08606`
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    // Given the invalid stride computation, stride is likely to also be misaligned, so allow above errors
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        if (ray_tracing_properties.maxShaderGroupStride ==
            std::numeric_limits<decltype(ray_tracing_properties.maxShaderGroupStride)>::max()) {
            printf("ray_tracing_properties.maxShaderGroupStride has maximum possible value, skipping related tests\n");
        } else {
            invalid_stride.stride = ray_tracing_properties.maxShaderGroupStride + 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                        &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-None-08606");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vk::CmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                        &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRFeatureDisabled) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    // VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR maintenance1_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const VkDeviceAddress device_address = buffer.address();

    m_commandBuffer->begin();
    // No VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR was in the device create info pNext chain
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637");
        vk::CmdTraceRaysIndirect2KHR(m_commandBuffer->handle(), device_address);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, CmdTraceRaysIndirect2KHRAddress) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirect2KHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR maintenance1_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&maintenance1_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    if (maintenance1_features.rayTracingPipelineTraceRaysIndirect2 == VK_FALSE) {
        GTEST_SKIP() << "rayTracingPipelineTraceRaysIndirect2 not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_info.size = 4096;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const VkDeviceAddress device_address = buffer.address();

    m_commandBuffer->begin();
    // indirectDeviceAddress is not a multiple of 4
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-None-08606");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634");
        vk::CmdTraceRaysIndirect2KHR(m_commandBuffer->handle(), device_address + 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, AccelerationStructureVersionInfoKHR) {
    TEST_DESCRIPTION("Validate VkAccelerationStructureVersionInfoKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(ray_tracing_features);
    if (ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        GTEST_SKIP() << "rayTracing not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_features));

    VkAccelerationStructureVersionInfoKHR valid_version = vku::InitStructHelper();
    VkAccelerationStructureCompatibilityKHR compatablity;
    uint8_t mode[] = {VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR};
    valid_version.pVersionData = mode;
    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-sType-sType");
        vk::GetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.pVersionData = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-pVersionData-parameter");
        vk::GetDeviceAccelerationStructureCompatibilityKHR(device(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, CmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    AddOptionalExtensions(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&bda_features);
    ray_query_features.rayQuery = VK_TRUE;
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    bda_features.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&ray_query_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    const bool index_type_uint8 = IsExtensionsEnabled(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);

    VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_struct_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(acc_struct_properties);

    // Command buffer not in recording mode
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // dstAccelerationStructure == VK_NULL_HANDLE
    {
        // Command buffer build
        {
            auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull());

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }

        // Command buffer indirect build
        if (accel_features.accelerationStructureIndirectBuild == VK_TRUE) {
            auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull());
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildCmdBufferIndirect(*m_device, m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }

        // Host build
        if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
            auto build_info_null_dst = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device);
            build_info_null_dst.SetDstAS(vkt::as::blueprint::AccelStructNull());
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            build_info_null_dst.BuildHost(instance(), *m_device);
            m_errorMonitor->VerifyFound();
        }
    }

    // Positive build tests
    {
        m_commandBuffer->begin();
        auto build_info_ppGeometries = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info_ppGeometries.BuildCmdBuffer(*m_device, m_commandBuffer->handle(), true);
        m_commandBuffer->end();
    }

    {
        m_commandBuffer->begin();
        auto build_info_pGeometries = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info_pGeometries.BuildCmdBuffer(*m_device, m_commandBuffer->handle(), false);
        m_commandBuffer->end();
    }

    m_commandBuffer->begin();

    // Invalid info count
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetInfoCount(0);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetNullInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetNullBuildRangeInfos(true);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        InitRenderTarget();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    // Invalid dst buffer
    {
        auto buffer_ci =
            vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        vkt::Buffer invalid_buffer;
        invalid_buffer.init_no_mem(*m_device, buffer_ci);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetDstAS()->SetDeviceBuffer(std::move(invalid_buffer));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetInfo().sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();

        build_info.SetType(VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Total number of triangles in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03795");
            build_info.GetSizeInfo(m_device->handle());
            m_errorMonitor->VerifyFound();
        }
    }
    // Total number of AABBs in all geometries superior to VkPhysicalDeviceAccelerationStructurePropertiesKHR::maxPrimitiveCount
    {
        constexpr auto primitive_count = vvl::kU32Max;
        // Check that primitive count is indeed superior to limit
        if (primitive_count > acc_struct_properties.maxPrimitiveCount) {
            auto build_info =
                vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
            build_info.GetGeometries()[0].SetPrimitiveCount(primitive_count);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03794");
            build_info.GetSizeInfo(m_device->handle());
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride in pGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(m_device->handle(), false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (not a multiple of 8)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03545");
        build_info.GetSizeInfo(m_device->handle(), true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in pGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(m_device->handle(), false);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride in ppGeometry.geometry.aabbs (superior to UINT32_MAX)
    {
        auto build_info =
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::AABB);
        build_info.GetGeometries()[0].SetStride(8ull * vvl::kU32Max);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryAabbsDataKHR-stride-03820");
        build_info.GetSizeInfo(m_device->handle(), true);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex stride
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetStride(VkDeviceSize(vvl::kU32Max) + 1);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Invalid index type
    if (index_type_uint8) {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetTrianglesIndexType(VK_INDEX_TYPE_UINT8_EXT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // ppGeometries and pGeometries both valid pointer
    {
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        for (const auto &geometry : build_info.GetGeometries()) {
            geometries.emplace_back(geometry.GetVkObj());
        }
        build_info.GetInfo().pGeometries = geometries.data();  // .ppGeometries is set in .BuildCmdBuffer()
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        // computed scratch buffer size will be 0 since vkGetAccelerationStructureBuildSizesKHR fails
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        vkt::Buffer bad_usage_buffer(*m_device, 1024,
                                     VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_flags);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(bad_usage_buffer), 3);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        // no VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT => scratch address will be set to 0
        auto bad_scratch = std::make_shared<vkt::Buffer>(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        build_info.SetScratchBuffer(std::move(bad_scratch));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6040
TEST_F(NegativeRayTracing, DISABLED_AccelerationStructuresOverlappingMemory) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when source/destination acceleration structures and scratch buffers overlap.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&bda_features);
    accel_features.accelerationStructure = VK_TRUE;
    bda_features.bufferDeviceAddress = VK_TRUE;
    ray_query_features.rayQuery = VK_TRUE;

    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&ray_query_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    constexpr size_t build_info_count = 3;

    // All buffers used to back source/destination acceleration struct and scratch will be bound to this memory chunk
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 8192;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test overlapping scratch buffers
    {
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (auto &scratch_buffer : scratch_buffers) {
            scratch_buffer = std::make_shared<vkt::Buffer>();
            scratch_buffer->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffer->handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.SetScratchBuffer(std::move(scratch_buffer));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the scratch buffers are bound to the same memory, 03704 will be triggered for each pair of elements in
        // `build_infos`
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }

    // Test overlapping destination acceleration structures
    {
        VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
        dst_blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        dst_blas_buffer_ci.size = 4096;
        dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (auto &dst_blas_buffer : dst_blas_buffers) {
            dst_blas_buffer.init_no_mem(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffer.handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffer));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the destination acceleration structures are bound to the same memory, 03702 will be triggered for each pair of
        // elements in `build_infos`
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }

    // Test overlapping destination acceleration structure and scratch buffer
    {
        VkBufferCreateInfo dst_blas_buffer_ci = vku::InitStructHelper();
        dst_blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        dst_blas_buffer_ci.size = 4096;
        dst_blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                   VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            dst_blas_buffers[i].init_no_mem(*m_device, dst_blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffers[i].handle(), buffer_memory.handle(), 0);
            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffers[i]->handle(), buffer_memory.handle(), 0);

            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);
            build_info.SetScratchBuffer(std::move(scratch_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the destination acceleration structures and scratch buffers are bound to the same memory, 03702, 03703 and
        // 03704 will be triggered for each pair of elements in `build_infos`. 03703 will also be triggered for individual elements.
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
        }
        for (size_t i = 0; i < build_info_count; ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }

    // Test overlapping source acceleration structure and destination acceleration structures
    {
        VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
        blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        blas_buffer_ci.size = 4096;
        blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<vkt::Buffer> dst_blas_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), src_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            dst_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), dst_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            // 1st step: build destination acceleration struct
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);
            build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_commandBuffer->begin();
            build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
            m_commandBuffer->end();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct with its
            // underlying buffer bound to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03701
            build_info.SetSrcAS(build_info.GetDstAS());
            build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096));
            build_info.GetDstAS()->SetDeviceBuffer(std::move(dst_blas_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03701 and 03702 will be
        // triggered for each pair of elements in `build_infos`
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702");
        }
        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }

    // Test overlapping source acceleration structure and scratch buffer
    {
        VkBufferCreateInfo blas_buffer_ci = vku::InitStructHelper();
        blas_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        blas_buffer_ci.size = 4096;
        blas_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        scratch_buffer_ci.size = 4096;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        std::vector<vkt::Buffer> src_blas_buffers(build_info_count);
        std::vector<std::shared_ptr<vkt::Buffer>> scratch_buffers(build_info_count);
        std::vector<vkt::as::BuildGeometryInfoKHR> build_infos;
        for (size_t i = 0; i < build_info_count; ++i) {
            src_blas_buffers[i].init_no_mem(*m_device, blas_buffer_ci);
            vk::BindBufferMemory(m_device->device(), src_blas_buffers[i].handle(), buffer_memory.handle(), 0);

            scratch_buffers[i] = std::make_shared<vkt::Buffer>();
            scratch_buffers[i]->init_no_mem(*m_device, scratch_buffer_ci);
            vk::BindBufferMemory(m_device->device(), scratch_buffers[i]->handle(), buffer_memory.handle(), 0);

            // 1st step: build destination acceleration struct
            auto build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            build_info.GetDstAS()->SetDeviceBuffer(std::move(src_blas_buffers[i]));
            build_info.GetDstAS()->SetSize(4096);  // Do this to ensure dst accel struct buffer and scratch do overlap
            build_info.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
            m_commandBuffer->begin();
            build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
            m_commandBuffer->end();

            // Possible 2nd step: insert memory barrier on acceleration structure buffer
            // => no need here since 2nd build call will not be executed by the driver

            // 3rd step: set destination acceleration struct as source, create new destination acceleration struct,
            // bound scratch buffer to the same memory as the source acceleration struct, and build using update mode to trigger
            // 03705
            build_info.SetSrcAS(build_info.GetDstAS());
            build_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
            build_info.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(4096));
            build_info.SetScratchBuffer(std::move(scratch_buffers[i]));
            build_infos.emplace_back(std::move(build_info));
        }

        // Since all the source and destination acceleration structures are bound to the same memory, 03701 and 03702 will be
        // triggered for each pair of elements in `build_infos`. 03705 will also be triggered for individual elements.
        for (size_t i = 0; i < binom<size_t>(build_info_count, 2); ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705");
        }
        for (size_t i = 0; i < build_info_count; ++i) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705");
        }

        m_commandBuffer->begin();
        vkt::as::BuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle(), build_infos);
        m_commandBuffer->end();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracing, ObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    vkt::as::BuildGeometryInfoKHR build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_commandBuffer->begin();
    build_geometry_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // This test used to destroy buffers used for building the acceleration structure,
    // to see if there life span was correctly tracked.
    // Following issue 6461, buffers associated to a device address are not tracked anymore, as it is impossible
    // to track the "correct" one: there is not a 1 to 1 mapping between a buffer and a device address.

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vk::DestroyAccelerationStructureKHR(m_device->handle(), build_geometry_info.GetDstAS()->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeRayTracing, CmdCopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION("Validate CmdCopyAccelerationStructureToMemoryKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_struct_features = vku::InitStructHelper(&ray_query_features);
    GetPhysicalDeviceFeatures2(acc_struct_features);
    RETURN_IF_SKIP(InitState(nullptr, &acc_struct_features));

    constexpr VkDeviceSize buffer_size = 4096;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.pNext = NULL;
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    VkAccelerationStructureKHR as;
    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);

    constexpr intptr_t alignment_padding = 256 - 1;
    int8_t output[buffer_size + alignment_padding];
    VkDeviceOrHostAddressKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(((intptr_t)output + alignment_padding) & ~alignment_padding);
    VkCopyAccelerationStructureToMemoryInfoKHR copy_info = vku::InitStructHelper();
    copy_info.src = as;
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    vkt::CommandBuffer cb(m_device, m_commandPool);
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
    vk::CmdCopyAccelerationStructureToMemoryKHR(cb.handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    cb.end();

    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(NegativeRayTracing, UpdateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Test for updating an acceleration structure without a srcAccelerationStructure");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true))

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_address_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_structure_features = vku::InitStructHelper(&buffer_address_features);
    GetPhysicalDeviceFeatures2(acc_structure_features);
    RETURN_IF_SKIP(InitState(nullptr, &acc_structure_features));

    m_commandBuffer->begin();

    vkt::as::BuildGeometryInfoKHR build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    build_geometry_info.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    // computed scratch buffer size is empty, so scratch buffer address can be 0 and invalid
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
    // Update acceleration structure, with .srcAccelerationStructure == VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-04630");
    build_geometry_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeRayTracing, BuffersAndBufferDeviceAddressesMapping) {
    TEST_DESCRIPTION(
        "Test that buffers and buffer device addresses mapping is correctly handled."
        "Bound multiple buffers to the same memory so that they have the same buffer device address."
        "Some buffers are valid for use in vkCmdBuildAccelerationStructuresKHR, others are not."
        "Using buffer device addresses obtained from invalid buffers will result in a valid call to "
        "vkCmdBuildAccelerationStructuresKHR,"
        "because for this call to be valid, at least one buffer retrieved from the buffer device addresses must be valid."
        "Valid and invalid buffers having the same address, the call is valid."
        "Removing those valid buffers should cause calls to vkCmdBuildAccelerationStructuresKHR to be invalid,"
        "as long as valid buffers are correctly removed from the internal buffer device addresses to buffers mapping.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_addr_features = vku::InitStructHelper(&ray_tracing_features);
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&buffer_addr_features);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(true, &features2))
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    // Allocate common buffer memory
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Create buffers, with correct and incorrect usage
    constexpr size_t N = 3;
    std::array<std::unique_ptr<vkt::as::BuildGeometryInfoKHR>, N> build_geometry_info_vec{};
    const VkBufferUsageFlags good_buffer_usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkBufferUsageFlags bad_buffer_usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = 4096;
    buffer_ci.usage = good_buffer_usage;
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            buffer_ci.usage = bad_buffer_usage;
        }

        vkt::Buffer vbo;
        vbo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), vbo.handle(), buffer_memory.handle(), 0);

        vkt::Buffer ibo;
        ibo.init_no_mem(*m_device, buffer_ci);
        vk::BindBufferMemory(device(), ibo.handle(), buffer_memory.handle(), 0);

        // Those calls to vkGetBufferDeviceAddressKHR will internally record vbo and ibo device addresses
        {
            const VkDeviceAddress vbo_address = vbo.address();
            const VkDeviceAddress ibo_address = ibo.address();
            if (vbo_address != ibo_address) {
                GTEST_SKIP()
                    << "Bounding two buffers to the same memory location does not result in identical buffer device addresses";
            }
        }
        build_geometry_info_vec[i] = std::make_unique<vkt::as::BuildGeometryInfoKHR>(
            vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceVertexBuffer(std::move(vbo), 2);
        build_geometry_info_vec[i]->GetGeometries()[0].SetTrianglesDeviceIndexBuffer(std::move(ibo));
    }

    // The first series of calls to vkCmdBuildAccelerationStructuresKHR should succeed,
    // since the first vbo and ibo do have the VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR flag.
    // After deleting the valid vbo and ibo, calls are expected to fail.

    for (size_t i = 0; i < N; ++i) {
        m_commandBuffer->begin();
        build_geometry_info_vec[i]->BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->end();
    }

    for (size_t i = 0; i < N; ++i) {
        m_commandBuffer->begin();
        if (i > 0) {
            // for vbo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
            // for ibo
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        }
        build_geometry_info_vec[i]->VkCmdBuildAccelerationStructuresKHR(*m_device, m_commandBuffer->handle());
        if (i > 0) {
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();

        build_geometry_info_vec[i] = nullptr;
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresProperties) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&bda_features);
    RETURN_IF_SKIP(InitFramework())

    GetPhysicalDeviceFeatures2(ray_query_features);
    RETURN_IF_SKIP(InitState(nullptr, &ray_query_features));
    const bool rt_maintenance_1 = IsExtensionsEnabled(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    // On host query with invalid query type
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        vkt::as::BuildGeometryInfoKHR as_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        as_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        as_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = 1;
        constexpr size_t data_size = sizeof(uint32_t) * stride;
        uint8_t data[data_size];
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
        m_errorMonitor->VerifyFound();

        // query types not known without extension
        if (rt_maintenance_1) {
            // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, but stride is not a multiple of the size of VkDeviceSize
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06731");
            vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                         VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, data_size, data, stride);
            m_errorMonitor->VerifyFound();

            // queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, but stride is not a
            // multiple of the size of VkDeviceSize
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06733");
            vk::WriteAccelerationStructuresPropertiesKHR(
                m_device->handle(), 1, &as_build_info.GetDstAS()->handle(),
                VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR, data_size, data, stride);
            m_errorMonitor->VerifyFound();
        }
    }

    // On device query with invalid query type
    {
        vkt::as::BuildGeometryInfoKHR as_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();

        as_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &as_build_info.GetDstAS()->handle(),
                                                        VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracing, WriteAccelerationStructuresPropertiesMaintenance1) {
    TEST_DESCRIPTION("Test queryType validation in vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features = vku::InitStructHelper();
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&accel_features);
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features = vku::InitStructHelper(&bda_features);
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR ray_tracing_maintenance1 = vku::InitStructHelper(&ray_query_features);

    RETURN_IF_SKIP(InitFramework())

    GetPhysicalDeviceFeatures2(ray_tracing_maintenance1);
    RETURN_IF_SKIP(InitState(nullptr, &ray_tracing_maintenance1));

    // On host query with invalid query type
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = sizeof(VkDeviceSize);
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_OCCLUSION, data_size, data, stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with missing BLAS flag
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // missing flag
        // blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = sizeof(VkDeviceSize);
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On host query type with invalid stride
    if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.GetDstAS()->SetDeviceBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();
        blas_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        m_commandBuffer->end();

        constexpr size_t stride = 1;
        constexpr size_t data_size = sizeof(VkDeviceSize) * stride;
        uint8_t data[data_size];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03448");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-queryType-03450");
        vk::WriteAccelerationStructuresPropertiesKHR(m_device->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                     VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, data_size, data,
                                                     stride);
        m_errorMonitor->VerifyFound();
    }

    // On device query with invalid query type
    {
        vkt::as::BuildGeometryInfoKHR blas_build_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

        VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
        query_pool_ci.queryCount = 1;

        query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        vkt::QueryPool query_pool(*m_device, query_pool_ci);
        ASSERT_TRUE(query_pool.initialized());

        m_commandBuffer->begin();

        blas_build_info.BuildCmdBuffer(*m_device, m_commandBuffer->handle());
        // Incorrect query type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryType-06742");
        vk::CmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer->handle(), 1, &blas_build_info.GetDstAS()->handle(),
                                                        VK_QUERY_TYPE_OCCLUSION, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeRayTracingNV, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_props);

    RETURN_IF_SKIP(InitState())

    uint32_t maxBlocks = ray_tracing_props.maxDescriptorSetAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of descriptor set acceleration structures, skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxDescriptorSetAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i <= maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02381");
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ValidateGeometry) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer ibo;
    ibo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer tbo;
    tbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkt::Buffer aabbbo;
    aabbbo.init(*m_device, 1024, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkBufferCreateInfo unbound_buffer_ci = vku::InitStructHelper();
    unbound_buffer_ci.size = 1024;
    unbound_buffer_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    vkt::Buffer unbound_buffer;
    unbound_buffer.init_no_mem(*m_device, unbound_buffer_ci);

    constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    constexpr std::array<uint32_t, 3> indicies = {0, 1, 2};
    constexpr std::array aabbs = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    constexpr std::array transforms = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo.memory().map();
    std::memcpy(mapped_vbo_buffer_data, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo.memory().unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo.memory().map();
    std::memcpy(mapped_ibo_buffer_data, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo.memory().unmap();

    uint8_t *mapped_tbo_buffer_data = (uint8_t *)tbo.memory().map();
    std::memcpy(mapped_tbo_buffer_data, (uint8_t *)transforms.data(), sizeof(float) * transforms.size());
    tbo.memory().unmap();

    uint8_t *mapped_aabbbo_buffer_data = (uint8_t *)aabbbo.memory().map();
    std::memcpy(mapped_aabbbo_buffer_data, (uint8_t *)aabbs.data(), sizeof(float) * aabbs.size());
    aabbbo.memory().unmap();

    VkGeometryNV valid_geometry_triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles = vku::InitStructHelper();
    valid_geometry_triangles.geometry.triangles.vertexData = vbo.handle();
    valid_geometry_triangles.geometry.triangles.vertexOffset = 0;
    valid_geometry_triangles.geometry.triangles.vertexCount = 3;
    valid_geometry_triangles.geometry.triangles.vertexStride = 12;
    valid_geometry_triangles.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    valid_geometry_triangles.geometry.triangles.indexData = ibo.handle();
    valid_geometry_triangles.geometry.triangles.indexOffset = 0;
    valid_geometry_triangles.geometry.triangles.indexCount = 3;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.transformData = tbo.handle();
    valid_geometry_triangles.geometry.triangles.transformOffset = 0;
    valid_geometry_triangles.geometry.aabbs = vku::InitStructHelper();

    VkGeometryNV valid_geometry_aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs = vku::InitStructHelper();
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
        as_create_info.info = vku::InitStructHelper();
        as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        as_create_info.info.instanceCount = 0;
        as_create_info.info.geometryCount = 1;
        as_create_info.info.pGeometries = &geometry;
        return as_create_info;
    };

    VkAccelerationStructureNV as;

    // Invalid vertex format.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R64_UINT;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexFormat-02430");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - not multiple of component size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02429");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 12 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex buffer - no such buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = VkBuffer(123456789);

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexData-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#if 0
    // XXX Subtest disabled because this is the wrong VUID.
    // No VUIDs currently exist to require memory is bound (spec bug).
    // Invalid vertex buffer - no memory bound.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = unbound_buffer.handle();

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#endif

    // Invalid index offset - not multiple of index size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02432");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02431");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index count - must be 0 if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
        geometry.geometry.triangles.indexCount = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexCount-02436");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index data - must be VK_NULL_HANDLE if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = ibo.handle();
        geometry.geometry.triangles.indexCount = 0;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexData-02434");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid transform offset - not multiple of 16.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02438");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid transform offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02437");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid aabb offset - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02440");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 8 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02439");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb stride - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-stride-02441");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV or VK_GEOMETRY_TYPE_AABBS_NV
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryNV-geometryType-03503");
        m_errorMonitor->SetUnexpectedError("VUID-VkGeometryNV-geometryType-parameter");
        vk::CreateAccelerationStructureNV(device(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCreateAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();

    VkAccelerationStructureNV as = VK_NULL_HANDLE;

    // Top level can not have geometry
    {
        VkAccelerationStructureCreateInfoNV bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        bad_top_level_create_info.info.instanceCount = 0;
        bad_top_level_create_info.info.geometryCount = 1;
        bad_top_level_create_info.info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02425");
        vk::CreateAccelerationStructureNV(device(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Bot level can not have instances
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_bot_level_create_info.info.instanceCount = 1;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02426");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        bad_bot_level_create_info.info.instanceCount = 0;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-04623");
        vk::CreateAccelerationStructureNV(device(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not prefer both fast trace and fast build
    {
        VkAccelerationStructureCreateInfoNV bad_flags_level_create_info = as_create_info;
        bad_flags_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_flags_level_create_info.info.instanceCount = 0;
        bad_flags_level_create_info.info.geometryCount = 1;
        bad_flags_level_create_info.info.pGeometries = &geometry;
        bad_flags_level_create_info.info.flags =
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-flags-02592");
        vk::CreateAccelerationStructureNV(device(), &bad_flags_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not have geometry or instance for compacting
    {
        VkAccelerationStructureCreateInfoNV bad_compacting_as_create_info = as_create_info;
        bad_compacting_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_compacting_as_create_info.info.instanceCount = 0;
        bad_compacting_as_create_info.info.geometryCount = 1;
        bad_compacting_as_create_info.info.pGeometries = &geometry;
        bad_compacting_as_create_info.info.flags = 0;
        bad_compacting_as_create_info.compactedSize = 1024;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421");
        vk::CreateAccelerationStructureNV(device(), &bad_compacting_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not mix different geometry types into single bottom level acceleration structure
    {
        VkGeometryNV aabb_geometry = vku::InitStructHelper();
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles = vku::InitStructHelper();
        aabb_geometry.geometry.aabbs = vku::InitStructHelper();
        // Buffer contents do not matter for this test.
        aabb_geometry.geometry.aabbs.aabbData = geometry.geometry.triangles.vertexData;
        aabb_geometry.geometry.aabbs.numAABBs = 1;
        aabb_geometry.geometry.aabbs.offset = 0;
        aabb_geometry.geometry.aabbs.stride = 24;

        std::vector<VkGeometryNV> geometries = {geometry, aabb_geometry};

        VkAccelerationStructureCreateInfoNV mix_geometry_types_as_create_info = as_create_info;
        mix_geometry_types_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        mix_geometry_types_as_create_info.info.instanceCount = 0;
        mix_geometry_types_as_create_info.info.geometryCount = static_cast<uint32_t>(geometries.size());
        mix_geometry_types_as_create_info.info.pGeometries = geometries.data();
        mix_geometry_types_as_create_info.info.flags = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureInfoNV-type-02786");
        vk::CreateAccelerationStructureNV(device(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateBindAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState())

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    vkt::AccelerationStructure as(*m_device, as_create_info, false);

    VkMemoryRequirements as_memory_requirements = as.memory_requirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = vku::InitStructHelper();
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = vku::InitStructHelper();
    as_memory_alloc.allocationSize = as_memory_requirements.size;
    ASSERT_TRUE(m_device->phy().set_memory_type(as_memory_requirements.memoryTypeBits, &as_memory_alloc, 0));

    // Can not bind already freed memory
    {
        VkDeviceMemory as_memory_freed = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_freed));
        vk::FreeMemory(device(), as_memory_freed, NULL);

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_freed = as_bind_info;
        as_bind_info_freed.memory = as_memory_freed;
        as_bind_info_freed.memoryOffset = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-parameter");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_freed);
        m_errorMonitor->VerifyFound();
    }

    // Can not bind with bad alignment
    if (as_memory_requirements.alignment > 1) {
        VkMemoryAllocateInfo as_memory_alloc_bad_alignment = as_memory_alloc;
        as_memory_alloc_bad_alignment.allocationSize += 1;

        VkDeviceMemory as_memory_bad_alignment = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_alignment, NULL, &as_memory_bad_alignment));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_alignment = as_bind_info;
        as_bind_info_bad_alignment.memory = as_memory_bad_alignment;
        as_bind_info_bad_alignment.memoryOffset = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_alignment);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_alignment, NULL);
    }

    // Can not bind with offset outside the allocation
    {
        VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
        as_bind_info_bad_offset.memory = as_memory_bad_offset;
        as_bind_info_bad_offset.memoryOffset =
            (as_memory_alloc.allocationSize + as_memory_requirements.alignment) & ~(as_memory_requirements.alignment - 1);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_offset, NULL);
    }

    // Can not bind with offset that doesn't leave enough size
    {
        VkDeviceSize offset = (as_memory_requirements.size - 1) & ~(as_memory_requirements.alignment - 1);
        if (offset > 0 && (as_memory_requirements.size < (as_memory_alloc.allocationSize - as_memory_requirements.alignment))) {
            VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
            as_bind_info_bad_offset.memory = as_memory_bad_offset;
            as_bind_info_bad_offset.memoryOffset = offset;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_offset, NULL);
        }
    }

    // Can not bind with memory that has unsupported memory type
    {
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

        uint32_t supported_memory_type_bits = as_memory_requirements.memoryTypeBits;
        uint32_t unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~supported_memory_type_bits;
        if (unsupported_mem_type_bits != 0) {
            VkMemoryAllocateInfo as_memory_alloc_bad_type = as_memory_alloc;
            ASSERT_TRUE(m_device->phy().set_memory_type(unsupported_mem_type_bits, &as_memory_alloc_bad_type, 0));

            VkDeviceMemory as_memory_bad_type = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc_bad_type, NULL, &as_memory_bad_type));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_type = as_bind_info;
            as_bind_info_bad_type.memory = as_memory_bad_type;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
            vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_type);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_type, NULL);
        }
    }

    // Can not bind memory twice
    {
        vkt::AccelerationStructure as_twice(*m_device, as_create_info, false);

        VkDeviceMemory as_memory_twice_1 = VK_NULL_HANDLE;
        VkDeviceMemory as_memory_twice_2 = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_1));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_2));
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_1 = as_bind_info;
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_2 = as_bind_info;
        as_bind_info_twice_1.accelerationStructure = as_twice.handle();
        as_bind_info_twice_2.accelerationStructure = as_twice.handle();
        as_bind_info_twice_1.memory = as_memory_twice_1;
        as_bind_info_twice_2.memory = as_memory_twice_2;

        ASSERT_EQ(VK_SUCCESS, vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        vk::BindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(NegativeRayTracingNV, ValidateWriteDescriptorSetAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState())

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkWriteDescriptorSetAccelerationStructureNV acc = vku::InitStructHelper();
    acc.accelerationStructureCount = 1;
    VkAccelerationStructureCreateInfoNV top_level_as_create_info = vku::InitStructHelper();
    top_level_as_create_info.info = vku::InitStructHelper();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    vkt::AccelerationStructure top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    descriptor_write.pNext = &acc;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(NegativeRayTracingNV, ValidateCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
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

    vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Incompatible type
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_type = bot_level_as_create_info.info;
    as_build_info_with_incompatible_type.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    as_build_info_with_incompatible_type.instanceCount = 1;
    as_build_info_with_incompatible_type.geometryCount = 0;

    // This is duplicated since it triggers one error for different types and one error for lower instance count - the
    // build info is incompatible but still needs to be valid to get past the stateless checks.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_type, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible flags
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_flags = bot_level_as_create_info.info;
    as_build_info_with_incompatible_flags.flags = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_flags, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible build size
    VkGeometryNV geometry_with_more_vertices = geometry;
    geometry_with_more_vertices.geometry.triangles.vertexCount += 1;

    VkAccelerationStructureInfoNV as_build_info_with_incompatible_geometry = bot_level_as_create_info.info;
    as_build_info_with_incompatible_geometry.pGeometries = &geometry_with_more_vertices;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_geometry, VK_NULL_HANDLE, 0,
                                        VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer too small
    VkBufferCreateInfo too_small_scratch_buffer_info = vku::InitStructHelper();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    vkt::Buffer too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, too_small_scratch_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer with offset too small
    VkDeviceSize scratch_buffer_offset = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(),
                                        scratch_buffer_offset);
    m_errorMonitor->VerifyFound();

    // Src must have been built before
    vkt::AccelerationStructure bot_level_as_updated(*m_device, bot_level_as_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02490");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                        bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buffer (invalid usage)
    VkBufferCreateInfo create_info = vku::InitStructHelper();
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    const vkt::Buffer bot_level_as_invalid_scratch = bot_level_as.create_scratch_buffer(*m_device, &create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-scratch-02781");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_invalid_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid instance data.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-instanceData-02782");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info,
                                        bot_level_as_invalid_scratch.handle(), 0, VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE,
                                        bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // must be called outside renderpass
    InitRenderTarget();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingNV, ObjInUseCmdBuildAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
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

    vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

    const vkt::Buffer bot_level_as_scratch = bot_level_as.create_scratch_buffer(*m_device);

    m_commandBuffer->begin();
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), ibo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), vbo.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(device(), bot_level_as_scratch.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    vk::DestroyAccelerationStructureNV(device(), bot_level_as.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeRayTracingNV, ValidateGetAccelerationStructureHandle) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
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

    // Not enough space for the handle
    {
        vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        vkt::AccelerationStructure bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
        vk::GetAccelerationStructureHandleNV(device(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeRayTracingNV, ValidateCmdCopyAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest(false))
    RETURN_IF_SKIP(InitState());

    vkt::Buffer vbo;
    vkt::Buffer ibo;
    VkGeometryNV geometry;
    nv::rt::GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = vku::InitStructHelper();
    as_create_info.info = vku::InitStructHelper();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.instanceCount = 0;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;

    vkt::AccelerationStructure src_as(*m_device, as_create_info);
    vkt::AccelerationStructure dst_as(*m_device, as_create_info);
    vkt::AccelerationStructure dst_as_without_mem(*m_device, as_create_info, false);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = vku::InitStructHelper();
    bot_level_as_create_info.info = vku::InitStructHelper();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    const vkt::Buffer bot_level_as_scratch = src_as.create_scratch_buffer(*m_device);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-04963");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        src_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1u, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src must have been created with allow compaction flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-03411");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV);
    m_errorMonitor->VerifyFound();

    // Dst must have been bound with memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    m_errorMonitor->VerifyFound();

    // mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-03410");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);
    m_errorMonitor->VerifyFound();

    // mode must be a valid VkCopyAccelerationStructureModeKHR value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-parameter");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR);
    m_errorMonitor->VerifyFound();

    // This command must only be called outside of a render pass instance
    InitRenderTarget();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();

    vkt::DeviceMemory host_memory;
    host_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(
                                    *m_device, dst_as_without_mem.memory_requirements().memoryRequirements,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    VkBindAccelerationStructureMemoryInfoNV bind_info = vku::InitStructHelper();
    bind_info.accelerationStructure = dst_as_without_mem.handle();
    bind_info.memory = host_memory.handle();
    vk::BindAccelerationStructureMemoryNV(*m_device, 1, &bind_info);

    uint64_t handle;
    vk::GetAccelerationStructureHandleNV(*m_device, dst_as_without_mem.handle(), sizeof(uint64_t), &handle);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-buffer-03719");
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-buffer-03718");
    const vkt::Buffer bot_level_as_scratch2 = dst_as_without_mem.create_scratch_buffer(*m_device);
    vk::CmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                        dst_as_without_mem.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    vk::CmdCopyAccelerationStructureNV(m_commandBuffer->handle(), src_as.handle(), dst_as_without_mem.handle(),
                                       VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();
}
