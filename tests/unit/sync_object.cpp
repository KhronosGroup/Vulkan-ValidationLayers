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
#include "../framework/barrier_queue_family.h"

TEST_F(NegativeSyncObject, ImageBarrierSubpassConflicts) {
    TEST_DESCRIPTION("Add a pipeline barrier within a subpass that has conflicting state");
    RETURN_IF_SKIP(Init())

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dep};
    vkt::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    rpci.dependencyCount = 0;
    rpci.pDependencies = nullptr;

    vkt::RenderPass rp_noselfdep(*m_device, rpci);
    ASSERT_TRUE(rp_noselfdep.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    fbci.renderPass = rp_noselfdep.handle();
    vkt::Framebuffer fb_noselfdep(*m_device, fbci);
    ASSERT_TRUE(fb_noselfdep.initialized());

    m_commandBuffer->begin();
    VkRenderPassBeginInfo rpbi = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp_noselfdep.handle(), fb_noselfdep.handle(),
                                                                      VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();

    rpbi.renderPass = rp.handle();
    rpbi.framebuffer = fb.handle();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    // Mis-match src stage mask
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // // Now mis-match dst stage mask
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // Set srcQueueFamilyIndex to something other than IGNORED
    img_barrier.srcQueueFamilyIndex = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // Mis-match mem barrier src access mask
    mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();

    // Mis-match mem barrier dst access mask. Also set srcAccessMask to 0 which should not cause an error
    mem_barrier.srcAccessMask = 0;
    mem_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();

    // Mis-match image barrier src access mask
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Mis-match image barrier dst access mask
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Mis-match dependencyFlags
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0 /* wrong */, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // Send non-zero bufferMemoryBarrierCount
    // Construct a valid BufferMemoryBarrier to avoid any parameter errors
    // First we need a valid buffer to reference
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    // Add image barrier w/ image handle that's not in framebuffer
    VkImageObj lone_image(m_device);
    lone_image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    img_barrier.image = lone_image.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Have image barrier with mis-matched layouts
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
}

