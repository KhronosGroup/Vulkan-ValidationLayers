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

struct NegativeSyncValRenderPass : public VkSyncValTest {};

TEST_F(NegativeSyncValRenderPass, ClearColorAttachmentWAW) {
    TEST_DESCRIPTION("WAW hazard when color attachment is cleared inside render pass");
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;

    vkt::Image src_image(*m_device, width, height, color_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(color_format, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_LOAD_OP_NONE,
                                         VK_ATTACHMENT_STORE_OP_NONE);
    render_pass.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    render_pass.AddColorAttachment(0);
    render_pass.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), width, height);

    // Copy to the left half
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.extent = {width / 2, height, 1};

    // Clear the right half
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT, 0};
    VkClearRect clear_rect = {};
    clear_rect.rect = {{width / 2, 0}, {width / 2, height}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // loadOp/storeOp=NONE, so no hazard can be detected at this point
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);

    // Clear operation hazards with the previous copy.
    // The copy region and the clear region do not overlap but the clear operation can
    // access the entire attachment, so it can race with copy accesses and makes result
    // of the copy operation undefined. Note, that storeOp=NONE works as DONT_CARE here
    // (due to write) and the clear results are discarded.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValRenderPass, ClearDepthAspectWAW) {
    TEST_DESCRIPTION("WAW hazard when depth aspect of attachment is cleared inside render pass");
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image src_image(*m_device, width, height, depth_stencil_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(depth_stencil_format, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                         VK_ATTACHMENT_LOAD_OP_NONE, VK_ATTACHMENT_STORE_OP_NONE);
    render_pass.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    render_pass.AddDepthStencilAttachment(0);
    render_pass.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), width, height);

    // Copy to the left half
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.extent = {width / 2, height, 1};

    // Clear the right half
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT, 0};
    VkClearRect clear_rect = {};
    clear_rect.rect = {{width / 2, 0}, {width / 2, height}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // loadOp/storeOp=NONE, so no hazard can be detected at this point
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);

    // Clear operation hazards with the previous copy.
    // The copy region and the clear region do not overlap but the clear operation can
    // access the entire attachment, so it can race with copy accesses and makes result
    // of the copy operation undefined. Note, that storeOp=NONE works as DONT_CARE here
    // (due to write) and the clear results are discarded.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValRenderPass, ClearStencilAspectWAW) {
    TEST_DESCRIPTION("WAW hazard when stencil aspect of attachment is cleared inside render pass");
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image src_image(*m_device, width, height, depth_stencil_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkAttachmentDescription attachment{};
    attachment.format = depth_stencil_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(attachment);
    render_pass.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    render_pass.AddDepthStencilAttachment(0);
    render_pass.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), width, height);

    // Copy to the left half
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.extent = {width / 2, height, 1};

    // Clear the right half
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_STENCIL_BIT};
    VkClearRect clear_rect = {};
    clear_rect.rect = {{width / 2, 0}, {width / 2, height}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // loadOp/storeOp/stencilLoadOp/stencilStoreOp=NONE, so no hazard can be detected at this point
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);

    // Clear operation hazards with the previous copy.
    // The copy region and the clear region do not overlap but the clear operation can
    // access the entire attachment, so it can race with copy accesses and makes result
    // of the copy operation undefined. Note, that storeOp=NONE works as DONT_CARE here
    // (due to write) and the clear results are discarded.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValRenderPass, ClearDepthCopyStencilWAW) {
    TEST_DESCRIPTION("Clearing depth and copying to stencil causes hazard because depth and stencil can interleave");
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image src_image(*m_device, width, height, depth_stencil_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkAttachmentDescription attachment{};
    attachment.format = depth_stencil_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(attachment);
    render_pass.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    render_pass.AddDepthStencilAttachment(0);
    render_pass.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), width, height);

    // Copy to stencil
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.extent = {width, height, 1};

    // Clear depth
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT};
    VkClearRect clear_rect = {};
    clear_rect.rect = {{0, 0}, {width, height}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // loadOp/storeOp/stencilLoadOp/stencilStoreOp=NONE, so no hazard can be detected at this point
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncValRenderPass, InputAttachmentReadAndResolveWrite) {
    TEST_DESCRIPTION("Insufficient synchronization between input attachment read and multisample resolve write");
    RETURN_IF_SKIP(InitSyncVal());

    VkImageCreateInfo multi_sample_image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    multi_sample_image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image multi_sample_image(*m_device, multi_sample_image_ci);
    vkt::ImageView multi_sample_image_view = multi_sample_image.CreateView();

    VkImageCreateInfo single_sample_image_ci = vkt::Image::ImageCreateInfo2D(
        32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::Image single_sample_image(*m_device, single_sample_image_ci);
    vkt::ImageView single_sample_image_view = single_sample_image.CreateView();

    VkAttachmentDescription multi_sample_attachment = {};
    multi_sample_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    multi_sample_attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    multi_sample_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    multi_sample_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    multi_sample_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    multi_sample_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription single_sample_attachment = {};
    single_sample_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    single_sample_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    single_sample_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    single_sample_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    single_sample_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    single_sample_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference multi_sample_attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference single_sample_attachment_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass0{};
    subpass0.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0.colorAttachmentCount = 1;
    subpass0.pColorAttachments = &multi_sample_attachment_ref;
    subpass0.pResolveAttachments = &single_sample_attachment_ref;

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.inputAttachmentCount = 1;
    subpass1.pInputAttachments = &single_sample_attachment_ref;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 1;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    // this does not make resolve writes visible
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    // try to read resolved attachment as input attachment in the second subpass
    subpass_dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

    const VkSubpassDescription subpasses[2] = {subpass0, subpass1};
    const VkAttachmentDescription attachments[2] = {multi_sample_attachment, single_sample_attachment};
    const VkImageView image_views[2] = {multi_sample_image_view, single_sample_image_view};

    VkRenderPassCreateInfo renderpass_ci = vku::InitStructHelper();
    renderpass_ci.attachmentCount = 2;
    renderpass_ci.pAttachments = attachments;
    renderpass_ci.subpassCount = 2;
    renderpass_ci.pSubpasses = subpasses;
    renderpass_ci.dependencyCount = 1;
    renderpass_ci.pDependencies = &subpass_dependency;

    const vkt::RenderPass render_pass(*m_device, renderpass_ci);
    const vkt::Framebuffer framebuffer(*m_device, render_pass, 2, image_views, 32, 32);

    VkShaderObj vs(*m_device, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs_input_attachment_read(*m_device, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs_input_attachment_read.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    pipe.gp_ci_.renderPass = render_pass;
    pipe.gp_ci_.subpass = 1;
    pipe.ms_ci_ = ms_state_ci;
    pipe.CreateGraphicsPipeline();
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, single_sample_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    // Resolve at the end of this subpass
    m_command_buffer.NextSubpass();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    // Read resolved data as input attachment
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}
