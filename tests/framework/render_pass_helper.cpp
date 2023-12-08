/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "render_pass_helper.h"

RenderPassSingleSubpass::RenderPassSingleSubpass(VkLayerTest& test, vkt::Device* device) : layer_test_(test) {
    // default VkDevice, can be overwritten if multi-device tests
    device_ = (device) ? device : layer_test_.DeviceObj();

    rp_create_info_ = vku::InitStructHelper();

    rp_create_info_.dependencyCount = 0;  // default to not having one
    rp_create_info_.pDependencies = &subpass_dependency_;

    subpass_description_.flags = 0;
    subpass_description_.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description_.inputAttachmentCount = 0;
    subpass_description_.pInputAttachments = nullptr;
    subpass_description_.colorAttachmentCount = 0;
    subpass_description_.pColorAttachments = nullptr;
    subpass_description_.pResolveAttachments = nullptr;
    subpass_description_.pDepthStencilAttachment = nullptr;
    subpass_description_.preserveAttachmentCount = 0;
    subpass_description_.pPreserveAttachments = nullptr;
    rp_create_info_.subpassCount = 1;
    rp_create_info_.pSubpasses = &subpass_description_;
}

RenderPassSingleSubpass::~RenderPassSingleSubpass() { Destroy(); }

VkRenderPassCreateInfo RenderPassSingleSubpass::GetCreateInfo() {
    rp_create_info_.attachmentCount = attachment_descriptions_.size();
    rp_create_info_.pAttachments = attachment_descriptions_.data();
    return rp_create_info_;
}

void RenderPassSingleSubpass::AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                       VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
    attachment_descriptions_.push_back({0, format, VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                        VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout});
}

void RenderPassSingleSubpass::AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkImageLayout initialLayout,
                                                       VkImageLayout finalLayout) {
    attachment_descriptions_.push_back({0, format, samples, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout,
                                        finalLayout});
}

void RenderPassSingleSubpass::AddAttachmentReference(VkAttachmentReference reference) {
    attachments_references_.push_back(reference);
}

void RenderPassSingleSubpass::AddInputAttachment(uint32_t index) {
    input_attachments_.push_back(attachments_references_[index]);
    subpass_description_.inputAttachmentCount = input_attachments_.size();
    subpass_description_.pInputAttachments = input_attachments_.data();
}

void RenderPassSingleSubpass::AddColorAttachment(uint32_t index) {
    color_attachments_.push_back(attachments_references_[index]);
    subpass_description_.colorAttachmentCount = color_attachments_.size();
    subpass_description_.pColorAttachments = color_attachments_.data();
}

void RenderPassSingleSubpass::AddResolveAttachment(uint32_t index) {
    resolve_attachments_ = attachments_references_[index];
    subpass_description_.pResolveAttachments = &resolve_attachments_;
}

void RenderPassSingleSubpass::AddDepthStencilAttachment(uint32_t index) {
    ds_attachments_ = attachments_references_[index];
    subpass_description_.pDepthStencilAttachment = &ds_attachments_;
}

void RenderPassSingleSubpass::AddSubpassDependency(VkSubpassDependency dependency) {
    subpass_dependency_ = dependency;
    rp_create_info_.dependencyCount = 1;
}

void RenderPassSingleSubpass::AddSubpassDependency(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                   VkDependencyFlags dependencyFlags) {
    subpass_dependency_.srcSubpass = 0;
    subpass_dependency_.dstSubpass = 0;
    subpass_dependency_.srcStageMask = srcStageMask;
    subpass_dependency_.srcStageMask = srcStageMask;
    subpass_dependency_.dstStageMask = dstStageMask;
    subpass_dependency_.srcAccessMask = srcAccessMask;
    subpass_dependency_.dstAccessMask = dstAccessMask;
    subpass_dependency_.dependencyFlags = dependencyFlags;
    rp_create_info_.dependencyCount = 1;
}

void RenderPassSingleSubpass::CreateRenderPass(void* pNext) {
    VkRenderPassCreateInfo rp_create_info = GetCreateInfo();
    rp_create_info.pNext = pNext;
    render_pass_.init(*device_, rp_create_info);
}
