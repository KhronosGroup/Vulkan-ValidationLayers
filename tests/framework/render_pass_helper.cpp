/*
 * Copyright (c) 2023-2026 The Khronos Group Inc.
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "render_pass_helper.h"

InterfaceRenderPassSingleSubpass::InterfaceRenderPassSingleSubpass(VkLayerTest& test, vkt::Device* device) : layer_test_(test) {
    // default VkDevice, can be overwritten if multi-device tests
    device_ = (device) ? device : layer_test_.DeviceObj();
}

RenderPassSingleSubpass::RenderPassSingleSubpass(VkLayerTest& test, vkt::Device* device)
    : InterfaceRenderPassSingleSubpass(test, device) {
    rp_create_info_ = vku::InitStructHelper();

    rp_create_info_.dependencyCount = 0;  // default to not having one

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

VkRenderPassCreateInfo RenderPassSingleSubpass::GetCreateInfo() {
    rp_create_info_.attachmentCount = attachment_descriptions_.size();
    rp_create_info_.pAttachments = attachment_descriptions_.data();
    rp_create_info_.pDependencies = subpass_dependencies_.data();
    return rp_create_info_;
}

void RenderPassSingleSubpass::AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                       VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
    attachment_descriptions_.emplace_back(VkAttachmentDescription{0, format, VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp,
                                                                  VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                  initialLayout, finalLayout});
}

void RenderPassSingleSubpass::AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkImageLayout initialLayout,
                                                       VkImageLayout finalLayout) {
    attachment_descriptions_.emplace_back(VkAttachmentDescription{0, format, samples, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                  VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                  VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout});
}

void RenderPassSingleSubpass::AddAttachmentDescription(const VkAttachmentDescription& attachment_description) {
    attachment_descriptions_.emplace_back(attachment_description);
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
    resolve_attachment_ = attachments_references_[index];
    subpass_description_.pResolveAttachments = &resolve_attachment_;
}

void RenderPassSingleSubpass::AddDepthStencilAttachment(uint32_t index) {
    ds_attachment_ = attachments_references_[index];
    subpass_description_.pDepthStencilAttachment = &ds_attachment_;
}

void RenderPassSingleSubpass::AddSubpassDependency(VkSubpassDependency dependency) {
    subpass_dependencies_.push_back(dependency);
    rp_create_info_.dependencyCount = (uint32_t)subpass_dependencies_.size();
}

void RenderPassSingleSubpass::AddSubpassDependency(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                   VkDependencyFlags dependencyFlags) {
    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = srcStageMask;
    subpass_dependency.srcStageMask = srcStageMask;
    subpass_dependency.dstStageMask = dstStageMask;
    subpass_dependency.srcAccessMask = srcAccessMask;
    subpass_dependency.dstAccessMask = dstAccessMask;
    subpass_dependency.dependencyFlags = dependencyFlags;
    AddSubpassDependency(subpass_dependency);
}

void RenderPassSingleSubpass::CreateRenderPass(void* pNext, VkRenderPassCreateFlags flags) {
    VkRenderPassCreateInfo rp_create_info = GetCreateInfo();
    rp_create_info.pNext = pNext;
    rp_create_info.flags = flags;
    render_pass_.Init(*device_, rp_create_info);
}

RenderPass2SingleSubpass::RenderPass2SingleSubpass(VkLayerTest& test, vkt::Device* device)
    : InterfaceRenderPassSingleSubpass(test, device) {
    rp_create_info_ = vku::InitStructHelper();

    rp_create_info_.dependencyCount = 0;  // default to not having one

    subpass_description_ = vku::InitStructHelper();
    subpass_description_.flags = 0;
    subpass_description_.viewMask = 0;
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

VkRenderPassCreateInfo2 RenderPass2SingleSubpass::GetCreateInfo() {
    rp_create_info_.attachmentCount = attachment_descriptions_.size();
    rp_create_info_.pAttachments = attachment_descriptions_.data();
    rp_create_info_.pDependencies = subpass_dependencies_.data();
    return rp_create_info_;
}

void RenderPass2SingleSubpass::AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                        VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
    attachment_descriptions_.push_back(vku::InitStruct<VkAttachmentDescription2>(
        nullptr, 0u, format, VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout));
}

void RenderPass2SingleSubpass::AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkImageLayout initialLayout,
                                                        VkImageLayout finalLayout) {
    attachment_descriptions_.push_back(vku::InitStruct<VkAttachmentDescription2>(
        nullptr, 0u, format, samples, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout));
}

void RenderPass2SingleSubpass::SetAttachmentDescriptionPNext(uint32_t index, void* pNext) {
    attachment_descriptions_[index].pNext = pNext;
}

void RenderPass2SingleSubpass::AddInputAttachment(uint32_t attachment_index, VkImageLayout layout, VkImageAspectFlags aspect_mask,
                                                  void* pNext) {
    VkAttachmentReference2 attachment_ref = vku::InitStructHelper(pNext);
    attachment_ref.attachment = attachment_index;
    attachment_ref.layout = layout;
    attachment_ref.aspectMask = aspect_mask;

    input_attachments_.push_back(attachment_ref);
    subpass_description_.inputAttachmentCount = input_attachments_.size();
    subpass_description_.pInputAttachments = input_attachments_.data();
}

void RenderPass2SingleSubpass::AddColorAttachment(uint32_t attachment_index, VkImageLayout layout, VkImageAspectFlags aspect_mask,
                                                  void* pNext) {
    VkAttachmentReference2 attachment_ref = vku::InitStructHelper(pNext);
    attachment_ref.attachment = attachment_index;
    attachment_ref.layout = layout;
    attachment_ref.aspectMask = aspect_mask;

    color_attachments_.push_back(attachment_ref);
    subpass_description_.colorAttachmentCount = color_attachments_.size();
    subpass_description_.pColorAttachments = color_attachments_.data();
}

void RenderPass2SingleSubpass::AddResolveAttachment(uint32_t attachment_index, VkImageLayout layout, VkImageAspectFlags aspect_mask,
                                                    void* pNext) {
    resolve_attachment_ = vku::InitStructHelper(pNext);
    resolve_attachment_.attachment = attachment_index;
    resolve_attachment_.layout = layout;
    resolve_attachment_.aspectMask = aspect_mask;
    subpass_description_.pResolveAttachments = &resolve_attachment_;
}

void RenderPass2SingleSubpass::AddDepthStencilAttachment(uint32_t attachment_index, VkImageLayout layout,
                                                         VkImageAspectFlags aspect_mask, void* pNext) {
    ds_attachment_ = vku::InitStructHelper(pNext);
    ds_attachment_.attachment = attachment_index;
    ds_attachment_.layout = layout;
    ds_attachment_.aspectMask = aspect_mask;
    subpass_description_.pDepthStencilAttachment = &ds_attachment_;
}

void RenderPass2SingleSubpass::AddDepthStencilResolveAttachment(uint32_t attachment_index, VkImageLayout layout,
                                                                VkResolveModeFlagBits depth_resolve_mode,
                                                                VkResolveModeFlagBits stencil_resolve_mode) {
    ds_resolve_attachment_ = vku::InitStructHelper();
    ds_resolve_attachment_.attachment = attachment_index;
    ds_resolve_attachment_.layout = layout;

    ds_resolve_ = vku::InitStructHelper();
    ds_resolve_.pDepthStencilResolveAttachment = &ds_resolve_attachment_;
    ds_resolve_.depthResolveMode = depth_resolve_mode;
    ds_resolve_.stencilResolveMode = stencil_resolve_mode;

    subpass_description_.pNext = &ds_resolve_;
}

void RenderPass2SingleSubpass::AddFragmentShadingRateAttachment(uint32_t attachment_index, VkImageLayout layout,
                                                                VkExtent2D texel_size) {
    fsr_attachment_ = vku::InitStructHelper();
    fsr_attachment_.attachment = attachment_index;
    fsr_attachment_.layout = layout;

    fsr_attachment_info_ = vku::InitStructHelper();
    fsr_attachment_info_.pFragmentShadingRateAttachment = &fsr_attachment_;
    fsr_attachment_info_.shadingRateAttachmentTexelSize = texel_size;
    subpass_description_.pNext = &fsr_attachment_info_;
}

void RenderPass2SingleSubpass::AddSubpassDependency(VkSubpassDependency2 dependency) {
    subpass_dependencies_.push_back(dependency);
    rp_create_info_.dependencyCount = (uint32_t)subpass_dependencies_.size();
}

void RenderPass2SingleSubpass::AddSubpassDependency(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                    VkDependencyFlags dependencyFlags) {
    VkSubpassDependency2 subpass_dependency = vku::InitStructHelper();
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = srcStageMask;
    subpass_dependency.srcStageMask = srcStageMask;
    subpass_dependency.dstStageMask = dstStageMask;
    subpass_dependency.srcAccessMask = srcAccessMask;
    subpass_dependency.dstAccessMask = dstAccessMask;
    subpass_dependency.dependencyFlags = dependencyFlags;
    AddSubpassDependency(subpass_dependency);
}

void RenderPass2SingleSubpass::CreateRenderPass(void* pNext, VkRenderPassCreateFlags flags) {
    VkRenderPassCreateInfo2 rp_create_info = GetCreateInfo();
    rp_create_info.pNext = pNext;
    rp_create_info.flags = flags;
    render_pass_.Init(*device_, rp_create_info);
}
