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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

TEST_F(PositivePushDescriptor, NullDstSet) {
    TEST_DESCRIPTION("Use null dstSet in CmdPushDescriptorSetKHR");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    // Create push descriptor set layout
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.InitState();
    helper.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &ds_layout});
    helper.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float) * 3;
    vkt::Buffer vbo(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = data_size;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

TEST_F(PositivePushDescriptor, UnboundSet) {
    TEST_DESCRIPTION("Ensure that no validation errors are produced for not bound push descriptor sets");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

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
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

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
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    // Now use the descriptor layouts to create a pipeline layout
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float);
    vkt::Buffer buffer(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Update descriptor set
    descriptor_set.WriteDescriptorBufferInfo(2, buffer.handle(), 0, data_size);
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

TEST_F(PositivePushDescriptor, SetUpdatingSetNumber) {
    TEST_DESCRIPTION(
        "Ensure that no validation errors are produced when the push descriptor set number changes "
        "between two vk::CmdPushDescriptorSetKHR calls.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    // Create a descriptor to push
    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    const VkDescriptorSetLayoutBinding ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const VkDescriptorSetLayoutBinding ds_binding_1 = {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       nullptr};
    const vkt::DescriptorSetLayout ds_layout(*m_device, {ds_binding_0, ds_binding_1});
    ASSERT_TRUE(ds_layout.initialized());

    const VkDescriptorSetLayoutBinding push_ds_binding_0 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                            nullptr};
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {push_ds_binding_0},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    ASSERT_TRUE(push_ds_layout.initialized());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    CreatePipelineHelper pipe0(*this);
    CreatePipelineHelper pipe1(*this);
    {
        // Note: the push descriptor set is set number 2.
        const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout, &ds_layout, &push_ds_layout, &ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=2) layout(binding=0) uniform foo { vec4 y; } bar;
            void main(){
               x = bar.y;
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe0.InitState();
        pipe0.shader_stages_[1] = fs.GetStageCreateInfo();
        pipe0.gp_ci_.layout = pipeline_layout.handle();
        pipe0.CreateGraphicsPipeline();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe0.Handle());

        const VkWriteDescriptorSet descriptor_write =
            vkt::Device::write_descriptor_set(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        // Note: pushing to desciptor set number 2.
        vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1,
                                    &descriptor_write);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    {
        // Note: the push descriptor set is now set number 3.
        const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout, &ds_layout, &ds_layout, &push_ds_layout});
        ASSERT_TRUE(pipeline_layout.initialized());

        const VkWriteDescriptorSet descriptor_write =
            vkt::Device::write_descriptor_set(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=3) layout(binding=0) uniform foo { vec4 y; } bar;
            void main(){
               x = bar.y;
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe1.InitState();
        pipe1.shader_stages_[1] = fs.GetStageCreateInfo();
        pipe1.gp_ci_.layout = pipeline_layout.handle();
        pipe1.CreateGraphicsPipeline();

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());

        // Note: now pushing to desciptor set number 3.
        vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 3, 1,
                                    &descriptor_write);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositivePushDescriptor, CreateDescriptorSetBindingWithIgnoredSamplers) {
    TEST_DESCRIPTION("Test that layers conditionally do ignore the pImmutableSamplers on vkCreateDescriptorSetLayout");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
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
            vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0u, size32(non_sampler_bindings), non_sampler_bindings);
        vkt::DescriptorSetLayout dsl(*m_device, dslci);
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
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(
            nullptr, static_cast<VkDescriptorSetLayoutCreateFlags>(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR),
            size32(non_sampler_bindings), non_sampler_bindings);
        vkt::DescriptorSetLayout dsl(*m_device, dslci);
    }
}

TEST_F(PositivePushDescriptor, ImmutableSampler) {
    TEST_DESCRIPTION("Use a push descriptor with an immutable sampler.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);
    VkSampler sampler_handle = sampler.handle();

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &sampler_handle}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler_handle;
    img_info.imageView = imageView;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
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

TEST_F(PositivePushDescriptor, TemplateBasic) {
    TEST_DESCRIPTION("Basic use of vkCmdPushDescriptorSetWithTemplateKHR");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

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

    VkDescriptorUpdateTemplateCreateInfoKHR update_template_ci = vku::InitStructHelper();
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

TEST_F(PositivePushDescriptor, WriteDescriptorSetNotAllocated) {
    TEST_DESCRIPTION("Try to update a descriptor that has yet to be allocated and make sure its ignored");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDescriptorSetLayoutBinding ds_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    VkDescriptorSetLayoutCreateInfo dsl_ci = vku::InitStructHelper();
    dsl_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &ds_binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, dsl_ci);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    // Valid, from spec:
    // "Each element of pDescriptorWrites is interpreted as in VkWriteDescriptorSet, except the dstSet member is ignored"
    VkDescriptorSet bad_set = CastFromUint64<VkDescriptorSet>(0xcadecade);

    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, 32};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = bad_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    m_commandBuffer->begin();
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                &descriptor_write);

    VkDescriptorSet null_set = CastFromUint64<VkDescriptorSet>(0);
    descriptor_write.dstSet = null_set;
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                &descriptor_write);
    m_commandBuffer->end();
}

TEST_F(PositivePushDescriptor, PushDescriptorWithTemplateMultipleSets) {
    TEST_DESCRIPTION("Basic use of vkCmdPushDescriptorSetWithTemplateKHR");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test crashes on AMD.";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
    };
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl1(*m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::DescriptorSetLayout push_dsl2(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl1, &push_dsl2});

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 1;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfoKHR update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout.handle();
    update_template_ci.set = 1;

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    vk::CreateDescriptorUpdateTemplateKHR(m_device->device(), &update_template_ci, nullptr, &update_template);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    m_commandBuffer->begin();
    vk::CmdPushDescriptorSetWithTemplateKHR(m_commandBuffer->handle(), update_template, pipeline_layout.handle(), 1,
                                            &update_template_data);
    m_commandBuffer->end();

    vk::DestroyDescriptorUpdateTemplateKHR(m_device->device(), update_template, nullptr);
}
