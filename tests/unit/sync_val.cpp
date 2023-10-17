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
#include <type_traits>

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include <utils/vk_layer_utils.h>

class NegativeSyncVal : public VkSyncValTest {};

TEST_F(NegativeSyncVal, BufferCopyHazards) {
    AddOptionalExtensions(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    bool has_amd_buffer_maker = IsExtensionsEnabled(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();

    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a.handle();
    buffer_barrier.offset = 0;
    buffer_barrier.size = 256;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &back2back);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);

    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_c.handle(), buffer_b.handle(), 1, &region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protect C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_c.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // CmdFillBuffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, 256, 1);
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, 256, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // CmdUpdateBuffer
    int i = 10;
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, sizeof(i), &i);
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, sizeof(i), &i);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Create secondary buffers to use
    vkt::CommandBuffer secondary_cb1(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb1 = secondary_cb1.handle();
    secondary_cb1.begin();
    vk::CmdCopyBuffer(scb1, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    secondary_cb1.end();

    vkt::CommandBuffer secondary_cb2(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb2 = secondary_cb2.handle();
    secondary_cb2.begin();
    vk::CmdCopyBuffer(scb2, buffer_a.handle(), buffer_c.handle(), 1, &front2front);
    secondary_cb2.end();

    vkt::CommandBuffer secondary_cb3(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb3 = secondary_cb3.handle();
    secondary_cb3.begin();
    vk::CmdPipelineBarrier(secondary_cb3.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    secondary_cb3.end();

    vkt::CommandBuffer secondary_cb4(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb4 = secondary_cb4.handle();
    secondary_cb4.begin();
    vk::CmdCopyBuffer(scb4, buffer_b.handle(), buffer_c.handle(), 1, &front2front);
    secondary_cb4.end();

    // One secondary CB hazard with active command buffer
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdExecuteCommands(cb, 1, &scb1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Two secondary CB hazard with each other
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    // This is also a "SYNC-HAZARD-WRITE-AFTER-WRITE" present, but only the first hazard is reported.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    {
        VkCommandBuffer two_cbs[2] = {scb1, scb2};
        vk::CmdExecuteCommands(cb, 2, two_cbs);
    }
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Two secondary CB hazard with each other
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
        VkCommandBuffer two_cbs[2] = {scb1, scb4};
        vk::CmdExecuteCommands(cb, 2, two_cbs);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();

    // Add a secondary CB with a barrier
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    {
        VkCommandBuffer three_cbs[3] = {scb1, scb3, scb4};
        vk::CmdExecuteCommands(cb, 3, three_cbs);
    }
    m_commandBuffer->end();

    m_commandBuffer->reset();
    // CmdWriteBufferMarkerAMD
    if (has_amd_buffer_maker) {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdWriteBufferMarkerAMD(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a.handle(), 0, 1);
        m_commandBuffer->end();

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
        vk::CmdWriteBufferMarkerAMD(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a.handle(), 0, 1);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    } else {
        printf("Test requires unsupported vkCmdWriteBufferMarkerAMD feature. Skipped.\n");
    }
}

TEST_F(NegativeSyncVal, BufferCopyHazardsSync2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();

    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkBufferMemoryBarrier2KHR buffer_barrier = vku::InitStructHelper();
        buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        buffer_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT_KHR;
        buffer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        buffer_barrier.buffer = buffer_a.handle();
        buffer_barrier.offset = 0;
        buffer_barrier.size = 256;
        VkDependencyInfoKHR dep_info = vku::InitStructHelper();
        dep_info.bufferMemoryBarrierCount = 1;
        dep_info.pBufferMemoryBarriers = &buffer_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
    }

    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &back2back);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkMemoryBarrier2KHR mem_barrier = vku::InitStructHelper();
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        VkDependencyInfoKHR dep_info = vku::InitStructHelper();
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &mem_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);

        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_c.handle(), buffer_b.handle(), 1, &region);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT_KHR;  // Protect C but not B
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_c.handle(), 1, &region);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeSyncVal, CmdClearAttachmentsHazards) {
    TEST_DESCRIPTION("Test for hazards when attachment is cleared inside render pass.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    const auto transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_usage;
    const auto ds_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | transfer_usage;

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, rt_format, transfer_usage, VK_IMAGE_TILING_OPTIMAL);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj image_ds(m_device);
    image_ds.InitNoLayout(width, height, 1, ds_format, transfer_usage, VK_IMAGE_TILING_OPTIMAL);
    image_ds.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj rt(m_device);
    rt.InitNoLayout(width, height, 1, rt_format, rt_usage, VK_IMAGE_TILING_OPTIMAL);
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj ds(m_device);
    ds.InitNoLayout(width, height, 1, ds_format, ds_usage, VK_IMAGE_TILING_OPTIMAL);
    ds.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachments[] = {attachment_without_load_store(rt_format),
                                                   attachment_without_load_store(ds_format)};

    const VkImageView views[] = {rt.targetView(rt_format, VK_IMAGE_ASPECT_COLOR_BIT),
                                 ds.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)};

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference depth_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = size32(attachments);
    rpci.pAttachments = attachments;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = size32(views);
    fbci.pAttachments = views;
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.framebuffer = framebuffer;
    rpbi.renderPass = render_pass;
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;

    const VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.gp_ci_.pDepthStencilState = &ds_ci;
    pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

    struct AspectInfo {
        VkImageAspectFlagBits aspect;
        VkImage src_image;
        VkImage dst_image;
    };
    const AspectInfo aspect_infos[] = {{VK_IMAGE_ASPECT_COLOR_BIT, image, rt},
                                       {VK_IMAGE_ASPECT_DEPTH_BIT, image_ds, ds},
                                       {VK_IMAGE_ASPECT_STENCIL_BIT, image_ds, ds}};

    // WAW hazard: copy to render target then clear it. Test each aspect (color/depth/stencil).
    for (const auto& info : aspect_infos) {
        const VkClearAttachment clear_attachment = {VkImageAspectFlags(info.aspect)};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {width / 2, height / 2};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VkImageAspectFlags(info.aspect), 0, 0, 1};
        copy_region.dstSubresource = {VkImageAspectFlags(info.aspect), 0, 0, 1};
        copy_region.extent = {width, height, 1};

        m_commandBuffer->begin();
        // Write 1
        vk::CmdCopyImage(*m_commandBuffer, info.src_image, VK_IMAGE_LAYOUT_GENERAL, info.dst_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                         &copy_region);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
        // Write 2
        vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRenderPass(*m_commandBuffer);
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        vk::QueueWaitIdle(m_default_queue);
    }

    // RAW hazard: clear render target then copy from it.
    // This tests that vkCmdClearAttachments correctly updates access state, so vkCmdCopyImage can detect hazard.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_STENCIL_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {width, height};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
        copy_region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
        copy_region.extent = {width, height, 1};

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
        // Write
        vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
        vk::CmdEndRenderPass(*m_commandBuffer);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
        // Read
        vk::CmdCopyImage(*m_commandBuffer, ds, VK_IMAGE_LAYOUT_GENERAL, image_ds, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        vk::QueueWaitIdle(m_default_queue);
    }

    // RAW hazard: two regions with a single pixel overlap, otherwise the same as the previous scenario.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {32, 32};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_region.srcOffset = {31, 31, 0};
        copy_region.dstOffset = {31, 31, 0};
        copy_region.extent = {64, 64, 1};

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
        // Write
        vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
        vk::CmdEndRenderPass(*m_commandBuffer);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
        // Read
        vk::CmdCopyImage(*m_commandBuffer, rt, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        vk::QueueWaitIdle(m_default_queue);
    }

    // Nudge regions by one pixel compared to the previous test, now they touch but do not overlap. There should be no errors.
    // Copy to the first region, clear the second region.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {32, 32};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT), 0, 0, 1};
        copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT), 0, 0, 1};
        copy_region.srcOffset = {32, 32, 0};
        copy_region.dstOffset = {32, 32, 0};
        copy_region.extent = {64, 64, 1};

        m_commandBuffer->begin();
        // Write 1
        vk::CmdCopyImage(*m_commandBuffer, image_ds, VK_IMAGE_LAYOUT_GENERAL, ds, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
        // Write 2
        vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
        vk::CmdEndRenderPass(*m_commandBuffer);
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer();
        vk::QueueWaitIdle(m_default_queue);
    }
}

