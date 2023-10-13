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

void BestPractices::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment* pClearAttachments, uint32_t rectCount,
                                                     const VkClearRect* pRects) {
    ValidationStateTracker::PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pClearAttachments, rectCount, pRects);

    auto cmd_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto* rp_state = cmd_state->activeRenderPass.get();
    auto* fb_state = cmd_state->activeFramebuffer.get();
    const bool is_secondary = cmd_state->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    if (rectCount == 0 || !rp_state) {
        return;
    }

    if (!is_secondary && !fb_state && !rp_state->use_dynamic_rendering && !rp_state->use_dynamic_rendering_inherited) {
        return;
    }

    // If we have a rect which covers the entire frame buffer, we have a LOAD_OP_CLEAR-like command.
    const bool full_clear = ClearAttachmentsIsFullClear(*cmd_state, rectCount, pRects);

    if (rp_state->UsesDynamicRendering()) {
        if (VendorCheckEnabled(kBPVendorNVIDIA)) {
            auto pColorAttachments = rp_state->dynamic_rendering_begin_rendering_info.pColorAttachments;

            for (uint32_t i = 0; i < attachmentCount; i++) {
                auto& clear_attachment = pClearAttachments[i];

                if (clear_attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    RecordResetScopeZcullDirection(*cmd_state);
                }
                if ((clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                    clear_attachment.colorAttachment != VK_ATTACHMENT_UNUSED && pColorAttachments) {
                    const auto& attachment = pColorAttachments[clear_attachment.colorAttachment];
                    if (attachment.imageView) {
                        auto image_view_state = Get<IMAGE_VIEW_STATE>(attachment.imageView);
                        const VkFormat format = image_view_state->create_info.format;
                        RecordClearColor(format, clear_attachment.clearValue.color);
                    }
                }
            }
        }

        // TODO: Implement other best practices for dynamic rendering

    } else {
        auto& subpass = rp_state->createInfo.pSubpasses[cmd_state->GetActiveSubpass()];
        for (uint32_t i = 0; i < attachmentCount; i++) {
            auto& attachment = pClearAttachments[i];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            VkImageAspectFlags aspects = attachment.aspectMask;

            if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
                if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                    RecordResetScopeZcullDirection(*cmd_state);
                }
            }
            if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                if (subpass.pDepthStencilAttachment) {
                    fb_attachment = subpass.pDepthStencilAttachment->attachment;
                }
            } else if (aspects & VK_IMAGE_ASPECT_COLOR_BIT) {
                fb_attachment = subpass.pColorAttachments[attachment.colorAttachment].attachment;
            }
            if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                if (full_clear) {
                    RecordAttachmentClearAttachments(*cmd_state, fb_attachment, attachment.colorAttachment, aspects, rectCount,
                                                     pRects);
                } else {
                    RecordAttachmentAccess(*cmd_state, fb_attachment, aspects);
                }
                if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                    const VkFormat format = rp_state->createInfo.pAttachments[fb_attachment].format;
                    RecordClearColor(format, attachment.clearValue.color);
                }
            }
        }
    }
}

bool BestPractices::ClearAttachmentsIsFullClear(const bp_state::CommandBuffer& cmd, uint32_t rectCount,
                                                const VkClearRect* pRects) const {
    if (cmd.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // We don't know the accurate render area in a secondary,
        // so assume we clear the entire frame buffer.
        // This is resolved in CmdExecuteCommands where we can check if the clear is a full clear.
        return true;
    }

    // If we have a rect which covers the entire frame buffer, we have a LOAD_OP_CLEAR-like command.
    for (uint32_t i = 0; i < rectCount; i++) {
        auto& rect = pRects[i];
        auto& render_area = cmd.active_render_pass_begin_info.renderArea;
        if (rect.rect.extent.width == render_area.extent.width && rect.rect.extent.height == render_area.extent.height) {
            return true;
        }
    }

    return false;
}