TEST_F(NegativeSyncObject, BufferMemoryBarrierNoBuffer) {
    // Try to add a buffer memory barrier with no buffer.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-RequiredParameter");

    RETURN_IF_SKIP(Init())
    m_commandBuffer->begin();

    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
    buf_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.buffer = VK_NULL_HANDLE;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                           nullptr, 1, &buf_barrier, 0, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, Barriers) {
    TEST_DESCRIPTION("A variety of ways to get VK_INVALID_BARRIER ");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // Make sure extensions for multi-planar and separate depth stencil images are enabled if possible
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const bool external_memory = IsExtensionsEnabled(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    const bool maintenance2 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    const bool feedback_loop_layout = IsExtensionsEnabled(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    const bool video_decode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR separate_depth_stencil_layouts_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(separate_depth_stencil_layouts_features);
    if (separate_depth_stencil_layouts_features.separateDepthStencilLayouts != VK_TRUE) {
        GTEST_SKIP() << "separateDepthStencilLayouts feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    // Add a token self-dependency for this test to avoid unexpected errors
    m_addRenderPassSelfDependency = true;
    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->phy().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, false);

    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test(" used with no memory bound. Memory should be bound by calling vkBindImageMemory()",
              " used with no memory bound. Memory should be bound by calling vkBindBufferMemory()");

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    conc_test.buffer_barrier_.buffer = buffer.handle();

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    conc_test.image_barrier_.image = image.handle();

    // New layout can't be UNDEFINED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    conc_test("VUID-VkImageMemoryBarrier-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Can't send buffer memory barrier during a render pass
    m_commandBuffer->EndRenderPass();

    // Duplicate barriers that change layout
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier img_barriers[2] = {img_barrier, img_barrier};

    // Transitions from UNDEFINED  are valid, even if duplicated
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);
    m_errorMonitor->VerifyFound();

    if (!external_memory) {
        printf("External memory extension not supported, skipping external queue family subcase\n");
    } else {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.create_info().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.create_info().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01188");

    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;

    // Now exercise barrier aspect bit errors, first DS
    VkImageObj ds_image(m_device);
    ds_image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier-image-03319");

    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test("VUID-VkImageMemoryBarrier-aspectMask-08702");

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    conc_test("VUID-VkImageMemoryBarrier-aspectMask-08703");

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

    // Now test depth-only
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D16_UNORM, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkImageObj d_image(m_device);
        d_image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = d_image.handle();

        // DEPTH bit must be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    }

    // Now test stencil-only
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_S8_UINT, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkImageObj s_image(m_device);
        s_image.Init(128, 128, 1, VK_FORMAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = s_image.handle();

        // Use of COLOR aspect on depth image is error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // must have the VK_IMAGE_ASPECT_STENCIL_BIT set
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    }

    // Finally test color
    VkImageObj c_image(m_device);
    c_image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(c_image.initialized());
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier-image-09241");

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier-image-09241");

    // Test multip-planar image
    if (mp_extensions) {
        VkFormatProperties format_properties;
        VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_properties);
        constexpr VkImageAspectFlags disjoint_sampled = VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        if (disjoint_sampled == (format_properties.optimalTilingFeatures & disjoint_sampled)) {
            VkImageCreateInfo image_create_info = vku::InitStructHelper();
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
            image_create_info.extent.width = 64;
            image_create_info.extent.height = 64;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

            VkImage mp_image;
            VkDeviceMemory plane_0_memory;
            VkDeviceMemory plane_1_memory;
            ASSERT_EQ(VK_SUCCESS, vk::CreateImage(m_device->device(), &image_create_info, NULL, &mp_image));

            VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

            VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
            mem_req_info2.image = mp_image;
            VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
            vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

            // Find a valid memory type index to memory to be allocated from
            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &plane_0_memory));

            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
            vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &plane_1_memory));

            VkBindImagePlaneMemoryInfo plane_0_memory_info = vku::InitStructHelper();
            plane_0_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            VkBindImagePlaneMemoryInfo plane_1_memory_info = vku::InitStructHelper();
            plane_1_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = vku::InitStructHelper(&plane_0_memory_info);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = plane_0_memory;
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = bind_image_info[0];
            bind_image_info[1].pNext = &plane_1_memory_info;
            bind_image_info[1].memory = plane_1_memory;
            vk::BindImageMemory2KHR(device(), 2, bind_image_info);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.image = mp_image;

            // Test valid usage first
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
            conc_test("", "", VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
            conc_test("VUID-VkImageMemoryBarrier-image-01672");

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
            conc_test("VUID-VkImageMemoryBarrier-image-01672");

            vk::FreeMemory(device(), plane_0_memory, NULL);
            vk::FreeMemory(device(), plane_1_memory, NULL);
            vk::DestroyImage(m_device->device(), mp_image, nullptr);
        }
    }

    // A barrier's new and old VkImageLayout must be compatible with an image's VkImageUsageFlags.
    {
        VkImageObj img_color(m_device);
        img_color.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_color.initialized());

        VkImageObj img_ds(m_device);
        img_ds.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds.initialized());

        VkImageObj img_xfer_src(m_device);
        img_xfer_src.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_src.initialized());

        VkImageObj img_xfer_dst(m_device);
        img_xfer_dst.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_dst.initialized());

        VkImageObj img_sampled(m_device);
        img_sampled.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_sampled.initialized());

        VkImageObj img_input(m_device);
        img_input.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_input.initialized());

        struct BadBufferTest {
            VkImageObj &image_obj;
            VkImageLayout bad_layout;
            std::string msg_code;
        };
        // clang-format off
        std::vector<BadBufferTest> bad_buffer_layouts = {
            // images _without_ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
        };
        if (video_decode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            // // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            // // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
        }
        if (video_encode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
        }
        // clang-format on

        for (const auto &test : bad_buffer_layouts) {
            const VkImageLayout bad_layout = test.bad_layout;
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
                                            (bad_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
                continue;
            }
            conc_test.image_barrier_.image = test.image_obj.handle();
            const VkImageUsageFlags usage = test.image_obj.usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(test.msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_layout;
            conc_test(test.msg_code);
        }

        if (feedback_loop_layout) {
            conc_test.image_barrier_.image = img_color.handle();
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test("VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07006");
        }

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = image.handle();
    }

    // Attempt barrier where srcAccessMask is not supported by srcStageMask
    // Have bit that's supported (transfer write), and another that isn't to verify multi-bit validation
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-vkCmdPipelineBarrier-srcAccessMask-02815");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-vkCmdPipelineBarrier-dstAccessMask-02816");

    // Attempt to mismatch barriers/waitEvents calls with incompatible queues
    // Create command pool with incompatible queueflags
    const std::vector<VkQueueFamilyProperties> queue_props = m_device->phy().queue_properties_;
    const std::optional<uint32_t> queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, false);
    if (!queue_family_index) {
        GTEST_SKIP() << "No non-graphics queue supporting compute found; skipped";
    }

    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-06461");

    vkt::CommandPool command_pool(*m_device, queue_family_index.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    vk::CmdPipelineBarrier(bad_command_buffer.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Check for error for trying to wait on pipeline stage not supported by this queue. Specifically since our queue is not a
    // compute queue, vk::CmdWaitEvents cannot have it's source stage mask be VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-06459");
    vkt::Event event(*m_device);
    vk::CmdWaitEvents(bad_command_buffer.handle(), 1, &event.handle(), /*source stage mask*/ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    bad_command_buffer.end();
}

TEST_F(NegativeSyncObject, Sync2Barriers) {
    TEST_DESCRIPTION("Synchronization2 test for invalid Memory Barriers");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    const bool maintenance2 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    const bool feedback_loop_layout = IsExtensionsEnabled(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    const bool video_decode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    // Add a token self-dependency for this test to avoid unexpected errors
    m_addRenderPassSelfDependency = true;
    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    Barrier2QueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    Barrier2QueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, false);

    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test(" used with no memory bound. Memory should be bound by calling vkBindImageMemory()",
              " used with no memory bound. Memory should be bound by calling vkBindBufferMemory()");

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    conc_test.buffer_barrier_.buffer = buffer.handle();

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    conc_test.image_barrier_.image = image.handle();

    // New layout can't be PREINITIALIZED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    conc_test("VUID-VkImageMemoryBarrier2-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Can't send buffer memory barrier during a render pass
    m_commandBuffer->EndRenderPass();

    // Duplicate barriers that change layout
    VkImageMemoryBarrier2KHR img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier2KHR img_barriers[2] = {img_barrier, img_barrier};

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 2;
    dep_info.pImageMemoryBarriers = img_barriers;
    dep_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Transitions from UNDEFINED  are valid, even if duplicated
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-oldLayout-01197");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);
    m_errorMonitor->VerifyFound();

    {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &img_barrier;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        img_barrier.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        img_barrier.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.create_info().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier2-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.create_info().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01188");

    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;

    // Now exercise barrier aspect bit errors, first DS
    VkImageObj ds_image(m_device);
    ds_image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    {
        conc_test("VUID-VkImageMemoryBarrier2-image-03320");

        // Having only one of depth or stencil set for DS image is an error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        conc_test("VUID-VkImageMemoryBarrier2-image-03320");
    }

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

    // Now test depth-only
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D16_UNORM, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkImageObj d_image(m_device);
        d_image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = d_image.handle();

        // DEPTH bit must be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
        conc_test("depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("depth-only image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT set.");
    }

    // Now test stencil-only
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_S8_UINT, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkImageObj s_image(m_device);
        s_image.Init(128, 128, 1, VK_FORMAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = s_image.handle();

        // Use of COLOR aspect on depth image is error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("stencil-only image formats must have the VK_IMAGE_ASPECT_STENCIL_BIT set.");
    }

    // Finally test color
    VkImageObj c_image(m_device);
    c_image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(c_image.initialized());
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier2-image-09241");

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier2-image-09241");

    // A barrier's new and old VkImageLayout must be compatible with an image's VkImageUsageFlags.
    {
        VkImageObj img_color(m_device);
        img_color.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_color.initialized());

        VkImageObj img_ds(m_device);
        img_ds.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds.initialized());

        VkImageObj img_xfer_src(m_device);
        img_xfer_src.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_src.initialized());

        VkImageObj img_xfer_dst(m_device);
        img_xfer_dst.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_dst.initialized());

        VkImageObj img_sampled(m_device);
        img_sampled.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_sampled.initialized());

        VkImageObj img_input(m_device);
        img_input.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_input.initialized());

        struct BadBufferTest {
            VkImageObj &image_obj;
            VkImageLayout bad_layout;
            std::string msg_code;
        };
        // clang-format off
        std::vector<BadBufferTest> bad_buffer_layouts = {
            // images _without_ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
        };
        if (video_decode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
        }
        if (video_encode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
        }
        // clang-format on

        for (const auto &test : bad_buffer_layouts) {
            const VkImageLayout bad_layout = test.bad_layout;
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
                                            (bad_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
                continue;
            }
            conc_test.image_barrier_.image = test.image_obj.handle();
            const VkImageUsageFlags usage = test.image_obj.usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(test.msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_layout;
            conc_test(test.msg_code);
        }

        if (feedback_loop_layout) {
            conc_test.image_barrier_.image = img_color.handle();
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test("VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07006");
        }

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = image.handle();
    }

    // Attempt barrier where srcAccessMask is not supported by srcStageMask
    // Have lower-order bit that's supported (shader write), but higher-order bit not supported to verify multi-bit validation
    // TODO: synchronization2 has a separate VUID for every access flag. Gotta test them all..
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-VkBufferMemoryBarrier2-srcAccessMask-03909");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-VkBufferMemoryBarrier2-dstAccessMask-03911");

    // Attempt to mismatch barriers/waitEvents calls with incompatible queues
    // Create command pool with incompatible queueflags
    const std::vector<VkQueueFamilyProperties> queue_props = m_device->phy().queue_properties_;
    const std::optional<uint32_t> queue_family_index =
        m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, false);
    if (!queue_family_index) {
        GTEST_SKIP() << "No non-graphics queue supporting compute found";
    }

    VkBufferMemoryBarrier2KHR buf_barrier = vku::InitStructHelper();
    buf_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buf_barrier;

    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dep_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-srcStageMask-03849");

    vkt::CommandPool command_pool(*m_device, queue_family_index.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vk::CmdPipelineBarrier2KHR(bad_command_buffer.handle(), &dep_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, DepthStencilImageNonSeparate) {
    TEST_DESCRIPTION("test barrier with depth/stencil image, with wrong aspect mask with not separateDepthStencilLayouts");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, true);

    m_commandBuffer->begin();

    const VkFormat depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier-image-03320");

    // Having only one of depth or stencil set for DS image is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    conc_test("VUID-VkImageMemoryBarrier-image-03320");
}