TEST_F(NegativeSyncVal, CopyOptimalImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    ASSERT_TRUE(image_a.initialized());

    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    ASSERT_TRUE(image_b.initialized());

    VkImageObj image_c(m_device);
    image_ci.flags |= VK_IMAGE_CREATE_ALIAS_BIT;
    image_c.Init(image_ci);
    ASSERT_TRUE(image_c.initialized());

    VkImageObj image_c_alias(m_device);
    image_c_alias.init_no_mem(*m_device, image_ci);
    image_c_alias.bind_memory(image_c.memory(), 0);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_0_to_1 = {layers_0, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_front = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_back = {layers_0, half_offset, layers_0, half_offset, half_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_1_to_1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);

    // Safe all transfer accesses
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);

    // Write to both versions of an alias
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_alias.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_alias.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_back);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // Test secondary command buffers
    // Create secondary buffers to use
    vkt::CommandBuffer secondary_cb1(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb1 = secondary_cb1.handle();
    secondary_cb1.begin();
    vk::CmdCopyImage(scb1, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    secondary_cb1.end();

    auto record_primary = [&]() {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
        vk::CmdExecuteCommands(cb, 1, &scb1);
        m_commandBuffer->end();
    };

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    record_primary();
    m_errorMonitor->VerifyFound();

    // With a barrier...
    secondary_cb1.reset();
    secondary_cb1.begin();
    vk::CmdPipelineBarrier(scb1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(scb1, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    secondary_cb1.end();
    record_primary();

    auto image_transition_barrier = image_barrier;
    image_transition_barrier.image = image_a.handle();
    image_transition_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_transition_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_transition_barrier.srcAccessMask = 0;
    image_transition_barrier.dstAccessMask = 0;

    secondary_cb1.reset();
    secondary_cb1.begin();
    // Use the wrong stage, get an error
    vk::CmdPipelineBarrier(scb1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_transition_barrier);
    secondary_cb1.end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    record_primary();
    m_errorMonitor->VerifyFound();

    // CmdResolveImage hazard testing
    VkImageFormatProperties formProps = {{0, 0, 0}, 0, 0, 0, 0};
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &formProps);

    if (!(formProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
        printf("CmdResolveImage Test requires unsupported VK_SAMPLE_COUNT_2_BIT feature. Skipped.\n");
    } else {
        VkImageObj image_s2_a(m_device), image_s2_b(m_device);
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        image_s2_a.Init(image_ci);
        ASSERT_TRUE(image_s2_a.initialized());

        image_s2_b.Init(image_ci);
        ASSERT_TRUE(image_s2_b.initialized());

        VkImageResolve r_full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        image_s2_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_s2_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_commandBuffer->end();

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdCopyImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                         &full_region);
        vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
        vk::CmdResolveImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeSyncVal, CopyOptimalImageHazardsSync2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    ASSERT_TRUE(image_a.initialized());

    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    ASSERT_TRUE(image_b.initialized());

    VkImageObj image_c(m_device);
    image_c.Init(image_ci);
    ASSERT_TRUE(image_c.initialized());

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_0_to_1 = {layers_0, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_front = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_back = {layers_0, half_offset, layers_0, half_offset, half_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkImageMemoryBarrier2KHR image_barrier = vku::InitStructHelper();
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        image_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT_KHR;
        image_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        image_barrier.image = image_a.handle();
        image_barrier.subresourceRange = full_subresource_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDependencyInfoKHR dep_info = vku::InitStructHelper();
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
    }

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_1_to_1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkMemoryBarrier2KHR mem_barrier = vku::InitStructHelper();
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        VkDependencyInfoKHR dep_info = vku::InitStructHelper();
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &mem_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

        // Use barrier to protect last reader, but not last writer...
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT_KHR;  // Protects C but not B
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, CopyOptimalMultiPlanarHazards) {
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());

    RETURN_IF_SKIP(InitState())

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    // Verify format
    bool supported = ImageFormatIsSupported(instance(), gpu(), image_ci,
                                            VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 2};
    VkImageSubresourceLayers layer0_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer0_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer1_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_all_plane0_to_all_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane0 = {layer0_plane0, zero_offset, layer0_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane1 = {layer0_plane0, zero_offset, layer0_plane1, zero_offset, half_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_front = {layer1_plane1, zero_offset, layer1_plane1, zero_offset,
                                                               one_four_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_back = {layer1_plane1, one_four_offset, layer1_plane1, one_four_offset,
                                                              one_four_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_back);

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, CopyLinearImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_front = {layers_all, zero_offset, layers_all, zero_offset, half_extent};
    VkImageCopy region_back = {layers_all, half_offset, layers_all, half_offset, half_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_b.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_back);
}

TEST_F(NegativeSyncVal, CopyLinearMultiPlanarHazards) {
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());

    RETURN_IF_SKIP(InitState())

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    // Verify format
    bool supported = ImageFormatIsSupported(instance(), gpu(), image_ci,
                                            VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer_all_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_plane0_to_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_plane0_to_plane1 = {layer_all_plane0, zero_offset, layer_all_plane1, zero_offset, half_extent};
    VkImageCopy region_plane1_to_plane1_front = {layer_all_plane1, zero_offset, layer_all_plane1, zero_offset, one_four_extent};
    VkImageCopy region_plane1_to_plane1_back = {layer_all_plane1, one_four_offset, layer_all_plane1, one_four_offset,
                                                one_four_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_back);

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, CopyBufferImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 2048, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 2048, transfer_usage, mem_prop);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{16, 16, 0};
    VkExtent3D half_extent{16, 16, 1};  // <-- image type is 2D

    VkBufferImageCopy region_buffer_front_image_0_front = {0, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_front = {0, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_back = {0, 16, 16, layers_1, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_front = {1024, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_back = {1024, 16, 16, layers_0, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_front = {1024, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_back = {1024, 16, 16, layers_1, half_offset, half_extent};

    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_0_back);

    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_1_front);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_1_back);

    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_0_back);

    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.buffer = buffer_b.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_1_front);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_1_back);

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, BlitImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_0_offset{16, 16, 0};
    VkOffset3D half_1_offset{16, 16, 1};
    VkOffset3D full_offset{32, 32, 1};
    VkImageBlit region_0_front_1_front = {layers_0, {zero_offset, half_1_offset}, layers_1, {zero_offset, half_1_offset}};
    VkImageBlit region_1_front_0_front = {layers_1, {zero_offset, half_1_offset}, layers_0, {zero_offset, half_1_offset}};
    VkImageBlit region_1_back_0_back = {layers_1, {half_0_offset, full_offset}, layers_0, {half_0_offset, full_offset}};

    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdBlitImage(cb, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdBlitImage(cb, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdBlitImage(cb, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_front_0_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    vk::CmdBlitImage(cb, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_back_0_back, VK_FILTER_NEAREST);

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, RenderPassBeginTransitionHazard) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())
    const VkSubpassDependency external_subpass_dependency = {VK_SUBPASS_EXTERNAL,
                                                             0,
                                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                             VK_DEPENDENCY_BY_REGION_BIT};
    m_additionalSubpassDependencies.push_back(external_subpass_dependency);
    InitRenderTarget(2);

    // Render Target Information
    auto *rt_0 = m_renderTargets[0].get();
    auto *rt_1 = m_renderTargets[1].get();

    // Other buffers with which to interact
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(m_width, m_height, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{m_width, m_height, 1};  // <-- image type is 2D
    VkImageSubresourceLayers layer_color{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageCopy region_to_copy = {layer_color, zero_offset, layer_color, zero_offset, full_extent};

    auto cb = m_commandBuffer->handle();

    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_0->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_1->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    rt_0->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, rt_0->handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_to_copy);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageSubresourceRange rt_full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.image = rt_0->handle();
    image_barrier.subresourceRange = rt_full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &image_barrier);
    vk::CmdCopyImage(cb, rt_1->handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_to_copy);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    // A global execution barrier that the implict external dependency can chain with should work...
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0,
                           nullptr);

    // With the barrier above, the layout transition has a chained execution sync operation, and the default
    // implict VkSubpassDependency safes the load op clear vs. the layout transition...
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
}

TEST_F(NegativeSyncVal, CmdDispatchDrawHazards) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    // Enable VK_KHR_draw_indirect_count for KHR variants
    AddOptionalExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    const bool has_khr_indirect = IsExtensionsEnabled(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    if (has_khr_indirect) {
        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            features12.drawIndirectCount = VK_TRUE;
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, &features12));
    InitRenderTarget();

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_c_a(m_device), image_c_b(m_device);
    const auto image_c_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine, VK_IMAGE_TILING_OPTIMAL);
    image_c_a.Init(image_c_ci);
    image_c_b.Init(image_c_ci);

    VkImageView imageview_c = image_c_a.targetView(format);
    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj image_s_a(m_device), image_s_b(m_device);
    const auto image_s_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage, VK_IMAGE_TILING_OPTIMAL);
    image_s_a.Init(image_s_ci);
    image_s_b.Init(image_s_ci);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_s_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageView imageview_s = image_s_a.targetView(format);

    vkt::Sampler sampler_s, sampler_c;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_s.init(*m_device, sampler_ci);
    sampler_c.init(*m_device, sampler_ci);

    vkt::Buffer buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*m_device, buffer_a.create_info(2048, buffer_usage, nullptr), mem_prop);
    buffer_b.init(*m_device, buffer_b.create_info(2048, buffer_usage, nullptr), mem_prop);

    vkt::BufferView bufferview;
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    bufferview.init(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview.handle());
    descriptor_set.UpdateDescriptorSets();

    // Dispatch
    const char *csSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } ub0;
        layout(set=0, binding=1) uniform sampler2D cis1;
        layout(set=0, binding=2, rgba8) uniform readonly image2D si2;
        layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;
        void main(){
            vec4 vColor4;
            vColor4.x = ub0.x;
            vColor4 = texture(cis1, vec2(0));
            vColor4 = imageLoad(si2, ivec2(0));
            vColor4 = imageLoad(stb3, 0);
        }
    )glsl";

    vkt::Event event;
    event.init(*m_device, vkt::Event::create_info(0));
    VkEvent event_handle = event.handle();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();

    VkBufferCopy buffer_region = {0, 0, 2048};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);

    VkImageSubresourceLayers layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{16, 16, 1};
    VkImageCopy image_region = {layer, zero_offset, layer, zero_offset, full_extent};
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_commandBuffer->handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
    m_commandBuffer->reset();
    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
    m_commandBuffer->reset();

    // DispatchIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_dispatchIndirect(*m_device, sizeof(VkDispatchIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_dispatchIndirect2(*m_device, sizeof(VkDispatchIndirectCommand), buffer_usage, mem_prop);
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer_dispatchIndirect.handle(), 0);
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDispatchIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_dispatchIndirect2.handle(), buffer_dispatchIndirect.handle(), 1,
                      &buffer_region);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer_dispatchIndirect.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Draw
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    vkt::Buffer vbo, vbo2;
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vbo.init(*m_device, vbo.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);
    vbo2.init(*m_device, vbo2.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, csSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitState();
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(vbo_data)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), vbo2.handle(), vbo.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Repeat the draw test with a WaitEvent to protect it.
    m_commandBuffer->reset();
    m_commandBuffer->begin();

    vk::CmdCopyBuffer(m_commandBuffer->handle(), vbo2.handle(), vbo.handle(), 1, &buffer_region);

    VkBufferMemoryBarrier vbo_barrier = vku::InitStructHelper();
    vbo_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vbo_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vbo_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vbo_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vbo_barrier.buffer = vbo.handle();
    vbo_barrier.offset = buffer_region.dstOffset;
    vbo_barrier.size = buffer_region.size;

    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, nullptr, 1,
                                &vbo_barrier, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndexed
    const float ibo_data[3] = {0.f, 0.f, 0.f};
    vkt::Buffer ibo, ibo2;
    buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    ibo.init(*m_device, ibo.create_info(sizeof(ibo_data), buffer_usage, nullptr), mem_prop);
    ibo2.init(*m_device, ibo2.create_info(sizeof(ibo_data), buffer_usage, nullptr), mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 3, 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(ibo_data)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), ibo2.handle(), ibo.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_drawIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_drawIndirect2(*m_device, sizeof(VkDrawIndirectCommand), buffer_usage, mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDrawIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_drawIndirect2.handle(), buffer_drawIndirect.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndexedIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_drawIndexedIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_drawIndexedIndirect2(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDrawIndexedIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_drawIndexedIndirect2.handle(), buffer_drawIndexedIndirect.handle(), 1,
                      &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    if (has_khr_indirect) {
        // DrawIndirectCount
        {
            buffer_usage =
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            vkt::Buffer buffer_count(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);
            vkt::Buffer buffer_count2(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);

            m_commandBuffer->reset();
            m_commandBuffer->begin();
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);
            vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                        sizeof(VkDrawIndirectCommand));
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();

            m_commandBuffer->reset();
            m_commandBuffer->begin();

            buffer_region = {0, 0, sizeof(uint32_t)};
            vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
            vk::CmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                        sizeof(VkDrawIndirectCommand));
            m_errorMonitor->VerifyFound();

            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
        }

        // DrawIndexedIndirectCount
        {
            buffer_usage =
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            vkt::Buffer buffer_count(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);
            vkt::Buffer buffer_count2(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);

            m_commandBuffer->reset();
            m_commandBuffer->begin();
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);
            vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0,
                                               buffer_count.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();

            m_commandBuffer->reset();
            m_commandBuffer->begin();

            buffer_region = {0, 0, sizeof(uint32_t)};
            vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
            vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0,
                                               buffer_count.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
            m_errorMonitor->VerifyFound();

            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
        }
    } else {
        printf("Test requires unsupported vkCmdDrawIndirectCountKHR & vkDrawIndexedIndirectCountKHR feature. Skipped.\n");
    }
}

TEST_F(NegativeSyncVal, CmdClear) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    // CmdClearColorImage
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

    m_commandBuffer->begin();

    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();
    VkClearColorValue ccv = {};
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // CmdClearDepthStencilImage
    format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj image_ds_a(m_device), image_ds_b(m_device);
    image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_ds_a.Init(image_ci);
    image_ds_b.Init(image_ci);

    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image_ds_a.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);
    image_ds_b.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);

    m_commandBuffer->begin();
    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange ds_range = {ds_aspect, 0, 1, 0, 1};

    vk::CmdClearDepthStencilImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &ds_range);
    m_commandBuffer->end();

    VkImageSubresourceLayers ds_layers_all{ds_aspect, 0, 0, 1};
    VkImageCopy ds_full_region = {ds_layers_all, zero_offset, ds_layers_all, zero_offset, full_extent};

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &ds_full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, CmdQuery) {
    // CmdCopyQueryPoolResults
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    if ((m_device->phy().queue_properties_.empty()) || (m_device->phy().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(cb, query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_a.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    vk::CmdResetQueryPool(cb, query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_a.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_b.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    // TODO:Track VkQueryPool
    // TODO:CmdWriteTimestamp
}

TEST_F(NegativeSyncVal, CmdDrawDepthStencil) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    auto format_ds = FindSupportedDepthStencilFormat(gpu());

    // Vulkan doesn't support copying between different depth stencil formats, so the formats have to change.
    auto format_dp = format_ds;
    auto format_st = format_ds;

    VkImageObj image_ds(m_device);
    VkImageObj image_dp(m_device);
    VkImageObj image_st(m_device);
    image_ds.Init(16, 16, 1, format_ds, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                  VK_IMAGE_TILING_OPTIMAL);
    image_dp.Init(16, 16, 1, format_dp, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                  VK_IMAGE_TILING_OPTIMAL);
    image_st.Init(16, 16, 1, format_st, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                  VK_IMAGE_TILING_OPTIMAL);
    VkImageView image_view_ds = image_ds.targetView(format_ds, VK_IMAGE_ASPECT_DEPTH_BIT);
    VkImageView image_view_dp = image_dp.targetView(format_dp, VK_IMAGE_ASPECT_DEPTH_BIT);
    VkImageView image_view_st = image_st.targetView(format_st, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pDepthStencilAttachment = &attach;

    VkAttachmentDescription attach_desc = {};
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    attach_desc.format = format_ds;
    vkt::RenderPass rp_ds(*m_device, rpci);
    attach_desc.format = format_dp;
    vkt::RenderPass rp_dp(*m_device, rpci);
    attach_desc.format = format_st;
    vkt::RenderPass rp_st(*m_device, rpci);

    vkt::Framebuffer fb_ds, fb_dp, fb_st;
    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_ds.handle(), 1, &image_view_ds, 16, 16, 1};
    fb_ds.init(*m_device, fbci);
    fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_dp.handle(), 1, &image_view_dp, 16, 16, 1};
    fb_dp.init(*m_device, fbci);
    fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_st.handle(), 1, &image_view_st, 16, 16, 1};
    fb_st.init(*m_device, fbci);

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    CreatePipelineHelper g_pipe_ds(*this), g_pipe_dp(*this), g_pipe_st(*this);
    g_pipe_ds.gp_ci_.renderPass = rp_ds.handle();
    g_pipe_ds.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_ds.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe_ds.CreateGraphicsPipeline());
    g_pipe_dp.gp_ci_.renderPass = rp_dp.handle();
    ds_ci.stencilTestEnable = VK_FALSE;
    g_pipe_dp.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_dp.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe_dp.CreateGraphicsPipeline());
    g_pipe_st.gp_ci_.renderPass = rp_st.handle();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.stencilTestEnable = VK_TRUE;
    g_pipe_st.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_st.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe_st.CreateGraphicsPipeline());

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
    m_renderPassBeginInfo.pClearValues = nullptr;
    m_renderPassBeginInfo.clearValueCount = 0;

    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_ds.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_dp.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_st.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    image_ds.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_dp.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_st.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {16, 16, 1};

    vk::CmdCopyImage(m_commandBuffer->handle(), image_ds.handle(), VK_IMAGE_LAYOUT_GENERAL, image_dp.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdCopyImage(m_commandBuffer->handle(), image_ds.handle(), VK_IMAGE_LAYOUT_GENERAL, image_st.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds.handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp.handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st.handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, RenderPassLoadHazardVsInitialLayout) {
    AddOptionalExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();
    const bool load_store_op_none = IsExtensionsEnabled(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_input = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_color(m_device), image_input(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 1, format, usage_color, VK_IMAGE_TILING_OPTIMAL);
    image_color.Init(image_ci);
    image_ci.usage = usage_input;
    image_input.Init(image_ci);
    VkImageView attachments[] = {image_color.targetView(format), image_input.targetView(format)};

    VkAttachmentDescription attachmentDescriptions[] = {
        // Result attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-NONE in BeginRenderPass.
                                     // It should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // Input attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

    const VkAttachmentReference resultAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference inputAttachmentRef = {1u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     1u,
                                                     &inputAttachmentRef,
                                                     1u,
                                                     &resultAttachmentRef,
                                                     0,
                                                     0,
                                                     0u,
                                                     0};

    const VkSubpassDependency subpassDependency = {VK_SUBPASS_EXTERNAL,
                                                   0,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
                                                   VK_DEPENDENCY_BY_REGION_BIT};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   2u,
                                                   attachmentDescriptions,
                                                   1u,
                                                   &subpassDescription,
                                                   1u,
                                                   &subpassDependency};
    vkt::RenderPass rp(*m_device, renderPassInfo);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 2, attachments, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fbci);

    image_input.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_commandBuffer->begin();

    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    // Even though we have no accesses prior, the layout transition *is* an access, so load can be validated vs. layout transition
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    vkt::RenderPass rp_no_load_store;
    if (load_store_op_none) {
        attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        rp_no_load_store.init(*m_device, renderPassInfo);
        m_renderPassBeginInfo.renderPass = rp_no_load_store.handle();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_commandBuffer->EndRenderPass();
    } else {
        printf("VK_EXT_load_store_op_none not supported, skipping sub-test\n");
    }
}

TEST_F(NegativeSyncVal, RenderPassWithWrongDepthStencilInitialLayout) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_ds = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageObj image_color(m_device), image_color2(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 1, color_format, usage_color, VK_IMAGE_TILING_OPTIMAL);
    image_color.Init(image_ci);
    image_color2.Init(image_ci);
    VkImageObj image_ds(m_device);
    image_ds.Init(32, 32, 1, ds_format, usage_ds, VK_IMAGE_TILING_OPTIMAL);

    const VkAttachmentDescription colorAttachmentDescription = {(VkAttachmentDescriptionFlags)0,
                                                                color_format,
                                                                VK_SAMPLE_COUNT_1_BIT,
                                                                VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                VK_ATTACHMENT_STORE_OP_STORE,
                                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentDescription depthStencilAttachmentDescription = {
        (VkAttachmentDescriptionFlags)0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-WRITE_AFTER_WRITE in BeginRenderPass.
                                    // It should be VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.push_back(colorAttachmentDescription);
    attachmentDescriptions.push_back(depthStencilAttachmentDescription);

    const VkAttachmentReference colorAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentReference depthStencilAttachmentRef = {1u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     0u,
                                                     0,
                                                     1u,
                                                     &colorAttachmentRef,
                                                     0,
                                                     &depthStencilAttachmentRef,
                                                     0u,
                                                     0};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   (uint32_t)attachmentDescriptions.size(),
                                                   &attachmentDescriptions[0],
                                                   1u,
                                                   &subpassDescription,
                                                   0u,
                                                   0};
    vkt::RenderPass rp(*m_device, renderPassInfo);

    VkImageView fb_attachments[] = {image_color.targetView(color_format),
                                    image_ds.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)};
    const VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, 0, 0u, rp.handle(), 2u, fb_attachments, 32, 32, 1u,
    };
    vkt::Framebuffer fb(*m_device, fbci);
    fb_attachments[0] = image_color2.targetView(color_format);
    vkt::Framebuffer fb1(*m_device, fbci);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.gp_ci_.renderPass = rp.handle();

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    g_pipe.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    m_commandBuffer->begin();
    VkClearValue clear = {};
    std::array<VkClearValue, 2> clear_values = { {clear, clear} };
    m_renderPassBeginInfo.pClearValues = clear_values.data();
    m_renderPassBeginInfo.clearValueCount = clear_values.size();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp.handle();

    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.framebuffer = fb1.handle();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

