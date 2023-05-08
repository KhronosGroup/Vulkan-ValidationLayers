/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

#include "generated/vk_extension_helper.h"
#include "../framework/ray_tracing_objects.h"

#include <array>

class PositiveDescriptors : public VkPositiveLayerTest {};

TEST_F(PositiveDescriptors, CopyNonupdatedDescriptors) {
    TEST_DESCRIPTION("Copy non-updated descriptors");
    unsigned int i;

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet src_descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });
    OneOffDescriptorSet dst_descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                         {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

    const unsigned int copy_size = 2;
    VkCopyDescriptorSet copy_ds_update[copy_size];
    memset(copy_ds_update, 0, sizeof(copy_ds_update));
    for (i = 0; i < copy_size; i++) {
        copy_ds_update[i] = LvlInitStruct<VkCopyDescriptorSet>();
        copy_ds_update[i].srcSet = src_descriptor_set.set_;
        copy_ds_update[i].srcBinding = i;
        copy_ds_update[i].dstSet = dst_descriptor_set.set_;
        copy_ds_update[i].dstBinding = i;
        copy_ds_update[i].descriptorCount = 1;
    }
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, copy_size, copy_ds_update);
}

TEST_F(PositiveDescriptors, DeleteDescriptorSetLayoutsBeforeDescriptorSets) {
    TEST_DESCRIPTION("Create DSLayouts and DescriptorSets and then delete the DSLayouts before the DescriptorSets.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkResult err;

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    auto ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool ds_pool_one(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorSet descriptorSet;
    {
        const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

        auto alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = ds_pool_one.handle();
        alloc_info.pSetLayouts = &ds_layout.handle();
        err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
        ASSERT_VK_SUCCESS(err);
    }  // ds_layout destroyed
    vk::FreeDescriptorSets(m_device->device(), ds_pool_one.handle(), 1, &descriptorSet);
}

TEST_F(PositiveDescriptors, IgnoreUnrelatedDescriptor) {
    TEST_DESCRIPTION(
        "Ensure that the vkUpdateDescriptorSets validation code is ignoring VkWriteDescriptorSet members that are not related to "
        "the descriptor type specified by VkWriteDescriptorSet::descriptorType.  Correct validation behavior will result in the "
        "test running to completion without validation errors.");

    const uintptr_t invalid_ptr = 0xcdcdcdcd;

    ASSERT_NO_FATAL_FAILURE(Init());

    // Verify VK_FORMAT_R8_UNORM supports VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    const VkFormat format_texel_case = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), format_texel_case, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Test requires to support VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT";
    }

    // Image Case
    {
        VkImageObj image(m_device);
        image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

        VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        // Set pBufferInfo and pTexelBufferView to invalid values, which should
        // be
        //  ignored for descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pBufferInfo.
        descriptor_write.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo *>(invalid_ptr);
        descriptor_write.pTexelBufferView = reinterpret_cast<const VkBufferView *>(invalid_ptr);

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    }

    // Buffer Case
    {
        uint32_t queue_family_index = 0;
        auto buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 1024;

        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        // Set pImageInfo and pTexelBufferView to invalid values, which should
        // be
        //  ignored for descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pImageInfo.
        descriptor_write.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo *>(invalid_ptr);
        descriptor_write.pTexelBufferView = reinterpret_cast<const VkBufferView *>(invalid_ptr);

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    }

    // Texel Buffer Case
    {
        uint32_t queue_family_index = 0;
        auto buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        auto buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
        buff_view_ci.buffer = buffer.handle();
        buff_view_ci.format = format_texel_case;
        buff_view_ci.range = VK_WHOLE_SIZE;
        vk_testing::BufferView buffer_view(*m_device, buff_view_ci);

        OneOffDescriptorSet descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           });

        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        descriptor_write.pTexelBufferView = &buffer_view.handle();

        // Set pImageInfo and pBufferInfo to invalid values, which should be
        //  ignored for descriptorType ==
        //  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.
        // This will most likely produce a crash if the parameter_validation
        // layer
        // does not correctly ignore pImageInfo and pBufferInfo.
        descriptor_write.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo *>(invalid_ptr);
        descriptor_write.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo *>(invalid_ptr);

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    }
}

