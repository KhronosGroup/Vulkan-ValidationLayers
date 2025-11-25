/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "utils/convert_utils.h"

class PositiveSubpass : public VkLayerTest {};

TEST_F(PositiveSubpass, SubpassImageBarrier) {
    TEST_DESCRIPTION("Subpass with image barrier (self-dependency)");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

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
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle());

    // VkImageMemoryBarrier
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // VkDependencyInfo with VkImageMemoryBarrier2
    const auto safe_barrier2 = ConvertVkImageMemoryBarrierToV2(barrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    // Test vkCmdPipelineBarrier subpass barrier
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &barrier);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();

    // Test vkCmdPipelineBarrier2 subpass barrier
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
    m_command_buffer.Barrier(*safe_barrier2.ptr(), VK_DEPENDENCY_BY_REGION_BIT);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSubpass, SubpassWithEventWait) {
    TEST_DESCRIPTION("Subpass waits for the event set outside of this subpass");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

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
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass, 1, &image_view.handle());

    // VkImageMemoryBarrier
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // VkDependencyInfo with VkImageMemoryBarrier2
    const auto safe_barrier2 = ConvertVkImageMemoryBarrierToV2(barrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = 0;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = safe_barrier2.ptr();

    // vkCmdWaitEvents inside render pass
    {
        vkt::Event event(*m_device);
        m_command_buffer.Begin();
        vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
        vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
        vk::CmdEndRenderPass(m_command_buffer);
        m_command_buffer.End();
    }

    // vkCmdWaitEvents2 inside render pass.
    // It's also a regression test for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4258
    {
        vkt::Event event2(*m_device);
        m_command_buffer.Begin();
        vk::CmdSetEvent2(m_command_buffer, event2, &dependency_info);
        m_command_buffer.BeginRenderPass(render_pass, framebuffer, 32, 32);
        vk::CmdWaitEvents2(m_command_buffer, 1, &event2.handle(), &dependency_info);
        vk::CmdEndRenderPass(m_command_buffer);
        m_command_buffer.End();
    }
}

TEST_F(PositiveSubpass, InputAttachmentMissingSpecConstant) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInputAttachmentArrayDynamicIndexing);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddInputAttachment(1);
    rp.CreateRenderPass();

    const char* fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const int index = 3; // invalid if left as 3
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[4];
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(xs[index]);
        }
    )glsl";

    uint32_t data = 0;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveSubpass, InputAttachmentMissingSpecConstant2) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddInputAttachment(1);
    rp.CreateRenderPass();

    const char *fsSource = R"glsl(
        #version 450
        layout (constant_id = 0) const int index = 4; // over VkDescriptorSetLayoutBinding::descriptorCount
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[index];
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(xs[0]);
        }
    )glsl";

    uint32_t data = 2;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveSubpass, AccessFlags3) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkMemoryBarrierAccessFlags3KHR memory_barrier_access_flags = vku::InitStructHelper();
    memory_barrier_access_flags.srcAccessMask3 = VK_ACCESS_3_NONE_KHR;
    memory_barrier_access_flags.dstAccessMask3 = VK_ACCESS_3_NONE_KHR;

    VkMemoryBarrier2 memory_barrier = vku::InitStructHelper(&memory_barrier_access_flags);

    VkSubpassDependency2 subpass_dependency = vku::InitStructHelper(&memory_barrier);
    subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R32_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2 reference = vku::InitStructHelper();
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_GENERAL;
    reference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &reference;

    auto rpci2 =
        vku::InitStruct<VkRenderPassCreateInfo2>(nullptr, 0u, 1u, &attach_desc, 1u, &subpass, 1u, &subpass_dependency, 0u, nullptr);
    vkt::RenderPass rp(*m_device, rpci2);
}

TEST_F(PositiveSubpass, AllCommandsInSubpassDependency) {
    TEST_DESCRIPTION("Test ALL_COMMANDS_BIT is allowed in subpass dependency");
    RETURN_IF_SKIP(Init());

    VkSubpassDependency subpass_dep{};
    subpass_dep.srcSubpass = 0;
    subpass_dep.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // Here!
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    subpass_dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dstAccessMask = VK_ACCESS_NONE;

    RenderPassSingleSubpass rp(*this);
    rp.AddSubpassDependency(subpass_dep);
    rp.CreateRenderPass();
}

TEST_F(PositiveSubpass, TopOfPipeInSubpassDependency) {
    TEST_DESCRIPTION("Test TOP_OF_PIPE is allowed in subpass dependency");
    RETURN_IF_SKIP(Init());

    VkSubpassDependency subpass_dep{};
    subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep.dstSubpass = 0;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // Here!
    subpass_dep.srcAccessMask = VK_ACCESS_NONE;
    subpass_dep.dstAccessMask = VK_ACCESS_NONE;

    RenderPassSingleSubpass rp(*this);
    rp.AddSubpassDependency(subpass_dep);
    rp.CreateRenderPass();
}

TEST_F(PositiveSubpass, BottomOfPipeInSubpassDependency) {
    TEST_DESCRIPTION("Test BOTTOM_OF_PIPE is allowed in subpass dependency");
    RETURN_IF_SKIP(Init());

    VkSubpassDependency subpass_dep{};
    subpass_dep.srcSubpass = 0;
    subpass_dep.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;  // Here!
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    subpass_dep.srcAccessMask = VK_ACCESS_NONE;
    subpass_dep.dstAccessMask = VK_ACCESS_NONE;

    RenderPassSingleSubpass rp(*this);
    rp.AddSubpassDependency(subpass_dep);
    rp.CreateRenderPass();
}

