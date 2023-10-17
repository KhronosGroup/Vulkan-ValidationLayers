/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../framework/layer_validation_tests.h"

TEST_F(PositiveSubpass, SubpassImageBarrier) {
    TEST_DESCRIPTION("Subpass with image barrier (self-dependency)");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    const VkAttachmentDescription attachment = {0,
                                                VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_GENERAL};
    const VkSubpassDependency dependency = {0,
                                            0,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                            VK_DEPENDENCY_BY_REGION_BIT};
    const VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 1, &ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo render_pass_begin = vku::InitStructHelper();
    render_pass_begin.renderPass = render_pass;
    render_pass_begin.framebuffer = framebuffer;
    render_pass_begin.renderArea = VkRect2D{{0, 0}, {32, 32}};

    // VkImageMemoryBarrier
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    // VkDependencyInfo with VkImageMemoryBarrier2
    const safe_VkImageMemoryBarrier2 safe_barrier2 = ConvertVkImageMemoryBarrierToV2(
        barrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = safe_barrier2.ptr();

    // Test vkCmdPipelineBarrier subpass barrier
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(*m_commandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &barrier);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();

    // Test vkCmdPipelineBarrier2 subpass barrier
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(*m_commandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdPipelineBarrier2(*m_commandBuffer, &dependency_info);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
}

TEST_F(PositiveSubpass, SubpassWithEventWait) {
    TEST_DESCRIPTION("Subpass waits for the event set outside of this subpass");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(sync2_features);
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    const VkAttachmentDescription attachment = {0,
                                                VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_SAMPLE_COUNT_1_BIT,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_GENERAL};
    const VkSubpassDependency dependency = {VK_SUBPASS_EXTERNAL,
                                            0,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                            0};
    const VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 1, &ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;
    vkt::RenderPass render_pass(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo render_pass_begin = vku::InitStructHelper();
    render_pass_begin.renderPass = render_pass;
    render_pass_begin.framebuffer = framebuffer;
    render_pass_begin.renderArea = VkRect2D{{0, 0}, {32, 32}};

    // VkImageMemoryBarrier
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    // VkDependencyInfo with VkImageMemoryBarrier2
    const safe_VkImageMemoryBarrier2 safe_barrier2 = ConvertVkImageMemoryBarrierToV2(
        barrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = 0;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = safe_barrier2.ptr();

    // vkCmdWaitEvents inside render pass
    {
        vkt::Event event(*m_device);
        m_commandBuffer->begin();
        vk::CmdSetEvent(*m_commandBuffer, event, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
        vk::CmdBeginRenderPass(*m_commandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdWaitEvents(*m_commandBuffer, 1, &event.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
        vk::CmdEndRenderPass(*m_commandBuffer);
        m_commandBuffer->end();
    }

    // vkCmdWaitEvents2 inside render pass.
    // It's also a regression test for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4258
    {
        vkt::Event event2(*m_device);
        m_commandBuffer->begin();
        vk::CmdSetEvent2(*m_commandBuffer, event2, &dependency_info);
        vk::CmdBeginRenderPass(*m_commandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdWaitEvents2(*m_commandBuffer, 1, &event2.handle(), &dependency_info);
        vk::CmdEndRenderPass(*m_commandBuffer);
        m_commandBuffer->end();
    }
}
