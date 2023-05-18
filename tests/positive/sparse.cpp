/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveSparse : public VkPositiveLayerTest {};

TEST_F(PositiveSparse, BindS) {
    TEST_DESCRIPTION("Bind 2 memory ranges to one image using vkQueueBindSparse, destroy the image and then free the memory");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }
    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Device does not support sparse bindings";
    }

    VkImage image;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory memory_one, memory_two;
    bool pass;
    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Find an image big enough to allow sparse mapping of 2 memory regions
    // Increase the image size until it is at least twice the
    // size of the required alignment, to ensure we can bind both
    // allocated memory blocks to the image on aligned offsets.
    while (memory_reqs.size < (memory_reqs.alignment * 2)) {
        vk::DestroyImage(m_device->device(), image, nullptr);
        image_create_info.extent.width *= 2;
        image_create_info.extent.height *= 2;
        err = vk::CreateImage(m_device->device(), &image_create_info, nullptr, &image);
        ASSERT_VK_SUCCESS(err);
        vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    }
    // Allocate 2 memory regions of minimum alignment size, bind one at 0, the other
    // at the end of the first
    memory_info.allocationSize = memory_reqs.alignment;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory_one);
    ASSERT_VK_SUCCESS(err);
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory_two);
    ASSERT_VK_SUCCESS(err);
    VkSparseMemoryBind binds[2];
    binds[0].flags = 0;
    binds[0].memory = memory_one;
    binds[0].memoryOffset = 0;
    binds[0].resourceOffset = 0;
    binds[0].size = memory_info.allocationSize;
    binds[1].flags = 0;
    binds[1].memory = memory_two;
    binds[1].memoryOffset = 0;
    binds[1].resourceOffset = memory_info.allocationSize;
    binds[1].size = memory_info.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = 2;
    opaqueBindInfo.pBinds = binds;

    VkFence fence = VK_NULL_HANDLE;
    VkBindSparseInfo bindSparseInfo = LvlInitStruct<VkBindSparseInfo>();
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    vk::QueueBindSparse(m_device->m_queue, 1, &bindSparseInfo, fence);
    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyImage(m_device->device(), image, NULL);
    vk::FreeMemory(m_device->device(), memory_one, NULL);
    vk::FreeMemory(m_device->device(), memory_two, NULL);
}

TEST_F(PositiveSparse, BindFreeMemory) {
    TEST_DESCRIPTION("Test using a sparse image after freeing memory that was bound to it.");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }
    if (!m_device->phy().features().sparseResidencyImage2D) {
        GTEST_SKIP() << "Device does not support sparseResidencyImage2D";
    }

    VkImage image;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 512;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory memory;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    memory_info.allocationSize = memory_reqs.size;
    bool pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory);
    ASSERT_VK_SUCCESS(err);

    VkSparseMemoryBind bind;
    bind.flags = 0;
    bind.memory = memory;
    bind.memoryOffset = 0;
    bind.resourceOffset = 0;
    bind.size = memory_info.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = 1;
    opaqueBindInfo.pBinds = &bind;

    VkFence fence = VK_NULL_HANDLE;
    VkBindSparseInfo bindSparseInfo = LvlInitStruct<VkBindSparseInfo>();
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    // Bind to the memory
    vk::QueueBindSparse(m_device->m_queue, 1, &bindSparseInfo, fence);

    // Bind back to NULL
    bind.memory = VK_NULL_HANDLE;
    vk::QueueBindSparse(m_device->m_queue, 1, &bindSparseInfo, fence);

    vk::QueueWaitIdle(m_device->m_queue);

    // Free the memory, then use the image in a new command buffer
    vk::FreeMemory(m_device->device(), memory, NULL);

    m_commandBuffer->begin();

    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdClearColorImage(m_commandBuffer->handle(), image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyImage(m_device->device(), image, NULL);
}