TEST_F(NegativeSyncObject, DepthStencilImageNonSeparateSync2) {
    TEST_DESCRIPTION("test barrier with depth/stencil image, with wrong aspect mask with not separateDepthStencilLayouts");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));
    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    Barrier2QueueFamilyTestHelper::Context test_context(this, qf_indices);
    Barrier2QueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, true);

    m_commandBuffer->begin();

    const VkFormat depth_format = FindSupportedDepthStencilFormat(gpu());
    VkImageObj ds_image(m_device);
    ds_image.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test("VUID-VkImageMemoryBarrier2-image-03320");

    // Having only one of depth or stencil set for DS image is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    conc_test("VUID-VkImageMemoryBarrier2-image-03320");
}

TEST_F(NegativeSyncObject, BarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->phy().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        printf("Device has apiVersion greater than 1.0 -- skipping test cases that require external memory to be disabled.\n");
    } else {
        if (only_one_family) {
            printf("Single queue family found -- VK_SHARING_MODE_CONCURRENT testcases skipped.\n");
        } else {
            std::vector<uint32_t> families = {submit_family, other_family};
            BarrierQueueFamilyTestHelper conc_test(&test_context);
            conc_test.Init(&families);
            {
                // src
                static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
                static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
                conc_test(img_vuid, buf_vuid, submit_family, VK_QUEUE_FAMILY_IGNORED);
            }
            {
                // dst
                static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09054";
                static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09051";
                conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, submit_family);
            }
            {
                // neither
                static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
                static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
                conc_test(img_vuid, buf_vuid, submit_family, submit_family);
            }
            conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
        }

        BarrierQueueFamilyTestHelper excl_test(&test_context);
        excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

        excl_test("VUID-VkImageMemoryBarrier-image-09117", "VUID-VkBufferMemoryBarrier-buffer-09095", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        excl_test("VUID-VkImageMemoryBarrier-image-09118", "VUID-VkBufferMemoryBarrier-buffer-09096", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        excl_test(submit_family, submit_family);
        excl_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
    }

    if (only_one_family) {
        printf("Single queue family found -- VK_SHARING_MODE_EXCLUSIVE submit testcases skipped.\n");
    } else {
        BarrierQueueFamilyTestHelper excl_test(&test_context);
        excl_test.Init(nullptr);

        // Although other_family does not match submit_family, because the barrier families are
        // equal here, no ownership transfer actually happens, and this barrier is valid by the spec.
        excl_test(other_family, other_family, submit_family);

        // positive test (testing both the index logic and the QFO transfer tracking.
        excl_test(submit_family, other_family, submit_family);
        excl_test(submit_family, other_family, other_family);
        excl_test(other_family, submit_family, other_family);
        excl_test(other_family, submit_family, submit_family);

        // negative testing for QFO transfer tracking
        // Duplicate release in one CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00001", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001", submit_family,
                  other_family, submit_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
        // Duplicate pending release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00003", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00003", submit_family,
                  other_family, submit_family);
        // Duplicate acquire in one CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00001", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001", submit_family,
                  other_family, other_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
        // No pending release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00004", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00004", submit_family,
                  other_family, other_family);
        // Duplicate release in two CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00002", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002", submit_family,
                  other_family, submit_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);
        // Duplicate acquire in two CB
        excl_test(submit_family, other_family, submit_family);  // need a succesful release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00002", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002", submit_family,
                  other_family, other_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);

        // Need a third queue family to test this.
        uint32_t third_family = VK_QUEUE_FAMILY_IGNORED;
        for (uint32_t candidate = 0; candidate < queue_family_count; ++candidate) {
            if (candidate != submit_family && candidate != other_family &&
                m_device->phy().queue_properties_[candidate].queueCount != 0) {
                third_family = candidate;
                break;
            }
        }

        if (third_family == VK_QUEUE_FAMILY_IGNORED) {
            printf("No third queue family found -- test skipped.\n");
        } else {
            excl_test("VUID-vkQueueSubmit-pSubmits-04626", "VUID-vkQueueSubmit-pSubmits-04626", other_family, third_family,
                      submit_family);
        }
    }
}