TEST_F(PositiveDescriptors, ImmutableSamplerOnlyDescriptor) {
    TEST_DESCRIPTION("Bind a DescriptorSet with only an immutable sampler and make sure that we don't warn for no update.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                 });

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    sampler.destroy();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, EmptyDescriptorUpdate) {
    TEST_DESCRIPTION("Update last descriptor in a set that includes an empty binding");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        GTEST_SKIP() << "This test should not run on Nexus Player";
    }

    // Create layout with two uniform buffer descriptors w/ empty binding between them
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                         {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 /*!*/, 0, nullptr},
                                         {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });

    // Create a buffer to be used for update
    auto buff_ci = LvlInitStruct<VkBufferCreateInfo>();
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = 256;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vk_testing::Buffer buffer(*m_device, buff_ci);

    // Only update the descriptor at binding 2
    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    buff_info.offset = 0;
    buff_info.range = VK_WHOLE_SIZE;
    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = ds.set_;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(PositiveDescriptors, PushDescriptorNullDstSet) {
    TEST_DESCRIPTION("Use null dstSet in CmdPushDescriptorSetKHR");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});
    // Create push descriptor set layout
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.InitState();
    helper.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&push_ds_layout, &ds_layout});
    helper.CreateGraphicsPipeline();

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = sizeof(vbo_data);
    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    m_commandBuffer->begin();

    // In Intel GPU, it needs to bind pipeline before push descriptor set.
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                                &descriptor_write);
}