struct CreateRenderPassHelper {
    class SubpassDescriptionStore {
      public:
        using AttachRefVec = std::vector<VkAttachmentReference>;
        using PreserveVec = std::vector<uint32_t>;

        SubpassDescriptionStore() = default;
        SubpassDescriptionStore(const AttachRefVec& input, const AttachRefVec& color) : input_store(input), color_store(color) {}
        void SetResolve(const AttachRefVec& resolve) { resolve_store = resolve; }
        void SetDepthStencil(const AttachRefVec& ds) { ds_store = ds; }
        void SetPreserve(const PreserveVec& preserve) { preserve_store = preserve; }

        VkSubpassDescription operator*() const {
            VkSubpassDescription desc = {0u,
                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         static_cast<uint32_t>(input_store.size()),
                                         vvl::DataOrNull(input_store),
                                         static_cast<uint32_t>(color_store.size()),
                                         vvl::DataOrNull(color_store),
                                         vvl::DataOrNull(resolve_store),
                                         vvl::DataOrNull(ds_store),
                                         static_cast<uint32_t>(preserve_store.size()),
                                         vvl::DataOrNull(preserve_store)};
            return desc;
        }

      private:
        AttachRefVec input_store;
        AttachRefVec color_store;
        AttachRefVec resolve_store;
        AttachRefVec ds_store;
        PreserveVec preserve_store;
    };

    VkImageUsageFlags usage_color =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageUsageFlags usage_input =
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkClearColorValue ccv = {};

    vkt::Device* dev;
    const static uint32_t kDefaultImageSize = 64;
    uint32_t width = kDefaultImageSize;
    uint32_t height = kDefaultImageSize;
    std::shared_ptr<VkImageObj> image_color;
    std::shared_ptr<VkImageObj> image_input;
    VkImageView view_input = VK_NULL_HANDLE;
    VkImageView view_color = VK_NULL_HANDLE;

    VkAttachmentReference color_ref;
    VkAttachmentReference input_ref;
    std::vector<VkImageView> attachments;
    VkAttachmentDescription fb_attach_desc;
    VkAttachmentDescription input_attach_desc;
    std::vector<VkAttachmentDescription> attachment_descs;
    std::vector<VkAttachmentReference> input_attachments;
    std::vector<VkAttachmentReference> color_attachments;
    std::vector<VkSubpassDependency> subpass_dep;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<SubpassDescriptionStore> subpass_description_store;
    VkRenderPassCreateInfo render_pass_create_info;
    std::shared_ptr<vkt::RenderPass> render_pass;
    std::shared_ptr<vkt::Framebuffer> framebuffer;
    VkRenderPassBeginInfo render_pass_begin;
    std::vector<VkClearValue> clear_colors;

    CreateRenderPassHelper(vkt::Device* dev_)
        : dev(dev_),
          image_color(std::make_shared<VkImageObj>(dev)),
          image_input(std::make_shared<VkImageObj>(dev)),
          color_ref(DefaultColorRef()),
          input_ref(DefaultInputRef()),
          fb_attach_desc(DefaultFbAttachDesc()),
          input_attach_desc(DefaultInputAttachDesc()) {}

    CreateRenderPassHelper(const CreateRenderPassHelper& other) = default;

    void InitImageAndView() {
        auto image_ci = VkImageObj::ImageCreateInfo2D(width, height, 1, 1, format, usage_input, VK_IMAGE_TILING_OPTIMAL);
        image_input->InitNoLayout(image_ci);
        image_ci.usage = usage_color;
        image_color->InitNoLayout(image_ci);

        view_input = image_input->targetView(format);
        view_color = image_color->targetView(format);
        attachments = {view_color, view_input};
    }

    static VkAttachmentReference DefaultColorRef() {
        return {
            0u,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }

    static VkAttachmentReference DefaultInputRef() {
        return {
            1u,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    };

    static VkAttachmentReference UnusedColorAttachmentRef() {
        return {
            VK_ATTACHMENT_UNUSED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    };

    VkAttachmentDescription DefaultFbAttachDesc() {
        return VkAttachmentDescription{
            0u,
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    VkAttachmentDescription DefaultInputAttachDesc() const {
        return VkAttachmentDescription{
            0u,
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
        };
    }

    void InitAllAttachmentsToLayoutGeneral() {
        fb_attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        fb_attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        color_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
        input_attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        input_attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        input_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    void SetAttachmentLayout(VkImageObj* image, const VkAttachmentDescription& attach_desc) {
        if (image && image->initialized() && (attach_desc.initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)) {
            image->SetLayout(attach_desc.initialLayout);
        }
    }

    void SetColorLayout() { SetAttachmentLayout(image_color.get(), fb_attach_desc); }
    void SetInputLayout() { SetAttachmentLayout(image_input.get(), input_attach_desc); }

    void InitAttachmentLayouts() {
        SetColorLayout();
        SetInputLayout();
    }

    void InitAttachmentArrays() {
        // Add attachments
        if (attachment_descs.empty()) {
            attachment_descs = {fb_attach_desc, input_attach_desc};
        }
        if (color_attachments.empty()) {
            color_attachments = {color_ref};
        }
        if (input_attachments.empty()) {
            input_attachments = {input_ref};
        }
    }

    void AddSubpassDescription(const std::vector<VkAttachmentReference>& input, const std::vector<VkAttachmentReference>& color) {
        subpass_description_store.emplace_back(input, color);
    }

    // Capture the current input and color attachements, which can then be modified
    void AddInputColorSubpassDescription() { subpass_description_store.emplace_back(input_attachments, color_attachments); }

    // Create a subpass description with all the attachments preserved
    void AddPreserveInputColorSubpassDescription() {
        std::vector<uint32_t> preserve;
        preserve.reserve(input_attachments.size() + color_attachments.size());
        for (const auto& att : input_attachments) {
            preserve.push_back(att.attachment);
        }
        for (const auto& att : color_attachments) {
            preserve.push_back(att.attachment);
        }
        subpass_description_store.emplace_back();
        subpass_description_store.back().SetPreserve(preserve);
    }

    // This is the default for a single subpass renderpass, don't call if you want to change that
    void InitSubpassDescription() {
        if (subpass_description_store.empty()) {
            // The default subpass has input and color attachments
            AddInputColorSubpassDescription();
        }
    }

    void InitSubpasses() {
        if (subpasses.empty()) {
            subpasses.reserve(subpass_description_store.size());
            for (const auto& desc_store : subpass_description_store) {
                subpasses.emplace_back(*desc_store);
            }
        }
    }

    void InitRenderPassInfo() {
        render_pass_create_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0u,
                                   static_cast<uint32_t>(attachment_descs.size()),
                                   attachment_descs.data(),
                                   static_cast<uint32_t>(subpasses.size()),
                                   subpasses.data(),
                                   static_cast<uint32_t>(subpass_dep.size()),
                                   subpass_dep.data()};
    }

    void InitRenderPass() {
        InitAttachmentArrays();
        InitSubpassDescription();
        InitSubpasses();
        InitRenderPassInfo();
        render_pass = std::make_shared<vkt::RenderPass>();
        render_pass->init(*dev, render_pass_create_info);
    }

    void InitFramebuffer() {
        framebuffer = std::make_shared<vkt::Framebuffer>();
        VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                        0,
                                        0u,
                                        render_pass->handle(),
                                        static_cast<uint32_t>(attachments.size()),
                                        attachments.data(),
                                        width,
                                        height,
                                        1u};
        framebuffer->init(*dev, fbci);
    }

    void InitState() {
        InitImageAndView();
    }

    void InitBeginInfo() {
        render_pass_begin = vku::InitStructHelper();
        render_pass_begin.renderArea = {{0, 0}, {width, height}};
        render_pass_begin.renderPass = render_pass->handle();
        render_pass_begin.framebuffer = framebuffer->handle();

        // Simplistic ensure enough clear colors, if not provided
        // TODO: Should eventually be smart enough to fill in color/depth as appropos
        VkClearValue fill_in;
        fill_in.color = ccv;
        for (size_t i = clear_colors.size(); i < attachments.size(); ++i) {
            clear_colors.push_back(fill_in);
        }
        render_pass_begin.clearValueCount = static_cast<uint32_t>(clear_colors.size());
        render_pass_begin.pClearValues = clear_colors.data();
    }

    void InitPipelineHelper(CreatePipelineHelper& g_pipe) {
        g_pipe.ResetShaderInfo(kVertexMinimalGlsl, kFragmentSubpassLoadGlsl);
        g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe.gp_ci_.renderPass = render_pass->handle();
        g_pipe.InitState();
        ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());
    }

    void Init() {
        InitState();
        InitRenderPass();
        InitFramebuffer();
        InitBeginInfo();
    }
};

struct SyncTestPipeline {
    VkLayerTest& test;
    VkRenderPass rp;
    CreatePipelineHelper g_pipe;
    VkShaderObj vs;
    VkShaderObj fs;
    VkSamplerCreateInfo sampler_info;
    vkt::Sampler sampler;
    VkImageView view_input = VK_NULL_HANDLE;
    SyncTestPipeline(VkLayerTest& test_, VkRenderPass rp_)
        : test(test_),
          rp(rp_),
          g_pipe(test),
          vs(&test, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT),
          fs(&test, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT),
          sampler_info(SafeSaneSamplerCreateInfo()),
          sampler() {}
    void InitState() {
        sampler.init(*test.DeviceObj(), sampler_info);
        g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe.gp_ci_.renderPass = rp;
        g_pipe.InitState();
    }
    void Init() {
        ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());
        g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe.descriptor_set_->UpdateDescriptorSets();
    }
};

TEST_F(NegativeSyncVal, LayoutTransition) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    CreateRenderPassHelper rp_helper(m_device);
    rp_helper.Init();
    const VkImage image_input_handle = rp_helper.image_input->handle();
    const VkRenderPass rp = rp_helper.render_pass->handle();

    SyncTestPipeline st_pipe(*this, rp);
    st_pipe.InitState();
    st_pipe.view_input = rp_helper.view_input;
    st_pipe.Init();
    const auto& g_pipe = st_pipe.g_pipe;

    m_commandBuffer->begin();
    auto cb = m_commandBuffer->handle();
    VkClearColorValue ccv = {};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    const VkImageMemoryBarrier preClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   0, 0, image_input_handle,           full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);

