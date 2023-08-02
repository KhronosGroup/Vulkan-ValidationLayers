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

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"

TEST_F(NegativeObjectLifetime, CmdBufferBufferDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a buffer dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    // Destroy buffer dependency prior to submit to cause ERROR
    vk::DestroyBuffer(m_device->device(), buffer, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    vk::FreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(NegativeObjectLifetime, CmdBarrierBufferDestroyed) {
    ASSERT_NO_FATAL_FAILURE(Init());

    auto buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vk_testing::Buffer buffer;
    buffer.init_no_mem(*m_device, buf_info);
    ASSERT_TRUE(buffer.initialized());

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    vk_testing::DeviceMemory buffer_mem(*m_device, alloc_info);
    ASSERT_TRUE(buffer_mem.initialized());

    ASSERT_VK_SUCCESS(vk::BindBufferMemory(m_device->device(), buffer.handle(), buffer_mem.handle(), 0));

    m_commandBuffer->begin();
    auto buf_barrier = LvlInitStruct<VkBufferMemoryBarrier>();
    buf_barrier.buffer = buffer.handle();
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 1, &buf_barrier, 0, NULL);
    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), buffer_mem.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(NegativeObjectLifetime, CmdBarrierImageDestroyed) {
    ASSERT_NO_FATAL_FAILURE(Init());

    vk_testing::Image image;
    vk_testing::DeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);

    image.init_no_mem(*m_device, image_ci);

    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    image_mem.init(*m_device, alloc_info);

    auto err = vk::BindImageMemory(m_device->device(), image.handle(), image_mem.handle(), 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, &img_barrier);
    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), image_mem.handle(), NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(NegativeObjectLifetime, Sync2CmdBarrierBufferDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    GetPhysicalDeviceFeatures2(sync2_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->begin();
    auto buf_barrier = LvlInitStruct<VkBufferMemoryBarrier2KHR>();
    buf_barrier.buffer = buffer;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dep_info = LvlInitStruct<VkDependencyInfoKHR>();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buf_barrier;

    m_commandBuffer->PipelineBarrier2KHR(&dep_info);

    vk::FreeMemory(m_device->handle(), mem, NULL);

    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyBuffer(m_device->handle(), buffer, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, Sync2CmdBarrierImageDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    GetPhysicalDeviceFeatures2(sync2_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImage image;
    VkDeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);

    auto err = vk::CreateImage(device(), &image_ci, nullptr, &image);
    ASSERT_VK_SUCCESS(err);

    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &image_mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindImageMemory(m_device->device(), image, image_mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->begin();
    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier2KHR>();
    img_barrier.image = image;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dep_info = LvlInitStruct<VkDependencyInfoKHR>();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;

    m_commandBuffer->PipelineBarrier2KHR(&dep_info);

    vk::FreeMemory(m_device->handle(), image_mem, NULL);

    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyImage(m_device->handle(), image, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, CmdBufferBufferViewDestroyed) {
    TEST_DESCRIPTION("Delete bufferView bound to cmd buffer, then attempt to submit cmd buffer.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    CreatePipelineHelper pipe(*this);
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    VkBufferView view;

    {
        uint32_t queue_family_index = 0;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;
        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        descriptor_set.WriteDescriptorBufferView(0, view);
        descriptor_set.UpdateDescriptorSets();

        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
            layout(location=0) out vec4 x;
            void main(){
               x = imageLoad(s, 0);
            }
        )glsl";
        VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        pipe.InitInfo();
        pipe.InitState();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
        err = pipe.CreateGraphicsPipeline();
        if (err != VK_SUCCESS) {
            GTEST_SKIP() << "Unable to compile shader";
        }

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        // Bind pipeline to cmd buffer - This causes crash on Mali
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
    }
    // buffer is released.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Descriptor in binding #0 index 0 is using buffer");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyBufferView(m_device->device(), view, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Descriptor in binding #0 index 0 is using bufferView");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    bvci.buffer = buffer.handle();
    VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    descriptor_set.descriptor_writes.clear();
    descriptor_set.WriteDescriptorBufferView(0, view);
    descriptor_set.UpdateDescriptorSets();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Delete BufferView in order to invalidate cmd buffer
    vk::DestroyBufferView(m_device->device(), view, NULL);
    // Now attempt submit of cmd buffer
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBufferView");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, CmdBufferImageDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an image dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    {
        const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = tex_format;
        image_create_info.extent.width = 32;
        image_create_info.extent.height = 32;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_create_info.flags = 0;
        VkImageObj image(m_device);
        image.init(&image_create_info);

        m_commandBuffer->begin();
        VkClearColorValue ccv;
        ccv.float32[0] = 1.0f;
        ccv.float32[1] = 1.0f;
        ccv.float32[2] = 1.0f;
        ccv.float32[3] = 1.0f;
        VkImageSubresourceRange isr = {};
        isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        isr.baseArrayLayer = 0;
        isr.baseMipLevel = 0;
        isr.layerCount = 1;
        isr.levelCount = 1;
        vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
        m_commandBuffer->end();
    }
    // Destroy image dependency prior to submit to cause ERROR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, CmdBufferFramebufferImageDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a framebuffer image dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format doesn't support required features";
    }
    VkFramebuffer fb;
    VkImageView view;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    {
        VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent.width = 32;
        image_ci.extent.height = 32;
        image_ci.extent.depth = 1;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.flags = 0;
        VkImageObj image(m_device);
        image.init(&image_ci);

        VkImageViewCreateInfo ivci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image.handle(),
            VK_IMAGE_VIEW_TYPE_2D,
            VK_FORMAT_B8G8R8A8_UNORM,
            {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
        ASSERT_VK_SUCCESS(err);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 32, 32, 1};
        err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
        ASSERT_VK_SUCCESS(err);

        // Just use default renderpass with our framebuffer
        m_renderPassBeginInfo.framebuffer = fb;
        m_renderPassBeginInfo.renderArea.extent.width = 32;
        m_renderPassBeginInfo.renderArea.extent.height = 32;
        // Create Null cmd buffer for submit
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
    // Destroy image attached to framebuffer to invalidate cmd buffer
    // Now attempt to submit cmd buffer and verify error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(NegativeObjectLifetime, FramebufferAttachmentMemoryFreed) {
    TEST_DESCRIPTION("Attempt to create framebuffer with attachment which memory was freed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format doesn't support required features";
    }
    VkFramebuffer fb;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.flags = 0;

    vk_testing::Image image;
    image.init_no_mem(*m_device, image_ci);

    vk_testing::DeviceMemory *image_memory = new vk_testing::DeviceMemory;
    image_memory->init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0));
    image.bind_memory(*image_memory, 0);

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vk_testing::ImageView view(*m_device, ivci);

    VkFramebufferCreateInfo fci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view.handle(), 32, 32, 1};

    // Introduce error:
    // Free the attachment image memory, then create framebuffer.
    delete image_memory;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess");
    vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, DescriptorPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete a DescriptorPool with a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create image to update the descriptor with
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Update descriptor with image and sampler
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put pool in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy pool while in-flight, causing error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    vk::DestroyDescriptorPool(m_device->device(), pipe.descriptor_set_->pool_, NULL);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
    // TODO : It seems Validation layers think ds_pool was already destroyed, even though it wasn't?
}