bool BestPractices::ValidateClearAttachment(const bp_state::CommandBuffer& cmd, uint32_t fb_attachment, uint32_t color_attachment,
                                            VkImageAspectFlags aspects, const Location& loc) const {
    const RENDER_PASS_STATE* rp = cmd.activeRenderPass.get();
    bool skip = false;

    if (!rp || fb_attachment == VK_ATTACHMENT_UNUSED) {
        return skip;
    }

    const auto& rp_state = cmd.render_pass_state;

    auto attachment_itr =
        std::find_if(rp_state.touchesAttachments.begin(), rp_state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    // Only report aspects which haven't been touched yet.
    VkImageAspectFlags new_aspects = aspects;
    if (attachment_itr != rp_state.touchesAttachments.end()) {
        new_aspects &= ~attachment_itr->aspects;
    }

    // Warn if this is issued prior to Draw Cmd and clearing the entire attachment
    if (!cmd.has_draw_cmd) {
        skip |= LogPerformanceWarning(kVUID_BestPractices_DrawState_ClearCmdBeforeDraw, cmd.Handle(), loc,
                                      "issued on %s prior to any Draw Cmds in current render pass. It is recommended you "
                                      "use RenderPass LOAD_OP_CLEAR on attachments instead.",
                                      FormatHandle(cmd).c_str());
    }

    if ((new_aspects & VK_IMAGE_ASPECT_COLOR_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |=
            LogPerformanceWarning(kVUID_BestPractices_ClearAttachments_ClearAfterLoad, device, loc,
                                  "issued on %s for color attachment #%u in this subpass, "
                                  "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                                  "it is more efficient.",
                                  FormatHandle(cmd).c_str(), color_attachment);
    }

    if ((new_aspects & VK_IMAGE_ASPECT_DEPTH_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |=
            LogPerformanceWarning(kVUID_BestPractices_ClearAttachments_ClearAfterLoad, device, loc,
                                  "issued on %s for the depth attachment in this subpass, "
                                  "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                                  "it is more efficient.",
                                  FormatHandle(cmd).c_str());

        if (VendorCheckEnabled(kBPVendorNVIDIA)) {
            const auto cmd_state = GetRead<bp_state::CommandBuffer>(cmd.commandBuffer());
            assert(cmd_state);
            skip |= ValidateZcullScope(*cmd_state, loc);
        }
    }

    if ((new_aspects & VK_IMAGE_ASPECT_STENCIL_BIT) &&
        rp->createInfo.pAttachments[fb_attachment].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
        skip |=
            LogPerformanceWarning(kVUID_BestPractices_ClearAttachments_ClearAfterLoad, device, loc,
                                  "issued on %s for the stencil attachment in this subpass, "
                                  "but LOAD_OP_LOAD was used. If you need to clear the framebuffer, always use LOAD_OP_CLEAR as "
                                  "it is more efficient.",
                                  FormatHandle(cmd).c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                       const VkClearRect* pRects, const ErrorObject& error_obj) const {
    bool skip = false;
    const auto cb_node = GetRead<bp_state::CommandBuffer>(commandBuffer);
    if (!cb_node) return skip;

    if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // Defer checks to ExecuteCommands.
        return skip;
    }

    // Only care about full clears, partial clears might have legitimate uses.
    const bool is_full_clear = ClearAttachmentsIsFullClear(*cb_node, rectCount, pRects);

    // Check for uses of ClearAttachments along with LOAD_OP_LOAD,
    // as it can be more efficient to just use LOAD_OP_CLEAR
    const RENDER_PASS_STATE* rp = cb_node->activeRenderPass.get();
    if (rp) {
        if (rp->use_dynamic_rendering || rp->use_dynamic_rendering_inherited) {
            const auto pColorAttachments = rp->dynamic_rendering_begin_rendering_info.pColorAttachments;

            if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                for (uint32_t i = 0; i < attachmentCount; i++) {
                    const auto& attachment = pAttachments[i];
                    if (attachment.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                        skip |= ValidateZcullScope(*cb_node, error_obj.location);
                    }
                    if ((attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) && attachment.colorAttachment != VK_ATTACHMENT_UNUSED) {
                        const auto& color_attachment = pColorAttachments[attachment.colorAttachment];
                        if (color_attachment.imageView) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(color_attachment.imageView);
                            const VkFormat format = image_view_state->create_info.format;
                            skip |= ValidateClearColor(commandBuffer, format, attachment.clearValue.color, error_obj.location);
                        }
                    }
                }
            }

            if (is_full_clear) {
                // TODO: Implement ValidateClearAttachment for dynamic rendering
            }

        } else {
            const auto& subpass = rp->createInfo.pSubpasses[cb_node->GetActiveSubpass()];

            if (is_full_clear) {
                for (uint32_t i = 0; i < attachmentCount; i++) {
                    const auto& attachment = pAttachments[i];

                    if (attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                        uint32_t color_attachment = attachment.colorAttachment;
                        uint32_t fb_attachment = subpass.pColorAttachments[color_attachment].attachment;
                        skip |= ValidateClearAttachment(*cb_node, fb_attachment, color_attachment, attachment.aspectMask,
                                                        error_obj.location);
                    }

                    if (subpass.pDepthStencilAttachment &&
                        (attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                        uint32_t fb_attachment = subpass.pDepthStencilAttachment->attachment;
                        skip |= ValidateClearAttachment(*cb_node, fb_attachment, VK_ATTACHMENT_UNUSED, attachment.aspectMask,
                                                        error_obj.location);
                    }
                }
            }
            if (VendorCheckEnabled(kBPVendorNVIDIA) && rp->createInfo.pAttachments) {
                for (uint32_t attachment_idx = 0; attachment_idx < attachmentCount; ++attachment_idx) {
                    const auto& attachment = pAttachments[attachment_idx];

                    if (attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                        const uint32_t fb_attachment = subpass.pColorAttachments[attachment.colorAttachment].attachment;
                        if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                            const VkFormat format = rp->createInfo.pAttachments[fb_attachment].format;
                            skip |= ValidateClearColor(commandBuffer, format, attachment.clearValue.color, error_obj.location);
                        }
                    }
                }
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        for (uint32_t attachment_idx = 0; attachment_idx < attachmentCount; attachment_idx++) {
            if (pAttachments[attachment_idx].aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
                bool black_check = false;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[0] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[1] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[2] != 0.0f;
                black_check |= pAttachments[attachment_idx].clearValue.color.float32[3] != 0.0f &&
                               pAttachments[attachment_idx].clearValue.color.float32[3] != 1.0f;

                bool white_check = false;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[0] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[1] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[2] != 1.0f;
                white_check |= pAttachments[attachment_idx].clearValue.color.float32[3] != 0.0f &&
                               pAttachments[attachment_idx].clearValue.color.float32[3] != 1.0f;

                if (black_check && white_check) {
                    skip |= LogPerformanceWarning(
                        kVUID_BestPractices_ClearAttachment_FastClearValues, device, error_obj.location,
                        "%s Performance warning: vkCmdClearAttachments() clear value for color attachment %" PRId32
                        " is not a fast clear value."
                        "Consider changing to one of the following:"
                        "RGBA(0, 0, 0, 0) "
                        "RGBA(0, 0, 0, 1) "
                        "RGBA(1, 1, 1, 0) "
                        "RGBA(1, 1, 1, 1)",
                        VendorSpecificTag(kBPVendorAMD), attachment_idx);
                }
            } else {
                if ((pAttachments[attachment_idx].clearValue.depthStencil.depth != 0 &&
                     pAttachments[attachment_idx].clearValue.depthStencil.depth != 1) &&
                    pAttachments[attachment_idx].clearValue.depthStencil.stencil != 0) {
                    skip |= LogPerformanceWarning(kVUID_BestPractices_ClearAttachment_FastClearValues, device, error_obj.location,
                                                  "%s Performance warning: vkCmdClearAttachments() clear value for depth/stencil "
                                                  "attachment %" PRId32
                                                  " is not a fast clear value."
                                                  "Consider changing to one of the following:"
                                                  "D=0.0f, S=0"
                                                  "D=1.0f, S=0",
                                                  VendorSpecificTag(kBPVendorAMD), attachment_idx);
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdResolveImage(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image,
                                            const Location& loc) const {
    bool skip = false;
    auto src_image_type = Get<IMAGE_STATE>(src_image)->createInfo.imageType;
    auto dst_image_type = Get<IMAGE_STATE>(dst_image)->createInfo.imageType;

    if (src_image_type != dst_image_type) {
        skip |= LogPerformanceWarning(kVUID_BestPractices_DrawState_MismatchedImageType, command_buffer, loc,
                                      "srcImage type (%s) and dstImage type (%s) are not the same.",
                                      string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
    }

    skip |= VendorCheckEnabled(kBPVendorArm) &&
            LogPerformanceWarning(kVUID_BestPractices_CmdResolveImage_ResolvingImage, command_buffer, loc,
                                  "%s Attempting to resolve a multisampled image. "
                                  "This is a very slow and extremely bandwidth intensive path. "
                                  "You should always resolve multisampled images on-tile with pResolveAttachments in VkRenderPass.",
                                  VendorSpecificTag(kBPVendorArm));
    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, srcImage, dstImage, error_obj.location);
    return skip;
}

bool BestPractices::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR* pResolveImageInfo,
                                                       const ErrorObject& error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->dstImage,
                                    error_obj.location.dot(Field::pResolveImageInfo));
    return skip;
}

void BestPractices::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdResolveImage, src, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                           pRegions[i].srcSubresource);
        QueueValidateImage(funcs, Func::vkCmdResolveImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                           pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                     const VkResolveImageInfo2KHR* pResolveImageInfo) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo);
}

void BestPractices::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(pResolveImageInfo->srcImage);
    auto dst = Get<bp_state::Image>(pResolveImageInfo->dstImage);
    uint32_t regionCount = pResolveImageInfo->regionCount;

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdResolveImage2, src, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                           pResolveImageInfo->pRegions[i].srcSubresource);
        QueueValidateImage(funcs, Func::vkCmdResolveImage2, dst, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                           pResolveImageInfo->pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue* pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(image);

    for (uint32_t i = 0; i < rangeCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdClearColorImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, pRanges[i]);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordClearColor(dst->createInfo.format, *pColor);
    }
}

void BestPractices::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange* pRanges) {
    ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount,
                                                                   pRanges);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(image);

    for (uint32_t i = 0; i < rangeCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdClearDepthStencilImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, pRanges[i]);
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < rangeCount; i++) {
            RecordResetZcullDirection(*cb, image, pRanges[i]);
        }
    }
}

