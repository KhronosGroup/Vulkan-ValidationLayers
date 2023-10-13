/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

static inline bool RenderPassUsesAttachmentAsResolve(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpass_info = createInfo.pSubpasses[subpass];
        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
            }
        }
    }

    return false;
}

static inline bool RenderPassUsesAttachmentOnTile(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpass_info = createInfo.pSubpasses[subpass];

        // If an attachment is ever used as a color attachment,
        // resolve attachment or depth stencil attachment,
        // it needs to exist on tile at some point.

        for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
            if (subpass_info.pColorAttachments[i].attachment == attachment) return true;
        }

        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
            }
        }

        if (subpass_info.pDepthStencilAttachment && subpass_info.pDepthStencilAttachment->attachment == attachment) return true;
    }

    return false;
}

static inline bool RenderPassUsesAttachmentAsImageOnly(const safe_VkRenderPassCreateInfo2& createInfo, uint32_t attachment) {
    if (RenderPassUsesAttachmentOnTile(createInfo, attachment)) {
        return false;
    }

    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; subpass++) {
        const auto& subpassInfo = createInfo.pSubpasses[subpass];

        for (uint32_t i = 0; i < subpassInfo.inputAttachmentCount; i++) {
            if (subpassInfo.pInputAttachments[i].attachment == attachment) {
                return true;
            }
        }
    }

    return false;
}

bool BestPractices::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((vkuFormatIsColor(format) || vkuFormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(kVUID_BestPractices_RenderPass_Attatchment, device, error_obj.location,
                                   "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                   "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
            if (vkuFormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning(kVUID_BestPractices_RenderPass_Attatchment, device, error_obj.location,
                                   "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD "
                                   "and initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.");
            }
        }

        const auto& attachment = pCreateInfo->pAttachments[i];
        if (attachment.samples > VK_SAMPLE_COUNT_1_BIT) {
            bool access_requires_memory =
                attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE;

            if (vkuFormatHasStencil(format)) {
                access_requires_memory |= attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
                                          attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE;
            }

            if (access_requires_memory) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory, device, error_obj.location,
                    "Attachment %u in the VkRenderPass is a multisampled image with %u samples, but it uses loadOp/storeOp "
                    "which requires accessing data from memory. Multisampled images should always be loadOp = CLEAR or DONT_CARE, "
                    "storeOp = DONT_CARE. This allows the implementation to use lazily allocated memory effectively.",
                    i, static_cast<uint32_t>(attachment.samples));
            }
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)) {
        for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
            if (pCreateInfo->pSubpasses[i].pResolveAttachments) {
                for (uint32_t j = 0; j < pCreateInfo->pSubpasses[i].colorAttachmentCount; ++j) {
                    const auto attachment = pCreateInfo->pSubpasses[i].pResolveAttachments[j].attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        const auto format = pCreateInfo->pAttachments[attachment].format;
                        VkSubpassResolvePerformanceQueryEXT performance_query = vku::InitStructHelper();
                        VkFormatProperties2 format_properties2 = vku::InitStructHelper(&performance_query);
                        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties2);
                        if (performance_query.optimal == VK_FALSE) {
                            skip |= LogPerformanceWarning(
                                kVUID_BestPractices_SubpassResolve_NonOptimalFormat, device, error_obj.location,
                                "Attachment %" PRIu32
                                " in the VkRenderPass has the format %s and is used as a resolve attachment, "
                                "but VkSubpassResolvePerformanceQueryEXT::optimal is VK_FALSE.",
                                i, string_VkFormat(format));
                        }
                    }
                }
            }
        }
    }

    for (uint32_t dependency = 0; dependency < pCreateInfo->dependencyCount; dependency++) {
        const Location dependency_loc = create_info_loc.dot(Field::pDependencies, dependency);
        skip |=
            CheckPipelineStageFlags(dependency_loc.dot(Field::srcStageMask), pCreateInfo->pDependencies[dependency].srcStageMask);
        skip |=
            CheckPipelineStageFlags(dependency_loc.dot(Field::dstStageMask), pCreateInfo->pDependencies[dependency].dstStageMask);
    }

    return skip;
}