TEST_F(NegativeSyncObject, BufferBarrierWithHostStage) {
    TEST_DESCRIPTION("Buffer barrier includes VK_PIPELINE_STAGE_2_HOST_BIT as srcStageMask or dstStageMask");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    vkt::Buffer buffer(*m_device, 32);
    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.buffer = buffer.handle();
    barrier.size = VK_WHOLE_SIZE;
    // source and destination families should be equal if HOST stage is used
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = 0;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &barrier;

    // HOST stage as source
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcStageMask-03851");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // HOST stage as destination
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier2-srcStageMask-03851");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncObject, ImageBarrierWithHostStage) {
    TEST_DESCRIPTION("Image barrier includes VK_PIPELINE_STAGE_2_HOST_BIT as srcStageMask or dstStageMask");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    sync2_features.synchronization2 = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image.handle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    // source and destination families should be equal if HOST stage is used
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = 0;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    // HOST stage as source
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcStageMask-03854");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // HOST stage as destination
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-srcStageMask-03854");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

// TODO - Figure out if test or VU are bad
TEST_F(NegativeSyncObject, BarrierQueueFamilyWithMemExt) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families when memory extension is enabled ");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    if (only_one_family) {
        printf("Single queue family found -- VK_SHARING_MODE_CONCURRENT testcases skipped.\n");
    } else {
        std::vector<uint32_t> families = {submit_family, other_family};
        BarrierQueueFamilyTestHelper conc_test(&test_context);

        conc_test.Init(&families);
        static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
        static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
        conc_test(img_vuid, buf_vuid, submit_family, submit_family);
        conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
        conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR);
        conc_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED);

        conc_test("VUID-VkImageMemoryBarrier-None-09053", "VUID-VkBufferMemoryBarrier-None-09050", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        conc_test("VUID-VkImageMemoryBarrier-None-09054", "VUID-VkBufferMemoryBarrier-None-09051", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        // This is to flag the errors that would be considered only "unexpected" in the parallel case above
        conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR);
        conc_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED);
    }

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    excl_test("VUID-VkImageMemoryBarrier-image-09118", "VUID-VkBufferMemoryBarrier-buffer-09096", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier-image-09117", "VUID-VkBufferMemoryBarrier-buffer-09095", invalid, submit_family);
    excl_test(submit_family, submit_family);
    excl_test(submit_family, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    excl_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, submit_family);
}

TEST_F(NegativeSyncObject, ImageBarrierWithBadRange) {
    TEST_DESCRIPTION("VkImageMemoryBarrier with an invalid subresourceRange");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkImageMemoryBarrier img_barrier_template = vku::InitStructHelper();
    img_barrier_template.srcAccessMask = 0;
    img_barrier_template.dstAccessMask = 0;
    img_barrier_template.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier_template.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier_template.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_template.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // subresourceRange to be set later for the for the purposes of this test
    img_barrier_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier_template.subresourceRange.baseArrayLayer = 0;
    img_barrier_template.subresourceRange.baseMipLevel = 0;
    img_barrier_template.subresourceRange.layerCount = 0;
    img_barrier_template.subresourceRange.levelCount = 0;

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr);
    img_barrier_template.image = conc_test.image_.handle();
    conc_test.image_barrier_ = img_barrier_template;
    // Nested scope here confuses clang-format, somehow
    // clang-format off

    // try for vk::CmdPipelineBarrier
    {
        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try levelCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            conc_test("VUID-VkImageSubresourceRange-levelCount-01720");
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01724");
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try layerCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            conc_test("VUID-VkImageSubresourceRange-layerCount-01721");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01725");
        }
    }

    m_commandBuffer->begin();
    // try for vk::CmdWaitEvents
    {
        vkt::Event event(*m_device);

        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try levelCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try layerCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }
    }
    // clang-format on
}

