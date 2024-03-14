/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

#include "generated/vk_extension_helper.h"
#include "../framework/ray_tracing_objects.h"

TEST_F(PositiveDescriptors, CopyNonupdatedDescriptors) {
    TEST_DESCRIPTION("Copy non-updated descriptors");
    unsigned int i;

    RETURN_IF_SKIP(Init());
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
        copy_ds_update[i] = vku::InitStructHelper();
        copy_ds_update[i].srcSet = src_descriptor_set.set_;
        copy_ds_update[i].srcBinding = i;
        copy_ds_update[i].dstSet = dst_descriptor_set.set_;
        copy_ds_update[i].dstBinding = i;
        copy_ds_update[i].descriptorCount = 1;
    }
    vk::UpdateDescriptorSets(device(), 0, NULL, copy_size, copy_ds_update);
}

TEST_F(PositiveDescriptors, DeleteDescriptorSetLayoutsBeforeDescriptorSets) {
    TEST_DESCRIPTION("Create DSLayouts and DescriptorSets and then delete the DSLayouts before the DescriptorSets.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    VkResult err;

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool_one(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorSet descriptorSet;
    {
        const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

        VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = ds_pool_one.handle();
        alloc_info.pSetLayouts = &ds_layout.handle();
        err = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptorSet);
        ASSERT_EQ(VK_SUCCESS, err);
    }  // ds_layout destroyed
    vk::FreeDescriptorSets(device(), ds_pool_one.handle(), 1, &descriptorSet);
}

TEST_F(PositiveDescriptors, PoolSizeCountZero) {
    TEST_DESCRIPTION("Allow poolSizeCount to zero.");
    RETURN_IF_SKIP(Init());

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 0;
    vkt::DescriptorPool ds_pool_one(*m_device, ds_pool_ci);
}

TEST_F(PositiveDescriptors, IgnoreUnrelatedDescriptor) {
    TEST_DESCRIPTION(
        "Ensure that the vkUpdateDescriptorSets validation code is ignoring VkWriteDescriptorSet members that are not related to "
        "the descriptor type specified by VkWriteDescriptorSet::descriptorType.  Correct validation behavior will result in the "
        "test running to completion without validation errors.");

    const uintptr_t invalid_ptr = 0xcdcdcdcd;

    RETURN_IF_SKIP(Init());

    // Verify VK_FORMAT_R8_UNORM supports VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    const VkFormat format_texel_case = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), format_texel_case, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Test requires to support VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT";
    }

    // Image Case
    {
        vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::ImageView view = image.CreateView();

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    }

    // Buffer Case
    {
        uint32_t queue_family_index = 0;
        VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        vkt::Buffer buffer(*m_device, buffer_create_info);

        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = 1024;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    }

    // Texel Buffer Case
    {
        uint32_t queue_family_index = 0;
        VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;

        vkt::Buffer buffer(*m_device, buffer_create_info);

        VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
        buff_view_ci.buffer = buffer.handle();
        buff_view_ci.format = format_texel_case;
        buff_view_ci.range = VK_WHOLE_SIZE;
        vkt::BufferView buffer_view(*m_device, buff_view_ci);

        OneOffDescriptorSet descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           });

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    }
}

TEST_F(PositiveDescriptors, ImmutableSamplerOnlyDescriptor) {
    TEST_DESCRIPTION("Bind a DescriptorSet with only an immutable sampler and make sure that we don't warn for no update.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                 });

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

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

    RETURN_IF_SKIP(Init());

    // Create layout with two uniform buffer descriptors w/ empty binding between them
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                         {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 /*!*/, 0, nullptr},
                                         {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });

    // Create a buffer to be used for update
    VkBufferCreateInfo buff_ci = vku::InitStructHelper();
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = 256;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, buff_ci);

    // Only update the descriptor at binding 2
    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    buff_info.offset = 0;
    buff_info.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = ds.set_;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
}