bool BestPractices::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const Location& loc) const {
    bool skip = false;

    if (!pRenderPassBegin) {
        return skip;
    }

    if (pRenderPassBegin->renderArea.extent.width == 0 || pRenderPassBegin->renderArea.extent.height == 0) {
        skip |= LogWarning(kVUID_BestPractices_BeginRenderPass_ZeroSizeRenderArea, device, loc,
                           "This render pass has a zero-size render area. It cannot write to any attachments, "
                           "and can only be used for side effects such as layout transitions.");
    }

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (rp_state) {
        if (rp_state->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
            const VkRenderPassAttachmentBeginInfo* rpabi = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
            if (rpabi) {
                skip = ValidateAttachments(rp_state->createInfo.ptr(), rpabi->attachmentCount, rpabi->pAttachments, loc);
            }
        }
        // Check if any attachments have LOAD operation on them
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            bool attachment_has_readback = false;
            if (!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachment_has_readback = true;
            }

            if (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                attachment_has_readback = true;
            }

            bool attachment_needs_readback = false;

            // Check if the attachment is actually used in any subpass on-tile
            if (attachment_has_readback && RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                attachment_needs_readback = true;
            }

            // Using LOAD_OP_LOAD is expensive on tiled GPUs, so flag it as a potential improvement
            if (attachment_needs_readback && (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
                skip |=
                    LogPerformanceWarning(kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback, device, loc,
                                          "%s %s: Attachment #%u in render pass has begun with VK_ATTACHMENT_LOAD_OP_LOAD.\n"
                                          "Submitting this renderpass will cause the driver to inject a readback of the attachment "
                                          "which will copy in total %u pixels (renderArea = "
                                          "{ %" PRId32 ", %" PRId32 ", %" PRIu32 ", %" PRIu32 " }) to the tile buffer.",
                                          VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), att,
                                          pRenderPassBegin->renderArea.extent.width * pRenderPassBegin->renderArea.extent.height,
                                          pRenderPassBegin->renderArea.offset.x, pRenderPassBegin->renderArea.offset.y,
                                          pRenderPassBegin->renderArea.extent.width, pRenderPassBegin->renderArea.extent.height);
            }
        }

        // Check if renderpass has at least one VK_ATTACHMENT_LOAD_OP_CLEAR

        bool clearing = false;

        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                clearing = true;
                break;
            }
        }

        // Check if there are ClearValues passed to BeginRenderPass even though no attachments will be cleared
        if (!clearing && pRenderPassBegin->clearValueCount > 0) {
            // Flag as warning because nothing will happen per spec, and pClearValues will be ignored
            skip |= LogWarning(
                kVUID_BestPractices_ClearValueWithoutLoadOpClear, device, loc,
                "This render pass does not have VkRenderPassCreateInfo.pAttachments->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR "
                "but VkRenderPassBeginInfo.clearValueCount > 0. VkRenderPassBeginInfo.pClearValues will be ignored and no "
                "attachments will be cleared.");
        }

        // Check if there are more clearValues than attachments
        if (pRenderPassBegin->clearValueCount > rp_state->createInfo.attachmentCount) {
            // Flag as warning because the overflowing clearValues will be ignored and could even be undefined on certain platforms.
            // This could signal a bug and there seems to be no reason for this to happen on purpose.
            skip |=
                LogWarning(kVUID_BestPractices_ClearValueCountHigherThanAttachmentCount, device, loc,
                           "This render pass has VkRenderPassBeginInfo.clearValueCount > VkRenderPassCreateInfo.attachmentCount "
                           "(%" PRIu32 " > %" PRIu32
                           ") and as such the clearValues that do not have a corresponding attachment will be ignored.",
                           pRenderPassBegin->clearValueCount, rp_state->createInfo.attachmentCount);
        }

        if (VendorCheckEnabled(kBPVendorNVIDIA) && rp_state->createInfo.pAttachments) {
            for (uint32_t i = 0; i < pRenderPassBegin->clearValueCount; ++i) {
                const auto& attachment = rp_state->createInfo.pAttachments[i];
                if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    const auto& clear_color = pRenderPassBegin->pClearValues[i].color;
                    skip |= ValidateClearColor(commandBuffer, attachment.format, clear_color, loc);
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                              const Location& loc) const {
    bool skip = false;

    auto cmd_state = Get<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
            const auto& color_attachment = pRenderingInfo->pColorAttachments[i];
            if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                const VkFormat format = Get<IMAGE_VIEW_STATE>(color_attachment.imageView)->create_info.format;
                skip |= ValidateClearColor(commandBuffer, format, color_attachment.clearValue.color, loc);
            }
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderPass(commandBuffer);
    auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (cb_node) {
        AddDeferredQueueOperations(*cb_node);
    }
}

void BestPractices::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassInfo) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassInfo);
    auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    if (cb_node) {
        AddDeferredQueueOperations(*cb_node);
    }
}

void BestPractices::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassInfo) {
    PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassInfo);
}

void BestPractices::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingCommon(commandBuffer);

    ValidationStateTracker::PreCallRecordCmdEndRendering(commandBuffer);
}