TEST_F(PositiveDescriptors, PushDescriptorUnboundSet) {
    TEST_DESCRIPTION("Ensure that no validation errors are produced for not bound push descriptor sets");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create descriptor set layout
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    OneOffDescriptorSet descriptor_set(m_device, {dsl_binding}, 0, nullptr, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                       nullptr);

    // Create push descriptor set layout
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Create PSO
    char const fsSource[] = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0) layout(binding=2) uniform foo1 { float x; } bar1;
        layout(set=1) layout(binding=2) uniform foo2 { float y; } bar2;
        void main(){
           x = vec4(bar1.x) + vec4(bar2.y);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    // Now use the descriptor layouts to create a pipeline layout
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&push_ds_layout, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    const float bo_data[1] = {1.f};
    VkConstantBufferObj buffer(m_device, sizeof(bo_data), (const void *)&bo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Update descriptor set
    descriptor_set.WriteDescriptorBufferInfo(2, buffer.handle(), 0, sizeof(bo_data));
    descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    // Push descriptors and bind descriptor set
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                descriptor_set.descriptor_writes.data());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                              &descriptor_set.set_, 0, NULL);

    // No errors should be generated.
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, BindingPartiallyBound) {
    TEST_DESCRIPTION("Ensure that no validation errors for invalid descriptors if binding is PARTIALLY_BOUND");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    InitFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    GetPhysicalDeviceFeatures2(indexing_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &indexing_features));
    if (!indexing_features.descriptorBindingPartiallyBound) {
        GTEST_SKIP() << "Partially bound bindings not supported, skipping test";
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    auto layout_createinfo_binding_flags = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    ds_binding_flags[0] = 0;
    // No Error
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    // Uncomment for Error
    // ds_binding_flags[1] = 0;

    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    uint32_t qfi = 0;
    auto buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &qfi;

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    auto index_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferObj index_buffer;
    index_buffer.init(*m_device, index_buffer_create_info);

    // Only update binding 0
    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo_0 { int val; } doit;
        layout(set = 0, binding = 1) uniform foo_1 { int val; } readit;
        void main() {
            if (doit.val == 0)
                gl_Position = vec4(0.0);
            else
                gl_Position = vec4(readit.val);
        }
    )glsl";

    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);
    auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 1, 0, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveDescriptors, PushDescriptorSetUpdatingSetNumber) {
    TEST_DESCRIPTION(
        "Ensure that no validation errors are produced when the push descriptor set number changes "
        "between two vk::CmdPushDescriptorSetKHR calls.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create a descriptor to push
    const uint32_t buffer_data[4] = {4, 5, 6, 7};
    VkConstantBufferObj buffer_obj(
        m_device, sizeof(buffer_data), &buffer_data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ASSERT_TRUE(buffer_obj.initialized());

    VkDescriptorBufferInfo buffer_info = {buffer_obj.handle(), 0, VK_WHOLE_SIZE};

    const VkDescriptorSetLayoutBinding ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const VkDescriptorSetLayoutBinding ds_binding_1 = {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const VkDescriptorSetLayoutObj ds_layout(m_device, {ds_binding_0, ds_binding_1});
    ASSERT_TRUE(ds_layout.initialized());

    const VkDescriptorSetLayoutBinding push_ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                            nullptr};
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {push_ds_binding_0},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    ASSERT_TRUE(push_ds_layout.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkPipelineObj pipe0(m_device);
    VkPipelineObj pipe1(m_device);
    {
        // Note: the push descriptor set is set number 2.
        const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout, &ds_layout, &push_ds_layout, &ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=2) layout(binding=0) uniform foo { vec4 y; } bar;
            void main(){
               x = bar.y;
            }
        )glsl";

        VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        VkPipelineObj &pipe = pipe0;
        pipe.SetViewport(m_viewports);
        pipe.SetScissor(m_scissors);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

        const VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
            vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        // Note: pushing to desciptor set number 2.
        vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1,
                                    &descriptor_write);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    {
        // Note: the push descriptor set is now set number 3.
        const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout, &ds_layout, &ds_layout, &push_ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        const VkWriteDescriptorSet descriptor_write = vk_testing::Device::write_descriptor_set(
            vk_testing::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=3) layout(binding=0) uniform foo { vec4 y; } bar;
            void main(){
               x = bar.y;
            }
        )glsl";

        VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        VkPipelineObj &pipe = pipe1;
        pipe.SetViewport(m_viewports);
        pipe.SetScissor(m_scissors);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

        // Note: now pushing to desciptor set number 3.
        vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 3, 1,
                                    &descriptor_write);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, DynamicOffsetWithInactiveBinding) {
    // Create a descriptorSet w/ dynamic descriptors where 1 binding is inactive
    // We previously had a bug where dynamic offset of inactive bindings was still being used

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    // Create two buffers to update the descriptors with
    // The first will be 2k and used for bindings 0 & 1, the second is 1k for binding 2
    uint32_t qfi = 0;
    auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 2048;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer_1, dynamic_uniform_buffer_2;
    dynamic_uniform_buffer_1.init(*m_device, buffCI);
    buffCI.size = 1024;
    dynamic_uniform_buffer_2.init(*m_device, buffCI);

    // Update descriptors
    const uint32_t BINDING_COUNT = 3;
    VkDescriptorBufferInfo buff_info[BINDING_COUNT] = {};
    buff_info[0].buffer = dynamic_uniform_buffer_1.handle();
    buff_info[0].offset = 0;
    buff_info[0].range = 256;
    buff_info[1].buffer = dynamic_uniform_buffer_1.handle();
    buff_info[1].offset = 256;
    buff_info[1].range = 512;
    buff_info[2].buffer = dynamic_uniform_buffer_2.handle();
    buff_info[2].offset = 0;
    buff_info[2].range = 512;

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = BINDING_COUNT;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor_write.pBufferInfo = buff_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0) layout(binding=0) uniform foo1 { int x; int y; } bar1;
        layout(set=0) layout(binding=2) uniform foo2 { int x; int y; } bar2;
        void main(){
           x = vec4(bar1.y) + vec4(bar2.y);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    // This update should succeed, but offset of inactive binding 1 oversteps binding 2 buffer size
    //   we used to have a bug in this case.
    uint32_t dyn_off[BINDING_COUNT] = {0, 1024, 256};
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, BINDING_COUNT, dyn_off);
    m_commandBuffer->Draw(1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, CreateDescriptorSetBindingWithIgnoredSamplers) {
    TEST_DESCRIPTION("Test that layers conditionally do ignore the pImmutableSamplers on vkCreateDescriptorSetLayout");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    const uint64_t fake_address_64 = 0xCDCDCDCDCDCDCDCD;
    const uint64_t fake_address_32 = 0xCDCDCDCD;
    const void *fake_pointer =
        sizeof(void *) == 8 ? reinterpret_cast<void *>(fake_address_64) : reinterpret_cast<void *>(fake_address_32);
    const VkSampler *hopefully_undereferencable_pointer = reinterpret_cast<const VkSampler *>(fake_pointer);

    // regular descriptors
    {
        const VkDescriptorSetLayoutBinding non_sampler_bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {8, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
        };
        const auto dslci =
            LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0u, size32(non_sampler_bindings), non_sampler_bindings);
        vk_testing::DescriptorSetLayout dsl(*m_device, dslci);
    }

    // push descriptors
    {
        const VkDescriptorSetLayoutBinding non_sampler_bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
            {6, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, hopefully_undereferencable_pointer},
        };
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(
            nullptr, static_cast<VkDescriptorSetLayoutCreateFlags>(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR),
            size32(non_sampler_bindings), non_sampler_bindings);
        vk_testing::DescriptorSetLayout dsl(*m_device, dslci);
    }
}