TEST_F(PositiveDescriptors, DynamicOffsetWithInactiveBinding) {
    // Create a descriptorSet w/ dynamic descriptors where 1 binding is inactive
    // We previously had a bug where dynamic offset of inactive bindings was still being used

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    // Create two buffers to update the descriptors with
    // The first will be 2k and used for bindings 0 & 1, the second is 1k for binding 2
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = vku::InitStructHelper();
    buffCI.size = 2048;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    vkt::Buffer dynamic_uniform_buffer_1, dynamic_uniform_buffer_2;
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

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = BINDING_COUNT;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor_write.pBufferInfo = buff_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);

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
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // This update should succeed, but offset of inactive binding 1 oversteps binding 2 buffer size
    //   we used to have a bug in this case.
    uint32_t dyn_off[BINDING_COUNT] = {0, 1024, 256};
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, BINDING_COUNT, dyn_off);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, CopyMutableDescriptors) {
    TEST_DESCRIPTION("Copy mutable descriptors.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_lists[2] = {};
    mutable_descriptor_type_lists[0].descriptorTypeCount = 2;
    mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
    mutable_descriptor_type_lists[1].descriptorTypeCount = 0;
    mutable_descriptor_type_lists[1].pDescriptorTypes = nullptr;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 2;
    mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 2;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

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

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;

    vkt::DescriptorSetLayout set_layout(*m_device, create_info);
    VkDescriptorSetLayout set_layout_handle = set_layout.handle();

    VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = layouts;

    VkDescriptorSet descriptor_sets[2];
    VkResult result = vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
        GTEST_SKIP() << "Pool memory not allocated";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vkt::Buffer buffer(*m_device, buffer_ci);

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = buffer_ci.size;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCopyDescriptorSet copy_set = vku::InitStructHelper();
    copy_set.srcSet = descriptor_sets[0];
    copy_set.srcBinding = 0;
    copy_set.dstSet = descriptor_sets[1];
    copy_set.dstBinding = 1;
    copy_set.descriptorCount = 1;

    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
}

TEST_F(PositiveDescriptors, CopyAccelerationStructureMutableDescriptors) {
    TEST_DESCRIPTION("Copy acceleration structure descriptor in a mutable descriptor.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor_type_features = vku::InitStructHelper();
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_struct_features = vku::InitStructHelper(&mutable_descriptor_type_features);
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper(&acc_struct_features);
    GetPhysicalDeviceFeatures2(bda_features);
    RETURN_IF_SKIP(InitState(nullptr, &bda_features));

    std::array descriptor_types = {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR};

    VkMutableDescriptorTypeListEXT mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = descriptor_types.size();
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types.data();

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    pool_sizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper(&mdtci);
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = pool_sizes.size();
    ds_pool_ci.pPoolSizes = pool_sizes.data();

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

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

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper(&mdtci);
    create_info.bindingCount = bindings.size();
    create_info.pBindings = bindings.data();

    vkt::DescriptorSetLayout set_layout(*m_device, create_info);

    std::array<VkDescriptorSetLayout, 2> layouts = {set_layout.handle(), set_layout.handle()};

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = layouts.size();
    allocate_info.pSetLayouts = layouts.data();

    std::array<VkDescriptorSet, layouts.size()> descriptor_sets;
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets.data());

    auto tlas = vkt::as::blueprint::AccelStructSimpleOnDeviceTopLevel(*m_device, 4096);
    tlas->Build();

    VkWriteDescriptorSetAccelerationStructureKHR blas_descriptor = vku::InitStructHelper();
    blas_descriptor.accelerationStructureCount = 1;
    blas_descriptor.pAccelerationStructures = &tlas->handle();

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = blas_descriptor.accelerationStructureCount;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    descriptor_write.pNext = &blas_descriptor;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCopyDescriptorSet copy_set = vku::InitStructHelper();
    copy_set.srcSet = descriptor_sets[0];
    copy_set.srcBinding = 0;
    copy_set.dstSet = descriptor_sets[1];
    copy_set.dstBinding = 1;
    copy_set.descriptorCount = 1;

    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy_set);
}

