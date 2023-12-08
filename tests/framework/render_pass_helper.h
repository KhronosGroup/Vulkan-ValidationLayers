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

#pragma once

#include "layer_validation_tests.h"
#include <vector>

// Helper designed to quickly make a renderPass/framebuffer that only has a single Subpass.
// The goal is to keep the class simple as possible.
//
// Common usage:
//   RenderPassSingleSubpass rp(*this);
//   rp.AddAttachmentDescription(input_format); // sets description[0]
//   rp.AddAttachmentDescription(color_format); // sets description[1]
//   rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});                  // sets reference[0]
//   rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}); // sets reference[1]
//   rp.AddInputAttachment(0); // index maps to reference[0]
//   rp.AddColorAttachment(1); // index maps to reference[1]
//   rp.CreateRenderPass();
class RenderPassSingleSubpass {
  public:
    RenderPassSingleSubpass(VkLayerTest &test, vkt::Device *device = nullptr);
    ~RenderPassSingleSubpass();

    VkRenderPassCreateInfo GetCreateInfo();
    VkRenderPass Handle() { return render_pass_; }

    // Ordered from most likely to be custom vs will use defauly
    void AddAttachmentDescription(VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                  VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
    // Overload for setting sampler count
    void AddAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
                                  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                                  VkImageLayout finalLayout = VK_IMAGE_LAYOUT_GENERAL);

    void AddAttachmentReference(VkAttachmentReference reference);

    // Pass in index to VkAttachmentReference
    void AddInputAttachment(uint32_t index);
    void AddColorAttachment(uint32_t index);
    void AddResolveAttachment(uint32_t index);
    void AddDepthStencilAttachment(uint32_t index);

    void AddSubpassDependency(VkSubpassDependency dependency);
    void AddSubpassDependency(VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VkAccessFlags srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                              VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT);

    void CreateRenderPass(void *pNext = nullptr);

    // Explicit destroy for those tests that need to test render pass lifetime
    void Destroy() { render_pass_.destroy(); };

  private:
    VkLayerTest &layer_test_;
    vkt::Device *device_;

    vkt::RenderPass render_pass_;
    VkRenderPassCreateInfo rp_create_info_;

    std::vector<VkAttachmentDescription> attachment_descriptions_;

    std::vector<VkAttachmentReference> attachments_references_;  // global pool
    std::vector<VkAttachmentReference> input_attachments_;
    std::vector<VkAttachmentReference> color_attachments_;
    VkAttachmentReference resolve_attachments_;
    VkAttachmentReference ds_attachments_;

    VkSubpassDescription subpass_description_;
    VkSubpassDependency subpass_dependency_;
};