TEST_F(NegativeSyncObject, Sync2BarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families with synchronization2");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->phy().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->phy().queue_properties_[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);
    Barrier2QueueFamilyTestHelper::Context test_context2(this, qf_indices);

    Barrier2QueueFamilyTestHelper excl_test(&test_context2);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.
    excl_test("VUID-VkImageMemoryBarrier2-image-09118", "VUID-VkBufferMemoryBarrier2-buffer-09096", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier2-image-09117", "VUID-VkBufferMemoryBarrier2-buffer-09095", invalid, submit_family);
    excl_test(submit_family, submit_family);
    excl_test(submit_family, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    excl_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, submit_family);
}

TEST_F(NegativeSyncObject, BarrierQueues) {
    TEST_DESCRIPTION("Test buffer memory with both src and dst queue VK_QUEUE_FAMILY_EXTERNAL.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferMemoryBarrier-srcQueueFamilyIndex-04087");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncObject, BarrierAccessSync2) {
    TEST_DESCRIPTION("Test barrier VkAccessFlagBits2.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_commandBuffer->begin();

    // srcAccessMask and srcStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03900");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03901");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03902");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03903");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03904");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03905");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03906");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    if (DeviceExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME)) {
        mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-07272");
        vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);
    }

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03907");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-07454");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03909");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03910");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03911");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03912");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03913");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03914");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03915");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03916");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-03917");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    // now test dstAccessMask and dstStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    mem_barrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03900");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03901");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03902");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03903");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03904");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03905");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03906");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    if (DeviceExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME)) {
        mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-07272");
        vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);
    }

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03907");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-07454");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03909");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03910");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03911");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03912");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03913");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03914");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03915");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03916");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-dstAccessMask-03917");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BarrierAccessVideoDecode) {
    TEST_DESCRIPTION("Test barrier with access decode read bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-04858");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-04859");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-04860");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcAccessMask-04861");
    vk::CmdPipelineBarrier2KHR(m_commandBuffer->handle(), &dependency_info);

    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, Sync2LayoutFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceVulkan13Features features_13 = vku::InitStructHelper();
    features_13.synchronization2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features_13));

    VkImageCreateInfo info = vkt::Image::create_info();
    info.format = VK_FORMAT_B8G8R8A8_UNORM;
    info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image{m_device};
    image.init(&info);

    m_commandBuffer->begin();
    VkImageMemoryBarrier2 img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    VkDependencyInfoKHR dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-synchronization2-03848");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-synchronization2-07793");  // oldLayout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-synchronization2-07794");  // newLayout
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dep_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SubmitSignaledFence) {
    vkt::Fence testFence;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-fence-00063");

    VkFenceCreateInfo fenceInfo = vku::InitStructHelper();
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    m_commandBuffer->begin();
    m_commandBuffer->end();

    testFence.init(*m_device, fenceInfo);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_default_queue, 1, &submit_info, testFence.handle());
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, QueueSubmitWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");

    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    if (m_device->graphics_queues().size() < 2) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Semaphore semaphore(*m_device);

    VkQueue other = m_device->graphics_queues()[1]->handle();

    {
        VkSubmitInfo signal_submit = vku::InitStructHelper();
        signal_submit.signalSemaphoreCount = 1;
        signal_submit.pSignalSemaphores = &semaphore.handle();

        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo wait_submit = vku::InitStructHelper();
        wait_submit.waitSemaphoreCount = 1;
        wait_submit.pWaitSemaphores = &semaphore.handle();
        wait_submit.pWaitDstStageMask = &stage_flags;

        vk::QueueSubmit(m_default_queue, 1, &signal_submit, VK_NULL_HANDLE);
        vk::QueueSubmit(m_default_queue, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-00068");
        vk::QueueSubmit(other, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
        vk::QueueWaitIdle(m_default_queue);
        vk::QueueWaitIdle(other);
    }
    if (m_device->phy().queue_properties_[m_device->graphics_queues()[0]->get_family_index()].queueFlags &
        VK_QUEUE_SPARSE_BINDING_BIT) {
        VkBindSparseInfo signal_bind = vku::InitStructHelper();
        signal_bind.signalSemaphoreCount = 1;
        signal_bind.pSignalSemaphores = &semaphore.handle();

        VkBindSparseInfo wait_bind = vku::InitStructHelper();
        wait_bind.waitSemaphoreCount = 1;
        wait_bind.pWaitSemaphores = &semaphore.handle();

        vk::QueueBindSparse(m_default_queue, 1, &signal_bind, VK_NULL_HANDLE);
        vk::QueueBindSparse(m_default_queue, 1, &wait_bind, VK_NULL_HANDLE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueBindSparse-pWaitSemaphores-01116");
        vk::QueueBindSparse(other, 1, &wait_bind, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(m_default_queue);
        vk::QueueWaitIdle(other);
    }

    // sync 2
    if (IsExtensionsEnabled(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        VkSemaphoreSubmitInfo signal_sem_info = vku::InitStructHelper();
        signal_sem_info.semaphore = semaphore.handle();
        signal_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        VkSubmitInfo2 signal_submit = vku::InitStructHelper();
        signal_submit.signalSemaphoreInfoCount = 1;
        signal_submit.pSignalSemaphoreInfos = &signal_sem_info;

        VkSemaphoreSubmitInfo wait_sem_info = vku::InitStructHelper();
        wait_sem_info.semaphore = semaphore.handle();
        wait_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        VkSubmitInfo2 wait_submit = vku::InitStructHelper();
        wait_submit.waitSemaphoreInfoCount = 1;
        wait_submit.pWaitSemaphoreInfos = &wait_sem_info;

        vk::QueueSubmit2KHR(m_default_queue, 1, &signal_submit, VK_NULL_HANDLE);
        vk::QueueSubmit2KHR(m_default_queue, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-semaphore-03871");
        vk::QueueSubmit2KHR(other, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(m_default_queue);
        vk::QueueWaitIdle(other);
    }
}

TEST_F(NegativeSyncObject, QueueSubmit2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using QueueSubmit2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitState())

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-synchronization2-03866");
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-synchronization2-03866");
        vk::QueueSubmit2(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSyncObject, WaitEventsDifferentQueues) {
    TEST_DESCRIPTION("Using CmdWaitEvents with invalid barrier queues");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }

    vkt::Event event(*m_device);
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkBufferMemoryBarrier buffer_memory_barrier = vku::InitStructHelper();
    buffer_memory_barrier.srcAccessMask = 0;
    buffer_memory_barrier.dstAccessMask = 0;
    buffer_memory_barrier.buffer = buffer.handle();
    buffer_memory_barrier.offset = 0;
    buffer_memory_barrier.size = 256;
    buffer_memory_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_memory_barrier.dstQueueFamilyIndex = no_gfx.value();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageMemoryBarrier image_memory_barrier = vku::InitStructHelper();
    image_memory_barrier.srcAccessMask = 0;
    image_memory_barrier.dstAccessMask = 0;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_memory_barrier.image = image.handle();
    image_memory_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_memory_barrier.dstQueueFamilyIndex = no_gfx.value();
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();
    vk::CmdSetEvent(m_commandBuffer->handle(), event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr, 1, &buffer_memory_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncObject, SemaphoreTypeCreateInfoCore) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with a 1.2 core version");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    // Core 1.2 supports timelineSemaphore feature bit but not enabled
    RETURN_IF_SKIP(InitState())

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SemaphoreTypeCreateInfoExtension) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);  // before timelineSemaphore was added to core
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Enabled extension but not the timelineSemaphore feature bit
    RETURN_IF_SKIP(InitState())

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, MixedTimelineAndBinarySemaphores) {
    TEST_DESCRIPTION("Submit mixtures of timeline and binary semaphores");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    // index 1 should be a binary semaphore
    semaphore_create_info.pNext = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));
    VkSemaphore extra_binary;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &extra_binary));

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;
    semaphore_signal_info.value = 10;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    // this array needs a length of 2, even though the binary semaphore won't look at the values array
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 2;
    submit_info.pSignalSemaphores = semaphore;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    uint64_t values[2] = {signalValue, 0 /*ignored*/};
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = values;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // the indexes in pWaitSemaphores and pWaitSemaphoreValues should match
    VkSemaphore reversed[2] = {semaphore[1], semaphore[0]};
    uint64_t reversed_values[2] = {vvl::kU64Max /* ignored */, 20};
    VkPipelineStageFlags wait_stages[2] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 2;
    submit_info.pWaitSemaphores = reversed;
    submit_info.pWaitDstStageMask = wait_stages;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = reversed_values;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // if we only signal a binary semaphore we don't need a 'values' array
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &extra_binary;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
    vk::DestroySemaphore(m_device->device(), extra_binary, nullptr);
}