void BestPractices::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy* pRegions) {
    ValidationStateTracker::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout,
                                                      regionCount, pRegions);

    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdCopyImage, src, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ, pRegions[i].srcSubresource);
        QueueValidateImage(funcs, Func::vkCmdCopyImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE, pRegions[i].dstSubresource);
    }
}

void BestPractices::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdCopyBufferToImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                           pRegions[i].imageSubresource);
    }
}

void BestPractices::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdCopyImageToBuffer, src, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                           pRegions[i].imageSubresource);
    }
}

void BestPractices::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit* pRegions, VkFilter filter) {
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    auto& funcs = cb->queue_submit_functions;
    auto src = Get<bp_state::Image>(srcImage);
    auto dst = Get<bp_state::Image>(dstImage);

    for (uint32_t i = 0; i < regionCount; i++) {
        QueueValidateImage(funcs, Func::vkCmdBlitImage, src, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ, pRegions[i].srcSubresource);
        QueueValidateImage(funcs, Func::vkCmdBlitImage, dst, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE, pRegions[i].dstSubresource);
    }
}

template <typename RegionType>
bool BestPractices::ValidateCmdBlitImage(VkCommandBuffer command_buffer, uint32_t region_count, const RegionType* regions,
                                         const Location& loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < region_count; i++) {
        const RegionType region = regions[i];
        if ((region.srcOffsets[0].x == region.srcOffsets[1].x) || (region.srcOffsets[0].y == region.srcOffsets[1].y) ||
            (region.srcOffsets[0].z == region.srcOffsets[1].z)) {
            skip |= LogWarning(kVUID_BestPractices_DrawState_InvalidExtents, command_buffer, loc,
                               "pRegions[%" PRIu32 "].srcOffsets specify a zero-volume area", i);
        }
        if ((region.dstOffsets[0].x == region.dstOffsets[1].x) || (region.dstOffsets[0].y == region.dstOffsets[1].y) ||
            (region.dstOffsets[0].z == region.dstOffsets[1].z)) {
            skip |= LogWarning(kVUID_BestPractices_DrawState_InvalidExtents, command_buffer, loc,
                               "pRegions[%" PRIu32 "].dstOffsets specify a zero-volume area", i);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit* pRegions, VkFilter filter, const ErrorObject& error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, regionCount, pRegions, error_obj.location);
}