TEST_F(PositiveDescriptors, ImageViewAsDescriptorReadAndInputAttachment) {
    TEST_DESCRIPTION("Test reading from a descriptor that uses same image view as framebuffer input attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView image_view(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view_handle, width, height);

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0, rgba8) readonly uniform image2D image1;
            layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput inputColor;
            void main(){
                color = subpassLoad(inputColor) + imageLoad(image1, ivec2(0));
            }
        )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const vkt::DescriptorSetLayout descriptor_set_layout(*m_device, {layout_binding});
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    const vkt::DescriptorSetLayout descriptor_set_layout2(*m_device, {layout_binding});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_layout, &descriptor_set_layout2});
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

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
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstSet = descriptor_set2.set_;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp.Handle(), framebuffer.handle(), width, height, 1, m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &descriptor_set2.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, UpdateImageDescriptorSetThatHasImageViewUsage) {
    TEST_DESCRIPTION("Update a descriptor set with an image view that includes VkImageViewUsageCreateInfo");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkImageViewUsageCreateInfo image_view_usage_ci = vku::InitStructHelper();
    image_view_usage_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&image_view_usage_ci);
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

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
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    vkt::Image image1(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image2(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    vkt::ImageView view1 = image1.CreateView();
    vkt::ImageView view2 = image2.CreateView();

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

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    };
    const auto &testing_thread2 = [&]() {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = view2;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 1;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_write.pImageInfo = &image_info;

        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    };

    std::array<std::thread, 2> threads = {std::thread(testing_thread1), std::thread(testing_thread2)};
    for (auto &t : threads) t.join();
}

TEST_F(PositiveDescriptors, BindingEmptyDescriptorSets) {
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet empty_ds(m_device, {});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&empty_ds.layout_});

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

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t width = m_width;
    const uint32_t height = m_height;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    vkt::Image image_input(*m_device, width, height, 1, format, usage);
    image_input.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view_input = image_input.CreateView();

    // Create render pass with a subpass that has input attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view_input.handle(), width, height);

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        void main() {
        vec4 color = subpassLoad(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    OneOffDescriptorSet descriptor_set(m_device, {binding});
    descriptor_set.WriteDescriptorImageInfo(0, view_input, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();
    const vkt::DescriptorSetLayout ds_layout_unused(*m_device, {binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_, &ds_layout_unused});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderPass = rp.Handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // This draw command will likely produce a crash in case of a regression.
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, UpdateDescritorSetsNoLongerInUse) {
    TEST_DESCRIPTION("Use descriptor in the draw call and then update descriptor when it is no longer in use");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer cb0(*m_device, m_commandPool);
    vkt::CommandBuffer cb1(*m_device, m_commandPool);

    for (int mode = 0; mode < 2; mode++) {
        const bool use_single_command_buffer = (mode == 0);
        //
        // Create resources.
        //
        const VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2};
        VkDescriptorPoolCreateInfo descriptor_pool_ci = vku::InitStructHelper();
        descriptor_pool_ci.flags = 0;
        descriptor_pool_ci.maxSets = 2;
        descriptor_pool_ci.poolSizeCount = 1;
        descriptor_pool_ci.pPoolSizes = &pool_size;
        vkt::DescriptorPool pool(*m_device, descriptor_pool_ci);

        const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      nullptr};
        VkDescriptorSetLayoutCreateInfo set_layout_ci = vku::InitStructHelper();
        set_layout_ci.flags = 0;
        set_layout_ci.bindingCount = 1;
        set_layout_ci.pBindings = &binding;
        vkt::DescriptorSetLayout set_layout(*m_device, set_layout_ci);

        VkDescriptorSet set_A = VK_NULL_HANDLE;
        VkDescriptorSet set_B = VK_NULL_HANDLE;
        {
            const VkDescriptorSetLayout set_layouts[2] = {set_layout, set_layout};
            VkDescriptorSetAllocateInfo set_alloc_info = vku::InitStructHelper();
            set_alloc_info.descriptorPool = pool;
            set_alloc_info.descriptorSetCount = 2;
            set_alloc_info.pSetLayouts = set_layouts;
            VkDescriptorSet sets[2] = {};
            ASSERT_EQ(VK_SUCCESS, vk::AllocateDescriptorSets(device(), &set_alloc_info, sets));
            set_A = sets[0];
            set_B = sets[1];
        }

        VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
        buffer_ci.size = 1024;
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        vkt::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkShaderObj fs(this, kFragmentUniformGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &set_layout.handle();
        vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_[1] = fs.GetStageCreateInfo();
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        auto update_set = [this](VkDescriptorSet set, VkBuffer buffer) {
            VkDescriptorBufferInfo buffer_info = {};
            buffer_info.buffer = buffer;
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE;

            VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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
            vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            cb.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdDraw(cb, 0, 0, 0, 0);
            vk::CmdEndRenderPass(cb);
            cb.end();
            m_default_queue->submit(cb);
        }

        // Wait for the queue. After this set A should be no longer in use.
        m_default_queue->wait();

        // Bind set B to a command buffer and submit the command buffer;
        {
            auto &cb = use_single_command_buffer ? *m_commandBuffer : cb1;
            cb.begin();
            vk::CmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &set_B, 0, nullptr);
            vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            cb.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdDraw(cb, 0, 0, 0, 0);
            vk::CmdEndRenderPass(cb);
            cb.end();
            m_default_queue->submit(cb);
        }

        // Update set A. It should not cause VU 03047 error.
        vkt::Buffer buffer2(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        update_set(set_A, buffer2);

        m_default_queue->wait();
    }
}

TEST_F(PositiveDescriptors, DSUsageBitsFlags2) {
    TEST_DESCRIPTION(
        "Attempt to update descriptor sets for buffers that do not have correct usage bits sets with VkBufferUsageFlagBits2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    const VkFormat buffer_format = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), buffer_format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT for this format";
    }

    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT_KHR;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&buffer_usage_flags);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;  // would be wrong, but ignored
    vkt::Buffer buffer(*m_device, buffer_create_info);

    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = buffer_format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, buff_view_ci);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view.handle();
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = nullptr;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