void BestPractices::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) { PreCallRecordCmdEndRendering(commandBuffer); }

void BestPractices::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                    VkSubpassContents contents) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderingCommon(commandBuffer);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     const VkSubpassBeginInfo* pSubpassBeginInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkRenderPassBeginInfo* pRenderPassBegin,
                                                        const VkSubpassBeginInfo* pSubpassBeginInfo) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void BestPractices::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    ValidationStateTracker::PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo);
    RecordCmdBeginRenderingCommon(commandBuffer);
}

void BestPractices::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo);
}

void BestPractices::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);

    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        IMAGE_VIEW_STATE* depth_image_view = nullptr;

        const auto depth_attachment = rp->createInfo.pSubpasses[cmd_state->GetActiveSubpass()].pDepthStencilAttachment;
        if (depth_attachment) {
            const uint32_t attachment_index = depth_attachment->attachment;
            if (attachment_index != VK_ATTACHMENT_UNUSED) {
                depth_image_view = (*cmd_state->active_attachments)[attachment_index];
            }
        }
        if (depth_image_view && (depth_image_view->create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->image();
            const VkImageSubresourceRange& subresource_range = depth_image_view->create_info.subresourceRange;
            RecordBindZcullScope(*cmd_state, depth_image, subresource_range);
        } else {
            RecordUnbindZcullScope(*cmd_state);
        }
    }
}

void BestPractices::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin) {
    if (!pRenderPassBegin) {
        return;
    }

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (rp_state) {
        // Check load ops
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (!RenderPassUsesAttachmentAsImageOnly(rp_state->createInfo, att) &&
                !RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                continue;
            }

            // If renderpass doesn't load attachment, no need to validate image in queue
            if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT) ||
                (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT)) {
                continue;
            }

            IMAGE_SUBRESOURCE_USAGE_BP usage = IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED;

            if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ||
                (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE;
            } else if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ||
                       (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED;
            } else if (RenderPassUsesAttachmentAsImageOnly(rp_state->createInfo, att)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS;
            }

            auto framebuffer = Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
            std::shared_ptr<IMAGE_VIEW_STATE> image_view = nullptr;

            if (framebuffer->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
                if (rpabi) {
                    image_view = Get<IMAGE_VIEW_STATE>(rpabi->pAttachments[att]);
                }
            } else {
                image_view = Get<IMAGE_VIEW_STATE>(framebuffer->createInfo.pAttachments[att]);
            }

            QueueValidateImageView(cb->queue_submit_functions, Func::vkCmdBeginRenderPass, image_view.get(), usage);
        }

        // Check store ops
        for (uint32_t att = 0; att < rp_state->createInfo.attachmentCount; att++) {
            const auto& attachment = rp_state->createInfo.pAttachments[att];

            if (!RenderPassUsesAttachmentOnTile(rp_state->createInfo, att)) {
                continue;
            }

            // If renderpass doesn't store attachment, no need to validate image in queue
            if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.storeOp == VK_ATTACHMENT_STORE_OP_NONE) ||
                (vkuFormatHasStencil(attachment.format) && attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_NONE)) {
                continue;
            }

            IMAGE_SUBRESOURCE_USAGE_BP usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_DISCARDED;

            if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE) ||
                (vkuFormatHasStencil(attachment.format) && attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_STORED;
            }

            auto framebuffer = Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);

            std::shared_ptr<IMAGE_VIEW_STATE> image_view;
            if (framebuffer->createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
                if (rpabi) {
                    image_view = Get<IMAGE_VIEW_STATE>(rpabi->pAttachments[att]);
                }
            } else {
                image_view = Get<IMAGE_VIEW_STATE>(framebuffer->createInfo.pAttachments[att]);
            }

            QueueValidateImageView(cb->queue_submit_functions_after_render_pass, Func::vkCmdEndRenderPass, image_view.get(), usage);
        }
    }
}

