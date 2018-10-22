/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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

#include <string.h>

#include "convert_to_renderpass2.h"
#include "vk_typemap_helper.h"
#include "vk_format_utils.h"

static void ConvertVkAttachmentReferenceToV2KHR(const VkAttachmentReference* in_struct,
                                                safe_VkAttachmentReference2KHR* out_struct) {
    out_struct->sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->attachment = in_struct->attachment;
    out_struct->layout = in_struct->layout;
    out_struct->aspectMask =
        VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;  // Uninitialized - must be filled in by top level struct for input attachments
}

static void ConvertVkSubpassDependencyToV2KHR(const VkSubpassDependency* in_struct, safe_VkSubpassDependency2KHR* out_struct) {
    out_struct->sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->srcSubpass = in_struct->srcSubpass;
    out_struct->dstSubpass = in_struct->dstSubpass;
    out_struct->srcStageMask = in_struct->srcStageMask;
    out_struct->dstStageMask = in_struct->dstStageMask;
    out_struct->srcAccessMask = in_struct->srcAccessMask;
    out_struct->dstAccessMask = in_struct->dstAccessMask;
    out_struct->dependencyFlags = in_struct->dependencyFlags;
    out_struct->viewOffset = 0;
}

static void ConvertVkSubpassDescriptionToV2KHR(const VkSubpassDescription* in_struct, safe_VkSubpassDescription2KHR* out_struct) {
    out_struct->sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->flags = in_struct->flags;
    out_struct->pipelineBindPoint = in_struct->pipelineBindPoint;
    out_struct->viewMask = 0;
    out_struct->inputAttachmentCount = in_struct->inputAttachmentCount;
    out_struct->pInputAttachments = nullptr;
    out_struct->colorAttachmentCount = in_struct->colorAttachmentCount;
    out_struct->pColorAttachments = nullptr;
    out_struct->pResolveAttachments = nullptr;
    out_struct->preserveAttachmentCount = in_struct->preserveAttachmentCount;
    out_struct->pPreserveAttachments = nullptr;

    if (out_struct->inputAttachmentCount && in_struct->pInputAttachments) {
        out_struct->pInputAttachments = new safe_VkAttachmentReference2KHR[out_struct->inputAttachmentCount];
        for (uint32_t i = 0; i < out_struct->inputAttachmentCount; ++i) {
            ConvertVkAttachmentReferenceToV2KHR(&in_struct->pInputAttachments[i], &out_struct->pInputAttachments[i]);
        }
    }
    if (out_struct->colorAttachmentCount && in_struct->pColorAttachments) {
        out_struct->pColorAttachments = new safe_VkAttachmentReference2KHR[out_struct->colorAttachmentCount];
        for (uint32_t i = 0; i < out_struct->colorAttachmentCount; ++i) {
            ConvertVkAttachmentReferenceToV2KHR(&in_struct->pColorAttachments[i], &out_struct->pColorAttachments[i]);
        }
    }
    if (out_struct->colorAttachmentCount && in_struct->pResolveAttachments) {
        out_struct->pResolveAttachments = new safe_VkAttachmentReference2KHR[out_struct->colorAttachmentCount];
        for (uint32_t i = 0; i < out_struct->colorAttachmentCount; ++i) {
            ConvertVkAttachmentReferenceToV2KHR(&in_struct->pResolveAttachments[i], &out_struct->pResolveAttachments[i]);
        }
    }
    if (in_struct->pDepthStencilAttachment) {
        out_struct->pDepthStencilAttachment = new safe_VkAttachmentReference2KHR();
        ConvertVkAttachmentReferenceToV2KHR(in_struct->pDepthStencilAttachment, out_struct->pDepthStencilAttachment);
    } else {
        out_struct->pDepthStencilAttachment = NULL;
    }
    if (in_struct->pPreserveAttachments) {
        out_struct->pPreserveAttachments = new uint32_t[in_struct->preserveAttachmentCount];
        memcpy((void*)out_struct->pPreserveAttachments, (void*)in_struct->pPreserveAttachments,
               sizeof(uint32_t) * in_struct->preserveAttachmentCount);
    }
}

static void ConvertVkAttachmentDescriptionToV2KHR(const VkAttachmentDescription* in_struct,
                                                  safe_VkAttachmentDescription2KHR* out_struct) {
    out_struct->sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->flags = in_struct->flags;
    out_struct->format = in_struct->format;
    out_struct->samples = in_struct->samples;
    out_struct->loadOp = in_struct->loadOp;
    out_struct->storeOp = in_struct->storeOp;
    out_struct->stencilLoadOp = in_struct->stencilLoadOp;
    out_struct->stencilStoreOp = in_struct->stencilStoreOp;
    out_struct->initialLayout = in_struct->initialLayout;
    out_struct->finalLayout = in_struct->finalLayout;
}

