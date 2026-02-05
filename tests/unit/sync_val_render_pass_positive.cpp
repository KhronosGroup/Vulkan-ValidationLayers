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

TEST_F(PositiveSyncValRenderPass, SyncInputAttachmentReadAndResolveWrite) {
    TEST_DESCRIPTION("Synchronize input attachment read with multisample resolve write");
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