void BestPractices::RecordCmdBeginRenderingCommon(VkCommandBuffer commandBuffer) {
    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::shared_ptr<IMAGE_VIEW_STATE> depth_image_view_shared_ptr;
        IMAGE_VIEW_STATE* depth_image_view = nullptr;
        std::optional<VkAttachmentLoadOp> load_op;

        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto depth_attachment = rp->dynamic_rendering_begin_rendering_info.pDepthAttachment;
            if (depth_attachment) {
                load_op.emplace(depth_attachment->loadOp);
                depth_image_view_shared_ptr = Get<IMAGE_VIEW_STATE>(depth_attachment->imageView);
                depth_image_view = depth_image_view_shared_ptr.get();
            }

            for (uint32_t i = 0; i < rp->dynamic_rendering_begin_rendering_info.colorAttachmentCount; ++i) {
                const auto& color_attachment = rp->dynamic_rendering_begin_rendering_info.pColorAttachments[i];
                if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    const VkFormat format = Get<IMAGE_VIEW_STATE>(color_attachment.imageView)->create_info.format;
                    RecordClearColor(format, color_attachment.clearValue.color);
                }
            }

        } else {
            if (rp->createInfo.pAttachments) {
                if (rp->createInfo.subpassCount > 0) {
                    const auto depth_attachment = rp->createInfo.pSubpasses[0].pDepthStencilAttachment;
                    if (depth_attachment) {
                        const uint32_t attachment_index = depth_attachment->attachment;
                        if (attachment_index != VK_ATTACHMENT_UNUSED) {
                            load_op.emplace(rp->createInfo.pAttachments[attachment_index].loadOp);
                            depth_image_view = (*cmd_state->active_attachments)[attachment_index];
                        }
                    }
                }
                for (uint32_t i = 0; i < cmd_state->active_render_pass_begin_info.clearValueCount; ++i) {
                    const auto& attachment = rp->createInfo.pAttachments[i];
                    if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                        const auto& clear_color = cmd_state->active_render_pass_begin_info.pClearValues[i].color;
                        RecordClearColor(attachment.format, clear_color);
                    }
                }
            }
        }
        if (depth_image_view && (depth_image_view->create_info.subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->image();
            const VkImageSubresourceRange& subresource_range = depth_image_view->create_info.subresourceRange;
            RecordBindZcullScope(*cmd_state, depth_image, subresource_range);
        } else {
            RecordUnbindZcullScope(*cmd_state);
        }
        if (load_op) {
            if (*load_op == VK_ATTACHMENT_LOAD_OP_CLEAR || *load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
                RecordResetScopeZcullDirection(*cmd_state);
            }
        }
    }
}

void BestPractices::RecordCmdEndRenderingCommon(VkCommandBuffer commandBuffer) {
    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    auto rp = cmd_state->activeRenderPass.get();
    assert(rp);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::optional<VkAttachmentStoreOp> store_op;

        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto depth_attachment = rp->dynamic_rendering_begin_rendering_info.pDepthAttachment;
            if (depth_attachment) {
                store_op.emplace(depth_attachment->storeOp);
            }
        } else {
            if (rp->createInfo.subpassCount > 0) {
                const uint32_t last_subpass = rp->createInfo.subpassCount - 1;
                const auto depth_attachment = rp->createInfo.pSubpasses[last_subpass].pDepthStencilAttachment;
                if (depth_attachment) {
                    const uint32_t attachment = depth_attachment->attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        store_op.emplace(rp->createInfo.pAttachments[attachment].storeOp);
                    }
                }
            }
        }

        if (store_op) {
            if (*store_op == VK_ATTACHMENT_STORE_OP_DONT_CARE || *store_op == VK_ATTACHMENT_STORE_OP_NONE) {
                RecordResetScopeZcullDirection(*cmd_state);
            }
        }

        RecordUnbindZcullScope(*cmd_state);
    }
}

bool BestPractices::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject& error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, error_obj);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj.location);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo* pRenderPassBegin,
                                                          const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                          const ErrorObject& error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    skip |= ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj.location);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
    skip |= ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj.location);
    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                        const ErrorObject& error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

void BestPractices::PostRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin) {
    // Reset the renderpass state
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    // TODO - move this logic to the Render Pass state as cb->has_draw_cmd should stay true for lifetime of command buffer
    cb->has_draw_cmd = false;
    assert(cb);
    auto& render_pass_state = cb->render_pass_state;
    render_pass_state.touchesAttachments.clear();
    render_pass_state.earlyClearAttachments.clear();
    render_pass_state.numDrawCallsDepthOnly = 0;
    render_pass_state.numDrawCallsDepthEqualCompare = 0;
    render_pass_state.colorAttachment = false;
    render_pass_state.depthAttachment = false;
    render_pass_state.drawTouchAttachments = true;
    // Don't reset state related to pipeline state.

    // Reset NV state
    cb->nv = {};

    auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);

    // track depth / color attachment usage within the renderpass
    for (size_t i = 0; i < rp_state->createInfo.subpassCount; i++) {
        // record if depth/color attachments are in use for this renderpass
        if (rp_state->createInfo.pSubpasses[i].pDepthStencilAttachment != nullptr) render_pass_state.depthAttachment = true;

        if (rp_state->createInfo.pSubpasses[i].colorAttachmentCount > 0) render_pass_state.colorAttachment = true;
    }
}

