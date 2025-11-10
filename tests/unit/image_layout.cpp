/*
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeImageLayout : public ImageTest {};

TEST_F(NegativeImageLayout, Blit) {
    TEST_DESCRIPTION("Incorrect vkCmdBlitImage layouts");
    RETURN_IF_SKIP(Init());

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;

    vkt::Image img_src_transfer(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_dst_transfer(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_general(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image img_color(*m_device, 64, 64, fmt,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    img_src_transfer.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    img_dst_transfer.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    img_general.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    img_color.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {32, 32, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    // Illegal srcImageLayout/dstImageLayout
    m_command_buffer.Begin();
    vk::CmdBlitImage(m_command_buffer, img_general, VK_IMAGE_LAYOUT_GENERAL, img_general, VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region,
                     VK_FILTER_LINEAR);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImageLayout-01398");
    vk::CmdBlitImage(m_command_buffer, img_src_transfer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, img_dst_transfer,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImageLayout-01399");
    vk::CmdBlitImage(m_command_buffer, img_src_transfer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, img_dst_transfer,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Destination image in invalid layout at start of the CB
    m_command_buffer.Begin();
    vk::CmdBlitImage(m_command_buffer, img_src_transfer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, img_color, VK_IMAGE_LAYOUT_GENERAL,
                     1, &blit_region, VK_FILTER_LINEAR);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Source image in invalid layout at start of the CB
    m_command_buffer.Begin();
    vk::CmdBlitImage(m_command_buffer, img_color, VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &blit_region, VK_FILTER_LINEAR);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Source image in invalid layout in the middle of CB
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = img_general;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImageLayout-00221");
    vk::CmdBlitImage(m_command_buffer, img_general, VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Destination image in invalid layout in the middle of CB
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.image = img_dst_transfer;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImageLayout-00226");
    vk::CmdBlitImage(m_command_buffer, img_src_transfer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, img_dst_transfer,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, Compute) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    {  // Verify invalid image layout with CmdDispatch
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.Begin();
        vk::CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &pipe.descriptor_set_.set_, 0,
                                  nullptr);
        vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdDispatch(cmd, 1, 1, 1);
        cmd.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
        m_default_queue->Submit(cmd);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    {  // Verify invalid image layout with CmdDispatchBaseKHR
        vkt::CommandBuffer cmd(*m_device, m_command_pool);
        cmd.Begin();
        vk::CmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &pipe.descriptor_set_.set_, 0,
                                  nullptr);
        vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdDispatchBaseKHR(cmd, 0, 0, 0, 1, 1, 1);
        cmd.End();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
        m_default_queue->Submit(cmd);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImageLayout, Compute11) {
    TEST_DESCRIPTION("Attempt to use an image with an invalid layout in a compute shader using vkCmdDispatchBase");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1) in;
    layout(set=0, binding=0) uniform sampler2D s;
    void main(){
        vec4 v = 2.0 * texture(s, vec2(0.0));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatchBase(m_command_buffer, 0, 0, 0, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

// inspired by https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5846
TEST_F(NegativeImageLayout, MultipleCommandDispatches) {
    TEST_DESCRIPTION("Make sure we can detect the exact dispatch command that caused the error later");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        void main(){
            vec4 v = texture(s, vec2(0.0));
        }
    )glsl";

    OneOffDescriptorSet descriptor_set0(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set1(m_device,
                                        {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set0.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.CreateComputePipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Image good_image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView good_image_view = good_image.CreateView();
    descriptor_set0.WriteDescriptorImageInfo(0, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set0.UpdateDescriptorSets();

    vkt::Image bad_image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    bad_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView bad_image_view = bad_image.CreateView();
    descriptor_set1.WriteDescriptorImageInfo(0, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set1.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    // contains good image
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set0.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // contains bad image
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set1.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // contains good image again
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set0.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8918
// This fails to create descriptor on RADV, might need to adjust OneOffDescriptorSet
TEST_F(NegativeImageLayout, DISABLED_Mutable) {
    TEST_DESCRIPTION("Invalid image layout with mutable descriptors");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());

    const char *cs = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        void main(){
            vec4 v = 2.0 * texture(s, vec2(0.0));
        }
    )glsl";

    VkDescriptorType desc_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkMutableDescriptorTypeListEXT type_list = {2, desc_types};
    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &type_list;

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}, 0,
                                       &mdtci);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    const VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 64, 64, fmt, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, PushDescriptor) {
    TEST_DESCRIPTION("Use a push descriptor with a mismatched image layout.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const vkt::DescriptorSetLayout ds_layout(
        *m_device, {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    auto pipeline_layout = vkt::PipelineLayout(*m_device, {&ds_layout});

    const char *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 color;
        void main(){
           color = textureLod(tex, vec2(0.5, 0.5), 0.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorImageInfo img_info = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.dstBinding = 0;

    // Image layout is known at submit time
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdPushDescriptorSetKHR(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_write);
    vk::CmdDraw(m_command_buffer, 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Image layout is known at record time
    m_command_buffer.Begin();
    image.ImageMemoryBarrier(m_command_buffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdPushDescriptorSetKHR(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_write);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-imageLayout-00344");
    vk::CmdDraw(m_command_buffer, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// INVALID_IMAGE_LAYOUT tests (one other case is hit by MapMemWithoutHostVisibleBit and not here)
TEST_F(NegativeImageLayout, Basic) {
    TEST_DESCRIPTION("Generally these involve having images in the wrong layout when they're copied or transitioned.");
    // 3 in ValidateCmdBufImageLayouts
    // *  -1 Attempt to submit cmd buf w/ deleted image
    // *  -2 Cmd buf submit of image w/ layout not matching first use w/ subresource
    // *  -3 Cmd buf submit of image w/ layout not matching first use w/o subresource

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image depth_image(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImageLayout-01917");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImageLayout-00128");
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_UNDEFINED, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImageLayout-00133");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImageLayout-01395");
    vk::CmdCopyImage(m_command_buffer, src_image, VK_IMAGE_LAYOUT_GENERAL, dst_image, VK_IMAGE_LAYOUT_UNDEFINED, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Equivalent tests using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageCopy2 copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2,
                                           NULL,
                                           copy_region.srcSubresource,
                                           copy_region.srcOffset,
                                           copy_region.dstSubresource,
                                           copy_region.dstOffset,
                                           copy_region.extent};
        VkCopyImageInfo2 copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
                                             NULL,
                                             src_image,
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             dst_image,
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             1,
                                             &copy_region2};

        m_command_buffer.FullMemoryBarrier();
        vk::CmdCopyImage2KHR(m_command_buffer, &copy_image_info2);

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImageLayout-00128");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImageLayout-01917");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_command_buffer, &copy_image_info2);
        m_errorMonitor->VerifyFound();

        // Now verify same checks for dst
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        m_command_buffer.FullMemoryBarrier();
        vk::CmdCopyImage2KHR(m_command_buffer, &copy_image_info2);

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImageLayout-00133");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImageLayout-01395");
        copy_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk::CmdCopyImage2KHR(m_command_buffer, &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Convert dst and depth images to TRANSFER_DST for subsequent tests
    VkImageMemoryBarrier transfer_dst_image_barrier[1] = {};
    transfer_dst_image_barrier[0] = vku::InitStructHelper();
    transfer_dst_image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transfer_dst_image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transfer_dst_image_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transfer_dst_image_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transfer_dst_image_barrier[0].image = dst_image;
    transfer_dst_image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    transfer_dst_image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                           NULL, 1, transfer_dst_image_barrier);
    transfer_dst_image_barrier[0].image = depth_image;
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                           NULL, 1, transfer_dst_image_barrier);

    // Cause errors due to clearing with invalid image layouts
    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail due to explicitly prohibited layout for color clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-01394");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_command_buffer, dst_image, VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for color clear.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-00004");
    vk::CmdClearColorImage(m_command_buffer, dst_image, VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();

    VkClearDepthStencilValue depth_clear_value = {};
    clear_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Fail due to explicitly prohibited layout for depth clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00012");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_command_buffer, depth_image, VK_IMAGE_LAYOUT_UNDEFINED, &depth_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for depth clear.
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    vk::CmdClearDepthStencilImage(m_command_buffer, depth_image, VK_IMAGE_LAYOUT_GENERAL, &depth_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();

    VkImageMemoryBarrier image_barrier[1] = {};
    // In synchronization2, if oldLayout == newLayout, we're not doing an ILT and these fields don't need to match
    // the image's layout.
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].image = src_image;
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                           NULL, 1, image_barrier);

    // Now cause error due to bad image layout transition in PipelineBarrier
    image_barrier[0] = vku::InitStructHelper();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_barrier[0].image = src_image;
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-01197");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-01210");
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
                           NULL, 1, image_barrier);
    m_errorMonitor->VerifyFound();

    // Finally some layout errors at RenderPass create time
    // Just hacking in specific state to get to the errors we want so don't copy this unless you know what you're doing.
    VkAttachmentReference attach = {};
    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &attach;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkRenderPass rp;

    // For this error we need a valid renderpass so create default one
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach.attachment = 0;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Can't do a CLEAR load on READ_ONLY initialLayout
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rpci.pAttachments = &attach_desc;
    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription-format-03283");
    vk::CreateRenderPass(device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, CopyColorToDepthOnComputeQueue) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_10_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredFeature(vkt::Feature::maintenance10);
    RETURN_IF_SKIP(Init());

    auto compute_without_graphics_queue_i = m_device->QueueFamily(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!compute_without_graphics_queue_i.has_value()) {
        GTEST_SKIP() << "Need a queue that supports compute but not graphics";
    }
    const bool ds_supports_copy_on_compute_queue = FormatFeatures2AreSupported(
        Gpu(), VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR);
    if (ds_supports_copy_on_compute_queue) {
        GTEST_SKIP() << "Test requires format features to not support depth copy on compute queue";
    }
    vkt::CommandPool pool(*m_device, *compute_without_graphics_queue_i);
    vkt::CommandBuffer cb(*m_device, pool);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R16_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image depth_image(*m_device, image_create_info, vkt::set_layout);

    cb.Begin();
    VkImageCopy copy_region{};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-11782");
    vk::CmdCopyImage(cb, src_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, CopyColorToDepthOnTransferQueue) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_10_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredFeature(vkt::Feature::maintenance10);
    RETURN_IF_SKIP(Init());

    auto compute_without_graphics_queue_i =
        m_device->QueueFamily(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    if (!compute_without_graphics_queue_i.has_value()) {
        GTEST_SKIP() << "Need a queue that supports transfer but not graphics";
    }
    const bool ds_supports_copy_on_transfer_queue = FormatFeatures2AreSupported(
        Gpu(), VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR);
    if (ds_supports_copy_on_transfer_queue) {
        GTEST_SKIP() << "Test requires format features to not support depth copy on transfer queue";
    }

    vkt::CommandPool pool(*m_device, *compute_without_graphics_queue_i);
    vkt::CommandBuffer cb(*m_device, pool);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R16_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image depth_image(*m_device, image_create_info, vkt::set_layout);

    cb.Begin();
    VkImageCopy copy_region{};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-11783");
    vk::CmdCopyImage(cb, src_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, StorageImage) {
    TEST_DESCRIPTION("Attempt to update a STORAGE_IMAGE descriptor w/o GENERAL layout.");
    RETURN_IF_SKIP(Init());

    if ((m_device->FormatFeaturesOptimal(VK_FORMAT_R8G8B8A8_UNORM) & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT; skipped.";
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::ImageView view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-04152");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, ArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);

    // layer 0 now VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // layer 1 is still VK_IMAGE_LAYOUT_UNDEFINED.
    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.image = image;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Get layer with undefined layout
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 1, 1);

    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, MultiArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);

    // layer 0 now VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // layer 1 is still VK_IMAGE_LAYOUT_UNDEFINED.
    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.image = image;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Bind view to both layers
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    const char *fs_source = R"glsl(
        #version 460
        layout(set=0, binding=0) uniform sampler2DArray s;
        layout(location=0) out vec4 x;
        void main(){
            x = texture(s, vec3(1, 1, 1)); // accesses invalid layer
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DescriptorArrayStaticIndex) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    const char *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        // [0] is good layout
        // [1] is bad layout
        layout(set = 0, binding = 0) uniform sampler2D tex[2];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[1], vec2(0, 0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout;
    pipe.CreateGraphicsPipeline();

    vkt::Image bad_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView bad_image_view = bad_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::ImageView good_image_view = good_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, LayoutTransitionRenderPassObject) {
    TEST_DESCRIPTION("Image barrier must not perform layout transition inside render pass instance");
    RETURN_IF_SKIP(Init());

    VkSubpassDependency subpass_dep{};
    subpass_dep.srcSubpass = 0;
    subpass_dep.dstSubpass = 0;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency(subpass_dep);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp, 1, &view.handle());

    VkImageMemoryBarrier render_pass_barrier = vku::InitStructHelper();
    render_pass_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    render_pass_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // Attempt to perform image layout transition. It is forbidden if the barrier is inside
    // render pass instance (either started with vkCmdBeginRenderPass or vkCmdBeginRendering).
    render_pass_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    render_pass_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    render_pass_barrier.image = image;
    render_pass_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, fb);
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &render_pass_barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, LayoutTransitionRenderPassObject2) {
    TEST_DESCRIPTION("Image barrier must not perform layout transition inside render pass instance");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkSubpassDependency subpass_dep{};
    subpass_dep.srcSubpass = 0;
    subpass_dep.dstSubpass = 0;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency(subpass_dep);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 64, 64, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp, 1, &view.handle());

    VkImageMemoryBarrier2 render_pass_barrier = vku::InitStructHelper();
    render_pass_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_pass_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    render_pass_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_pass_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // Attempt to perform image layout transition. It is forbidden if the barrier is inside
    // render pass instance (either started with vkCmdBeginRenderPass or vkCmdBeginRendering).
    render_pass_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    render_pass_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    render_pass_barrier.image = image;
    render_pass_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, fb);
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-oldLayout-01181");
    m_command_buffer.Barrier(render_pass_barrier, VK_DEPENDENCY_BY_REGION_BIT);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, TimelineSemaphoreOrdering) {
    TEST_DESCRIPTION("Timeline semaphore makes execution order different than submission order");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_command_buffer.Barrier(layout_transition);
    m_command_buffer.End();

    m_second_command_buffer.Begin();
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_second_command_buffer.Barrier(layout_transition);
    m_second_command_buffer.End();

    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingColorAttachmentLayout) {
    TEST_DESCRIPTION("Color attachment layout does not match expected layout");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    VkClearColorValue clear_color{};
    VkImageSubresourceRange clear_subresource{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_subresource);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09592");
    m_command_buffer.BeginRendering(rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingColorAttachmentLayout2) {
    TEST_DESCRIPTION("Transition image layout after image was used as dynamic rendering attachment");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    // Does not match COLOR_ATTACHMENT_OPTIMAL layout specified by attachment
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-oldLayout-01197");
    m_command_buffer.Barrier(layout_transition);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, DynamicRenderingDepthAttachmentLayout) {
    TEST_DESCRIPTION("Depth attachment layout does not match expected layout");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment;

    VkClearDepthStencilValue clear_value{};
    VkImageSubresourceRange clear_subresource{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdClearDepthStencilImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_subresource);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09588");
    m_command_buffer.BeginRendering(rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingDepthAttachmentLayout2) {
    TEST_DESCRIPTION("Transition image layout after image was used as dynamic rendering attachment");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment;

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    // Does not match DEPTH_ATTACHMENT_OPTIMAL layout specified by attachment
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-oldLayout-01197");
    m_command_buffer.Barrier(layout_transition);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, DynamicRenderingStencilAttachmentLayout) {
    TEST_DESCRIPTION("Stencil attachment layout does not match expected layout");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pStencilAttachment = &stencil_attachment;

    VkClearDepthStencilValue clear_value{};
    VkImageSubresourceRange clear_subresource{VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdClearDepthStencilImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_subresource);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09590");
    m_command_buffer.BeginRendering(rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingStencilAttachmentLayout2) {
    TEST_DESCRIPTION("Transition image layout after image was used as dynamic rendering attachment");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pStencilAttachment = &stencil_attachment;

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    // Does not match STENCIL_ATTACHMENT_OPTIMAL layout specified by attachment
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-oldLayout-01197");
    m_command_buffer.Barrier(layout_transition);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeImageLayout, DynamicRenderingColorAttachmentLayoutSubmitTime) {
    TEST_DESCRIPTION("Color attachment layout does not match expected layout. Submit time validation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView image_view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09592");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingDepthAttachmentLayoutSubmitTime) {
    TEST_DESCRIPTION("Depth attachment layout does not match expected layout. Submit time validation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09588");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageLayout, DynamicRenderingStencilAttachmentLayoutSubmitTime) {
    TEST_DESCRIPTION("Stencil attachment layout does not match expected layout. Submit time validation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image image(*m_device, 128, 128, depth_stencil_format,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {128, 128};
    rendering_info.layerCount = 1;
    rendering_info.pStencilAttachment = &stencil_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRendering-pRenderingInfo-09590");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}
