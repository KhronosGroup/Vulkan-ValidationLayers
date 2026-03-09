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

TEST_F(PositiveSyncValRenderPass, SyncStoreOpWriteWithPreviousRead) {
    TEST_DESCRIPTION("Synchronize StoreOp writes with previous copy reads");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Image image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::ImageView image_view = image.CreateView();

    vkt::Buffer buffer(*m_device, 64 * 64 * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                         VK_ATTACHMENT_LOAD_OP_NONE, VK_ATTACHMENT_STORE_OP_STORE);
    render_pass.AddColorAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    render_pass.CreateRenderPass();
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 64, 64);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);

    // Prevent WAR
    m_command_buffer.Barrier(barrier);

    // There is no loadOp access (LOAD_OP_NONE)
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 64, 64);

    m_command_buffer.EndRenderPass();
}

TEST_F(PositiveSyncValRenderPass, SyncInputAttachmentReadWithResolveWrite) {
    TEST_DESCRIPTION("Synchronize input attachment reads with previous multisample resolve writes");
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
    // that's writing to resolve attachment in the first subpass
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // that's reading resolved attachment as input attachment in the second subpass
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
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, SyncAttachmentWriteWithResolveRead) {
    TEST_DESCRIPTION("Synchronize attachment writes with resolve reads in the previous subpass");
    RETURN_IF_SKIP(InitSyncVal());

    VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView();

    VkImageCreateInfo resolve_image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image resolve_image(*m_device, resolve_image_ci);
    vkt::ImageView resolve_image_view = resolve_image.CreateView();

    VkAttachmentDescription attachment = {};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription resolve_attachment = {};
    resolve_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference resolve_attachment_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass0{};
    subpass0.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0.colorAttachmentCount = 1;
    subpass0.pColorAttachments = &attachment_ref;
    subpass0.pResolveAttachments = &resolve_attachment_ref;

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount = 1;
    subpass1.pColorAttachments = &attachment_ref;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 1;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubpassDescription subpasses[2] = {subpass0, subpass1};
    const VkAttachmentDescription attachments[2] = {attachment, resolve_attachment};
    const VkImageView image_views[2] = {image_view, resolve_image_view};

    VkRenderPassCreateInfo renderpass_ci = vku::InitStructHelper();
    renderpass_ci.attachmentCount = 2;
    renderpass_ci.pAttachments = attachments;
    renderpass_ci.subpassCount = 2;
    renderpass_ci.pSubpasses = subpasses;
    renderpass_ci.dependencyCount = 1;
    renderpass_ci.pDependencies = &subpass_dependency;

    const vkt::RenderPass render_pass(*m_device, renderpass_ci);
    const vkt::Framebuffer framebuffer(*m_device, render_pass, 2, image_views, 32, 32);

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = 0;

    VkClearRect clear_rect{};
    clear_rect.rect = {{0, 0}, {32, 32}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    // Read multisample attachment during resolve
    m_command_buffer.NextSubpass();
    // Write to multisample attachment
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, SyncMultisampleReadWithPreviousWrite) {
    TEST_DESCRIPTION("Synchronize multisample attachment resolve reads with previous writes");
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

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount = 1;
    subpass1.pColorAttachments = &multi_sample_attachment_ref;
    subpass1.pResolveAttachments = &single_sample_attachment_ref;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 1;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // clear multisample attachment in the first subpass
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // read multisample attachment in the second subpass during resolve
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

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

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = 0;

    VkClearRect clear_rect{};
    clear_rect.rect = {{0, 0}, {32, 32}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);

    // Write to multisample attachment
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);

    m_command_buffer.NextSubpass();

    // The multisample read happens at the end of this subpass (part of resolve)
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, StencilNotWritable) {
    TEST_DESCRIPTION("Test that stencil access is not a write when KEEP is used for all stencil ops");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    auto depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image image(*m_device, 32, 32, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);

    VkAttachmentDescription depth_stencil_attachment{};
    depth_stencil_attachment.format = depth_stencil_format;
    depth_stencil_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    depth_stencil_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    depth_stencil_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
    depth_stencil_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_stencil_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(depth_stencil_attachment);
    render_pass.AddDepthStencilAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    render_pass.CreateRenderPass();
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 32, 32);

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.writeMask = 255;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_ci = vku::InitStructHelper();
    depth_stencil_ci.depthTestEnable = VK_TRUE;
    depth_stencil_ci.stencilTestEnable = VK_TRUE;
    depth_stencil_ci.front = stencil;
    depth_stencil_ci.back = stencil;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.gp_ci_.pDepthStencilState = &depth_stencil_ci;
    pipe.CreateGraphicsPipeline();

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, FinalLayoutTransitionProtectsPreviousCopyRead) {
    TEST_DESCRIPTION("Implicit subpass dependency creates execution dependency with READ outside of render pass instance");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_ATTACHMENT_LOAD_OP_NONE, VK_ATTACHMENT_STORE_OP_NONE);
    rp.AddColorAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.CreateRenderPass();
    vkt::Framebuffer framebuffer(*m_device, rp, 1, &image_view.handle());

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {32, 32, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    m_command_buffer.BeginRenderPass(rp, framebuffer, 32, 32);
    // loadOp=NONE and storeOp=NONE, so image copy is followed directly by layout transition.
    // There is an implicit dependency with VK_SUBPASS_EXTERNAL that specifies srcStageMask as
    // ALL_COMMANDS, and this creates an execution dependency with image's COPY_READ, which is
    // enough to prevent WRITE-AFTER-READ hazard.
    m_command_buffer.EndRenderPass();
}

