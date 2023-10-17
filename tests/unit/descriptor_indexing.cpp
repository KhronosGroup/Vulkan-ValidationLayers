/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativeDescriptorIndexing, UpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    descriptor_indexing_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(descriptor_indexing_features);

    descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    if (!descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    if (!m_device->phy().features().fragmentStoresAndAtomics) {
        GTEST_SKIP() << "Test requires (unsupported) fragmentStoresAndAtomics";
    }

    VkDescriptorBindingFlagsEXT flags[3] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                                            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    // Descriptor set has two bindings - only the second is update_after_bind
    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes[3] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
        {binding[2].descriptorType, binding[2].descriptorCount},
    };
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    dspci.poolSizeCount = 3;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet ds = VK_NULL_HANDLE;
    VkResult err = vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    ASSERT_EQ(VK_SUCCESS, err);

    VkBufferCreateInfo buffCI = vku::InitStructHelper();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vkt::Buffer dynamic_uniform_buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer.handle();
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;

    VkWriteDescriptorSet descriptor_write[2] = {};
    descriptor_write[0] = vku::InitStructHelper();
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = buffInfo;
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 1;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    // Create a dummy pipeline, since VL inspects which bindings are actually used at draw time
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform foo0 { float x0; } bar0;
        layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;
        layout(set=0, binding=2) buffer  foo2 { float x2; } bar2;
        void main(){
           color = vec4(bar0.x0 + bar1.x1 + bar2.x2);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // Make both bindings valid before binding to the command buffer
    vk::UpdateDescriptorSets(m_device->device(), 2, &descriptor_write[0], 0, NULL);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Two subtests. First only updates the update_after_bind binding and expects
    // no error. Second updates the other binding and expects an error when the
    // command buffer is ended.
    for (uint32_t i = 0; i < 2; ++i) {
        m_commandBuffer->begin();

        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds,
                                  0, NULL);

        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdDraw(m_commandBuffer->handle(), 0, 0, 0, 0);
        m_commandBuffer->EndRenderPass();

        // Valid to update binding 1 after being bound
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[1], 0, NULL);

        if (i == 0) {
            // expect no errors
            m_commandBuffer->end();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08114");
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
            vk::QueueWaitIdle(m_default_queue);
        } else {
            // Invalid to update binding 0 after being bound. But the error is actually
            // generated during vk::EndCommandBuffer
            vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[0], 0, NULL);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00059");

            vk::EndCommandBuffer(m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorIndexing, SetNonIdenticalWrite) {
    TEST_DESCRIPTION("VkWriteDescriptorSet must have identical VkDescriptorBindingFlagBits");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    descriptor_indexing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(descriptor_indexing_features);

    if (!descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    RETURN_IF_SKIP(InitState(nullptr, &descriptor_indexing_features));

    // not all identical VkDescriptorBindingFlags flags
    VkDescriptorBindingFlags flags[3] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, 0,
                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3};
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_sizes;
    dspci.maxSets = 3;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();
    VkDescriptorSet ds = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds));

    VkBufferCreateInfo buff_create_info = vku::InitStructHelper();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buff_create_info);

    VkDescriptorBufferInfo bufferInfo[3] = {};
    bufferInfo[0].buffer = buffer.handle();
    bufferInfo[0].offset = 0;
    bufferInfo[0].range = 1024;
    bufferInfo[1] = bufferInfo[0];
    bufferInfo[2] = bufferInfo[0];

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = bufferInfo;
    // If the dstBinding has fewer than descriptorCount, remainder will be used to update the subsequent binding
    descriptor_write.descriptorCount = 3;

    // binding 1 has a different VkDescriptorBindingFlags
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorIndexing, SetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create an update_after_bind set layout without loading the needed extension.");
    RETURN_IF_SKIP(Init())

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-ExtensionNotEnabled");
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorIndexing, SetLayout) {
    TEST_DESCRIPTION("Exercise various create/allocate-time errors related to VK_EXT_descriptor_indexing.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    descriptor_indexing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(descriptor_indexing_features);

    descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &descriptor_indexing_features));

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {
        {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT}};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = size32(flags);
    flags_create_info.pBindingFlags = nullptr;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-parameter");
        vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }

    {
        // VU for VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::bindingCount
        flags_create_info.pBindingFlags = flags.data();
        flags_create_info.bindingCount = 2;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-bindingCount-03002");
        vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }

    flags_create_info.bindingCount = 1;

    {
        // set is missing UPDATE_AFTER_BIND_POOL flag.
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000");
        // binding uses a feature we disabled
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformBufferUpdateAfterBind-03005");
        vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        ds_layout_ci.bindingCount = 0;
        flags_create_info.bindingCount = 0;
        vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

        VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
        VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
        dspci.poolSizeCount = 1;
        dspci.pPoolSizes = &pool_size;
        dspci.maxSets = 1;
        vkt::DescriptorPool pool(*m_device, dspci);

        VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
        ds_alloc_info.descriptorPool = pool.handle();
        ds_alloc_info.descriptorSetCount = 1;
        ds_alloc_info.pSetLayouts = &ds_layout.handle();

        VkDescriptorSet ds = VK_NULL_HANDLE;
        // mismatch between descriptor set and pool
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044");
        vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
        m_errorMonitor->VerifyFound();
    }

    if (descriptor_indexing_features.descriptorBindingVariableDescriptorCount) {
        VkDescriptorPoolSize pool_size = {binding.descriptorType, 3};
        VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
        dspci.poolSizeCount = 1;
        dspci.pPoolSizes = &pool_size;
        dspci.maxSets = 2;
        vkt::DescriptorPool pool(*m_device, dspci);
        {
            ds_layout_ci.flags = 0;
            ds_layout_ci.bindingCount = 1;
            flags_create_info.bindingCount = 1;
            flags[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

            VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_alloc_info = vku::InitStructHelper();
            count_alloc_info.descriptorSetCount = 1;
            // Set variable count larger than what was in the descriptor binding
            uint32_t variable_count = 2;
            count_alloc_info.pDescriptorCounts = &variable_count;

            VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(&count_alloc_info);
            ds_alloc_info.descriptorPool = pool.handle();
            ds_alloc_info.descriptorSetCount = 1;
            ds_alloc_info.pSetLayouts = &ds_layout.handle();

            VkDescriptorSet ds = VK_NULL_HANDLE;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-pSetLayouts-03046");
            vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
            m_errorMonitor->VerifyFound();
        }
        {
            // Now update descriptor set with a size that falls within the descriptor set layout size but that is more than the
            // descriptor set size
            binding.descriptorCount = 3;
            vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

            VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_alloc_info = vku::InitStructHelper();
            count_alloc_info.descriptorSetCount = 1;
            uint32_t variable_count = 2;
            count_alloc_info.pDescriptorCounts = &variable_count;

            VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(&count_alloc_info);
            ds_alloc_info.descriptorPool = pool.handle();
            ds_alloc_info.descriptorSetCount = 1;
            ds_alloc_info.pSetLayouts = &ds_layout.handle();

            VkDescriptorSet ds;
            VkResult err = vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
            ASSERT_EQ(VK_SUCCESS, err);
            vkt::Buffer buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            VkDescriptorBufferInfo buffer_info[3] = {};
            for (int i = 0; i < 3; i++) {
                buffer_info[i].buffer = buffer.handle();
                buffer_info[i].offset = 0;
                buffer_info[i].range = 128 * 128;
            }
            VkWriteDescriptorSet descriptor_writes[1] = {};
            descriptor_writes[0] = vku::InitStructHelper();
            descriptor_writes[0].dstSet = ds;
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].descriptorCount = 3;
            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].pBufferInfo = buffer_info;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
            vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorIndexing, SetLayoutBindings) {
    TEST_DESCRIPTION("Create descriptor set layout with incompatible bindings.");
    RETURN_IF_SKIP(InitBasicDescriptorIndexing())

    if (!descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    VkDescriptorSetLayoutBinding update_binding = {};
    update_binding.binding = 0;
    update_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_binding.descriptorCount = 1;
    update_binding.stageFlags = VK_SHADER_STAGE_ALL;
    update_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding dynamic_binding = {};
    dynamic_binding.binding = 1;
    dynamic_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    dynamic_binding.descriptorCount = 1;
    dynamic_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dynamic_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[2] = {update_binding, dynamic_binding};

    VkDescriptorBindingFlags flags[2] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, 0};

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;

    VkDescriptorSetLayoutCreateInfo create_info = vku::InitStructHelper();
    create_info.pNext = &flags_create_info;
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001");
    VkDescriptorSetLayout setLayout;
    vk::CreateDescriptorSetLayout(m_device->handle(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}