void BestPractices::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                     VkSubpassContents contents, const RecordObject& record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    PostRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    PostRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin);
}

void BestPractices::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo* pRenderPassBegin,
                                                         const VkSubpassBeginInfo* pSubpassBeginInfo,
                                                         const RecordObject& record_obj) {
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

bool BestPractices::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, error_obj.location);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                        const ErrorObject& error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, error_obj.location);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= StateTracker::PreCallValidateCmdEndRendering(commandBuffer, error_obj);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
        assert(cmd_state);
        skip |= ValidateZcullScope(*cmd_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool BestPractices::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const Location& loc) const {
    bool skip = false;
    const auto cmd = GetRead<bp_state::CommandBuffer>(commandBuffer);

    if (cmd == nullptr) return skip;
    auto& render_pass_state = cmd->render_pass_state;

    // Does the number of draw calls classified as depth only surpass the vendor limit for a specified vendor
    const bool depth_only_arm = render_pass_state.numDrawCallsDepthEqualCompare >= kDepthPrePassNumDrawCallsArm &&
                                render_pass_state.numDrawCallsDepthOnly >= kDepthPrePassNumDrawCallsArm;
    const bool depth_only_img = render_pass_state.numDrawCallsDepthEqualCompare >= kDepthPrePassNumDrawCallsIMG &&
                                render_pass_state.numDrawCallsDepthOnly >= kDepthPrePassNumDrawCallsIMG;

    // Only send the warning when the vendor is enabled and a depth prepass is detected
    bool uses_depth =
        (render_pass_state.depthAttachment || render_pass_state.colorAttachment) &&
        ((depth_only_arm && VendorCheckEnabled(kBPVendorArm)) || (depth_only_img && VendorCheckEnabled(kBPVendorIMG)));

    if (uses_depth) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_EndRenderPass_DepthPrePassUsage, device, loc,
            "%s %s: Depth pre-passes may be in use. In general, this is not recommended in tile-based deferred "
            "renderering architectures; such as those in Arm Mali or PowerVR GPUs. Since they can remove geometry "
            "hidden by other opaque geometry. Mali has Forward Pixel Killing (FPK), PowerVR has Hiden Surface "
            "Remover (HSR) in which case, using depth pre-passes for hidden surface removal may worsen performance.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
    }

    RENDER_PASS_STATE* rp = cmd->activeRenderPass.get();

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) && rp) {
        // If we use an attachment on-tile, we should access it in some way. Otherwise,
        // it is redundant to have it be part of the render pass.
        // Only consider it redundant if it will actually consume bandwidth, i.e.
        // LOAD_OP_LOAD is used or STORE_OP_STORE. CLEAR -> DONT_CARE is benign,
        // as is using pure input attachments.
        // CLEAR -> STORE might be considered a "useful" thing to do, but
        // the optimal thing to do is to defer the clear until you're actually
        // going to render to the image.

        uint32_t num_attachments = rp->createInfo.attachmentCount;
        for (uint32_t i = 0; i < num_attachments; i++) {
            if (!RenderPassUsesAttachmentOnTile(rp->createInfo, i) || RenderPassUsesAttachmentAsResolve(rp->createInfo, i)) {
                continue;
            }

            auto& attachment = rp->createInfo.pAttachments[i];

            VkImageAspectFlags bandwidth_aspects = 0;

            if (!vkuFormatIsStencilOnly(attachment.format) &&
                (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                if (vkuFormatHasDepth(attachment.format)) {
                    bandwidth_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
                } else {
                    bandwidth_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
                }
            }

            if (vkuFormatHasStencil(attachment.format) && (attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ||
                                                        attachment.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)) {
                bandwidth_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            if (!bandwidth_aspects) {
                continue;
            }

            auto itr = std::find_if(render_pass_state.touchesAttachments.begin(), render_pass_state.touchesAttachments.end(),
                                    [i](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == i; });
            uint32_t untouched_aspects = bandwidth_aspects;
            if (itr != render_pass_state.touchesAttachments.end()) {
                untouched_aspects &= ~itr->aspects;
            }

            if (untouched_aspects) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_EndRenderPass_RedundantAttachmentOnTile, device, loc,
                    "%s %s: Render pass was ended, but attachment #%u (format: %u, untouched aspects 0x%x) "
                    "was never accessed by a pipeline or clear command. "
                    "On tile-based architectures, LOAD_OP_LOAD and STORE_OP_STORE consume bandwidth and should not be part of the "
                    "render pass if the attachments are not intended to be accessed.",
                    VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), i, attachment.format, untouched_aspects);
            }
        }
    }

    return skip;
}
