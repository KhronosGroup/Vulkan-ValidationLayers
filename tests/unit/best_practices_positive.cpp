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

class VkPositiveBestPracticesLayerTest : public VkBestPracticesLayerTest {};

TEST_F(VkPositiveBestPracticesLayerTest, TestDestroyFreeNullHandles) {
    VkResult err;

    TEST_DESCRIPTION("Call all applicable destroy and free routines with NULL handles, expecting no validation errors");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vk::DestroyBuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyBufferView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyCommandPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDescriptorSetLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyDevice(VK_NULL_HANDLE, NULL);
    vk::DestroyEvent(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFence(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyFramebuffer(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImage(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyImageView(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyInstance(VK_NULL_HANDLE, NULL);
    vk::DestroyPipeline(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineCache(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyPipelineLayout(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyQueryPool(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyRenderPass(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySampler(device(), VK_NULL_HANDLE, NULL);
    vk::DestroySemaphore(device(), VK_NULL_HANDLE, NULL);
    vk::DestroyShaderModule(device(), VK_NULL_HANDLE, NULL);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    VkCommandBuffer command_buffers[3] = {};
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &command_buffers[1]);
    vk::FreeCommandBuffers(device(), command_pool, 3, command_buffers);
    vk::DestroyCommandPool(device(), command_pool, NULL);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
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
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(device(), &alloc_info, &descriptor_sets[1]);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::FreeDescriptorSets(device(), ds_pool.handle(), 3, descriptor_sets);

    vk::FreeMemory(device(), VK_NULL_HANDLE, NULL);
}

TEST_F(VkPositiveBestPracticesLayerTest, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound. Its purpose is to catch regression of this bug: "
        "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4597.");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    OneOffDescriptorSet empty_ds(m_device, {});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&empty_ds.layout_, &empty_ds.layout_});

    m_commandBuffer->begin();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &empty_ds.set_, 0, nullptr);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);

    // The draw command will most likely produce a crash in case of a regression.
    vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkPositiveBestPracticesLayerTest, DynStateIgnoreAttachments) {
    TEST_DESCRIPTION("Make sure pAttachments is ignored if dynamic state is enabled");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    if (!IsPlatformMockICD()) {
        // Several drivers have been observed to crash on the legal null pAttachments - restrict to MockICD for now
        GTEST_SKIP() << "This test only runs on MockICD";
    }
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    if (!extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
        !extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask) {
        GTEST_SKIP() << "DynamicState3 features not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    // pAttachments should be ignored with these four states set
    VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                                        VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT};
    VkPipelineDynamicStateCreateInfo dynamic_create_info = vku::InitStructHelper();
    dynamic_create_info.pDynamicStates = dynamic_states;
    dynamic_create_info.dynamicStateCount = 4;

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.pAttachments = nullptr;
    pipe.gp_ci_.pDynamicState = &dynamic_create_info;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->end();
}

TEST_F(VkPositiveBestPracticesLayerTest, ImageInputAttachmentLayout) {
    TEST_DESCRIPTION("Test transitioning image layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for input attachment");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

    VkImageMemoryBarrier image_memory_barrier = vku::InitStructHelper();
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_memory_barrier.image = image.handle();
    image_memory_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 0u, nullptr, 1u, &image_memory_barrier);
    m_commandBuffer->end();
}

TEST_F(VkPositiveBestPracticesLayerTest, PipelineLibraryNoRendering) {
    TEST_DESCRIPTION("Create a pipeline library without a render pass or rendering info");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    CreatePipelineHelper pre_raster_lib(*this);
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.InitState();
    pre_raster_lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    pre_raster_lib.gp_ci_.flags |= VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    pre_raster_lib.CreateGraphicsPipeline();
}

TEST_F(VkPositiveBestPracticesLayerTest, PushConstantSet) {
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { uint x[4]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0] * constants.x[1]);
        }
    )glsl";

    const char fsSource[] = R"glsl(
        #version 460
        layout(push_constant, std430) uniform foo { float x; } constants;
        layout(location = 0) out vec4 uFragColor;
        void main(){
            uFragColor = vec4(0,1,0,constants.x);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    uint32_t data[5];
    std::vector<VkPushConstantRange> push_constant_ranges = {{VK_SHADER_STAGE_VERTEX_BIT, 0, 16},
                                                             {VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4}};

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, push_constant_ranges);
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 16, data);
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4, data);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
