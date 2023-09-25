/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "convert_utils.h"

#include <vector>

#include <vulkan/utility/vk_format_utils.h>

#include <vulkan/utility/vk_struct_helper.hpp>

static safe_VkAttachmentDescription2 ToV2KHR(const VkAttachmentDescription& in_struct) {
    safe_VkAttachmentDescription2 v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
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

static safe_VkAttachmentReference2 ToV2KHR(const VkAttachmentReference& in_struct, const VkImageAspectFlags aspectMask = 0) {
    safe_VkAttachmentReference2 v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    v2.pNext = nullptr;
    v2.attachment = in_struct.attachment;
    v2.layout = in_struct.layout;
    v2.aspectMask = aspectMask;

    return v2;
}

static safe_VkSubpassDescription2 ToV2KHR(const VkSubpassDescription& in_struct, const uint32_t viewMask,
                                          const VkImageAspectFlags* input_attachment_aspect_masks) {
    safe_VkSubpassDescription2 v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
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
        v2.pInputAttachments = new safe_VkAttachmentReference2[v2.inputAttachmentCount];
        for (uint32_t i = 0; i < v2.inputAttachmentCount; ++i) {
            v2.pInputAttachments[i] = ToV2KHR(in_struct.pInputAttachments[i], input_attachment_aspect_masks[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pColorAttachments) {
        v2.pColorAttachments = new safe_VkAttachmentReference2[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pColorAttachments[i] = ToV2KHR(in_struct.pColorAttachments[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pResolveAttachments) {
        v2.pResolveAttachments = new safe_VkAttachmentReference2[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pResolveAttachments[i] = ToV2KHR(in_struct.pResolveAttachments[i]);
        }
    }
    if (in_struct.pDepthStencilAttachment) {
        v2.pDepthStencilAttachment = new safe_VkAttachmentReference2();
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

static safe_VkSubpassDependency2 ToV2KHR(const VkSubpassDependency& in_struct, int32_t viewOffset = 0) {
    safe_VkSubpassDependency2 v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
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

safe_VkRenderPassCreateInfo2 ConvertVkRenderPassCreateInfoToV2KHR(const VkRenderPassCreateInfo& create_info) {
    safe_VkRenderPassCreateInfo2 out_struct;
    const auto multiview_info = vku::FindStructInPNextChain<VkRenderPassMultiviewCreateInfo>(create_info.pNext);
    const auto* input_attachment_aspect_info = vku::FindStructInPNextChain<VkRenderPassInputAttachmentAspectCreateInfo>(create_info.pNext);
    const auto fragment_density_map_info = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(create_info.pNext);

    out_struct.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;

    // Fixup RPCI2 pNext chain.  Only FDM2 is valid on both chains.
    if (fragment_density_map_info) {
        out_struct.pNext = SafePnextCopy(fragment_density_map_info);
        auto base_struct = reinterpret_cast<const VkBaseOutStructure*>(out_struct.pNext);
        const_cast<VkBaseOutStructure*>(base_struct)->pNext = nullptr;
    } else {
        out_struct.pNext = nullptr;
    }

    out_struct.flags = create_info.flags;
    out_struct.attachmentCount = create_info.attachmentCount;
    out_struct.pAttachments = nullptr;  // to be filled
    out_struct.subpassCount = create_info.subpassCount;
    out_struct.pSubpasses = nullptr;  // to be filled
    out_struct.dependencyCount = create_info.dependencyCount;
    out_struct.pDependencies = nullptr;  // to be filled
    out_struct.correlatedViewMaskCount = multiview_info ? multiview_info->correlationMaskCount : 0;
    out_struct.pCorrelatedViewMasks = nullptr;  // to be filled

    // TODO: This should support VkRenderPassFragmentDensityMapCreateInfoEXT somehow
    // see https://github.com/KhronosGroup/Vulkan-Docs/issues/1027

    if (out_struct.attachmentCount && create_info.pAttachments) {
        out_struct.pAttachments = new safe_VkAttachmentDescription2[out_struct.attachmentCount];
        for (uint32_t i = 0; i < out_struct.attachmentCount; ++i) {
            out_struct.pAttachments[i] = ToV2KHR(create_info.pAttachments[i]);
        }
    }

    // translate VkRenderPassInputAttachmentAspectCreateInfo into vector
    std::vector<std::vector<VkImageAspectFlags>> input_attachment_aspect_masks(out_struct.subpassCount);
    // set defaults
    for (uint32_t si = 0; si < out_struct.subpassCount; ++si) {
        if (create_info.pSubpasses) {
            input_attachment_aspect_masks[si].resize(create_info.pSubpasses[si].inputAttachmentCount, 0);

            for (uint32_t iai = 0; iai < create_info.pSubpasses[si].inputAttachmentCount; ++iai) {
                if (out_struct.pAttachments && create_info.pSubpasses[si].pInputAttachments) {
                    const auto& input_attachment = create_info.pSubpasses[si].pInputAttachments[iai];
                    if (input_attachment.attachment != VK_ATTACHMENT_UNUSED) {
                        const auto format = out_struct.pAttachments[input_attachment.attachment].format;

                        if (vkuFormatIsColor(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_COLOR_BIT;
                        if (vkuFormatHasDepth(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_DEPTH_BIT;
                        if (vkuFormatHasStencil(format)) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_STENCIL_BIT;
                        if (vkuFormatPlaneCount(format) > 1) {
                            input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_0_BIT;
                            input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_1_BIT;
                        }
                        if (vkuFormatPlaneCount(format) > 2) input_attachment_aspect_masks[si][iai] |= VK_IMAGE_ASPECT_PLANE_2_BIT;
                    }
                }
            }
        }
    }
    // translate VkRenderPassInputAttachmentAspectCreateInfo
    if (input_attachment_aspect_info && input_attachment_aspect_info->pAspectReferences) {
        for (uint32_t i = 0; i < input_attachment_aspect_info->aspectReferenceCount; ++i) {
            const uint32_t subpass = input_attachment_aspect_info->pAspectReferences[i].subpass;
            const uint32_t input_attachment = input_attachment_aspect_info->pAspectReferences[i].inputAttachmentIndex;
            const VkImageAspectFlags aspect_mask = input_attachment_aspect_info->pAspectReferences[i].aspectMask;

            if (subpass < input_attachment_aspect_masks.size() &&
                input_attachment < input_attachment_aspect_masks[subpass].size()) {
                input_attachment_aspect_masks[subpass][input_attachment] = aspect_mask;
            }
        }
    }

    const bool has_view_mask = multiview_info && multiview_info->subpassCount && multiview_info->pViewMasks;
    if (out_struct.subpassCount && create_info.pSubpasses) {
        out_struct.pSubpasses = new safe_VkSubpassDescription2[out_struct.subpassCount];
        for (uint32_t i = 0; i < out_struct.subpassCount; ++i) {
            const uint32_t view_mask = has_view_mask ? multiview_info->pViewMasks[i] : 0;
            out_struct.pSubpasses[i] = ToV2KHR(create_info.pSubpasses[i], view_mask, input_attachment_aspect_masks[i].data());
        }
    }

    const bool has_view_offset = multiview_info && multiview_info->dependencyCount && multiview_info->pViewOffsets;
    if (out_struct.dependencyCount && create_info.pDependencies) {
        out_struct.pDependencies = new safe_VkSubpassDependency2[out_struct.dependencyCount];
        for (uint32_t i = 0; i < out_struct.dependencyCount; ++i) {
            const int32_t view_offset = has_view_offset ? multiview_info->pViewOffsets[i] : 0;
            out_struct.pDependencies[i] = ToV2KHR(create_info.pDependencies[i], view_offset);
        }
    }

    if (out_struct.correlatedViewMaskCount && multiview_info->pCorrelationMasks) {
        auto correlated_view_masks = new uint32_t[out_struct.correlatedViewMaskCount];
        for (uint32_t i = 0; i < out_struct.correlatedViewMaskCount; ++i) {
            correlated_view_masks[i] = multiview_info->pCorrelationMasks[i];
        }
        out_struct.pCorrelatedViewMasks = correlated_view_masks;
    }
    return out_struct;
}

safe_VkImageMemoryBarrier2 ConvertVkImageMemoryBarrierToV2(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags2 srcStageMask,
                                                           VkPipelineStageFlags2 dstStageMask) {
    VkImageMemoryBarrier2 barrier2 = vku::InitStructHelper();

    // As of Vulkan 1.3.153, the VkImageMemoryBarrier2 supports the same pNext structs as VkImageMemoryBarrier
    // (VkExternalMemoryAcquireUnmodifiedEXT and VkSampleLocationsInfoEXT). It means we can copy entire pNext
    // chain (as part of safe_VkImageMemoryBarrier2 constructor) without analyzing which pNext structures are supported in V2.
    barrier2.pNext = barrier.pNext;

    barrier2.srcStageMask = srcStageMask;
    barrier2.srcAccessMask = barrier.srcAccessMask;
    barrier2.dstStageMask = dstStageMask;
    barrier2.dstAccessMask = barrier.dstAccessMask;
    barrier2.oldLayout = barrier.oldLayout;
    barrier2.newLayout = barrier.newLayout;
    barrier2.srcQueueFamilyIndex = barrier.srcQueueFamilyIndex;
    barrier2.dstQueueFamilyIndex = barrier.dstQueueFamilyIndex;
    barrier2.image = barrier.image;
    barrier2.subresourceRange = barrier.subresourceRange;
    return safe_VkImageMemoryBarrier2(&barrier2);
}
