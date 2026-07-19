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

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "descriptor_helper.h"
#include "pipeline_helper.h"
#include "shader_helper.h"

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