TEST_F(PositiveSparse, BindMetadata) {
    TEST_DESCRIPTION("Bind memory for the metadata aspect of a sparse image");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }
    if (!m_device->phy().features().sparseResidencyImage2D) {
        GTEST_SKIP() << "Device does not support sparse residency for images";
    }

    // Create a sparse image
    VkImage image;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    // Query image memory requirements
    VkMemoryRequirements memory_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);

    // Query sparse memory requirements
    uint32_t sparse_reqs_count = 0;
    vk::GetImageSparseMemoryRequirements(m_device->device(), image, &sparse_reqs_count, nullptr);
    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(sparse_reqs_count);
    vk::GetImageSparseMemoryRequirements(m_device->device(), image, &sparse_reqs_count, sparse_reqs.data());

    // Find requirements for metadata aspect
    const VkSparseImageMemoryRequirements *metadata_reqs = nullptr;
    for (auto const &aspect_sparse_reqs : sparse_reqs) {
        if ((aspect_sparse_reqs.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) != 0) {
            metadata_reqs = &aspect_sparse_reqs;
        }
    }

    if (!metadata_reqs) {
        printf("Sparse image does not require memory for metadata.\n");
    } else {
        // Allocate memory for the metadata
        VkDeviceMemory metadata_memory = VK_NULL_HANDLE;
        VkMemoryAllocateInfo metadata_memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
        metadata_memory_info.allocationSize = metadata_reqs->imageMipTailSize;
        m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &metadata_memory_info, 0);
        err = vk::AllocateMemory(m_device->device(), &metadata_memory_info, NULL, &metadata_memory);
        ASSERT_VK_SUCCESS(err);

        // Bind metadata
        VkSparseMemoryBind sparse_bind = {};
        sparse_bind.resourceOffset = metadata_reqs->imageMipTailOffset;
        sparse_bind.size = metadata_reqs->imageMipTailSize;
        sparse_bind.memory = metadata_memory;
        sparse_bind.memoryOffset = 0;
        sparse_bind.flags = VK_SPARSE_MEMORY_BIND_METADATA_BIT;

        VkSparseImageOpaqueMemoryBindInfo opaque_bind_info = {};
        opaque_bind_info.image = image;
        opaque_bind_info.bindCount = 1;
        opaque_bind_info.pBinds = &sparse_bind;

        VkBindSparseInfo bind_info = LvlInitStruct<VkBindSparseInfo>();
        bind_info.imageOpaqueBindCount = 1;
        bind_info.pImageOpaqueBinds = &opaque_bind_info;

        vk::QueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);

        // Cleanup
        vk::QueueWaitIdle(m_device->m_queue);
        vk::FreeMemory(m_device->device(), metadata_memory, NULL);
    }

    vk::DestroyImage(m_device->device(), image, NULL);
}

TEST_F(PositiveSparse, BufferOverlapCopy) {
    TEST_DESCRIPTION("Test correct non overlapping sparse buffers' copy");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().sparseBinding) {
        GTEST_SKIP() << "Requires unsupported sparseBinding feature.";
    }

    // 2 semaphores needed since we need to bind twice before copying
    auto s_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, s_info);
    vk_testing::Semaphore semaphore2(*m_device, s_info);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 256;

    VkBufferCreateInfo b_info = vk_testing::Buffer::create_info(
        copy_info.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    VkBufferObj buffer_sparse;
    buffer_sparse.init_no_mem(*m_device, b_info);
    VkBufferObj buffer_sparse2;
    buffer_sparse2.init_no_mem(*m_device, b_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vk_testing::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);
    vk_testing::DeviceMemory buffer_mem2;
    buffer_mem2.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_infos[2] = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind;
    buffer_memory_bind_infos[1].buffer = buffer_sparse2.handle();
    buffer_memory_bind_infos[1].bindCount = 1;
    buffer_memory_bind_infos[1].pBinds = &buffer_memory_bind;

    auto bind_info = LvlInitStruct<VkBindSparseInfo>();
    bind_info.bufferBindCount = 2;
    bind_info.pBufferBinds = buffer_memory_bind_infos;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    const std::optional<uint32_t> sparse_index = m_device->QueueFamilyMatching(VK_QUEUE_SPARSE_BINDING_BIT, 0u);
    if (!sparse_index) {
        GTEST_SKIP() << "Required queue families not present";
    }
    VkQueue sparse_queue = m_device->graphics_queues()[sparse_index.value()]->handle();

    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    // Set up complete

    m_commandBuffer->begin();
    // This copy is be completely legal as long as we change the memory for buffer_sparse to not overlap with
    // buffer_sparse2's memory on queue submission, or viceversa
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_sparse.handle(), buffer_sparse2.handle(), 1, &copy_info);
    m_commandBuffer->end();

    // Rebind buffer_mem2 so it does not overlap
    buffer_memory_bind.memory = buffer_mem2.handle();
    bind_info.bufferBindCount = 1;
    bind_info.waitSemaphoreCount = 1;
    bind_info.pWaitSemaphores = &semaphore.handle();
    bind_info.pSignalSemaphores = &semaphore2.handle();
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);

    // Submitting copy command with non overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore2.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(PositiveSparse, Image) {
    TEST_DESCRIPTION("Use OpImageSparse* operations at draw time");

    ASSERT_NO_FATAL_FAILURE(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }
    if (!m_device->phy().features().sparseResidencyImage2D) {
        GTEST_SKIP() << "Device does not support sparse residency for images";
    }
    if (!m_device->phy().features().shaderResourceResidency) {
        GTEST_SKIP() << "Device does not support OpCapability SparseResidency";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkImageView image_view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler;
    sampler.init(*m_device, sampler_ci);

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    ds.UpdateDescriptorSets();

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_ARB_sparse_texture2 : enable

        layout(set = 0, binding = 0) uniform sampler2D s2D;

        layout(location = 0) out vec4 outColor;

        void main() {
            vec4 texel = vec4(1.0);
            int resident = 0;
            resident |= sparseTextureARB(s2D, vec2(0.0), texel);
            resident |= sparseTextureLodARB(s2D, vec2(0.0), 2.0, texel);
            resident |= sparseTexelFetchARB(s2D, ivec2(0), 2, texel);

            outColor = sparseTexelsResidentARB(resident) ? vec4(0.0) : vec4(1.0);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1, &ds.set_, 0,
                              nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