TEST_F(PositiveSyncValRenderPass, Multiview) {
    TEST_DESCRIPTION("Subpass broadcasts rendering into two views");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2 /*layers*/, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2 /*layers*/);

    const uint32_t view_mask = 3;  // layer 0 + layer 1
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 1;
    multiview_ci.pViewMasks = &view_mask;

    RenderPassSingleSubpass render_pass(*this);
    render_pass.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
    render_pass.AddColorAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    render_pass.CreateRenderPass(&multiview_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();

    // Subpass renders to 2 views
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewAsyncSubpasses) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10178
    TEST_DESCRIPTION("Two async subpasses with each subpass uses a single view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2 /*layers*/, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2 /*layers*/);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;
    // The second subpass is the same as the first one but uses a different view mask
    subpasses[1] = subpasses[0];

    // Assign different view masks to subpasses.
    // Each subpass renders to its own layer, so subpasses can work in parallel (no need to specify subpass dependency)
    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    // Test only loadOp and storeOp accesses
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewAsyncSubpasses2) {
    TEST_DESCRIPTION("Two async subpasses with each subpass uses a single view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2 /*layers*/, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2 /*layers*/);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;
    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();

    // Test loadOp/storeOp and draw accesses
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewAsyncSubpasses3) {
    TEST_DESCRIPTION("Two async subpasses with each subpass uses a single view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2 /*layers*/, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2 /*layers*/);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;
    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    CreatePipelineHelper pipe_subpass0(*this);
    pipe_subpass0.gp_ci_.renderPass = render_pass;
    pipe_subpass0.gp_ci_.subpass = 0;
    pipe_subpass0.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_subpass1(*this);
    pipe_subpass1.gp_ci_.renderPass = render_pass;
    pipe_subpass1.gp_ci_.subpass = 1;
    pipe_subpass1.CreateGraphicsPipeline();

    // Test each subpass issues a draw call
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_subpass0);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.NextSubpass();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_subpass1);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
TEST_F(PositiveSyncValRenderPass, MultiviewMultipleStoreOps) {
    TEST_DESCRIPTION("Multiview broadcasts storeOp to different subpasses that write to different views");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    // Each subpass executes storeOp for a separate view
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewMultipleLoadOps) {
    TEST_DESCRIPTION("Multiview broadcasts loadOp to different subpasses that synchronize with external copy");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(
        128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    vkt::Buffer buffer(*m_device, 128 * 128 * 4 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    VkSubpassDependency subpass_dependencies[2] = {};
    subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependencies[0].dstSubpass = 0;
    subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependencies[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    subpass_dependencies[1] = subpass_dependencies[0];
    subpass_dependencies[1].dstSubpass = 1;

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    render_pass_ci.dependencyCount = 2;
    render_pass_ci.pDependencies = subpass_dependencies;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    region.imageExtent = {128, 128, 1};

    m_command_buffer.Begin();

    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    // loadOp for view 0 happens at the beginning of this subpass (0).
    // Subpass dependency 0 synchronizes it with copy

    m_command_buffer.NextSubpass();
    // loadOp for view 1 happens at the beginning of this subpass (1).
    // Subpass dependency 1 synchronizes it with copy

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewAsyncSubpasses3DImage) {
    TEST_DESCRIPTION("Use slices of 3D image for multiview views");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {128, 128, 2};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image_3d(*m_device, image_ci);

    vkt::ImageView image_view = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2 /*slices*/);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewSubpassDependency) {
    TEST_DESCRIPTION("Use subpass dependency to sync multiview subpasses that have a shared view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 3 /*layers*/, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 3 /*layers*/);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    // Subpasses share layer 1
    const uint32_t view_masks[2] = {0x3 /* layer0 + layer 1*/, 0x6 /* layer 1 + layer 2*/};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkSubpassDependency subpass_dep = {};
    subpass_dep.srcSubpass = 0;
    subpass_dep.dstSubpass = 1;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &subpass_dep;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
}

TEST_F(PositiveSyncValRenderPass, MultiviewClearAttachments) {
    TEST_DESCRIPTION("Test that CmdClearAttachment clear itw own view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkClearRect clear_rect{};
    clear_rect.rect.extent = {128, 128};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.NextSubpass();
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewResolveRead) {
    TEST_DESCRIPTION("Test that resolve reads itw own view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkImageCreateInfo resolve_image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image resolve_image(*m_device, resolve_image_ci);
    vkt::ImageView resolve_image_view = resolve_image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkAttachmentDescription attachment = {};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription resolve_attachment = {};
    resolve_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference resolve_attachment_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass0{};
    subpass0.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0.colorAttachmentCount = 1;
    subpass0.pColorAttachments = &attachment_ref;
    subpass0.pResolveAttachments = &resolve_attachment_ref;

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount = 1;
    subpass1.pColorAttachments = &attachment_ref;

    const VkSubpassDescription subpasses[2] = {subpass0, subpass1};
    const VkAttachmentDescription attachments[2] = {attachment, resolve_attachment};
    const VkImageView image_views[2] = {image_view, resolve_image_view};

    const uint32_t view_masks[2] = {1, 2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo renderpass_ci = vku::InitStructHelper(&multiview_ci);
    renderpass_ci.attachmentCount = 2;
    renderpass_ci.pAttachments = attachments;
    renderpass_ci.subpassCount = 2;
    renderpass_ci.pSubpasses = subpasses;

    const vkt::RenderPass render_pass(*m_device, renderpass_ci);
    const vkt::Framebuffer framebuffer(*m_device, render_pass, 2, image_views, 32, 32);

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = 0;

    VkClearRect clear_rect{};
    clear_rect.rect = {{0, 0}, {32, 32}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    // Resolve reads view 0
    m_command_buffer.NextSubpass();
    // Clear writes to view 1
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewResolveRead2) {
    TEST_DESCRIPTION("Test that resolve reads itw own view");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkImageCreateInfo resolve_image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image resolve_image(*m_device, resolve_image_ci);
    vkt::ImageView resolve_image_view = resolve_image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    VkAttachmentDescription attachment = {};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription resolve_attachment = {};
    resolve_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference resolve_attachment_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass0{};
    subpass0.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0.colorAttachmentCount = 1;
    subpass0.pColorAttachments = &attachment_ref;

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount = 1;
    subpass1.pColorAttachments = &attachment_ref;
    subpass1.pResolveAttachments = &resolve_attachment_ref;

    const VkSubpassDescription subpasses[2] = {subpass0, subpass1};
    const VkAttachmentDescription attachments[2] = {attachment, resolve_attachment};
    const VkImageView image_views[2] = {image_view, resolve_image_view};

    const uint32_t view_masks[2] = {0x1, 0x2};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo renderpass_ci = vku::InitStructHelper(&multiview_ci);
    renderpass_ci.attachmentCount = 2;
    renderpass_ci.pAttachments = attachments;
    renderpass_ci.subpassCount = 2;
    renderpass_ci.pSubpasses = subpasses;

    const vkt::RenderPass render_pass(*m_device, renderpass_ci);
    const vkt::Framebuffer framebuffer(*m_device, render_pass, 2, image_views, 32, 32);

    VkClearAttachment clear_attachment{};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = 0;

    VkClearRect clear_rect{};
    clear_rect.rect = {{0, 0}, {32, 32}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    // Clear writes to view 0
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    m_command_buffer.NextSubpass();
    // Resolve reads view 1
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewLoadOpSubpassSelection) {
    TEST_DESCRIPTION("More advanced selection of the first loadOp subpass");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(
        128, 128, 1, 3, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 3);

    vkt::Buffer buffer(*m_device, 128 * 128 * 4 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    // Image copy writes to view 0 and view 1. It is enough to define external dependency with
    // subpass 0, because loadOp for view 0 and view 1 happens in subpass 0. View 1 is also used
    // in subpass 1 but it does loadOp only for view 2.
    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    const uint32_t view_masks[2] = {0x3 /* view 0 + view 1*/, 0x6 /* view 1 + view 2*/};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &subpass_dependency;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};  // copy to layer 0 and 1
    region.imageExtent = {128, 128, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncValRenderPass, MultiviewStoreOpSubpassSelection) {
    TEST_DESCRIPTION("More advanced selection of the last storeOp subpass");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitSyncVal());

    const VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(
        128, 128, 1, 3, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 3);

    vkt::Buffer buffer(*m_device, 128 * 128 * 4 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpasses[2] = {};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_ref;

    subpasses[1] = subpasses[0];

    const uint32_t view_masks[2] = {0x3 /* view 0 + view 1*/, 0x6 /* view 1 + view 2*/};
    VkRenderPassMultiviewCreateInfo multiview_ci = vku::InitStructHelper();
    multiview_ci.subpassCount = 2;
    multiview_ci.pViewMasks = view_masks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&multiview_ci);
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attachment;
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;
    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle(), 128, 128);

    // Even though view 1 is used by both subpasses, there is only single storeOp for
    // view 1 (in the last subpass that uses it which is subpass 1). That's why there
    // is no hazard from using view 1 in both subpasses without subpass dependency
    // (assuming that we don't have other accesses except storeOp which is true for this test).
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 128, 128);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