TEST_F(NegativeSyncObject, QueueSubmitNoTimelineSemaphoreInfo) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore but not a VkTimelineSemaphoreSubmitInfoKHR.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0] = vku::InitStructHelper();
    submit_info[0].commandBufferCount = 0;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_default_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    submit_info[0].pNext = &timeline_semaphore_submit_info;

    submit_info[1] = vku::InitStructHelper();
    submit_info[1].commandBufferCount = 0;
    submit_info[1].pWaitDstStageMask = &stageFlags;
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_default_queue, 2, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(NegativeSyncObject, QueueSubmitTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03240");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        VkTimelineSemaphoreSubmitInfoKHR tl_info_2 = vku::InitStructHelper();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        VkSubmitInfo submit_info2 = vku::InitStructHelper(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
        vk::QueueSubmit(m_default_queue, 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03244");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < vvl::kU64Max) {
            signalValue++;
            timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
            waitValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            submit_info.signalSemaphoreCount = 0;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03243");
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, QueueBindSparseTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    auto index = m_device->graphics_queue_node_index_;
    if ((m_device->phy().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == 0) {
        GTEST_SKIP() << "Sparse binding not supported, skipping test";
    }

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkBindSparseInfo submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    // error for both signal and wait
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info.pNext = &timeline_semaphore_submit_info;

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pNext-03248");
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    submit_info.signalSemaphoreCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    submit_info.waitSemaphoreCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pNext-03247");
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    submit_info.waitSemaphoreCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        VkTimelineSemaphoreSubmitInfoKHR tl_info_2 = vku::InitStructHelper();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        VkBindSparseInfo submit_info2 = vku::InitStructHelper(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
        vk::QueueBindSparse(m_default_queue, 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03251");
        vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < vvl::kU64Max) {
            waitValue = bigValue;

            submit_info.signalSemaphoreCount = 0;
            submit_info.waitSemaphoreCount = 1;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03250");
            vk::QueueBindSparse(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, Sync2QueueSubmitTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features vk12_features = vku::InitStructHelper();
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper(&vk12_features);
    GetPhysicalDeviceFeatures2(sync2_features);
    InitState(nullptr, &sync2_features);

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkSemaphoreSubmitInfo signal_sem_info = vku::InitStructHelper();
    signal_sem_info.value = semaphore_type_create_info.initialValue;
    signal_sem_info.semaphore = semaphore.handle();
    signal_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_sem_info;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signal_sem_info.value++;
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signal_sem_info.value++;
        VkSemaphoreSubmitInfo double_signal_info[2];
        double_signal_info[0] = signal_sem_info;
        double_signal_info[1] = signal_sem_info;

        submit_info.signalSemaphoreInfoCount = 2;
        submit_info.pSignalSemaphoreInfos = double_signal_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
        vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        signal_sem_info.value += timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        submit_info.waitSemaphoreInfoCount = 0;
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &signal_sem_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03884");
        vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signal_sem_info.value < vvl::kU64Max) {
            VkSemaphoreSubmitInfo wait_sem_info = vku::InitStructHelper();
            wait_sem_info.semaphore = semaphore.handle();
            wait_sem_info.value = signal_sem_info.value + 1;
            wait_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            signal_sem_info.value = 1;

            submit_info.signalSemaphoreInfoCount = 0;
            submit_info.waitSemaphoreInfoCount = 1;
            submit_info.pWaitSemaphoreInfos = &wait_sem_info;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03883");
            vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, QueueSubmitBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a queue with a waiting binary semaphore not previously signaled.");

    AddOptionalExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_features = vku::InitStructHelper();
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper(&timeline_features);
    GetPhysicalDeviceFeatures2(sync2_features);
    if (!sync2_features.synchronization2) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper();
    // VUIDs reported change if the extension is enabled, even if the timelineSemaphore feature isn't supported.

    {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo submit_info[3] = {};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].pWaitDstStageMask = &stage_flags;
        submit_info[0].waitSemaphoreCount = 1;
        submit_info[0].pWaitSemaphores = &semaphore[0].handle();
        submit_info[0].signalSemaphoreCount = 1;
        submit_info[0].pSignalSemaphores = &semaphore[1].handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-03238");
        vk::QueueSubmit(m_default_queue, 1, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].pWaitDstStageMask = &stage_flags;
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore[1].handle();
        submit_info[1].signalSemaphoreCount = 1;
        submit_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-03238");
        vk::QueueSubmit(m_default_queue, 2, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = vku::InitStructHelper();
        submit_info[2].signalSemaphoreCount = 1;
        submit_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 2, submit_info, VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    }
    if (m_device->phy().queue_properties_[m_device->graphics_queues()[0]->get_family_index()].queueFlags &
        VK_QUEUE_SPARSE_BINDING_BIT) {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkBindSparseInfo bind_info[3] = {};

        bind_info[0] = vku::InitStructHelper();
        bind_info[0].waitSemaphoreCount = 1;
        bind_info[0].pWaitSemaphores = &semaphore[0].handle();
        bind_info[0].signalSemaphoreCount = 1;
        bind_info[0].pSignalSemaphores = &semaphore[1].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueBindSparse-pWaitSemaphores-03245");
        vk::QueueBindSparse(m_default_queue, 1, &bind_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[1] = vku::InitStructHelper();
        bind_info[1].waitSemaphoreCount = 1;
        bind_info[1].pWaitSemaphores = &semaphore[1].handle();
        bind_info[1].signalSemaphoreCount = 1;
        bind_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueBindSparse-pWaitSemaphores-03245");
        vk::QueueBindSparse(m_default_queue, 2, bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[2] = vku::InitStructHelper();
        bind_info[2].signalSemaphoreCount = 1;
        bind_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_EQ(VK_SUCCESS, vk::QueueBindSparse(m_default_queue, 1, &bind_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueBindSparse(m_default_queue, 2, bind_info, VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    }

    if (IsExtensionsEnabled(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkSemaphoreSubmitInfo sem_info[3];
        for (int i = 0; i < 3; i++) {
            sem_info[i] = vku::InitStructHelper();
            sem_info[i].semaphore = semaphore[i].handle();
            sem_info[i].stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        VkSubmitInfo2 submit_info[3] = {};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].waitSemaphoreInfoCount = 1;
        submit_info[0].pWaitSemaphoreInfos = &sem_info[0];
        submit_info[0].signalSemaphoreInfoCount = 1;
        submit_info[0].pSignalSemaphoreInfos = &sem_info[1];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-semaphore-03873");
        vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].waitSemaphoreInfoCount = 1;
        submit_info[1].pWaitSemaphoreInfos = &sem_info[1];
        submit_info[1].signalSemaphoreInfoCount = 1;
        submit_info[1].pSignalSemaphoreInfos = &sem_info[2];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-semaphore-03873");
        vk::QueueSubmit2KHR(m_default_queue, 2, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = vku::InitStructHelper();
        submit_info[2].signalSemaphoreInfoCount = 1;
        submit_info[2].pSignalSemaphoreInfos = &sem_info[0];

        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue, 2, submit_info, VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    }
}

TEST_F(NegativeSyncObject, QueueSubmitTimelineSemaphoreOutOfOrder) {
    TEST_DESCRIPTION("Submit out-of-order timeline semaphores.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    // We need two queues for this
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());

    uint32_t family_index[2] = {0};
    uint32_t queue_index[2] = {0};

    if (queue_count > 1) {
        family_index[1]++;
    } else {
        // If there's only one family index, check if it supports more than 1 queue
        if (queue_props[0].queueCount > 1) {
            queue_index[1]++;
        } else {
            GTEST_SKIP() << "Multiple queues are required to run this test";
        }
    }

    float priorities[] = {1.0f, 1.0f};
    VkDeviceQueueCreateInfo queue_info[2] = {};
    queue_info[0] = vku::InitStructHelper();
    queue_info[0].queueFamilyIndex = family_index[0];
    queue_info[0].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[0].pQueuePriorities = &(priorities[0]);

    queue_info[1] = vku::InitStructHelper();
    queue_info[1].queueFamilyIndex = family_index[1];
    queue_info[1].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[1].pQueuePriorities = &(priorities[0]);

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = queue_count > 1 ? 2 : 1;
    dev_info.pQueueCreateInfos = &(queue_info[0]);
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    timeline_semaphore_features.timelineSemaphore = true;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&timeline_semaphore_features);
    dev_info.pNext = &features2;

    VkDevice dev;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(gpu(), &dev_info, nullptr, &dev));

    VkQueue queue[2];
    vk::GetDeviceQueue(dev, family_index[0], queue_index[0], &(queue[0]));
    vk::GetDeviceQueue(dev, family_index[1], queue_index[1], &(queue[1]));

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(dev, &semaphore_create_info, nullptr, &semaphore));

    uint64_t semaphoreValues[] = {10, 100, 0, 10};
    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &(semaphoreValues[0]);
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &(semaphoreValues[1]);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;

    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(queue[0], 1, &submit_info, VK_NULL_HANDLE));

    timeline_semaphore_submit_info.pWaitSemaphoreValues = &(semaphoreValues[2]);
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &(semaphoreValues[3]);

    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(queue[1], 1, &submit_info, VK_NULL_HANDLE));

    vk::DeviceWaitIdle(dev);
    vk::DestroySemaphore(dev, semaphore, nullptr);
    vk::DestroyDevice(dev, nullptr);
}

TEST_F(NegativeSyncObject, WaitSemaphoresType) {
    TEST_DESCRIPTION("Wait for a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[0])));

    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[1])));

    VkSemaphoreWaitInfo semaphore_wait_info = vku::InitStructHelper();
    semaphore_wait_info.semaphoreCount = 2;
    semaphore_wait_info.pSemaphores = &semaphore[0];
    const uint64_t wait_values[] = {10, 40};
    semaphore_wait_info.pValues = &wait_values[0];

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreWaitInfo-pSemaphores-03256");
    vk::WaitSemaphoresKHR(m_device->device(), &semaphore_wait_info, 10000);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
}

TEST_F(NegativeSyncObject, SignalSemaphoreType) {
    TEST_DESCRIPTION("Signal a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    vkt::Semaphore semaphore(*m_device);

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore.handle();
    semaphore_signal_info.value = 10;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-semaphore-03257");
    vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03258");
    vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 10;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t waitValue = 10;
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &(semaphore[1]);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &(semaphore[0]);
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03259");
    vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1];
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        VkSemaphore sem;

        semaphore_type_create_info.initialValue = 0;
        ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &sem));

        semaphore_signal_info.semaphore = sem;
        semaphore_signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03260");
        vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
        m_errorMonitor->VerifyFound();

        semaphore_signal_info.value--;
        ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));

        vk::DestroySemaphore(m_device->device(), sem, nullptr);

        // Regression test for value difference validations ran against binary semaphores
        {
            VkSemaphore timeline_sem;
            VkSemaphore binary_sem;

            semaphore_type_create_info.initialValue = 0;
            ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &timeline_sem));

            VkSemaphoreCreateInfo binary_semaphore_create_info = vku::InitStructHelper();

            ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(m_device->device(), &binary_semaphore_create_info, nullptr, &binary_sem));

            signalValue = 1;
            uint64_t offendingValue = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &timeline_sem;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &binary_sem;

            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &signalValue;

            // These two assignments are not required by the spec, but would segfault on older versions of validation layers
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pSignalSemaphoreValues = &offendingValue;

            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

            semaphore_signal_info.semaphore = timeline_sem;
            semaphore_signal_info.value = 1;
            vk::SignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);

            ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));

            vk::DestroySemaphore(m_device->device(), binary_sem, nullptr);
            vk::DestroySemaphore(m_device->device(), timeline_sem, nullptr);
        }
    }

    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
}