TEST_F(NegativeObjectLifetime, FramebufferInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyFramebuffer-framebuffer-00892");
    vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy everything
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If framebuffer is not VK_NULL_HANDLE, framebuffer must be a valid VkFramebuffer handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Framebuffer obj");
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(NegativeObjectLifetime, PushDescriptorUniformDestroySignaled) {
    TEST_DESCRIPTION("Destroy a uniform buffer in use by a push descriptor set");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    auto vbo = std::make_unique<VkConstantBufferObj>(m_device, sizeof(vbo_data), &vbo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo->handle();
    buff_info.offset = 0;
    buff_info.range = sizeof(vbo_data);
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
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
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), vbo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vbo.reset();
}

TEST_F(NegativeObjectLifetime, FramebufferImageInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use image that's child of framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 256;
    image_ci.extent.height = 256;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.flags = 0;
    VkImageObj image(m_device);
    image.init(&image_ci);

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it (and attached imageView) in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer to put framebuffer and children in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy image attached to framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyImage-image-01000");
    vk::DestroyImage(m_device->device(), image.handle(), NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy image and other objects
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If image is not VK_NULL_HANDLE, image must be a valid VkImage handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Image obj");
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(NegativeObjectLifetime, EventInUseDestroyedSignaled) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    m_commandBuffer->end();
    vk::DestroyEvent(m_device->device(), event, nullptr);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "that is invalid because bound");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, InUseDestroyedSignaled) {
    TEST_DESCRIPTION(
        "Use vkCmdExecuteCommands with invalid state in primary and secondary command buffers. Delete objects that are in use. "
        "Call VkQueueSubmit with an event that has been deleted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    VkFence fence;
    ASSERT_VK_SUCCESS(vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_test.GetBuffer(), 0, VK_WHOLE_SIZE,
                                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyEvent-event-01145");
    vk::DestroyEvent(m_device->device(), event, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroySemaphore-semaphore-01137");
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyFence-fence-01120");
    vk::DestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If semaphore is not VK_NULL_HANDLE, semaphore must be a valid VkSemaphore handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Semaphore obj");
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->SetUnexpectedError("If fence is not VK_NULL_HANDLE, fence must be a valid VkFence handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Fence obj");
    vk::DestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->SetUnexpectedError("If event is not VK_NULL_HANDLE, event must be a valid VkEvent handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Event obj");
    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(NegativeObjectLifetime, PipelineInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use pipeline.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPipelineLayoutObj pipeline_layout(m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyPipeline-pipeline-00765");
    // Create PSO to be used for draw-time errors below

    // Store pipeline handle so we can actually delete it before test finishes
    VkPipeline delete_this_pipeline;
    {  // Scope pipeline so it will be auto-deleted
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        delete_this_pipeline = pipe.pipeline_;

        m_commandBuffer->begin();
        // Bind pipeline to cmd buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        // Submit cmd buffer and then pipeline destroyed while in-flight
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }  // Pipeline deletion triggered here
    m_errorMonitor->VerifyFound();
    // Make sure queue finished and then actually delete pipeline
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If pipeline is not VK_NULL_HANDLE, pipeline must be a valid VkPipeline handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Pipeline obj");
    vk::DestroyPipeline(m_device->handle(), delete_this_pipeline, nullptr);
}

TEST_F(NegativeObjectLifetime, ImageViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use imageView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    // Create PSO to use the sampler
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyImageView-imageView-01026");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy imageView while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroyImageView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy imageView
    m_errorMonitor->SetUnexpectedError("If imageView is not VK_NULL_HANDLE, imageView must be a valid VkImageView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove ImageView obj");
    vk::DestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(NegativeObjectLifetime, BufferViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use bufferView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferView view;
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
        layout(location=0) out vec4 x;
        void main(){
           x = imageLoad(s, 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    err = pipe.CreateGraphicsPipeline();
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Unable to compile shader";
    }

    pipe.descriptor_set_->WriteDescriptorBufferView(0, view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBufferView-bufferView-00936");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy bufferView while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroyBufferView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy bufferView
    m_errorMonitor->SetUnexpectedError("If bufferView is not VK_NULL_HANDLE, bufferView must be a valid VkBufferView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove BufferView obj");
    vk::DestroyBufferView(m_device->device(), view, NULL);
}

TEST_F(NegativeObjectLifetime, SamplerInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use sampler.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    // Create PSO to use the sampler
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroySampler-sampler-01082");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy sampler while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroySampler(m_device->device(), sampler, nullptr);  // Destroyed too soon
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    // Now we can actually destroy sampler
    m_errorMonitor->SetUnexpectedError("If sampler is not VK_NULL_HANDLE, sampler must be a valid VkSampler handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Sampler obj");
    vk::DestroySampler(m_device->device(), sampler, NULL);  // Destroyed for real
}

TEST_F(NegativeObjectLifetime, CmdBufferEventDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an event dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo evci = LvlInitStruct<VkEventCreateInfo>();
    VkResult result = vk::CreateEvent(m_device->device(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkEvent");
    // Destroy event dependency prior to submit to cause ERROR
    vk::DestroyEvent(m_device->device(), event, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeObjectLifetime, ImportFdSemaphoreInUse) {
    TEST_DESCRIPTION("Import semaphore when semaphore is in use.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    if (!SemaphoreExportImportSupported(gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through fd handle";
    }

    // Create semaphore and export its fd handle
    auto export_info = LvlInitStruct<VkExportSemaphoreCreateInfo>();
    export_info.handleTypes = handle_type;
    auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&export_info);
    vk_testing::Semaphore export_semaphore(*m_device, create_info);
    int fd = -1;
    ASSERT_VK_SUCCESS(export_semaphore.export_handle(fd, handle_type));

    // Create a new semaphore and put it to work
    vk_testing::Semaphore semaphore(*m_device);
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    // Try to import fd handle while semaphore is still in use
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkImportSemaphoreFdKHR-semaphore-01142");
    semaphore.import_handle(fd, handle_type);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeObjectLifetime, ImportWin32SemaphoreInUse) {
    TEST_DESCRIPTION("Import semaphore when semaphore is in use.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create semaphore and export its Win32 handle
    auto export_info = LvlInitStruct<VkExportSemaphoreCreateInfo>();
    export_info.handleTypes = handle_type;
    auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&export_info);
    vk_testing::Semaphore export_semaphore(*m_device, create_info);
    HANDLE handle = NULL;
    ASSERT_VK_SUCCESS(export_semaphore.export_handle(handle, handle_type));

    // Create a new semaphore and put it to work
    vk_testing::Semaphore semaphore(*m_device);
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    // Try to import Win32 handle while semaphore is still in use
    // Waiting for: https://gitlab.khronos.org/vulkan/vulkan/-/issues/3507
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUIDUndefined);
    semaphore.import_handle(handle, handle_type);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
}
#endif
