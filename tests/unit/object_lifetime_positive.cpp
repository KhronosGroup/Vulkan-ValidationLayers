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

TEST_F(PositiveObjectLifetime, DestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    RETURN_IF_SKIP(Init());
    vk::DestroyBuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyBufferView(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyCommandPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorSetLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDevice(VK_NULL_HANDLE, NULL);
    vk::DestroyEvent(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFence(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFramebuffer(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImage(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImageView(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyInstance(VK_NULL_HANDLE, NULL);
    vk::DestroyPipeline(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineCache(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineLayout(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyQueryPool(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyRenderPass(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroySampler(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroySemaphore(m_device->device(), VK_NULL_HANDLE, NULL);
    vk::DestroyShaderModule(m_device->device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 3, descriptor_sets);

    vk::FreeMemory(m_device->device(), VK_NULL_HANDLE, NULL);
}

TEST_F(PositiveObjectLifetime, FreeCommandBuffersNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeCommandBuffers");

    RETURN_IF_SKIP(Init());

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_commandPool->handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkCommandBuffer free_command_buffers[2] = {command_buffer, VK_NULL_HANDLE};
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, &free_command_buffers[0]);
}

TEST_F(PositiveObjectLifetime, FreeDescriptorSetsNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeDescriptorSets");

    RETURN_IF_SKIP(Init());

    VkDescriptorPoolSize ds_type_count = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1};

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                nullptr};

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    // Only set first set, second is still null
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[0]);
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 2, descriptor_sets);
}