TEST_F(PositiveDescriptors, PushingDescriptorSetWithImmutableSampler) {
    TEST_DESCRIPTION("Use a push descriptor with an immutable sampler.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler;
    sampler.init(*m_device, sampler_ci);
    VkSampler sampler_handle = sampler.handle();

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &sampler_handle}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    VkDescriptorSetLayoutObj push_dsl(m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    VkPipelineLayoutObj pipeline_layout(m_device, {&push_dsl});

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler_handle;
    img_info.imageView = imageView;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pImageInfo = &img_info;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_write.dstSet = descriptor_set.set_;

    m_commandBuffer->begin();
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                &descriptor_write);
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, CopyMutableDescriptors) {
    TEST_DESCRIPTION("Copy mutable descriptors.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(mutable_descriptor_type_features);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        GTEST_SKIP() << "mutableDescriptorType feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_lists[2] = {};
    mutable_descriptor_type_lists[0].descriptorTypeCount = 2;
    mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
    mutable_descriptor_type_lists[1].descriptorTypeCount = 0;
    mutable_descriptor_type_lists[1].pDescriptorTypes = nullptr;

    auto mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoEXT>();
    mdtci.mutableDescriptorTypeListCount = 2;
    mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 2;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[1].descriptorCount = 2;

    auto ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[1].pImmutableSamplers = nullptr;

    auto create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;

    vk_testing::DescriptorSetLayout set_layout;
    set_layout.init(*m_device, create_info);
    VkDescriptorSetLayout set_layout_handle = set_layout.handle();

    VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

    auto allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = layouts;

    VkDescriptorSet descriptor_sets[2];
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = buffer_ci.size;

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    auto copy_set = LvlInitStruct<VkCopyDescriptorSet>();
    copy_set.srcSet = descriptor_sets[0];
    copy_set.srcBinding = 0;
    copy_set.dstSet = descriptor_sets[1];
    copy_set.dstBinding = 1;
    copy_set.descriptorCount = 1;

    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
}

TEST_F(PositiveDescriptors, CopyAccelerationStructureMutableDescriptors) {
    TEST_DESCRIPTION("Copy acceleration structure descriptor in a mutable descriptor.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&mutable_descriptor_type_features);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&acc_struct_features);
    auto features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        GTEST_SKIP() << "mutableDescriptorType feature not supported";
    }
    if (acc_struct_features.accelerationStructure == VK_FALSE) {
        GTEST_SKIP() << "accelerationStructure feature not supported";
    }
    if (bda_features.bufferDeviceAddress == VK_FALSE) {
        GTEST_SKIP() << "bufferDeviceAddress feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::array descriptor_types = {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = descriptor_types.size();
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types.data();

    auto mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoEXT>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[1].descriptorCount = 1;

    auto ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = pool_sizes.size();
    ds_pool_ci.pPoolSizes = pool_sizes.data();

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[1].pImmutableSamplers = nullptr;

    auto create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    create_info.bindingCount = bindings.size();
    create_info.pBindings = bindings.data();

    vk_testing::DescriptorSetLayout set_layout;
    set_layout.init(*m_device, create_info);

    std::array<VkDescriptorSetLayout, 2> layouts = {set_layout.handle(), set_layout.handle()};

    auto allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = layouts.size();
    allocate_info.pSetLayouts = layouts.data();

    std::array<VkDescriptorSet, layouts.size()> descriptor_sets;
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets.data());

    auto tlas = rt::as::blueprint::AccelStructSimpleOnDeviceTopLevel(DeviceValidationVersion(), 4096);
    tlas->Build(*m_device);

    auto blas_descriptor = LvlInitStruct<VkWriteDescriptorSetAccelerationStructureKHR>();
    blas_descriptor.accelerationStructureCount = 1;
    blas_descriptor.pAccelerationStructures = &tlas->handle();

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = blas_descriptor.accelerationStructureCount;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    descriptor_write.pNext = &blas_descriptor;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    auto copy_set = LvlInitStruct<VkCopyDescriptorSet>();
    copy_set.srcSet = descriptor_sets[0];
    copy_set.srcBinding = 0;
    copy_set.dstSet = descriptor_sets[1];
    copy_set.dstBinding = 1;
    copy_set.descriptorCount = 1;

    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
}

TEST_F(PositiveDescriptors, tImageViewAsDescriptorReadAndInputAttachment) {
    TEST_DESCRIPTION("Test reading from a descriptor that uses same image view as framebuffer input attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.init(&image_create_info);

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView image_view;
    image_view.init(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view_handle;

    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = m_renderPassClearValues.data();

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0, rgba8) readonly uniform image2D image1;
            layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput inputColor;
            void main(){
                color = subpassLoad(inputColor) + imageLoad(image1, ivec2(0));
            }
        )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    const VkDescriptorSetLayoutObj descriptor_set_layout2(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout, &descriptor_set_layout2});
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    OneOffDescriptorSet descriptor_set2(m_device,
                                        {
                                            {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                        });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstSet = descriptor_set2.set_;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &descriptor_set2.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, UpdateImageDescriptorSetThatHasImageViewUsage) {
    TEST_DESCRIPTION("Update a descriptor set with an image view that includes VkImageViewUsageCreateInfo");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    auto image_view_usage_ci = LvlInitStruct<VkImageViewUsageCreateInfo>();
    image_view_usage_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto image_view_ci = LvlInitStruct<VkImageViewCreateInfo>(&image_view_usage_ci);
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk_testing::ImageView image_view;
    image_view.init(*m_device, image_view_ci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler;
    sampler.init(*m_device, sampler_ci);

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, image_view.handle(), sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    ds.UpdateDescriptorSets();
}

TEST_F(PositiveDescriptors, MultipleThreadsUsingHostOnlyDescriptorSet) {
    TEST_DESCRIPTION("Test using host only descriptor set in multiple threads");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mutable_descriptor = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(mutable_descriptor);
    if (mutable_descriptor.mutableDescriptorType == VK_FALSE) {
        GTEST_SKIP() << "mutableDescriptorType feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image1(m_device);
    VkImageObj image2(m_device);
    image1.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    image2.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageView view1 = image1.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    VkImageView view2 = image2.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT, nullptr,
                                       VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT);

    const auto &testing_thread1 = [&]() {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view1;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
    };
    const auto &testing_thread2 = [&]() {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view2;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 1;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
    };

    std::array<std::thread, 2> threads = {std::thread(testing_thread1), std::thread(testing_thread2)};
    for (auto &t : threads) t.join();
}

TEST_F(PositiveDescriptors, BindingEmptyDescriptorSets) {
    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet empty_ds(m_device, {});
    const VkPipelineLayoutObj pipeline_layout(m_device, {&empty_ds.layout_});

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &empty_ds.set_, 0, nullptr);
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, DrawingWithUnboundUnusedSetWithInputAttachments) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with input attachment descriptors. "
        "The second descriptor set is unused and unbound. Its purpose is to catch regression of the following bug or similar "
        "issues when accessing unbound set: https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4576");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto width = static_cast<uint32_t>(m_viewports[0].width);
    const auto height = static_cast<uint32_t>(m_viewports[0].height);
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkImageObj image_input(m_device);
    image_input.Init(width, height, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    VkImageView view_input = image_input.targetView(format);

    // Create render pass with a subpass that has input attachment.
    vk_testing::RenderPass render_pass;
    {
        VkAttachmentDescription input_attachment = {};
        input_attachment.format = format;
        input_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        input_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        input_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        input_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        input_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        input_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        input_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        const VkAttachmentReference attachment_reference = {0, VK_IMAGE_LAYOUT_GENERAL};

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 1;
        subpass.pInputAttachments = &attachment_reference;

        auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
        rpci.attachmentCount = 1;
        rpci.pAttachments = &input_attachment;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;

        render_pass.init(*m_device, rpci);
        ASSERT_TRUE(render_pass.initialized());
    }

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &view_input;
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        void main() {
        vec4 color = subpassLoad(x);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    OneOffDescriptorSet descriptor_set(m_device, {binding});
    descriptor_set.WriteDescriptorImageInfo(0, view_input, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();
    const VkDescriptorSetLayoutObj ds_layout_unused(m_device, {binding});
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_, &ds_layout_unused});

    VkPipelineObj pipe(m_device);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    ASSERT_VK_SUCCESS(pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle()));

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderPass = render_pass.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // This draw command will likely produce a crash in case of a regression.
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, UpdateDescritorSetsNoLongerInUse) {
    TEST_DESCRIPTION("Use descriptor in the draw call and then update descriptor when it is no longer in use");
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj cb0(m_device, m_commandPool);
    VkCommandBufferObj cb1(m_device, m_commandPool);

    for (int mode = 0; mode < 2; mode++) {
        const bool use_single_command_buffer = (mode == 0);
        //
        // Create resources.
        //
        const VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2};
        auto descriptor_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
        descriptor_pool_ci.flags = 0;
        descriptor_pool_ci.maxSets = 2;
        descriptor_pool_ci.poolSizeCount = 1;
        descriptor_pool_ci.pPoolSizes = &pool_size;
        vk_testing::DescriptorPool pool(*m_device, descriptor_pool_ci);

        const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      nullptr};
        auto set_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
        set_layout_ci.flags = 0;
        set_layout_ci.bindingCount = 1;
        set_layout_ci.pBindings = &binding;
        vk_testing::DescriptorSetLayout set_layout(*m_device, set_layout_ci);

        VkDescriptorSet set_A = VK_NULL_HANDLE;
        VkDescriptorSet set_B = VK_NULL_HANDLE;
        {
            const VkDescriptorSetLayout set_layouts[2] = {set_layout, set_layout};
            auto set_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
            set_alloc_info.descriptorPool = pool;
            set_alloc_info.descriptorSetCount = 2;
            set_alloc_info.pSetLayouts = set_layouts;
            VkDescriptorSet sets[2] = {};
            ASSERT_VK_SUCCESS(vk::AllocateDescriptorSets(device(), &set_alloc_info, sets));
            set_A = sets[0];
            set_B = sets[1];
        }

        auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
        buffer_ci.size = 1024;
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        vk_testing::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

        auto pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &set_layout.handle();
        vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

        VkPipelineObj pipe(m_device);
        pipe.SetViewport(m_viewports);
        pipe.SetScissor(m_scissors);
        pipe.AddDefaultColorAttachment();
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);

        auto update_set = [this](VkDescriptorSet set, VkBuffer buffer) {
            VkDescriptorBufferInfo buffer_info = {};
            buffer_info.buffer = buffer;
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE;

            auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
            descriptor_write.dstSet = set;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.pBufferInfo = &buffer_info;
            vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
        };

        //
        // Test scenario.
        //
        update_set(set_A, buffer);
        update_set(set_B, buffer);

        // Bind set A to a command buffer and submit the command buffer;
        {
            auto &cb = use_single_command_buffer ? *m_commandBuffer : cb0;
            cb.begin();
            vk::CmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &set_A, 0, nullptr);
            vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
            cb.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdDraw(cb, 0, 0, 0, 0);
            vk::CmdEndRenderPass(cb);
            cb.end();
            auto submit_info = LvlInitStruct<VkSubmitInfo>();
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cb.handle();
            ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        }

        // Wait for the queue. After this set A should be no longer in use.
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));

        // Bind set B to a command buffer and submit the command buffer;
        {
            auto &cb = use_single_command_buffer ? *m_commandBuffer : cb1;
            cb.begin();
            vk::CmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &set_B, 0, nullptr);
            vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
            cb.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdDraw(cb, 0, 0, 0, 0);
            vk::CmdEndRenderPass(cb);
            cb.end();
            auto submit_info = LvlInitStruct<VkSubmitInfo>();
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cb.handle();
            ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        }

        // Update set A. It should not cause VU 03047 error.
        vk_testing::Buffer buffer2(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        update_set(set_A, buffer2);

        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    }
}