TEST_F(PositiveSubpass, ColorBlendEnable) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10535");
    RETURN_IF_SKIP(Init());
    const VkFormat depth_format = FindSupportedDepthStencilFormat(Gpu());

    VkAttachmentDescription attach_desc[3];
    attach_desc[0].flags = 0;
    attach_desc[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attach_desc[1] = attach_desc[0];
    attach_desc[1].format = depth_format;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    attach_desc[2] = attach_desc[0];
    attach_desc[2].format = VK_FORMAT_R32G32B32A32_UINT;
    attach_desc[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference[2] = {};
    color_reference[0].attachment = 0;
    color_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_reference[1].attachment = 2;
    color_reference[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ds_reference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpasses[2] = {};
    subpasses[0].colorAttachmentCount = 2;
    subpasses[0].pColorAttachments = color_reference;
    subpasses[0].pDepthStencilAttachment = &ds_reference;

    subpasses[1].colorAttachmentCount = 1;
    subpasses[1].pColorAttachments = &color_reference[0];
    subpasses[1].pDepthStencilAttachment = &ds_reference;

    VkSubpassDependency dependencies[2] = {
        {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT},
        {0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT}};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 3;
    rpci.pAttachments = attach_desc;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 2;
    rpci.pDependencies = dependencies;
    vkt::RenderPass rp(*m_device, rpci);

    vkt::Image image_0(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view_0 = image_0.CreateView();
    vkt::Image image_1(*m_device, 32, 32, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView image_view_1 = image_1.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    vkt::Image image_2(*m_device, 32, 32, VK_FORMAT_R32G32B32A32_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view_2 = image_2.CreateView();

    VkImageView fb_attachments[3] = {image_view_0, image_view_1, image_view_2};
    vkt::Framebuffer fb(*m_device, rp, 3, fb_attachments);

    VkPipelineDepthStencilStateCreateInfo ds_state_ci = vku::InitStructHelper();
    ds_state_ci.depthTestEnable = VK_FALSE;
    ds_state_ci.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_state[2];
    color_state[0] = DefaultColorBlendAttachmentState();
    color_state[0].blendEnable = VK_FALSE;
    color_state[1] = DefaultColorBlendAttachmentState();
    color_state[1].blendEnable = VK_FALSE;

    CreatePipelineHelper pipe0(*this);
    pipe0.gp_ci_.renderPass = rp;
    pipe0.gp_ci_.pDepthStencilState = &ds_state_ci;
    pipe0.cb_ci_.attachmentCount = 2;
    pipe0.cb_ci_.pAttachments = color_state;
    pipe0.CreateGraphicsPipeline();

    CreatePipelineHelper pipe1(*this);
    pipe1.gp_ci_.renderPass = rp;
    pipe1.gp_ci_.pDepthStencilState = &ds_state_ci;
    pipe1.cb_ci_.attachmentCount = 1;
    pipe1.cb_ci_.pAttachments = color_state;
    pipe1.gp_ci_.subpass = 1;
    pipe1.CreateGraphicsPipeline();

    VkClearValue clear_values[3];
    memset(clear_values, 0, sizeof(clear_values));

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, fb, 32, 32, 3, clear_values);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe0);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.NextSubpass();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSubpass, InputAttachmentLayout) {
    TEST_DESCRIPTION("Create renderpass where an input attachment is also uses as another type");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkAttachmentDescription attach0 = {0,
                                             VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_SAMPLE_COUNT_1_BIT,
                                             VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                             VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                             VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                             VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                             VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    const VkAttachmentDescription attach1 = {0,
                                             VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_SAMPLE_COUNT_1_BIT,
                                             VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                             VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                             VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                             VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                             VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const VkAttachmentReference ref0 = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference ref1 = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference inRef0 = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    const VkAttachmentReference inRef1 = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    // First subpass draws to attachment 0
    const VkSubpassDescription subpass0 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref0, nullptr, nullptr, 0, nullptr};
    // Second subpass reads attachment 0 as input-attachment, writes to attachment 1
    const VkSubpassDescription subpass1 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &inRef0, 1, &ref1, nullptr, nullptr, 0, nullptr};
    // Seconnd subpass reads attachment 1 as input-attachment, writes to attachment 0
    const VkSubpassDescription subpass2 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &inRef1, 1, &ref0, nullptr, nullptr, 0, nullptr};

    // Subpass 0 writes attachment 0 as output, subpass 1 reads as input (RAW)
    VkSubpassDependency dep0 = {0,
                                1,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                VK_DEPENDENCY_BY_REGION_BIT};
    // Subpass 1 writes attachment 1 as output, subpass 2 reads as input while (RAW)
    VkSubpassDependency dep1 = {1,
                                2,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                VK_DEPENDENCY_BY_REGION_BIT};
    // Subpass 1 reads attachment 0 as input, subpass 2 writes output (WAR)
    VkSubpassDependency dep2 = {1,
                                2,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_DEPENDENCY_BY_REGION_BIT};

    std::vector<VkAttachmentDescription> attachs = {attach0, attach1};
    std::vector<VkSubpassDescription> subpasses = {subpass0, subpass1, subpass2};
    std::vector<VkSubpassDependency> deps = {dep0, dep1, dep2};

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachs), attachs.data(), size32(subpasses),
                                                        subpasses.data(), size32(deps), deps.data());

    // Current setup should be OK -- no attachment is both input and output in same subpass
    vkt::RenderPass render_pass(*m_device, rpci);
    if (rp2_supported) {
        vkt::RenderPass rp2(*m_device, *ConvertVkRenderPassCreateInfoToV2KHR(rpci).ptr());
    }
}