TEST_F(NegativeSyncObject, Sync2SignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan12Features vk12_features = vku::InitStructHelper();
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper(&vk12_features);
    GetPhysicalDeviceFeatures2(sync2_features);
    InitState(nullptr, &sync2_features);

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore[2];
    semaphore[0].init(*m_device, semaphore_create_info);
    semaphore[1].init(*m_device, semaphore_create_info);

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore[0].handle();
    semaphore_signal_info.value = 10;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    VkSemaphoreSubmitInfoKHR signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore[0].handle();

    VkSemaphoreSubmitInfoKHR wait_info = vku::InitStructHelper();
    wait_info.semaphore = semaphore[0].handle();
    wait_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_info;

    // signal value > wait value
    signal_info.value = 11;
    wait_info.value = 11;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03881");
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // signal value == current value
    signal_info.value = 10;
    wait_info.value = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signal_info.value = 20;
    wait_info.semaphore = semaphore[1].handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03259");
    vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1].handle();
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        // Regression test for value difference validations ran against binary semaphores
        semaphore_type_create_info.initialValue = 0;
        vkt::Semaphore timeline_sem(*m_device, semaphore_create_info);

        vkt::Semaphore binary_sem(*m_device);

        wait_info.semaphore = timeline_sem.handle();
        wait_info.value = 1;

        signal_info.semaphore = binary_sem.handle();
        signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

        semaphore_signal_info.semaphore = timeline_sem.handle();
        semaphore_signal_info.value = 1;
        vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);

        ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
    }

    ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(m_default_queue));
}

