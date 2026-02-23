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

TEST_F(PositiveSyncValRenderPass, StencilKeepOps) {
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