void ConvertVkRenderPassCreateInfoToV2KHR(const VkRenderPassCreateInfo* in_struct, safe_VkRenderPassCreateInfo2KHR* out_struct) {
    out_struct->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
    out_struct->pNext = nullptr;
    out_struct->flags = in_struct->flags;
    out_struct->attachmentCount = in_struct->attachmentCount;
    out_struct->pAttachments = nullptr;
    out_struct->subpassCount = in_struct->subpassCount;
    out_struct->pSubpasses = nullptr;
    out_struct->dependencyCount = in_struct->dependencyCount;
    out_struct->pDependencies = nullptr;
    out_struct->correlatedViewMaskCount = 0;
    out_struct->pCorrelatedViewMasks = nullptr;
    if (out_struct->attachmentCount && in_struct->pAttachments) {
        out_struct->pAttachments = new safe_VkAttachmentDescription2KHR[out_struct->attachmentCount];
        for (uint32_t i = 0; i < out_struct->attachmentCount; ++i) {
            ConvertVkAttachmentDescriptionToV2KHR(&in_struct->pAttachments[i], &out_struct->pAttachments[i]);
        }
    }
    if (out_struct->subpassCount && in_struct->pSubpasses) {
        out_struct->pSubpasses = new safe_VkSubpassDescription2KHR[out_struct->subpassCount];
        for (uint32_t i = 0; i < out_struct->subpassCount; ++i) {
            ConvertVkSubpassDescriptionToV2KHR(&in_struct->pSubpasses[i], &out_struct->pSubpasses[i]);
        }
    }
    if (out_struct->dependencyCount && in_struct->pDependencies) {
        out_struct->pDependencies = new safe_VkSubpassDependency2KHR[out_struct->dependencyCount];
        for (uint32_t i = 0; i < out_struct->dependencyCount; ++i) {
            ConvertVkSubpassDependencyToV2KHR(&in_struct->pDependencies[i], &out_struct->pDependencies[i]);
        }
    }

    // Handle extension structs from KHR_multiview and KHR_maintenance2 to fill out the "filled in" bits.
    if (in_struct->pNext) {
        const VkRenderPassMultiviewCreateInfo* pMultiviewInfo =
            lvl_find_in_chain<VkRenderPassMultiviewCreateInfo>(in_struct->pNext);
        if (pMultiviewInfo) {
            for (uint32_t subpass = 0; subpass < pMultiviewInfo->subpassCount; ++subpass) {
                if (subpass < in_struct->subpassCount) {
                    out_struct->pSubpasses[subpass].viewMask = pMultiviewInfo->pViewMasks[subpass];
                }
            }
            for (uint32_t dependency = 0; dependency < pMultiviewInfo->dependencyCount; ++dependency) {
                if (dependency < in_struct->dependencyCount) {
                    out_struct->pDependencies[dependency].viewOffset = pMultiviewInfo->pViewOffsets[dependency];
                }
            }
            if (pMultiviewInfo->correlationMaskCount) {
                out_struct->correlatedViewMaskCount = pMultiviewInfo->correlationMaskCount;
                uint32_t* pCorrelatedViewMasks = new uint32_t[out_struct->correlatedViewMaskCount];
                for (uint32_t correlationMask = 0; correlationMask < pMultiviewInfo->correlationMaskCount; ++correlationMask) {
                    pCorrelatedViewMasks[correlationMask] = pMultiviewInfo->pCorrelationMasks[correlationMask];
                }
                out_struct->pCorrelatedViewMasks = pCorrelatedViewMasks;
            }
        }
        const VkRenderPassInputAttachmentAspectCreateInfo* pInputAttachmentAspectInfo =
            lvl_find_in_chain<VkRenderPassInputAttachmentAspectCreateInfo>(in_struct->pNext);
        if (pInputAttachmentAspectInfo) {
            for (uint32_t i = 0; i < pInputAttachmentAspectInfo->aspectReferenceCount; ++i) {
                uint32_t subpass = pInputAttachmentAspectInfo->pAspectReferences[i].subpass;
                uint32_t attachment = pInputAttachmentAspectInfo->pAspectReferences[i].inputAttachmentIndex;
                VkImageAspectFlags aspectMask = pInputAttachmentAspectInfo->pAspectReferences[i].aspectMask;
                if (subpass < in_struct->subpassCount && attachment < in_struct->pSubpasses[subpass].inputAttachmentCount) {
                    out_struct->pSubpasses[subpass].pInputAttachments[attachment].aspectMask = aspectMask;
                }
            }
        }
    }

    if (out_struct->subpassCount && out_struct->pSubpasses) {
        for (uint32_t i = 0; i < out_struct->subpassCount; ++i) {
            if (out_struct->pSubpasses[i].inputAttachmentCount && out_struct->pSubpasses[i].pInputAttachments) {
                for (uint32_t j = 0; j < out_struct->pSubpasses[i].inputAttachmentCount; ++j) {
                    safe_VkAttachmentReference2KHR& attachment_ref = out_struct->pSubpasses[i].pInputAttachments[j];
                    if (attachment_ref.aspectMask == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM &&
                        attachment_ref.attachment < out_struct->attachmentCount && out_struct->pAttachments) {
                        attachment_ref.aspectMask = 0;
                        VkFormat attachmentFormat = out_struct->pAttachments[attachment_ref.attachment].format;
                        if (FormatIsColor(attachmentFormat)) {
                            attachment_ref.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
                        }
                        if (FormatHasDepth(attachmentFormat)) {
                            attachment_ref.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
                        }
                        if (FormatHasStencil(attachmentFormat)) {
                            attachment_ref.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                        }
                    }
                }
            }
        }
    }
}