TEST_F(NegativeSyncObject, SemaphoreCounterType) {
    TEST_DESCRIPTION("Get payload from a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelinefeatures = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(timelinefeatures);
    RETURN_IF_SKIP(InitState())

    vkt::Semaphore semaphore(*m_device);

    uint64_t value = 0xdeadbeef;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetSemaphoreCounterValue-semaphore-03255");
    vk::GetSemaphoreCounterValueKHR(m_device->device(), semaphore.handle(), &value);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, EventStageMaskOneCommandBufferPass) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer commandBuffer1(m_device, m_commandPool);
    vkt::CommandBuffer commandBuffer2(m_device, m_commandPool);

    vkt::Event event(*m_device);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, EventStageMaskOneCommandBufferFail) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer commandBuffer1(m_device, m_commandPool);
    vkt::CommandBuffer commandBuffer2(m_device, m_commandPool);

    vkt::Event event(*m_device);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-parameter");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, EventStageMaskTwoCommandBufferPass) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer commandBuffer1(m_device, m_commandPool);
    vkt::CommandBuffer commandBuffer2(m_device, m_commandPool);

    vkt::Event event(*m_device);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    commandBuffer2.begin();
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.end();

    submit_info.pCommandBuffers = &commandBuffer2.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, EventStageMaskTwoCommandBufferFail) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    vkt::CommandBuffer commandBuffer1(m_device, m_commandPool);
    vkt::CommandBuffer commandBuffer2(m_device, m_commandPool);

    vkt::Event event(*m_device);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    commandBuffer2.begin();
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.end();

    submit_info.pCommandBuffers = &commandBuffer2.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-parameter");
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(NegativeSyncObject, QueueForwardProgressFenceWait) {
    TEST_DESCRIPTION("Call VkQueueSubmit with a semaphore that is already signaled but not waited on by the queue.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const char *queue_forward_progress_message = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";

    vkt::CommandBuffer cb1(m_device, m_commandPool);
    cb1.begin();
    cb1.end();

    vkt::Semaphore semaphore(*m_device);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb1.handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_commandBuffer->begin();
    m_commandBuffer->end();
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, queue_forward_progress_message);
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DeviceWaitIdle(m_device->device());
}

TEST_F(NegativeSyncObject, PipelineStageConditionalRenderingWithWrongQueue) {
    TEST_DESCRIPTION("Run CmdPipelineBarrier with VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT and wrong VkQueueFlagBits");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceConditionalRenderingFeaturesEXT cond_rendering_feature = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(cond_rendering_feature);
    if (cond_rendering_feature.conditionalRendering == VK_FALSE) {
        GTEST_SKIP() << "conditionalRendering feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &cond_rendering_feature));

    uint32_t only_transfer_queueFamilyIndex = vvl::kU32Max;

    const auto q_props = m_device->phy().queue_properties_;
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    for (uint32_t i = 0; i < (uint32_t)q_props.size(); i++) {
        if (q_props[i].queueFlags == VK_QUEUE_TRANSFER_BIT) {
            only_transfer_queueFamilyIndex = i;
            break;
        }
    }

    if (only_transfer_queueFamilyIndex == vvl::kU32Max) {
        GTEST_SKIP() << "Only VK_QUEUE_TRANSFER_BIT Queue is not supported";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    vkt::CommandPool commandPool(*m_device, only_transfer_queueFamilyIndex);
    vkt::CommandBuffer commandBuffer(m_device, &commandPool);

    commandBuffer.begin();

    VkImageMemoryBarrier imb = vku::InitStructHelper();
    imb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    imb.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-06461");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-06462");
    vk::CmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                           VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, 0, 0, nullptr, 0, nullptr, 1, &imb);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(NegativeSyncObject, WaitOnNoEvent) {
    RETURN_IF_SKIP(Init())
    VkEvent bad_event = CastToHandle<VkEvent, uintptr_t>(0xbaadbeef);
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-pEvents-parameter");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &bad_event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeSyncObject, InvalidDeviceOnlyEvent) {
    TEST_DESCRIPTION("Attempt to use device only event with host commands.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        sync2_features.pNext = &portability_subset_features;
    }
    GetPhysicalDeviceFeatures2(sync2_features);
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        if (portability_subset_features.events) {
            GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkEventCreateInfo event_ci = vku::InitStructHelper();
    event_ci.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
    vkt::Event ev(*m_device, event_ci);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetEvent-event-03823");
    vk::ResetEvent(*m_device, ev.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkSetEvent-event-03941");
    vk::SetEvent(*m_device, ev.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SetEvent2DependencyFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    m_commandBuffer->begin();

    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent2-dependencyFlags-03825");
    vk::CmdSetEvent2(m_commandBuffer->handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SetEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    m_commandBuffer->begin();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfoKHR dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent2-srcStageMask-09391");  // src
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent2-dstStageMask-09392");  // dst
    vk::CmdSetEvent2(m_commandBuffer->handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, WaitEventRenderPassHostBit) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
    }

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-07308");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