bool BestPractices::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                                    const ErrorObject& error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                                 const ErrorObject& error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                error_obj.location.dot(Field::pBlitImageInfo));
}

bool BestPractices::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue* pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const {
    bool skip = false;

    auto dst = Get<bp_state::Image>(image);

    if (VendorCheckEnabled(kBPVendorAMD)) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_ClearAttachment_ClearImage, device, error_obj.location,
            "%s Performance warning: using vkCmdClearColorImage is not recommended. Prefer using LOAD_OP_CLEAR or "
            "vkCmdClearAttachments instead",
            VendorSpecificTag(kBPVendorAMD));
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        skip |= ValidateClearColor(commandBuffer, dst->createInfo.format, *pColor, error_obj.location);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange* pRanges,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_ClearAttachment_ClearImage, device, error_obj.location,
            "%s Performance warning: using vkCmdClearDepthStencilImage is not recommended. Prefer using LOAD_OP_CLEAR or "
            "vkCmdClearAttachments instead",
            VendorSpecificTag(kBPVendorAMD));
    }
    const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < rangeCount; i++) {
            skip |= ValidateZcull(*cmd_state, image, pRanges[i], error_obj.location);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy* pRegions, const ErrorObject& error_obj) const {
    bool skip = false;
    std::stringstream src_image_hex;
    std::stringstream dst_image_hex;
    src_image_hex << "0x" << std::hex << HandleToUint64(srcImage);
    dst_image_hex << "0x" << std::hex << HandleToUint64(dstImage);

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto src_state = Get<IMAGE_STATE>(srcImage);
        auto dst_state = Get<IMAGE_STATE>(dstImage);

        if (src_state && dst_state) {
            VkImageTiling src_Tiling = src_state->createInfo.tiling;
            VkImageTiling dst_Tiling = dst_state->createInfo.tiling;
            if (src_Tiling != dst_Tiling && (src_Tiling == VK_IMAGE_TILING_LINEAR || dst_Tiling == VK_IMAGE_TILING_LINEAR)) {
                skip |= LogPerformanceWarning(kVUID_BestPractices_vkImage_AvoidImageToImageCopy, device, error_obj.location,
                                              "%s Performance warning: image %s and image %s have differing tilings. Use buffer to "
                                              "image (vkCmdCopyImageToBuffer) "
                                              "and image to buffer (vkCmdCopyBufferToImage) copies instead of image to image "
                                              "copies when converting between linear and optimal images",
                                              VendorSpecificTag(kBPVendorAMD), src_image_hex.str().c_str(),
                                              dst_image_hex.str().c_str());
            }
        }
    }

    return skip;
}