    vk::CmdClearColorImage(m_commandBuffer->handle(), image_input_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ccv, 1,
                           &full_subresource_range);

    const VkImageMemoryBarrier postClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        image_input_handle,
        full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &postClearBarrier);

    m_commandBuffer->BeginRenderPass(rp_helper.render_pass_begin);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    // Positive test for ordering rules between load and input attachment usage
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    // Positive test for store ordering vs. input attachment and dependency *to* external for layout transition
    m_commandBuffer->EndRenderPass();

    // Catch a conflict with the input attachment final layout transition
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_input_handle, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1,
                           &full_subresource_range);
    m_errorMonitor->VerifyFound();

    // There should be no hazard for ILT after ILT
    m_commandBuffer->end();
    vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);
    const VkImageMemoryBarrier wawBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_SHADER_READ_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        image_input_handle,
        full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &wawBarrier);
    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, SubpassMultiDep) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    CreateRenderPassHelper rp_helper_positive(m_device);

    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageSubresourceLayers mip_0_layer_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D image_zero{0, 0, 0};
    VkExtent3D image_size{rp_helper_positive.width, rp_helper_positive.height, 1};

    VkImageCopy full_region{mip_0_layer_0, image_zero, mip_0_layer_0, image_zero, image_size};

    rp_helper_positive.InitState();
    rp_helper_positive.InitAllAttachmentsToLayoutGeneral();

    // Copy the comon state to the other renderpass helper
    CreateRenderPassHelper rp_helper_negative(m_device);

    auto& subpass_dep_positive = rp_helper_positive.subpass_dep;

    subpass_dep_positive.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT});

    rp_helper_positive.InitRenderPass();
    rp_helper_positive.InitFramebuffer();
    rp_helper_positive.InitBeginInfo();

    auto& subpass_dep_negative = rp_helper_negative.subpass_dep;
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    // Show that the two barriers do *not* chain by breaking the positive barrier into two bits.
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});

    rp_helper_negative.InitAllAttachmentsToLayoutGeneral();

    // Negative and postive RP's are compatible.
    rp_helper_negative.attachments = rp_helper_positive.attachments;
    rp_helper_negative.InitRenderPass();
    rp_helper_negative.InitFramebuffer();
    rp_helper_negative.InitBeginInfo();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_info);

    CreatePipelineHelper g_pipe(*this);
    rp_helper_positive.InitPipelineHelper(g_pipe);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, rp_helper_positive.view_input, VK_NULL_HANDLE,
                                                     VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    auto cb = m_commandBuffer->handle();
    VkClearColorValue ccv = {};

    const VkImageMemoryBarrier xferDestBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                  nullptr,
                                                  VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_IMAGE_LAYOUT_GENERAL,
                                                  VK_IMAGE_LAYOUT_GENERAL,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  VK_NULL_HANDLE,
                                                  full_subresource_range};
    const VkImageMemoryBarrier xferDestToSrcBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_NULL_HANDLE,
        full_subresource_range,
    };

    const VkImage image_color = rp_helper_positive.image_color->handle();
    const VkImage image_input = rp_helper_positive.image_input->handle();

    VkImageMemoryBarrier preClearBarrier = xferDestBarrier;
    preClearBarrier.image = image_color;

    VkImageMemoryBarrier preCopyBarriers[2] = {xferDestToSrcBarrier, xferDestBarrier};
    preCopyBarriers[0].image = image_color;
    preCopyBarriers[1].image = image_input;
    // Positive test for ordering rules between load and input attachment usage

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);

    vk::CmdClearColorImage(m_commandBuffer->handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 2u,
                           preCopyBarriers);

    vk::CmdCopyImage(m_commandBuffer->handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, image_input, VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &full_region);

    // No post copy image barrier, we are testing the subpass dependencies

    // Postive renderpass multidependency test
    m_commandBuffer->BeginRenderPass(rp_helper_positive.render_pass_begin);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    // Positive test for store ordering vs. input attachment and dependency *to* external for layout transition
    m_commandBuffer->EndRenderPass();

    vk::CmdCopyImage(m_commandBuffer->handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, image_input, VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &full_region);

    // Postive renderpass multidependency test, will fail IFF the dependencies are acting indepently.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_commandBuffer->BeginRenderPass(rp_helper_negative.render_pass_begin);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, RenderPassAsyncHazard) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    // overall set up:
    // subpass 0:
    //   write image 0
    // subpass 1:
    //   read image 0
    //   write image 1
    // subpass 2:
    //   read image 0
    //   write image 2
    // subpass 3:
    //   read image 0
    //   write image 3
    //
    // subpasses 1 & 2 can run in parallel but both should depend on 0
    // subpass 3 must run after 1 & 2 because otherwise the store operation will
    // race with the reads in the other subpasses.

    constexpr VkFormat kFormat = VK_FORMAT_R8G8B8A8_UNORM;
    constexpr uint32_t kWidth = 32, kHeight = 32;
    constexpr uint32_t kNumImages = 4;

    VkImageCreateInfo src_img_info = vku::InitStructHelper();
    src_img_info.flags = 0;
    src_img_info.imageType = VK_IMAGE_TYPE_2D;
    src_img_info.format = kFormat;
    src_img_info.extent = {kWidth, kHeight, 1};
    src_img_info.mipLevels = 1;
    src_img_info.arrayLayers = 1;
    src_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    src_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    src_img_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    src_img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    src_img_info.queueFamilyIndexCount = 0;
    src_img_info.pQueueFamilyIndices = nullptr;
    src_img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo dst_img_info = vku::InitStructHelper();
    dst_img_info.flags = 0;
    dst_img_info.imageType = VK_IMAGE_TYPE_2D;
    dst_img_info.format = kFormat;
    dst_img_info.extent = {kWidth, kHeight, 1};
    dst_img_info.mipLevels = 1;
    dst_img_info.arrayLayers = 1;
    dst_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dst_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    dst_img_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    dst_img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    dst_img_info.queueFamilyIndexCount = 0;
    dst_img_info.pQueueFamilyIndices = nullptr;
    dst_img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<std::unique_ptr<VkImageObj>> images;
    for (uint32_t i = 0; i < kNumImages; i++) {
        images.emplace_back(new VkImageObj(m_device));
    }
    images[0]->Init(src_img_info);
    for (uint32_t i = 1; i < images.size(); i++) {
        images[i]->Init(dst_img_info);
    }

    std::array<VkImageView, kNumImages> attachments{};
    std::array<VkAttachmentDescription, kNumImages> attachment_descriptions{};
    std::array<VkAttachmentReference, kNumImages> color_refs{};
    std::array<VkImageMemoryBarrier, kNumImages> img_barriers{};

    for (uint32_t i = 0; i < attachments.size(); i++) {
        attachments[i] = images[i]->targetView(kFormat);
        attachment_descriptions[i] = {};
        attachment_descriptions[i].flags = 0;
        attachment_descriptions[i].format = kFormat;
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descriptions[i].finalLayout =
            (i == 0) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        color_refs[i] = {i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        img_barriers[i] = vku::InitStructHelper();
        img_barriers[i].srcAccessMask = 0;
        img_barriers[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        img_barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_barriers[i].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barriers[i].image = images[i]->handle();
        img_barriers[i].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
    }

    const VkAttachmentReference input_ref{0u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    std::array<std::array<uint32_t, 2>, kNumImages - 1> preserve_subpass{{{2, 3}, {1, 3}, {1, 2}}};

    std::array<VkSubpassDescription, kNumImages> subpasses{};

    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].inputAttachmentCount = 0;
    subpasses[0].pInputAttachments = nullptr;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_refs[0];

    for (uint32_t i = 1; i < subpasses.size(); i++) {
        subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[i].inputAttachmentCount = 1;
        subpasses[i].pInputAttachments = &input_ref;
        subpasses[i].colorAttachmentCount = 1;
        subpasses[i].pColorAttachments = &color_refs[i];
        subpasses[i].preserveAttachmentCount = preserve_subpass[i - 1].size();
        subpasses[i].pPreserveAttachments = preserve_subpass[i - 1].data();
    }

    VkRenderPassCreateInfo renderpass_info = vku::InitStructHelper();
    renderpass_info.flags = 0;
    renderpass_info.attachmentCount = attachment_descriptions.size();
    renderpass_info.pAttachments = attachment_descriptions.data();
    renderpass_info.subpassCount = subpasses.size();
    renderpass_info.pSubpasses = subpasses.data();
    renderpass_info.dependencyCount = 0;
    renderpass_info.pDependencies = nullptr;

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.attachmentCount = attachments.size();
    fbci.pAttachments = attachments.data();
    fbci.width = kWidth;
    fbci.height = kHeight;
    fbci.layers = 1;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_info);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkClearValue clear = {};
    clear.color = m_clear_color;
    std::array<VkClearValue, 4> clear_values = {{clear, clear, clear, clear}};

    // run the renderpass with no dependencies
    {
        vkt::RenderPass rp(*m_device, renderpass_info);

        fbci.renderPass = rp.handle();
        vkt::Framebuffer fb(*m_device, fbci);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        g_pipe_0.InitState();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.InitState();
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        // Test is intentionally running without dependencies.
        m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass");
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            // we're racing the writes from subpass 0 with our layout transitions... (from initial)
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-WRITE");
            m_commandBuffer->NextSubpass();
            m_errorMonitor->VerifyFound();
        }

        // m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-WRITE");
        // No sync error here, as all of the NextSubpass calls *failed*
        m_commandBuffer->EndRenderPass();
        // m_errorMonitor->VerifyFound();

        vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
    }

    // add dependencies from subpass 0 to the others, which are necessary but not sufficient
    std::vector<VkSubpassDependency> subpass_dependencies;
    for (uint32_t i = 1; i < subpasses.size(); i++) {
        VkSubpassDependency dep{0,
                                i,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                0};
        subpass_dependencies.push_back(dep);
    }
    renderpass_info.dependencyCount = subpass_dependencies.size();
    renderpass_info.pDependencies = subpass_dependencies.data();

    {
        vkt::RenderPass rp(*m_device, renderpass_info);

        fbci.renderPass = rp.handle();
        vkt::Framebuffer fb(*m_device, fbci);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        g_pipe_0.InitState();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.InitState();
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();

        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            if (i > 1) {
                // We've fixed the dependency with 0, but 2 and 3 still fight with 1
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-WRITE");
            }
            m_commandBuffer->NextSubpass();
            if (i > 1) {
                m_errorMonitor->VerifyFound();
            }

            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipes[i - 1].handle());
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      g_pipe_12.pipeline_layout_.handle(), 0, 1, &g_pipe_12.descriptor_set_->set_, 0, NULL);

            vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        }
        // There is no race, because the NextSubpass calls failed above
        m_commandBuffer->EndRenderPass();

        vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
    }

    // try again with correct dependencies to make subpasses:
    //   2 depend on 1 (avoid ILT hazard)
    subpass_dependencies.emplace_back(
        VkSubpassDependency{1, 2, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, 0});
    //   3 depend on 2 (avoid store hazard)
    subpass_dependencies.emplace_back(
        VkSubpassDependency{2, 3, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0});

    renderpass_info.dependencyCount = subpass_dependencies.size();
    renderpass_info.pDependencies = subpass_dependencies.data();
    {
        vkt::RenderPass rp(*m_device, renderpass_info);

        fbci.renderPass = rp.handle();
        vkt::Framebuffer fb(*m_device, fbci);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        g_pipe_0.InitState();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.InitState();
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            m_commandBuffer->NextSubpass();
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipes[i - 1].handle());
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      g_pipe_12.pipeline_layout_.handle(), 0, 1, &g_pipe_12.descriptor_set_->set_, 0, NULL);

            vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        }

        m_commandBuffer->EndRenderPass();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeSyncVal, EventsBufferCopy) {
    TEST_DESCRIPTION("Check Set/Wait protection for a variety of use cases using buffer copies");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    vkt::Event event;
    event.init(*m_device, vkt::Event::create_info(0));
    VkEvent event_handle = event.handle();

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();

    // Copy after set for WAR (note we are writing to the back half of c but only reading from the front
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_c.handle(), 1, &back2back);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // WAR prevented
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    // Just protect against WAR, only need a sync barrier.
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);

    // Wait shouldn't prevent this WAW though, as it's only a synchronization barrier
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Prevent WAR and WAW
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkMemoryBarrier mem_barrier_waw = vku::InitStructHelper();
    mem_barrier_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 1,
                                &mem_barrier_waw, 0, nullptr, 0, nullptr);
    // The WAW should be safe (on a memory barrier)
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    // The WAR should also be safe (on a sync barrier)
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_commandBuffer->end();

    // Barrier range check for WAW
    VkBufferMemoryBarrier buffer_barrier_front_waw = vku::InitStructHelper();
    buffer_barrier_front_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.buffer = buffer_b.handle();
    buffer_barrier_front_waw.offset = front2front.dstOffset;
    buffer_barrier_front_waw.size = front2front.size;

    // Front safe, back WAW
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 1,
                                &buffer_barrier_front_waw, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &back2back);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, EventsCopyImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    ASSERT_TRUE(image_a.initialized());

    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    ASSERT_TRUE(image_b.initialized());

    VkImageObj image_c(m_device);
    image_c.Init(image_ci);
    ASSERT_TRUE(image_c.initialized());

    vkt::Event event;
    event.init(*m_device, vkt::Event::create_info(0));
    VkEvent event_handle = event.handle();

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange layers_0_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_q0toq0 = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_q0toq3 = {layers_0, zero_offset, layers_0, half_offset, half_extent};
    VkImageCopy region_0_q3toq3 = {layers_0, half_offset, layers_0, half_offset, half_extent};

    auto cb = m_commandBuffer->handle();
    auto copy_general = [cb](const VkImageObj &from, const VkImageObj &to, const VkImageCopy &region) {
        vk::CmdCopyImage(cb, from.handle(), VK_IMAGE_LAYOUT_GENERAL, to.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    };

    auto set_layouts = [this, &image_a, &image_b, &image_c]() {
        image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    };

    // Scope check.  One access in, one access not
    m_commandBuffer->begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    copy_general(image_a, image_c, region_0_q3toq3);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    copy_general(image_c, image_a, region_0_q0toq0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    copy_general(image_c, image_a, region_0_q0toq3);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // WAR prevented
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    // Just protect against WAR, only need a sync barrier.
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    copy_general(image_c, image_a, full_region);

    // Wait shouldn't prevent this WAW though, as it's only a synchronization barrier
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_general(image_c, image_b, full_region);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Prevent WAR and WAW
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkMemoryBarrier mem_barrier_waw = vku::InitStructHelper();
    mem_barrier_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 1,
                                &mem_barrier_waw, 0, nullptr, 0, nullptr);
    // The WAW should be safe (on a memory barrier)
    copy_general(image_c, image_b, full_region);
    // The WAR should also be safe (on a sync barrier)
    copy_general(image_c, image_a, full_region);
    m_commandBuffer->end();

    // Barrier range check for WAW
    VkImageMemoryBarrier image_barrier_region0_waw = vku::InitStructHelper();
    image_barrier_region0_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier_region0_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier_region0_waw.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier_region0_waw.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier_region0_waw.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier_region0_waw.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier_region0_waw.image = image_b.handle();
    image_barrier_region0_waw.subresourceRange = layers_0_subresource_range;

    // Region 0 safe, back WAW
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 1, &image_barrier_region0_waw);
    copy_general(image_a, image_b, region_0_to_0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_general(image_a, image_b, region_1_to_1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, EventsCommandHazards) {
    TEST_DESCRIPTION("Check Set/Reset/Wait command hazard checking");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Event event;
    event.init(*m_device, vkt::Event::create_info(0));

    const VkEvent event_handle = event.handle();

    m_commandBuffer->begin();
    m_commandBuffer->ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdResetEvent-event-03834");
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    m_commandBuffer->begin();
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, nullptr,
                                0, nullptr, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-vkCmdResetEvent-missingbarrier-wait");
    m_commandBuffer->ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    m_commandBuffer->begin();
    m_commandBuffer->ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-vkCmdSetEvent-missingbarrier-reset");
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0U, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_commandBuffer->WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    m_commandBuffer->ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0U, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Need a barrier between set and a reset
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-vkCmdResetEvent-missingbarrier-set");
    m_commandBuffer->ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    m_commandBuffer->begin();
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-vkCmdSetEvent-missingbarrier-set");
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // Secondary command buffer events tests
    const auto cb = m_commandBuffer->handle();
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy front2front = {0, 0, 128};

    // Barrier range check for WAW
    VkBufferMemoryBarrier buffer_barrier_front_waw = vku::InitStructHelper();
    buffer_barrier_front_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.buffer = buffer_b.handle();
    buffer_barrier_front_waw.offset = front2front.dstOffset;
    buffer_barrier_front_waw.size = front2front.size;

    vkt::CommandBuffer secondary_cb1(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb1 = secondary_cb1.handle();
    secondary_cb1.begin();
    secondary_cb1.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 1,
                             &buffer_barrier_front_waw, 0, nullptr);
    vk::CmdCopyBuffer(scb1, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    secondary_cb1.end();

    // One secondary cb hazarding with primary
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdExecuteCommands(cb, 1, &scb1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // One secondary cb sharing event with primary
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_commandBuffer->SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdExecuteCommands(cb, 1, &scb1);
    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, CmdWaitEvents2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using CmdWaitEvents2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);

    vkt::Event event;
    event.init(*m_device, vkt::Event::create_info(0));
    VkEvent event_handle = event.handle();

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents2-synchronization2-03836");
    vk::CmdWaitEvents2KHR(m_commandBuffer->handle(), 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents2-synchronization2-03836");
        vk::CmdWaitEvents2(m_commandBuffer->handle(), 1, &event_handle, &dependency_info);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, Sync2FeatureDisabled) {
    TEST_DESCRIPTION("Call sync2 functions when the feature is disabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = vku::InitStructHelper();
    synchronization2.synchronization2 = VK_FALSE;  // Invalid
    GetPhysicalDeviceFeatures2(synchronization2);

    bool timestamp = false;

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits > 0) {
        timestamp = true;
    }

    m_commandBuffer->begin();

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-synchronization2-03848");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    vkt::Event event(*m_device);

    VkPipelineStageFlagBits2KHR stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent2-synchronization2-03829");
    vk::CmdResetEvent2KHR(m_commandBuffer->handle(), event.handle(), stage);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent2-synchronization2-03824");
    vk::CmdSetEvent2KHR(m_commandBuffer->handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    if (timestamp) {
        VkQueryPoolCreateInfo qpci = vku::InitStructHelper();
        qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
        qpci.queryCount = 1;

        vkt::QueryPool query_pool(*m_device, qpci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp2-synchronization2-03858");
        vk::CmdWriteTimestamp2KHR(m_commandBuffer->handle(), stage, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp2-synchronization2-03858");
            vk::CmdWriteTimestamp2(m_commandBuffer->handle(), stage, query_pool.handle(), 0);
            m_errorMonitor->VerifyFound();
        }
    }
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-synchronization2-03848");
        vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent2-synchronization2-03829");
        vk::CmdResetEvent2(m_commandBuffer->handle(), event.handle(), stage);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent2-synchronization2-03824");
        vk::CmdSetEvent2(m_commandBuffer->handle(), event.handle(), &dependency_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(NegativeSyncVal, DestroyedUnusedDescriptors) {
    TEST_DESCRIPTION("Verify unused descriptors are ignored and don't crash syncval if they've been destroyed.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncValFramework());

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    if (!indexing_features.descriptorBindingPartiallyBound) {
        GTEST_SKIP() << "Partially bound bindings not supported, skipping test\n";
    }
    if (!indexing_features.descriptorBindingUpdateUnusedWhilePending) {
        GTEST_SKIP() << "Updating unused while pending is not supported, skipping test\n";
    }

    InitRenderTarget();

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags =
        vku::InitStructHelper();
    constexpr size_t kNumDescriptors = 6;

    std::array<VkDescriptorBindingFlagsEXT, kNumDescriptors> ds_binding_flags;
    for (auto &elem : ds_binding_flags) {
        elem = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
    }

    layout_createinfo_binding_flags.bindingCount = ds_binding_flags.size();
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags.data();

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    uint32_t qfi = 0;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &qfi;

    vkt::Buffer doit_buffer(*m_device, buffer_create_info);

    auto buffer = std::make_unique<vkt::Buffer>();
    buffer->init(*m_device, buffer_create_info);

    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = doit_buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);
    buffer_info[1].buffer = buffer->handle();
    buffer_info[1].offset = 0;
    buffer_info[1].range = sizeof(uint32_t);

    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    vkt::Buffer texel_buffer(*m_device, buffer_create_info);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    auto texel_bufferview = std::make_unique<vkt::BufferView>();
    texel_bufferview->init(*m_device, bvci);

    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj sampled_image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
    sampled_image.Init(image_ci);
    auto sampled_view = std::make_unique<vkt::ImageView>();
    auto imageview_ci = sampled_image.BasicViewCreatInfo();
    sampled_view->init(*m_device, imageview_ci);

    VkImageObj combined_image(m_device);
    image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
    combined_image.Init(image_ci);
    imageview_ci = combined_image.BasicViewCreatInfo();
    auto combined_view = std::make_unique<vkt::ImageView>();
    combined_view->init(*m_device, imageview_ci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

    VkDescriptorImageInfo image_info[3] = {};
    image_info[0].sampler = sampler.handle();
    image_info[0].imageView = VK_NULL_HANDLE;
    image_info[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info[1].sampler = VK_NULL_HANDLE;
    image_info[1].imageView = sampled_view->handle();
    image_info[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info[2].sampler = sampler.handle();
    image_info[2].imageView = combined_view->handle();
    image_info[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Update all descriptors
    std::array<VkWriteDescriptorSet, kNumDescriptors> descriptor_writes;
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info[0];

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];

    descriptor_writes[2] = vku::InitStructHelper();
    descriptor_writes[2].dstSet = descriptor_set.set_;
    descriptor_writes[2].dstBinding = 2;
    descriptor_writes[2].descriptorCount = 1;
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    descriptor_writes[2].pTexelBufferView = &texel_bufferview->handle();

    descriptor_writes[3] = vku::InitStructHelper();
    descriptor_writes[3].dstSet = descriptor_set.set_;
    descriptor_writes[3].dstBinding = 3;
    descriptor_writes[3].descriptorCount = 1;
    descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_writes[3].pImageInfo = &image_info[0];

    descriptor_writes[4] = vku::InitStructHelper();
    descriptor_writes[4].dstSet = descriptor_set.set_;
    descriptor_writes[4].dstBinding = 4;
    descriptor_writes[4].descriptorCount = 1;
    descriptor_writes[4].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_writes[4].pImageInfo = &image_info[1];

    descriptor_writes[5] = vku::InitStructHelper();
    descriptor_writes[5].dstSet = descriptor_set.set_;
    descriptor_writes[5].dstBinding = 5;
    descriptor_writes[5].descriptorCount = 1;
    descriptor_writes[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[5].pImageInfo = &image_info[2];

    vk::UpdateDescriptorSets(m_device->device(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);

    // only descriptor 0 is used, the rest are going to get destroyed
    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo_0 { int val; } doit;
        layout(set = 0, binding = 1) uniform foo_1 { int val; } readit;
        layout(set = 0, binding = 2) uniform samplerBuffer texels;
        layout(set = 0, binding = 3) uniform sampler samp;
        layout(set = 0, binding = 4) uniform texture2D img;
        layout(set = 0, binding = 5) uniform sampler2D sampled_image;

        void main() {
            vec4 x;
            vec4 y;
            vec4 z;
            if (doit.val == 0) {
                gl_Position = vec4(0.0);
                x = vec4(0.0);
                y = vec4(0.0);
                z = vec4(0.0);
            } else {
                gl_Position = vec4(readit.val);
                x = texelFetch(texels, 5);
                y = texture(sampler2D(img, samp), vec2(0));
                z = texture(sampled_image, vec2(0));
	    }
        }
    )glsl";

    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // destroy resources for the unused descriptors
    buffer.reset();
    texel_bufferview.reset();
    sampled_view.reset();
    combined_view.reset();

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncVal, TestInvalidExternalSubpassDependency) {
    TEST_DESCRIPTION("Test write after write hazard with invalid external subpass dependency");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState())

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.srcStageMask = VK_SHADER_STAGE_ALL_GRAPHICS;
    subpass_dependency.dstStageMask = VK_SHADER_STAGE_ALL_GRAPHICS;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = 0;
    subpass_dependency.dependencyFlags = 0;

    VkAttachmentReference attach_ref1 = {};
    attach_ref1.attachment = 0;
    attach_ref1.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference attach_ref2 = {};
    attach_ref2.attachment = 0;
    attach_ref2.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkSubpassDescription subpass_descriptions[2] = {};
    subpass_descriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descriptions[0].pDepthStencilAttachment = &attach_ref1;
    subpass_descriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descriptions[1].pDepthStencilAttachment = &attach_ref2;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_D32_SFLOAT;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = subpass_descriptions;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_description;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &subpass_dependency;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0, 0, 0, 0}};

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_D32_SFLOAT;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj image1(m_device);
    image1.init(&image_ci);
    ASSERT_TRUE(image1.initialized());

    VkImageViewCreateInfo iv_ci = vku::InitStructHelper();
    iv_ci.image = image1.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv_ci.format = VK_FORMAT_D32_SFLOAT;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.baseArrayLayer = 0;
    iv_ci.subresourceRange.layerCount = 1;
    vkt::ImageView image_view1(*m_device, iv_ci);

    VkImageView framebuffer_attachments[1] = {image_view1.handle()};

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper();
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = framebuffer_attachments;
    fb_ci.width = 32;
    fb_ci.height = 32;
    fb_ci.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    VkRenderPassBeginInfo rp_bi = vku::InitStructHelper();
    rp_bi.renderPass = render_pass.handle();
    rp_bi.framebuffer = framebuffer.handle();
    rp_bi.renderArea.extent.width = 32;
    rp_bi.renderArea.extent.height = 32;
    rp_bi.clearValueCount = 1;
    rp_bi.pClearValues = &clear_value;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.depthWriteEnable = VK_FALSE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.gp_ci_.pDepthStencilState = &ds_ci;
    pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_bi);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, TestCopyingToCompressedImage) {
    TEST_DESCRIPTION("Copy from uncompressed to compressed image with and without overlap.");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    const bool copy_commands_2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    VkFormatProperties format_properties;
    VkFormat mp_format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(gpu(), mp_format, &format_properties);
    if ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT for VK_FORMAT_BC1_RGBA_UNORM_BLOCK, skipping test.\n";
    }

    VkImageObj src_image(m_device);
    src_image.Init(1, 1, 1, VK_FORMAT_R32G32_UINT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);
    VkImageObj dst_image(m_device);
    dst_image.Init(12, 4, 1, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_LINEAR);

    VkImageCopy copy_regions[2] = {};
    copy_regions[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_regions[0].srcSubresource.mipLevel = 0;
    copy_regions[0].srcSubresource.baseArrayLayer = 0;
    copy_regions[0].srcSubresource.layerCount = 1;
    copy_regions[0].srcOffset = {0, 0, 0};
    copy_regions[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_regions[0].dstSubresource.mipLevel = 0;
    copy_regions[0].dstSubresource.baseArrayLayer = 0;
    copy_regions[0].dstSubresource.layerCount = 1;
    copy_regions[0].dstOffset = {0, 0, 0};
    copy_regions[0].extent = {1, 1, 1};
    copy_regions[1].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_regions[1].srcSubresource.mipLevel = 0;
    copy_regions[1].srcSubresource.baseArrayLayer = 0;
    copy_regions[1].srcSubresource.layerCount = 1;
    copy_regions[1].srcOffset = {0, 0, 0};
    copy_regions[1].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_regions[1].dstSubresource.mipLevel = 0;
    copy_regions[1].dstSubresource.baseArrayLayer = 0;
    copy_regions[1].dstSubresource.layerCount = 1;
    copy_regions[1].dstOffset = {4, 0, 0};
    copy_regions[1].extent = {1, 1, 1};

    m_commandBuffer->begin();

    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[0]);
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[1]);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_regions[1].dstOffset = {4, 0, 0};
    vk::CmdCopyImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[1]);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    if (copy_commands_2) {
        m_commandBuffer->reset();

        VkImageCopy2KHR copy_regions2[2];
        copy_regions2[0] = vku::InitStructHelper();
        copy_regions2[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_regions2[0].srcSubresource.mipLevel = 0;
        copy_regions2[0].srcSubresource.baseArrayLayer = 0;
        copy_regions2[0].srcSubresource.layerCount = 1;
        copy_regions2[0].srcOffset = {0, 0, 0};
        copy_regions2[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_regions2[0].dstSubresource.mipLevel = 0;
        copy_regions2[0].dstSubresource.baseArrayLayer = 0;
        copy_regions2[0].dstSubresource.layerCount = 1;
        copy_regions2[0].dstOffset = {0, 0, 0};
        copy_regions2[0].extent = {1, 1, 1};
        copy_regions2[1] = vku::InitStructHelper();
        copy_regions2[1].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_regions2[1].srcSubresource.mipLevel = 0;
        copy_regions2[1].srcSubresource.baseArrayLayer = 0;
        copy_regions2[1].srcSubresource.layerCount = 1;
        copy_regions2[1].srcOffset = {0, 0, 0};
        copy_regions2[1].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_regions2[1].dstSubresource.mipLevel = 0;
        copy_regions2[1].dstSubresource.baseArrayLayer = 0;
        copy_regions2[1].dstSubresource.layerCount = 1;
        copy_regions2[1].dstOffset = {4, 0, 0};
        copy_regions2[1].extent = {1, 1, 1};

        VkCopyImageInfo2KHR copy_image_info = vku::InitStructHelper();
        copy_image_info.srcImage = src_image.handle();
        copy_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        copy_image_info.dstImage = dst_image.handle();
        copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        copy_image_info.regionCount = 2;
        copy_image_info.pRegions = copy_regions2;

        m_commandBuffer->begin();

        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-WRITE-AFTER-WRITE");
        copy_image_info.regionCount = 1;
        copy_image_info.pRegions = &copy_regions2[1];
        copy_regions[1].dstOffset = {7, 0, 0};
        vk::CmdCopyImage2KHR(m_commandBuffer->handle(), &copy_image_info);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeSyncVal, StageAccessExpansion) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_c_a(m_device), image_c_b(m_device);
    const auto image_c_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine, VK_IMAGE_TILING_OPTIMAL);
    image_c_a.Init(image_c_ci);
    image_c_b.Init(image_c_ci);

    VkImageView imageview_c = image_c_a.targetView(format);
    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj image_s_a(m_device), image_s_b(m_device);
    const auto image_s_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage, VK_IMAGE_TILING_OPTIMAL);
    image_s_a.Init(image_s_ci);
    image_s_b.Init(image_s_ci);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_s_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageView imageview_s = image_s_a.targetView(format);

    vkt::Sampler sampler_s, sampler_c;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_s.init(*m_device, sampler_ci);
    sampler_c.init(*m_device, sampler_ci);

    vkt::Buffer buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*m_device, buffer_a.create_info(2048, buffer_usage, nullptr), mem_prop);
    buffer_b.init(*m_device, buffer_b.create_info(2048, buffer_usage, nullptr), mem_prop);

    vkt::BufferView bufferview;
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    bufferview.init(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview.handle());
    descriptor_set.UpdateDescriptorSets();

    // Dispatch
    std::string csSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } ub0;
        layout(set=0, binding=1) uniform sampler2D cis1;
        layout(set=0, binding=2, rgba8) uniform readonly image2D si2;
        layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;
        void main(){
            vec4 vColor4;
            vColor4.x = ub0.x;
            vColor4 = texture(cis1, vec2(0));
            vColor4 = imageLoad(si2, ivec2(0));
            vColor4 = imageLoad(stb3, 0);
        }
    )glsl";

    // Draw
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    vkt::Buffer vbo, vbo2;
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vbo.init(*m_device, vbo.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);
    vbo2.init(*m_device, vbo2.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, csSource.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitState();
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    VkImageSubresourceLayers layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{16, 16, 1};
    VkImageCopy image_region = {layer, zero_offset, layer, zero_offset, full_extent};
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_commandBuffer->handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // wrong: dst stage should be VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &barrier, 0, nullptr, 0, nullptr);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // one error for each image copied above
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Try again with the correct dst stage on the barrier
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_commandBuffer->handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1,
                           &barrier, 0, nullptr, 0, nullptr);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

struct QSTestContext {
    vkt::Device* dev;
    uint32_t q_fam = ~0U;
    VkQueue q0 = VK_NULL_HANDLE;
    VkQueue q1 = VK_NULL_HANDLE;

    vkt::Buffer buffer_a;
    vkt::Buffer buffer_b;
    vkt::Buffer buffer_c;

    VkBufferCopy full_buffer;
    VkBufferCopy first_half;
    VkBufferCopy second_half;
    VkBufferCopy first_to_second;
    VkBufferCopy second_to_first;
    vkt::CommandPool pool;

    vkt::CommandBuffer cba;
    vkt::CommandBuffer cbb;
    vkt::CommandBuffer cbc;

    VkCommandBuffer h_cba = VK_NULL_HANDLE;
    VkCommandBuffer h_cbb = VK_NULL_HANDLE;
    VkCommandBuffer h_cbc = VK_NULL_HANDLE;

    vkt::Semaphore semaphore;
    vkt::Event event;

    vkt::CommandBuffer* current_cb = nullptr;

    QSTestContext(vkt::Device* device, vkt::Queue* force_q0 = nullptr, vkt::Queue* force_q1 = nullptr);
    VkCommandBuffer InitFromPool(vkt::CommandBuffer& cb_obj);
    bool Valid() const { return q1 != VK_NULL_HANDLE; }

    void Begin(vkt::CommandBuffer& cb);
    void BeginA() { Begin(cba); }
    void BeginB() { Begin(cbb); }
    void BeginC() { Begin(cbc); }

    void End();
    void Copy(vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region) {
        vk::CmdCopyBuffer(current_cb->handle(), from.handle(), to.handle(), 1, &copy_region);
    }
    void Copy(vkt::Buffer& from, vkt::Buffer& to) { Copy(from, to, full_buffer); }
    void CopyAToB() { Copy(buffer_a, buffer_b); }
    void CopyAToC() { Copy(buffer_a, buffer_c); }

    void CopyBToA() { Copy(buffer_b, buffer_a); }
    void CopyBToC() { Copy(buffer_b, buffer_c); }

    void CopyCToA() { Copy(buffer_c, buffer_a); }
    void CopyCToB() { Copy(buffer_c, buffer_b); }

    void CopyGeneral(const VkImageObj& from, const VkImageObj& to, const VkImageCopy& region) {
        vk::CmdCopyImage(current_cb->handle(), from.handle(), VK_IMAGE_LAYOUT_GENERAL, to.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                         &region);
    };

    VkBufferMemoryBarrier InitBufferBarrier(const vkt::Buffer& buffer, VkAccessFlags src, VkAccessFlags dst);
    VkBufferMemoryBarrier InitBufferBarrierRAW(const vkt::Buffer& buffer);
    VkBufferMemoryBarrier InitBufferBarrierWAR(const vkt::Buffer& buffer);
    void TransferBarrierWAR(const vkt::Buffer& buffer);
    void TransferBarrierRAW(const vkt::Buffer& buffer);
    void TransferBarrier(const VkBufferMemoryBarrier& buffer_barrier);

    void Submit(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                VkFence fence = VK_NULL_HANDLE);

    // X == Submit 2 but since we already have numeric overloads for the queues X -> eXtension version
    void SubmitX(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkPipelineStageFlags signal_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkFence fence = VK_NULL_HANDLE);

    void Submit0(vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkFence fence = VK_NULL_HANDLE) {
        Submit(q0, cb, wait, wait_mask, signal, fence);
    }
    void Submit0Wait(vkt::CommandBuffer& cb, VkPipelineStageFlags wait_mask) { Submit0(cb, semaphore.handle(), wait_mask); }
    void Submit0Signal(vkt::CommandBuffer& cb) { Submit0(cb, VK_NULL_HANDLE, 0U, semaphore.handle()); }

    void Submit1(vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkFence fence = VK_NULL_HANDLE) {
        Submit(q1, cb, wait, wait_mask, signal, fence);
    }
    void Submit1Wait(vkt::CommandBuffer& cb, VkPipelineStageFlags wait_mask) { Submit1(cb, semaphore.handle(), wait_mask); }
    void Submit1Signal(vkt::CommandBuffer& cb, VkPipelineStageFlags signal_mask) {
        Submit1(cb, VK_NULL_HANDLE, 0U, semaphore.handle());
    }
    void SetEvent(VkPipelineStageFlags src_mask) { event.cmd_set(*current_cb, src_mask); }
    void WaitEventBufferTransfer(vkt::Buffer& buffer, VkPipelineStageFlags src_mask, VkPipelineStageFlags dst_mask) {
        std::vector<VkBufferMemoryBarrier> buffer_barriers(1, InitBufferBarrierWAR(buffer));
        event.cmd_wait(*current_cb, src_mask, dst_mask, std::vector<VkMemoryBarrier>(), buffer_barriers,
                       std::vector<VkImageMemoryBarrier>());
    }

    void ResetEvent(VkPipelineStageFlags src_mask) { event.cmd_reset(*current_cb, src_mask); }

    void QueueWait(VkQueue q) { vk::QueueWaitIdle(q); }
    void QueueWait0() { QueueWait(q0); }
    void QueueWait1() { QueueWait(q1); }
    void DeviceWait() { vk::DeviceWaitIdle(dev->handle()); }

    void RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region);
    void RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to) { RecordCopy(cb, from, to, full_buffer); }
};

QSTestContext::QSTestContext(vkt::Device* device, vkt::Queue* force_q0, vkt::Queue* force_q1)
    : dev(device), q0(VK_NULL_HANDLE), q1(VK_NULL_HANDLE) {
    if (force_q0) {
        q0 = force_q0->handle();
        q_fam = force_q0->get_family_index();
        if (force_q1) {
            // The object has some assumptions that the queues are from the the same family, so enforce this here
            if (force_q1->get_family_index() == q_fam) {
                q1 = force_q1->handle();
            }
        } else {
            q1 = q0;  // Allow the two queues to be the same and valid if forced
        }
    } else {
        const auto& queues = device->dma_queues();

        const uint32_t q_count = static_cast<uint32_t>(queues.size());
        for (uint32_t q0_index = 0; q0_index < q_count; ++q0_index) {
            const auto* q0_entry = queues[q0_index];
            q0 = q0_entry->handle();
            q_fam = q0_entry->get_family_index();
            for (uint32_t q1_index = (q0_index + 1); q1_index < q_count; ++q1_index) {
                const auto* q1_entry = queues[q1_index];
                if (q_fam == q1_entry->get_family_index()) {
                    q1 = q1_entry->handle();
                    break;
                }
            }
            if (Valid()) {
                break;
            }
        }
    }

    if (!Valid()) return;

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*device, 256, transfer_usage, mem_prop);
    buffer_b.init(*device, 256, transfer_usage, mem_prop);
    buffer_c.init(*device, 256, transfer_usage, mem_prop);

    VkDeviceSize size = 256;
    VkDeviceSize half_size = size / 2;
    full_buffer = {0, 0, size};
    first_half = {0, 0, half_size};
    second_half = {half_size, half_size, half_size};
    first_to_second = {0, half_size, half_size};
    second_to_first = {half_size, 0, half_size};

    pool.init(*device, vkt::CommandPool::create_info(q_fam, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    h_cba = InitFromPool(cba);
    h_cbb = InitFromPool(cbb);
    h_cbc = InitFromPool(cbc);

    VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper();
    semaphore.init(*device, semaphore_ci);

    VkEventCreateInfo eci = vku::InitStructHelper();
    event.init(*device, eci);
}

VkCommandBuffer QSTestContext::InitFromPool(vkt::CommandBuffer& cb_obj) {
    cb_obj.Init(dev, &pool);
    return cb_obj.handle();
}

void QSTestContext::Begin(vkt::CommandBuffer& cb) {
    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    info.pInheritanceInfo = nullptr;

    cb.reset();
    cb.begin(&info);
    current_cb = &cb;
}

void QSTestContext::End() {
    current_cb->end();
    current_cb = nullptr;
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrier(const vkt::Buffer& buffer, VkAccessFlags src, VkAccessFlags dst) {
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = src;
    buffer_barrier.dstAccessMask = dst;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.offset = 0;
    buffer_barrier.size = 256;
    return buffer_barrier;
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrierRAW(const vkt::Buffer& buffer) {
    return InitBufferBarrier(buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrierWAR(const vkt::Buffer& buffer) {
    return InitBufferBarrier(buffer, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
}

void QSTestContext::TransferBarrier(const VkBufferMemoryBarrier& buffer_barrier) {
    vk::CmdPipelineBarrier(current_cb->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &buffer_barrier, 0, nullptr);
}

void QSTestContext::TransferBarrierWAR(const vkt::Buffer& buffer) { TransferBarrier(InitBufferBarrierWAR(buffer)); }
void QSTestContext::TransferBarrierRAW(const vkt::Buffer& buffer) { TransferBarrier(InitBufferBarrierRAW(buffer)); }

void QSTestContext::Submit(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait, VkPipelineStageFlags wait_mask, VkSemaphore signal,
                           VkFence fence) {
    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 1;
    VkCommandBuffer h_cb = cb.handle();
    submit1.pCommandBuffers = &h_cb;
    if (wait != VK_NULL_HANDLE) {
        submit1.waitSemaphoreCount = 1;
        submit1.pWaitSemaphores = &wait;
        submit1.pWaitDstStageMask = &wait_mask;
    }
    if (signal != VK_NULL_HANDLE) {
        submit1.signalSemaphoreCount = 1;
        submit1.pSignalSemaphores = &signal;
    }
    vk::QueueSubmit(q, 1, &submit1, fence);
}

void QSTestContext::SubmitX(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait, VkPipelineStageFlags wait_mask, VkSemaphore signal,
                            VkPipelineStageFlags signal_mask, VkFence fence) {
    VkSubmitInfo2 submit1 = vku::InitStructHelper();
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();

    cb_info.commandBuffer = cb.handle();
    submit1.commandBufferInfoCount = 1;
    submit1.pCommandBufferInfos = &cb_info;

    if (wait != VK_NULL_HANDLE) {
        wait_info.semaphore = wait;
        wait_info.stageMask = wait_mask;
        submit1.waitSemaphoreInfoCount = 1;
        submit1.pWaitSemaphoreInfos = &wait_info;
    }
    if (signal != VK_NULL_HANDLE) {
        signal_info.semaphore = signal;
        signal_info.stageMask = signal_mask;
        submit1.signalSemaphoreInfoCount = 1;
        submit1.pSignalSemaphoreInfos = &signal_info;
    }

    vk::QueueSubmit2(q, 1, &submit1, fence);
}

void QSTestContext::RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region) {
    Begin(cb);
    Copy(from, to, copy_region);
    End();
}

TEST_F(NegativeSyncVal, QSBufferCopyHazards) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->graphics_queues()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 2;
    VkCommandBuffer two_cbs[2] = {test.h_cba, test.h_cbb};
    submit1.pCommandBuffers = two_cbs;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::QueueSubmit(test.q0, 1, &submit1, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();

    VkSubmitInfo submit2[2] = {vku::InitStruct<VkSubmitInfo>(), vku::InitStruct<VkSubmitInfo>()};
    submit2[0].commandBufferCount = 1;
    submit2[0].pCommandBuffers = &test.h_cba;
    submit2[1].commandBufferCount = 1;
    submit2[1].pCommandBuffers = &test.h_cbb;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::QueueSubmit(test.q0, 2, submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the global queue contexts as empty
    test.Submit0(test.cba);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();

    // A little grey box testing to ensure the trim code is referenced
    test.BeginA();
    test.Copy(test.buffer_a, test.buffer_c, test.first_half);
    test.Copy(test.buffer_a, test.buffer_c, test.second_half);
    test.End();
    test.Submit0(test.cba);
    test.BeginB();
    test.TransferBarrierWAR(test.buffer_a);
    test.Copy(test.buffer_b, test.buffer_a);
    test.TransferBarrierRAW(test.buffer_c);
    test.TransferBarrierWAR(test.buffer_b);
    test.Copy(test.buffer_c, test.buffer_b);
    test.End();
    test.Submit0(test.cbb);

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSSubmit2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    QSTestContext test(m_device, m_device->graphics_queues()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    // Test that the signal mask is controlling the first scope
    test.SubmitX(test.q0, test.cba, VK_NULL_HANDLE, 0, test.semaphore.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // Since the last submit skipped, we need a wait that will success
    test.BeginC();
    test.End();
    test.Submit0Wait(test.cbc, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    test.DeviceWait();

    // This time with the correct first sync scope.
    test.SubmitX(test.q0, test.cba, VK_NULL_HANDLE, 0, test.semaphore.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.Submit0Wait(test.cbb, VK_PIPELINE_STAGE_TRANSFER_BIT);

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSBufferCopyVsIdle) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->graphics_queues()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    // Submit A
    test.Submit0(test.cba);

    // Submit B which hazards vs. A
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the previous submit as not
    // having happened. So we'll try again with a device wait idle
    // Submit B again, but after idling, which should remove the hazard
    test.DeviceWait();
    test.Submit0(test.cbb);

    // Submit the same command again for another hazard
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the previous submit as not
    // having happened. So we'll try again with a queue wait idle
    // Submit B again, but after idling, which should remove the hazard
    test.QueueWait0();
    test.Submit0(test.cbb);

    m_device->wait();
}

TEST_F(NegativeSyncVal, QSBufferCopyVsFence) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->graphics_queues()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    vkt::Fence fence;
    fence.init(*m_device, vkt::Fence::create_info());
    VkFence fence_handle = fence.handle();
    VkResult wait_result;
    vkt::CommandBuffer cbd;
    test.InitFromPool(cbd);

    // Set up four CB with copy commands
    // We'll wait for the first, but not the second
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_a, test.buffer_c);
    test.RecordCopy(test.cbc, test.buffer_a, test.buffer_b);

    // This is the one that should error
    test.RecordCopy(cbd, test.buffer_a, test.buffer_c);

    // Two copies *better* finish in a second...
    const uint64_t kFourSeconds = 1U << 30;
    // Copy A to B
    test.Submit0(test.cba, VK_NULL_HANDLE, 0U, VK_NULL_HANDLE, fence_handle);
    // Copy A to C
    test.Submit0(test.cbb);
    // Wait for A to B
    wait_result = fence.wait(kFourSeconds);

    if (wait_result != VK_SUCCESS) {
        ADD_FAILURE() << "Fence wait failed. Aborting test.";
        m_device->wait();
    }

    // A and B should be good to go...
    test.Submit0(test.cbc);

    // But C shouldn't
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    test.Submit0(cbd);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSBufferCopyQSORules) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family";
    }

    // Need an extra buffer and CB
    vkt::Buffer buffer_d(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::CommandBuffer cbd;
    test.InitFromPool(cbd);
    // This gives a noop command buffer w.r.t. buffers a, b, and c.
    test.RecordCopy(cbd, buffer_d, buffer_d, test.first_to_second);

    // Command Buffer A reads froms buffer A and writes to buffer B
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);

    // Command Buffer B reads froms buffer C and writes to buffer A, but has a barrier to protect the write to A when
    // executed on the same queue, given that commands in "queue submission order" are within the first scope of the barrier.
    test.BeginB();

    // Use the barrier to clean up the WAR, which will work for command buffers ealier in queue submission order, or with
    // correct semaphore operations between queues.
    test.TransferBarrierWAR(test.buffer_a);
    test.CopyCToA();
    test.End();

    // Command Buffer C does the same copy as B but without the barrier.
    test.RecordCopy(test.cbc, test.buffer_c, test.buffer_a);

    // Submit A and B on the same queue, to assure us the barrier *would* be sufficient given QSO
    // This is included in a "Sucess" section, just to verify CBA and CBB are set up correctly.
    test.Submit0(test.cba);
    test.Submit0(test.cbb);
    m_device->wait();  // DeviceWaitIdle, clearing the field for the next subcase

    // Submit A and B on the different queues. Since no semaphore is used between the queues, CB B hazards asynchronously with,
    // CB A with A being read and written on independent queues.
    test.Submit0(test.cba);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-READ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();
    m_device->wait();  // DeviceWaitIdle, clearing the field for the next subcase

    // Test full async detection
    test.Submit0(test.cba);
    test.Submit0(cbd);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-READ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Set up the semaphore for the next two cases

    m_device->wait();

    // Submit A and B on the different queues, with an ineffectual semaphore.  The wait mask is empty, thus nothing in CB B is in
    // the second excution scope of the waited signal.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0Signal(test.cba);
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);  // wait mask is BOTTOM, s.t. this is a wait-for-nothing.
    m_errorMonitor->VerifyFound();

    // The since second submit failed, it was skipped. So we can try again, without having to WaitDeviceIdle
    // Include transfers in the second execution scope of the waited signal, s.t. the PipelineBarrier in CB B can chain with it.
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_TRANSFER_BIT);  //

    m_device->wait();

    // Draw A and then C to verify the second access scope of the signal
    test.Submit0Signal(test.cba);
    test.Submit1Wait(test.cbc, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_device->wait();

    //  ... and again on the same queue
    test.Submit0Signal(test.cba);
    test.Submit0Wait(test.cbc, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_device->wait();
}

TEST_F(NegativeSyncVal, QSBufferEvents) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family";
    }

    // Command Buffer A reads froms buffer A and writes to buffer B
    test.BeginA();
    test.CopyAToB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.End();

    // Command Buffer B reads froms buffer C and writes to buffer A, but has a wait to protect the write to A when
    // executed on the same queue, given that commands in "queue submission order" are within the first scope of the barrier.
    test.BeginB();

    // Use the barrier to clean up the WAR, which will work for command buffers ealier in queue submission order, or with
    // correct semaphore operations between queues.
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Command Buffer C merges the operations from A and B, to ensure the set/wait is correct.
    //    reads froms buffer A and writes to buffer B
    //    reads froms buffer C and writes to buffer A, but has a barrier to protect the write to A when
    test.BeginC();
    test.CopyAToB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Ensure this would work on one queue (sanity check)
    vkt::CommandBuffer reset(test.dev, &test.pool);
    test.Begin(reset);
    test.ResetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.End();

    // Reset the event s.t. I reuse it
    test.Submit0(reset);
    m_device->wait();

    test.Submit0(test.cba);
    test.Submit0(test.cbb);

    // Ensure that the wait doesn't apply to async queues
    test.Submit0(reset);
    m_device->wait();

    test.Submit0(test.cba);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-READ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Ensure that the wait doesn't apply to access on other synchronized queues
    m_device->wait();

    test.Submit0Signal(test.cba);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    // Need to have a successful signal wait to get the semaphore in a usuable state.
    test.BeginC();
    test.End();
    test.Submit1Wait(test.cbc, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_device->wait();

    // Next ensure that accesses from other queues aren't included in the first scope
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);

    test.BeginB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Sanity check that same queue works
    test.Submit0(reset);
    m_device->wait();
    test.Submit0(test.cba);
    test.Submit0(test.cbb);

    // Reset the signal
    test.Submit0(reset);
    m_device->wait();

    test.Submit0Signal(test.cba);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, QSOBarrierHazard) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family.";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);

    VkImageObj image_a(m_device);
    image_a.Init(image_ci);
    ASSERT_TRUE(image_a.initialized());
    image_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    ASSERT_TRUE(image_b.initialized());
    image_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageSubresourceLayers all_layers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkImageCopy full_region = {all_layers, zero_offset, all_layers, zero_offset, full_extent};

    test.BeginA();
    test.CopyGeneral(image_a, image_b, full_region);
    test.End();

    test.BeginB();
    image_a.ImageMemoryBarrier(test.current_cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_NONE, VK_ACCESS_NONE,
                               VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    test.End();

    // We're going to do the copy first, then use the skip on fail, to test three different ways...
    test.Submit0Signal(test.cba);

    // First asynchronously fail -- the pipeline barrier in B shouldn't work on queue 1
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-RACING-READ ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Next synchronously fail -- the pipeline barrier in B shouldn't work on queue 1
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    // Then prove qso works (note that with the failure, the semaphore hasn't been waited, nor the layout changed)
    test.Submit0Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    m_device->wait();
}

TEST_F(NegativeSyncVal, QSRenderPass) {
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState());

    CreateRenderPassHelper rp_helper(m_device);
    rp_helper.InitAllAttachmentsToLayoutGeneral();

    rp_helper.InitState();
    rp_helper.InitAttachmentLayouts();  // Quiet any CoreChecks ImageLayout complaints
    m_device->wait();                   // and quiesce the system

    // The  dependency protects the input attachment but not the color attachment
    VkSubpassDependency protect_input_subpass_0 = {VK_SUBPASS_EXTERNAL,
                                                   0,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                                   0U};
    rp_helper.subpass_dep.push_back(protect_input_subpass_0);

    rp_helper.InitRenderPass();
    rp_helper.InitFramebuffer();
    rp_helper.InitBeginInfo();

    vkt::CommandBuffer cb0(m_device, m_commandPool);
    vkt::CommandBuffer cb1(m_device, m_commandPool);

    auto do_clear = [](vkt::CommandBuffer& cb_obj, CreateRenderPassHelper& rp_helper) {
        VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb_obj.handle(), rp_helper.image_input->handle(), VK_IMAGE_LAYOUT_GENERAL, &rp_helper.ccv, 1,
                               &full_subresource_range);
        vk::CmdClearColorImage(cb_obj.handle(), rp_helper.image_color->handle(), VK_IMAGE_LAYOUT_GENERAL, &rp_helper.ccv, 1,
                               &full_subresource_range);
    };

    // Single renderpass barrier  (sanity check)
    cb0.begin();
    do_clear(cb0, rp_helper);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb0.BeginRenderPass(rp_helper.render_pass_begin);
    m_errorMonitor->VerifyFound();
    // No "end render pass" as the begin fails

    cb0.end();
    cb0.reset();

    // Inter CB detection (dual cb), load is safe, clear errors at submit time
    cb0.begin();
    do_clear(cb0, rp_helper);
    cb0.end();

    cb1.begin();
    cb1.BeginRenderPass(rp_helper.render_pass_begin);
    cb1.EndRenderPass();
    cb1.end();

    VkSubmitInfo submit2 = vku::InitStructHelper();
    VkCommandBuffer two_cbs[2] = {cb0.handle(), cb1.handle()};
    submit2.commandBufferCount = 2;
    submit2.pCommandBuffers = two_cbs;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::QueueSubmit(m_default_queue, 1, &submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_device->wait();  // quiesce the system for the next subtest

    CreateRenderPassHelper rp_helper2(m_device);
    rp_helper2.InitAllAttachmentsToLayoutGeneral();

    rp_helper2.InitState();
    rp_helper2.InitAttachmentLayouts();  // Quiet any CoreChecks ImageLayout complaints
    m_device->wait();                    // and quiesce the system

    // The  dependency protects the input attachment but not the color attachment
    VkSubpassDependency protect_input_subpass_1 = protect_input_subpass_0;
    protect_input_subpass_1.dstSubpass = 1;
    rp_helper2.subpass_dep.push_back(protect_input_subpass_1);

    // Two subpasses to ensure that the "next subpass" error checks work
    rp_helper2.InitAttachmentArrays();
    rp_helper2.AddPreserveInputColorSubpassDescription();
    rp_helper2.AddInputColorSubpassDescription();

    rp_helper2.InitRenderPass();
    rp_helper2.InitFramebuffer();
    rp_helper2.InitBeginInfo();

    // Single CB sanity check
    cb0.reset();
    cb0.begin();
    do_clear(cb0, rp_helper2);
    cb0.BeginRenderPass(rp_helper2.render_pass_begin);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb0.NextSubpass();
    m_errorMonitor->VerifyFound();
    cb0.end();

    cb0.reset();
    cb0.begin();
    do_clear(cb0, rp_helper2);
    cb0.end();

    cb1.reset();
    cb1.begin();
    cb1.BeginRenderPass(rp_helper2.render_pass_begin);
    cb1.NextSubpass();
    cb1.EndRenderPass();
    cb1.end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::QueueSubmit(m_default_queue, 1, &submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_device->wait();  // and quiesce the system
}

// Wrap FAIL:
//  * DRY for common messages
//  * for test stability reasons sometimes cleanup code is required *prior* to the return hidden in FAIL
//  * result_arg_ *can* (should) have side-effect, but is referenced exactly once
//  * label_ must be converitble to bool, and *should* *not* have side-effects
//  * clean_ *can* (should) have side-effects
//    * "{}" or ";" are valid clean_ values for noop
#define REQUIRE_SUCCESS(result_arg_, label_, clean_)                    \
    {                                                                   \
        const VkResult result_ = (result_arg_);                         \
        if (result_ != VK_SUCCESS) {                                    \
            {                                                           \
                clean_;                                                 \
            }                                                           \
            if (bool(label_)) {                                         \
                FAIL() << string_VkResult(result_) << ": " << (label_); \
            } else {                                                    \
                FAIL() << string_VkResult(result_);                     \
            }                                                           \
        }                                                               \
    }

TEST_F(NegativeSyncVal, QSPresentAcquire) {
    TEST_DESCRIPTION("Try destroying a swapchain presentable image with vkDestroyImage");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework(true));  // Enable QueueSubmit validation
    RETURN_IF_SKIP(InitState())
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    std::vector<VkImage> images;
    ASSERT_EQ(VK_SUCCESS, vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    images.resize(image_count, VK_NULL_HANDLE);
    ASSERT_EQ(VK_SUCCESS, vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, images.data()));

    std::vector<bool> image_used(images.size(), false);

    const VkCommandBuffer cb = m_commandBuffer->handle();
    const VkQueue q = m_default_queue;
    const VkDevice dev = m_device->handle();

    VkFenceCreateInfo fence_ci = vku::InitStructHelper();
    vkt::Fence fence(*m_device, fence_ci);
    VkFence h_fence = fence.handle();

    // Test stability requires that we wait on pending operations before returning starts the Vk*Obj destructors
    auto cleanup = [this]() { m_device->wait(); };

    // Loop through the indices until we find one we are reusing...
    // When fence is non-null this can timeout so we need to track results
    auto present_image = [this, q](uint32_t index, vkt::Semaphore* sem, vkt::Fence* fence) {
        VkResult result = VK_SUCCESS;
        if (fence) {
            result = fence->wait(kWaitTimeout);
            if (VK_SUCCESS == result) {
                fence->reset();
            }
        }

        if (VK_SUCCESS == result) {
            VkPresentInfoKHR present_info = vku::InitStructHelper();
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = &index;
            VkSemaphore h_sem = VK_NULL_HANDLE;
            if (sem) {
                h_sem = sem->handle();
                present_info.waitSemaphoreCount = 1;
                present_info.pWaitSemaphores = &h_sem;
            }
            vk::QueuePresentKHR(q, &present_info);
        }
        return result;
    };

    // Acquire can always timeout, so we need to track results
    auto acquire_used_image = [this, &image_used, dev, &present_image](vkt::Semaphore* sem, vkt::Fence* fence, uint32_t& index) {
        VkSemaphore h_sem = sem ? sem->handle() : VK_NULL_HANDLE;
        VkFence h_fence = fence ? fence->handle() : VK_NULL_HANDLE;
        VkResult result = VK_SUCCESS;

        while (true) {
            result = vk::AcquireNextImageKHR(dev, m_swapchain, kWaitTimeout, h_sem, h_fence, &index);
            if ((result != VK_SUCCESS) || image_used[index]) break;

            result = present_image(index, sem, fence);
            if (result != VK_SUCCESS) break;
            image_used[index] = true;
        }
        return result;
    };

    uint32_t acquired_index = 0;
    REQUIRE_SUCCESS(acquire_used_image(nullptr, &fence, acquired_index), "acquire_used_image", cleanup());

    auto write_barrier_cb = [this](const VkImage h_image, VkImageLayout from, VkImageLayout to) {
        VkImageSubresourceRange full_image{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
        image_barrier.srcAccessMask = 0U;
        image_barrier.dstAccessMask = 0U;
        image_barrier.oldLayout = from;
        image_barrier.newLayout = to;
        image_barrier.image = h_image;

        image_barrier.subresourceRange = full_image;
        m_commandBuffer->begin();
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);
        m_commandBuffer->end();
    };
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Look for errors between the acquire and first use...
    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 1;
    submit1.pCommandBuffers = &cb;
    // No sync operations...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-PRESENT");
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Sync operations that should ignore present operations
    m_device->wait();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-PRESENT");
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Finally we wait for the fence associated with the acquire
    REQUIRE_SUCCESS(vk::WaitForFences(m_device->handle(), 1, &h_fence, VK_TRUE, kWaitTimeout), "WaitForFences", cleanup());
    fence.reset();
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);
    m_device->wait();

    // Release the image back to the present engine, so we don't run out
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout

    auto semaphore_ci = vkt::Semaphore::create_info(0);
    vkt::Semaphore sem(*m_device, semaphore_ci);
    const VkSemaphore h_sem = sem.handle();
    REQUIRE_SUCCESS(acquire_used_image(&sem, nullptr, acquired_index), "acquire_used_image", cleanup());

    m_commandBuffer->reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    submit1.waitSemaphoreCount = 1;
    submit1.pWaitDstStageMask = &wait_mask;
    submit1.pWaitSemaphores = &h_sem;

    // The wait mask doesn't match the operations in the command buffer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-READ");
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Now then wait mask matches the operations in the command buffer
    wait_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);

    // Try presenting without waiting for the ILT to finish
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-PRESENT-AFTER-WRITE");
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    // Let the ILT complete, and the release the image back
    m_device->wait();
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout

    REQUIRE_SUCCESS(acquire_used_image(VK_NULL_HANDLE, &fence, acquired_index), "acquire_used_index", cleanup());
    REQUIRE_SUCCESS(fence.wait(kWaitTimeout), "WaitForFences", cleanup());

    m_commandBuffer->reset();
    write_barrier_cb(images[acquired_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    fence.reset();
    submit1.waitSemaphoreCount = 0;
    submit1.pWaitDstStageMask = nullptr;
    submit1.pWaitSemaphores = nullptr;
    submit1.signalSemaphoreCount = 1;
    submit1.pSignalSemaphores = &h_sem;
    vk::QueueSubmit(q, 1, &submit1, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-PRESENT-AFTER-WRITE");
    present_image(acquired_index, nullptr, nullptr);  // present without fence can't timeout
    m_errorMonitor->VerifyFound();

    present_image(acquired_index, &sem, nullptr);  // present without fence can't timeout
    m_device->wait();
}

TEST_F(NegativeSyncVal, PresentDoesNotWaitForSubmit2) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework(true));
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS,
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index));

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dep_info);
    m_commandBuffer->end();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = acquire_semaphore;
    wait_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkCommandBufferSubmitInfo command_buffer_info = vku::InitStructHelper();
    command_buffer_info.commandBuffer = *m_commandBuffer;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = submit_semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &command_buffer_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2(m_default_queue, 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 0;  // DO NOT wait on submit. This should generate present after write (ILT) harard.
    present.pWaitSemaphores = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-PRESENT-AFTER-WRITE");
    vk::QueuePresentKHR(m_default_queue, &present);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, PresentDoesNotWaitForSubmit) {
    TEST_DESCRIPTION("Present does not specify semaphore to wait for submit.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(InitSyncValFramework(true));
    RETURN_IF_SKIP(InitState())
    if (!InitSwapchain()) {
        GTEST_SKIP() << "Cannot create surface or swapchain";
    }
    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);
    const auto swapchain_images = GetSwapchainImages(m_swapchain);

    uint32_t image_index = 0;
    ASSERT_EQ(VK_SUCCESS,
        vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, acquire_semaphore, VK_NULL_HANDLE, &image_index));

    VkImageMemoryBarrier layout_transition = vku::InitStructHelper();
    layout_transition.srcAccessMask = 0;
    layout_transition.dstAccessMask = 0;

    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layout_transition.image = swapchain_images[image_index];
    layout_transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layout_transition.subresourceRange.baseMipLevel = 0;
    layout_transition.subresourceRange.levelCount = 1;
    layout_transition.subresourceRange.baseArrayLayer = 0;
    layout_transition.subresourceRange.layerCount = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &layout_transition);
    m_commandBuffer->end();

    constexpr VkPipelineStageFlags semaphore_wait_stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquire_semaphore.handle();
    submit.pWaitDstStageMask = &semaphore_wait_stage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &submit_semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE));

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 0;  // DO NOT wait on submit. This should generate present after write (ILT) harard.
    present.pWaitSemaphores = nullptr;
    present.swapchainCount = 1;
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &image_index;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-PRESENT-AFTER-WRITE");
    vk::QueuePresentKHR(m_default_queue, &present);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, AvailabilityWithoutVisibilityForBuffer) {
    TEST_DESCRIPTION("Buffer barrier makes writes available but not visible. The second write generates WAW harard.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    const vkt::Buffer staging_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const vkt::Buffer buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region = {};
    region.size = size;

    m_commandBuffer->begin();
    // Perform a copy
    vk::CmdCopyBuffer(*m_commandBuffer, staging_buffer, buffer, 1, &region);

    // Make writes available
    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.buffer = buffer;
    barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier, 0, nullptr);

    // Perform one more copy. Should generate WAW due to missing visibility operation.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(*m_commandBuffer, staging_buffer, buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}