TEST_F(PositiveDescriptors, PushDescriptorTemplateBasic) {
    TEST_DESCRIPTION("Basic use of vkCmdPushDescriptorSetWithTemplateKHR");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    VkDescriptorSetLayoutObj push_dsl(m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    VkPipelineLayoutObj pipeline_layout(m_device, {&push_dsl});

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    auto update_template_ci = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfoKHR>();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout.handle();

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    vk::CreateDescriptorUpdateTemplateKHR(m_device->device(), &update_template_ci, nullptr, &update_template);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    m_commandBuffer->begin();
    vk::CmdPushDescriptorSetWithTemplateKHR(m_commandBuffer->handle(), update_template, pipeline_layout.handle(), 0,
                                            &update_template_data);
    m_commandBuffer->end();

    vk::DestroyDescriptorUpdateTemplateKHR(m_device->device(), update_template, nullptr);
}

TEST_F(PositiveDescriptors, UpdateAfterBind) {
    TEST_DESCRIPTION("Test UPDATE_AFTER_BIND does not reset command buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto descriptor_indexing = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeatures>();
    auto synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&descriptor_indexing);
    auto features2 = GetPhysicalDeviceFeatures2(synchronization2);
    if (descriptor_indexing.descriptorBindingStorageBufferUpdateAfterBind == VK_FALSE) {
        GTEST_SKIP() << "descriptorBindingStorageBufferUpdateAfterBind feature is not available";
    }
    if (synchronization2.synchronization2 == VK_FALSE) {
        GTEST_SKIP() << "synchronization2 feature is not available";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer buffer1, buffer2, buffer3;
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer1);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer2);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer3);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer1, &buffer_mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory memory1, memory2, memory3;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory1);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory2);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory3);

    vk::BindBufferMemory(device(), buffer1, memory1, 0);
    vk::BindBufferMemory(device(), buffer2, memory2, 0);
    vk::BindBufferMemory(device(), buffer3, memory3, 0);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    VkDescriptorBindingFlagsEXT flags[2] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, 0};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;
    OneOffDescriptorSet descriptor_set(m_device, binding_defs, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                                       &flags_create_info, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info = {buffer1, 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    descriptor_write.dstBinding = 1;
    buffer_info.buffer = buffer3;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstBinding = 0;

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer buf1 {
            float a;
        } ubuf1;
        layout (set = 0, binding = 1) buffer buf2 {
            float a;
        } ubuf2;
        void main() {
           float f = ubuf1.a * ubuf2.a;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyBuffer(device(), buffer1, nullptr);
    buffer_info.buffer = buffer2;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    auto cb_info = LvlInitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = m_commandBuffer->handle();

    auto submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyBuffer(device(), buffer2, nullptr);
    vk::DestroyBuffer(device(), buffer3, nullptr);

    vk::FreeMemory(device(), memory1, nullptr);
    vk::FreeMemory(device(), memory2, nullptr);
    vk::FreeMemory(device(), memory3, nullptr);
}

TEST_F(PositiveDescriptors, PartiallyBoundDescriptors) {
    TEST_DESCRIPTION("Test partially bound descriptors do not reset command buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto descriptor_indexing = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeatures>();
    auto synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&descriptor_indexing);
    auto features2 = GetPhysicalDeviceFeatures2(synchronization2);
    if (descriptor_indexing.descriptorBindingStorageBufferUpdateAfterBind == VK_FALSE) {
        GTEST_SKIP() << "descriptorBindingStorageBufferUpdateAfterBind feature is not available";
    }
    if (synchronization2.synchronization2 == VK_FALSE) {
        GTEST_SKIP() << "synchronization2 feature is not available";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer buffer1, buffer3;
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer1);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer3);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer1, &buffer_mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory memory1, memory3;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory1);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory3);

    vk::BindBufferMemory(device(), buffer1, memory1, 0);
    vk::BindBufferMemory(device(), buffer3, memory3, 0);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    VkDescriptorBindingFlagsEXT flags[2] = {VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, 0};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;
    OneOffDescriptorSet descriptor_set(m_device, binding_defs, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                                       &flags_create_info, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info = {buffer1, 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    descriptor_write.dstBinding = 1;
    buffer_info.buffer = buffer3;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstBinding = 0;

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer buf1 {
            float a;
        } ubuf1;
        layout (set = 0, binding = 1) buffer buf2 {
            float a;
        } ubuf2;
        void main() {
           float f = ubuf2.a;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyBuffer(device(), buffer1, nullptr);

    auto cb_info = LvlInitStruct<VkCommandBufferSubmitInfoKHR>();
    cb_info.commandBuffer = m_commandBuffer->handle();

    auto submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyBuffer(device(), buffer3, nullptr);

    vk::FreeMemory(device(), memory1, nullptr);
    vk::FreeMemory(device(), memory3, nullptr);
}
