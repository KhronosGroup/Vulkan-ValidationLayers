/*
 * Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "generated/vk_function_pointers.h"
#include "pipeline_helper.h"

class UndefinedCore : public VkLayerTest {};

TEST_F(UndefinedCore, BindInvalidPipelineLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6621");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    const char* fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=1) uniform foo1 { vec4 y; };
        void main(){
           x = y;
        }
    )glsl";
    VkShaderObj vs(*m_device, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(*m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    // consume VU so we create a pipeline handle
    // Will crash various drivers
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGraphicsPipelineCreateInfo-layout-07988");
    pipe.CreateGraphicsPipeline();
    if (pipe == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Driver failed to create a invalid pipeline handle";
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(UndefinedCore, RetireSignaledFence) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/11531");
    RETURN_IF_SKIP(Init());

    vkt::Fence fence(*m_device);
    m_default_queue->Submit(vkt::no_cmd, fence);
    m_default_queue->Wait();

    // This allows to run PreRecord during Submit for already signaled fence
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkQueueSubmit-fence-00063");
    m_default_queue->Submit(vkt::no_cmd, fence);

    // In the original issue this called Fence::Retire for the fence that was not reset (by vkResetFences)
    // so it had its std::promise set, which led to crash during retire (promise was double set).
    // The fix ensures that std::promise is always re-initialized when the fence is enqueued.
    m_default_queue->Wait();
}

TEST_F(UndefinedCore, DescriptorArrayLayoutMismatchDraw) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12294");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::DescriptorSetLayout ds_layout(*m_device,
                                       {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();
    VkDescriptorSet ds = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, vk::AllocateDescriptorSets(*m_device, &ds_alloc_info, &ds));

    const char* fs_source = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform sampler2D tex[10000];
        void main(){
           color = texture(tex[0], vec2(0.0));
        }
    )glsl";
    VkShaderObj fs(*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGraphicsPipelineCreateInfo-layout-07991");
    pipe.CreateGraphicsPipeline();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Image image(*m_device, 128, 128, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkDescriptorImageInfo image_info = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = ds;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &write, 0, nullptr);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &ds, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(UndefinedCore, DescriptorUpdateUndeclaredBinding) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/13579");
    RETURN_IF_SKIP(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(3, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkWriteDescriptorSet-dstBinding-00315");
    descriptor_set.UpdateDescriptorSets();

    descriptor_set.Clear();
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkWriteDescriptorSet-dstBinding-00316");
    descriptor_set.UpdateDescriptorSets();

    VkCopyDescriptorSet copy = vku::InitStructHelper();
    copy.srcSet = descriptor_set.set_;
    copy.srcBinding = 1;
    copy.dstSet = descriptor_set.set_;
    copy.dstBinding = 2;
    copy.descriptorCount = 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCopyDescriptorSet-srcBinding-00345");
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy);

    copy.srcBinding = 0;
    copy.dstBinding = 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCopyDescriptorSet-dstBinding-00347");
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy);
}

TEST_F(UndefinedCore, DescriptorCopyUpdateDescriptorCountOverrunsLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/13595");
    RETURN_IF_SKIP(Init());
    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_SAMPLER, 4};
    VkDescriptorPoolCreateInfo pool_ci = vku::InitStructHelper();
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = 1;
    pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, pool_ci);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo layout_ci = vku::InitStructHelper();
    layout_ci.bindingCount = 1;
    layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vk::CreateDescriptorSetLayout(device(), &layout_ci, nullptr, &ds_layout);

    VkDescriptorSetAllocateInfo alloc_ci = vku::InitStructHelper();
    alloc_ci.descriptorPool = pool.handle();
    alloc_ci.descriptorSetCount = 1;
    alloc_ci.pSetLayouts = &ds_layout;
    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(device(), &alloc_ci, &ds);

    VkCopyDescriptorSet copy = vku::InitStructHelper();
    copy.srcSet = ds;
    copy.srcBinding = 0;
    copy.dstSet = ds;
    copy.dstBinding = 0;
    copy.descriptorCount = 2;  // binding 0 declares exactly one descriptor
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCopyDescriptorSet-srcArrayElement-00346");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCopyDescriptorSet-dstArrayElement-00348");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkCopyDescriptorSet-srcSet-00349");
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &copy);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
}

TEST_F(UndefinedCore, VariableDescriptorCountImmutableSamplerOverrun) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/13596");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    RETURN_IF_SKIP(Init());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    VkSampler immutable_sampler = sampler.handle();

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci = vku::InitStructHelper();
    binding_flags_ci.bindingCount = 1;
    binding_flags_ci.pBindingFlags = &binding_flags;

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                            &immutable_sampler};
    VkDescriptorSetLayoutCreateInfo layout_ci = vku::InitStructHelper(&binding_flags_ci);
    layout_ci.bindingCount = 1;
    layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vk::CreateDescriptorSetLayout(device(), &layout_ci, nullptr, &ds_layout);

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8};
    VkDescriptorPoolCreateInfo pool_ci = vku::InitStructHelper();
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = 1;
    pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, pool_ci);

    uint32_t variable_count = 8;  // layout declared descriptorCount of 1
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_ci = vku::InitStructHelper();
    variable_count_ci.descriptorSetCount = 1;
    variable_count_ci.pDescriptorCounts = &variable_count;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-09380");
    VkDescriptorSetAllocateInfo alloc_ci = vku::InitStructHelper(&variable_count_ci);
    alloc_ci.descriptorPool = pool.handle();
    alloc_ci.descriptorSetCount = 1;
    alloc_ci.pSetLayouts = &ds_layout;
    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(device(), &alloc_ci, &ds);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
}
