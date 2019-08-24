/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Tobias Hector <@tobski>
 */

#include "convert_to_renderpass2.h"

#include <vector>

#include "vk_format_utils.h"
#include "vk_typemap_helper.h"

static safe_VkAttachmentDescription2KHR ToV2KHR(const VkAttachmentDescription& in_struct) {
    safe_VkAttachmentDescription2KHR v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
    v2.pNext = nullptr;
    v2.flags = in_struct.flags;
    v2.format = in_struct.format;
    v2.samples = in_struct.samples;
    v2.loadOp = in_struct.loadOp;
    v2.storeOp = in_struct.storeOp;
    v2.stencilLoadOp = in_struct.stencilLoadOp;
    v2.stencilStoreOp = in_struct.stencilStoreOp;
    v2.initialLayout = in_struct.initialLayout;
    v2.finalLayout = in_struct.finalLayout;

    return v2;
}

static safe_VkAttachmentReference2KHR ToV2KHR(const VkAttachmentReference& in_struct, const VkImageAspectFlags aspectMask = 0) {
    safe_VkAttachmentReference2KHR v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    v2.pNext = nullptr;
    v2.attachment = in_struct.attachment;
    v2.layout = in_struct.layout;
    v2.aspectMask = aspectMask;

    return v2;
}

