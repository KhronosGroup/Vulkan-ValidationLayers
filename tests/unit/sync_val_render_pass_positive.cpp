/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "../framework/sync_val_tests.h"
#include "../framework/render_pass_helper.h"

struct PositiveSyncValRenderPass : public VkSyncValTest {};

VkAttachmentDescription VkSyncValTest::AttachmentWithoutLoadStore(VkFormat format) {
    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    return attachment;
}

TEST_F(PositiveSyncValRenderPass, AttachmentClearRegionTouchesCopyRegion) {
    // This test is similar to NegativeSyncValRenderPass.AttachmentClearAndCopyRegionOverlap
    // but the regions do not overlap but only touch
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;

    vkt::Image src_image(*m_device, width, height, color_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(AttachmentWithoutLoadStore(color_format));
    render_pass.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    render_pass.AddColorAttachment(0);
    render_pass.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), width, height);

    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT, 0};

    VkClearRect clear_rect = {};
    clear_rect.rect = {{0, 0}, {32, 32}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {32, 32, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {32, 32, 0};
    copy_region.extent = {64, 64, 1};

    // Clear and copy regions touch at (x=31, y=31) but do not overlap
    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