TEST_F(PositiveDescriptors, AttachmentFeedbackLoopLayout) {
    TEST_DESCRIPTION("Read from image with layout attachment feedback loop");

    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT afll_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(afll_features);
    if (!afll_features.attachmentFeedbackLoopLayout) {
        GTEST_SKIP() << "attachmentFeedbackLoopLayout not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &afll_features));

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(
        *m_device, 32, 32, 1, format,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);

    vkt::ImageView image_view = image.CreateView();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,
                                VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                            VK_DEPENDENCY_BY_REGION_BIT | VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT);
    rp.CreateRenderPass();

    VkClearValue clear_value;
    clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view.handle());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    descriptor_set.WriteDescriptorImageInfo(0u, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT);
    descriptor_set.UpdateDescriptorSets();

    char const *frag_src = R"glsl(
        #version 450
        layout(set=0) layout(binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 color;
        void main(){
           color = texture(tex, vec2(0.5f));
        }
    )glsl";
    VkShaderObj fs(this, frag_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, &clear_value);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3u, 1u, 0u, 0u);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(PositiveDescriptors, VariableDescriptorCount) {
    TEST_DESCRIPTION("Allocate descriptors with variable count.");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // This test is valid for Vulkan 1.0 only -- skip if device has an API version greater than 1.0.
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // Create Pool w/ 1 Sampler descriptor, but try to alloc Uniform Buffer
    // descriptor from it
    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    pool_sizes[0].descriptorCount = 2;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 3;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 3;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo dsl_binding_flags = vku::InitStructHelper();
    dsl_binding_flags.bindingCount = 1u;
    dsl_binding_flags.pBindingFlags = &binding_flags;

    VkDescriptorSetLayoutCreateInfo dsl_ci = vku::InitStructHelper(&dsl_binding_flags);
    dsl_ci.bindingCount = 1u;
    dsl_ci.pBindings = &dsl_binding;

    const vkt::DescriptorSetLayout ds_layout(*m_device, dsl_ci);

    uint32_t descriptor_count = 1u;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_allocate = vku::InitStructHelper();
    variable_allocate.descriptorSetCount = 1u;
    variable_allocate.pDescriptorCounts = &descriptor_count;
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper(&variable_allocate);
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_set);
}

TEST_F(PositiveDescriptors, ShaderStageAll) {
    TEST_DESCRIPTION("VkDescriptorSetLayout stageFlags can be VK_SHADER_STAGE_ALL");

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;
    vkt::DescriptorSetLayout(*m_device, ds_layout_ci);
}