static safe_VkSubpassDescription2KHR ToV2KHR(const VkSubpassDescription& in_struct, const uint32_t viewMask,
                                             const VkImageAspectFlags* input_attachment_aspect_masks) {
    safe_VkSubpassDescription2KHR v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
    v2.pNext = nullptr;
    v2.flags = in_struct.flags;
    v2.pipelineBindPoint = in_struct.pipelineBindPoint;
    v2.viewMask = viewMask;
    v2.inputAttachmentCount = in_struct.inputAttachmentCount;
    v2.pInputAttachments = nullptr;  // to be filled
    v2.colorAttachmentCount = in_struct.colorAttachmentCount;
    v2.pColorAttachments = nullptr;        // to be filled
    v2.pResolveAttachments = nullptr;      // to be filled
    v2.pDepthStencilAttachment = nullptr;  // to be filled
    v2.preserveAttachmentCount = in_struct.preserveAttachmentCount;
    v2.pPreserveAttachments = nullptr;  // to be filled

    if (v2.inputAttachmentCount && in_struct.pInputAttachments) {
        v2.pInputAttachments = new safe_VkAttachmentReference2KHR[v2.inputAttachmentCount];
        for (uint32_t i = 0; i < v2.inputAttachmentCount; ++i) {
            v2.pInputAttachments[i] = ToV2KHR(in_struct.pInputAttachments[i], input_attachment_aspect_masks[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pColorAttachments) {
        v2.pColorAttachments = new safe_VkAttachmentReference2KHR[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pColorAttachments[i] = ToV2KHR(in_struct.pColorAttachments[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pResolveAttachments) {
        v2.pResolveAttachments = new safe_VkAttachmentReference2KHR[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pResolveAttachments[i] = ToV2KHR(in_struct.pResolveAttachments[i]);
        }
    }
    if (in_struct.pDepthStencilAttachment) {
        v2.pDepthStencilAttachment = new safe_VkAttachmentReference2KHR();
        *v2.pDepthStencilAttachment = ToV2KHR(*in_struct.pDepthStencilAttachment);
    }
    if (v2.preserveAttachmentCount && in_struct.pPreserveAttachments) {
        auto preserve_attachments = new uint32_t[v2.preserveAttachmentCount];
        for (uint32_t i = 0; i < v2.preserveAttachmentCount; ++i) {
            preserve_attachments[i] = in_struct.pPreserveAttachments[i];
        }
        v2.pPreserveAttachments = preserve_attachments;
    }

    return v2;
}

static safe_VkSubpassDependency2KHR ToV2KHR(const VkSubpassDependency& in_struct, int32_t viewOffset = 0) {
    safe_VkSubpassDependency2KHR v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
    v2.pNext = nullptr;
    v2.srcSubpass = in_struct.srcSubpass;
    v2.dstSubpass = in_struct.dstSubpass;
    v2.srcStageMask = in_struct.srcStageMask;
    v2.dstStageMask = in_struct.dstStageMask;
    v2.srcAccessMask = in_struct.srcAccessMask;
    v2.dstAccessMask = in_struct.dstAccessMask;
    v2.dependencyFlags = in_struct.dependencyFlags;
    v2.viewOffset = viewOffset;

    return v2;
}

void ConvertVkRenderPassCreateInfoToV2KHR(const VkRenderPassCreateInfo& in_struct, safe_VkRenderPassCreateInfo2KHR* out_struct) {
    using std::vector;
    const auto multiview_info = lvl_find_in_chain<VkRenderPassMultiviewCreateInfo>(in_struct.pNext);
    const auto* input_attachment_aspect_info = lvl_find_in_chain<VkRenderPassInputAttachmentAspectCreateInfo>(in_struct.pNext);

    out_struct->~safe_VkRenderPassCreateInfo2KHR();
    out_struct->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->flags = in_struct.flags;
    out_struct->attachmentCount = in_struct.attachmentCount;
    out_struct->pAttachments = nullptr;  // to be filled
    out_struct->subpassCount = in_struct.subpassCount;
    out_struct->pSubpasses = nullptr;  // to be filled
    out_struct->dependencyCount = in_struct.dependencyCount;
    out_struct->pDependencies = nullptr;  // to be filled
    out_struct->correlatedViewMaskCount = multiview_info ? multiview_info->correlationMaskCount : 0;
    out_struct->pCorrelatedViewMasks = nullptr;  // to be filled

    // TODO: This should support VkRenderPassFragmentDensityMapCreateInfoEXT somehow
    // see https://github.com/KhronosGroup/Vulkan-Docs/issues/1027

    if (out_struct->attachmentCount && in_struct.pAttachments) {
        out_struct->pAttachments = new safe_VkAttachmentDescription2KHR[out_struct->attachmentCount];
        for (uint32_t i = 0; i < out_struct->attachmentCount; ++i) {
            out_struct->pAttachments[i] = ToV2KHR(in_struct.pAttachments[i]);
        }
    }

    // translate VkRenderPassInputAttachmentAspectCreateInfo into vector
    vector<vector<VkImageAspectFlags>> input_attachment_aspect_masks(out_struct->subpassCount);
    // set defaults
    for (uint32_t si = 0; si < out_struct->subpassCount; ++si) {
        if (in_struct.pSubpasses) {
            input_attachment_aspect_masks[si].resize(in_struct.pSubpasses[si].inputAttachmentCount, 0);

            for (uint32_t iai = 0; iai < in_struct.pSubpasses[si].inputAttachmentCount; ++iai) {
                if (out_struct->pAttachments && in_struct.pSubpasses[si].pInputAttachments) {
                    const auto& input_attachment = in_struct.pSubpasses[si].pInputAttachments[iai];
                    const auto format = out_struct->pAttachments[input_attachment.attachment].format;

                    if (FormatIsColor(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_COLOR_BIT;
                    if (FormatHasDepth(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_DEPTH_BIT;
                    if (FormatHasStencil(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_STENCIL_BIT;
                    if (FormatPlaneCount(format) > 1) {
                        input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_0_BIT;
                        input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_1_BIT;
                    }
                    if (FormatPlaneCount(format) > 2) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_2_BIT;
                }
            }
        }
    }
    // translate VkRenderPassInputAttachmentAspectCreateInfo
    if (input_attachment_aspect_info && input_attachment_aspect_info->pAspectReferences) {
        for (uint32_t i = 0; i < input_attachment_aspect_info->aspectReferenceCount; ++i) {
            const uint32_t subpass = input_attachment_aspect_info->pAspectReferences[i].subpass;
            const uint32_t input_attachment = input_attachment_aspect_info->pAspectReferences[i].inputAttachmentIndex;
            const VkImageAspectFlags aspectMask = input_attachment_aspect_info->pAspectReferences[i].aspectMask;

            if (subpass < input_attachment_aspect_masks.size() &&
                input_attachment < input_attachment_aspect_masks[subpass].size()) {
                input_attachment_aspect_masks[subpass][input_attachment] = aspectMask;
            }
        }
    }

    const bool has_viewMask = multiview_info && multiview_info->subpassCount && multiview_info->pViewMasks;
    if (out_struct->subpassCount && in_struct.pSubpasses) {
        out_struct->pSubpasses = new safe_VkSubpassDescription2KHR[out_struct->subpassCount];
        for (uint32_t i = 0; i < out_struct->subpassCount; ++i) {
            const uint32_t viewMask = has_viewMask ? multiview_info->pViewMasks[i] : 0;
            out_struct->pSubpasses[i] = ToV2KHR(in_struct.pSubpasses[i], viewMask, input_attachment_aspect_masks[i].data());
        }
    }

    const bool has_viewOffset = multiview_info && multiview_info->dependencyCount && multiview_info->pViewOffsets;
    if (out_struct->dependencyCount && in_struct.pDependencies) {
        out_struct->pDependencies = new safe_VkSubpassDependency2KHR[out_struct->dependencyCount];
        for (uint32_t i = 0; i < out_struct->dependencyCount; ++i) {
            const int32_t viewOffset = has_viewOffset ? multiview_info->pViewOffsets[i] : 0;
            out_struct->pDependencies[i] = ToV2KHR(in_struct.pDependencies[i], viewOffset);
        }
    }

    if (out_struct->correlatedViewMaskCount && multiview_info->pCorrelationMasks) {
        auto correlated_view_masks = new uint32_t[out_struct->correlatedViewMaskCount];
        for (uint32_t i = 0; i < out_struct->correlatedViewMaskCount; ++i) {
            correlated_view_masks[i] = multiview_info->pCorrelationMasks[i];
        }
        out_struct->pCorrelatedViewMasks = correlated_view_masks;
    }
}
