/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include "best_practices/best_practices_validation.h"
#include "error_message/error_strings.h"
#include "best_practices/bp_state.h"
#include "state_tracker/render_pass_state.h"
#include "generated/dispatch_functions.h"

static inline bool RenderPassUsesAttachmentAsResolve(const vku::safe_VkRenderPassCreateInfo2& create_info, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < create_info.subpassCount; subpass++) {
        const auto& subpass_info = create_info.pSubpasses[subpass];
        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
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
        const Location attachment_loc = create_info_loc.dot(Field::pAttachments, i);
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((vkuFormatIsColor(format) || vkuFormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning("BestPractices-vkCreateRenderPass-attatchment-color-depth", device, attachment_loc,
                                   "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                   "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED and format %s. This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.",
                                   string_VkFormat(format));
            }
            if (vkuFormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= LogWarning("BestPractices-vkCreateRenderPass-attatchment-stencil", device, attachment_loc,
                                   "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD "
                                   "and initialLayout == VK_IMAGE_LAYOUT_UNDEFINED and format %s. This is probably not what you "
                                   "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                   "image truely is undefined at the start of the render pass.",
                                   string_VkFormat(format));
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
                    "BestPractices-vkCreateRenderPass-image-requires-memory", device, attachment_loc,
                    "Attachment %u in the VkRenderPass is a multisampled image with %u samples, but it uses loadOp/storeOp "
                    "which requires accessing data from memory. Multisampled images should always be loadOp = CLEAR or DONT_CARE, "
                    "storeOp = DONT_CARE. This allows the implementation to use lazily allocated memory effectively.",
                    i, static_cast<uint32_t>(attachment.samples));
            }
        }
    }

    if (IsExtEnabled(extensions.vk_ext_multisampled_render_to_single_sampled)) {
        for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
            if (!pCreateInfo->pSubpasses[i].pResolveAttachments) continue;
            for (uint32_t j = 0; j < pCreateInfo->pSubpasses[i].colorAttachmentCount; ++j) {
                const uint32_t attachment = pCreateInfo->pSubpasses[i].pResolveAttachments[j].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    const VkFormat format = pCreateInfo->pAttachments[attachment].format;
                    VkSubpassResolvePerformanceQueryEXT performance_query = vku::InitStructHelper();
                    VkFormatProperties2 format_properties2 = vku::InitStructHelper(&performance_query);
                    DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &format_properties2);
                    if (performance_query.optimal == VK_FALSE) {
                        skip |= LogPerformanceWarning(
                            "BestPractices-vkCreateRenderPass-SubpassResolve-NonOptimalFormat", device,
                            create_info_loc.dot(Field::pSubpasses, i).dot(Field::pResolveAttachments, j).dot(Field::attachment),
                            "(%" PRIu32
                            ") in the VkRenderPass has the format %s and is used as a resolve attachment, "
                            "but VkSubpassResolvePerformanceQueryEXT::optimal is VK_FALSE.",
                            attachment, string_VkFormat(format));
                    }
                }
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const Location& loc) const {
    bool skip = false;

    if (!pRenderPassBegin) return skip;

    auto rp_state = Get<vvl::RenderPass>(pRenderPassBegin->renderPass);
    ASSERT_AND_RETURN_SKIP(rp_state);

    if (rp_state->create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
        const VkRenderPassAttachmentBeginInfo* rpabi =
            vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
        if (rpabi) {
            skip |= ValidateAttachments(rp_state->create_info.ptr(), rpabi->attachmentCount, rpabi->pAttachments, loc);
        }
    }
    // Check if any attachments have LOAD operation on them
    for (uint32_t att = 0; att < rp_state->create_info.attachmentCount; att++) {
        const auto& attachment = rp_state->create_info.pAttachments[att];

        bool attachment_has_readback = false;
        if (!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
            attachment_has_readback = true;
        }

        if (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
            attachment_has_readback = true;
        }

        bool attachment_needs_readback = false;

        // Check if the attachment is actually used in any subpass on-tile
        if (attachment_has_readback && RenderPassUsesAttachmentOnTile(rp_state->create_info, att)) {
            attachment_needs_readback = true;
        }

        // Using LOAD_OP_LOAD is expensive on tiled GPUs, so flag it as a potential improvement
        if (attachment_needs_readback && (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
            const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
            skip |= LogPerformanceWarning("BestPractices-vkCmdBeginRenderPass-attachment-needs-readback", objlist, loc,
                                          "%s %s: Attachment #%u in render pass has begun with VK_ATTACHMENT_LOAD_OP_LOAD.\n"
                                          "Submitting this renderpass will cause the driver to inject a readback of the attachment "
                                          "which will copy in total %u pixels (renderArea = "
                                          "{ %s }) to the tile buffer.",
                                          VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), att,
                                          pRenderPassBegin->renderArea.extent.width * pRenderPassBegin->renderArea.extent.height,
                                          string_VkRect2D(pRenderPassBegin->renderArea).c_str());
        }
    }

    // Check if renderpass has at least one VK_ATTACHMENT_LOAD_OP_CLEAR

    bool clearing = false;

    for (uint32_t att = 0; att < rp_state->create_info.attachmentCount; att++) {
        const auto& attachment = rp_state->create_info.pAttachments[att];

        if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearing = true;
            break;
        }
    }

    // Check if there are ClearValues passed to BeginRenderPass even though no attachments will be cleared
    if (!clearing && pRenderPassBegin->clearValueCount > 0) {
        // Flag as warning because nothing will happen per spec, and pClearValues will be ignored
        const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
        skip |=
            LogWarning("BestPractices-ClearValueWithoutLoadOpClear", objlist, loc,
                       "This render pass does not have VkRenderPassCreateInfo.pAttachments->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR "
                       "but VkRenderPassBeginInfo.clearValueCount > 0. VkRenderPassBeginInfo.pClearValues will be ignored and no "
                       "attachments will be cleared.");
    }

    // Check if there are more clearValues than attachments
    if (pRenderPassBegin->clearValueCount > rp_state->create_info.attachmentCount) {
        // Flag as warning because the overflowing clearValues will be ignored and could even be undefined on certain platforms.
        // This could signal a bug and there seems to be no reason for this to happen on purpose.
        const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
        skip |= LogWarning("BestPractices-ClearValueCountHigherThanAttachmentCount", objlist, loc,
                           "This render pass has VkRenderPassBeginInfo.clearValueCount > VkRenderPassCreateInfo.attachmentCount "
                           "(%" PRIu32 " > %" PRIu32
                           ") and as such the clearValues that do not have a corresponding attachment will be ignored.",
                           pRenderPassBegin->clearValueCount, rp_state->create_info.attachmentCount);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA) && rp_state->create_info.pAttachments) {
        for (uint32_t i = 0; i < pRenderPassBegin->clearValueCount; ++i) {
            const auto& attachment = rp_state->create_info.pAttachments[i];
            if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                const auto& clear_color = pRenderPassBegin->pClearValues[i].color;
                skip |= ValidateClearColor(commandBuffer, attachment.format, clear_color, loc);
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                              const Location& loc) const {
    bool skip = false;
    const Location rendering_info_loc = loc.dot(Field::pRenderingInfo);

    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        const VkRenderingAttachmentInfo& color_attachment = pRenderingInfo->pColorAttachments[i];
        if (color_attachment.imageView == VK_NULL_HANDLE) continue;
        const Location color_attachment_loc = rendering_info_loc.dot(Field::pColorAttachments, i);

        auto image_view_state = Get<vvl::ImageView>(color_attachment.imageView);
        ASSERT_AND_CONTINUE(image_view_state);

        if (VendorCheckEnabled(kBPVendorNVIDIA)) {
            if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                const VkFormat format = image_view_state->create_info.format;
                skip |= ValidateClearColor(commandBuffer, format, color_attachment.clearValue.color, color_attachment_loc);
            }
        }

        // Check if accidently set resolve mode to none since everything else looks like it should be resolving
        if (color_attachment.resolveMode == VK_RESOLVE_MODE_NONE && color_attachment.resolveImageView != VK_NULL_HANDLE) {
            auto resolve_image_view_state = Get<vvl::ImageView>(color_attachment.resolveImageView);
            if (resolve_image_view_state && resolve_image_view_state->image_state->create_info.samples == VK_SAMPLE_COUNT_1_BIT &&
                image_view_state->image_state->create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
                const LogObjectList objlist(commandBuffer, resolve_image_view_state->Handle(), image_view_state->Handle());
                skip |= LogWarning("BestPractices-VkRenderingInfo-ResolveModeNone", commandBuffer,
                                   color_attachment_loc.dot(Field::resolveMode),
                                   "is VK_RESOLVE_MODE_NONE but resolveImageView is pointed to a valid VkImageView with "
                                   "VK_SAMPLE_COUNT_1_BIT and imageView is pointed to a VkImageView with %s. If "
                                   "VK_RESOLVE_MODE_NONE is set, the resolveImageView value is ignored.",
                                   string_VkSampleCountFlagBits(image_view_state->image_state->create_info.samples));
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                     const ErrorObject& error_obj) const {
    return ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj.location);
}

bool BestPractices::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                        const ErrorObject& error_obj) const {
    return ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj.location);
}

bool BestPractices::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject& error_obj) const {
    return ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj.location);
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
    return ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj.location);
}

bool BestPractices::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                     const ErrorObject& error_obj) const {
    bool skip = ValidateCmdEndRenderPass(commandBuffer, error_obj.location);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
        auto& sub_state = bp_state::SubState(*cb_state);
        skip |= ValidateZcullScope(sub_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                        const ErrorObject& error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    bool skip = ValidateCmdEndRenderPass(commandBuffer, error_obj.location);
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
        auto& sub_state = bp_state::SubState(*cb_state);
        skip |= ValidateZcullScope(sub_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
        auto& sub_state = bp_state::SubState(*cb_state);
        skip |= ValidateZcullScope(sub_state, error_obj.location);
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool BestPractices::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const Location& loc) const {
    bool skip = false;
    const auto cmd = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto& sub_state = bp_state::SubState(*cmd);

    auto& render_pass_state = sub_state.render_pass_state;

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
            "BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage", commandBuffer, loc,
            "%s %s: Depth pre-passes may be in use. In general, this is not recommended in tile-based deferred "
            "renderering architectures; such as those in Arm Mali or PowerVR GPUs. Since they can remove geometry "
            "hidden by other opaque geometry. Mali has Forward Pixel Killing (FPK), PowerVR has Hiden Surface "
            "Remover (HSR) in which case, using depth pre-passes for hidden surface removal may worsen performance.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
    }

    vvl::RenderPass* rp_state = cmd->active_render_pass.get();
    if (!rp_state) return skip;

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
        // If we use an attachment on-tile, we should access it in some way. Otherwise,
        // it is redundant to have it be part of the render pass.
        // Only consider it redundant if it will actually consume bandwidth, i.e.
        // LOAD_OP_LOAD is used or STORE_OP_STORE. CLEAR -> DONT_CARE is benign,
        // as is using pure input attachments.
        // CLEAR -> STORE might be considered a "useful" thing to do, but
        // the optimal thing to do is to defer the clear until you're actually
        // going to render to the image.

        uint32_t num_attachments = rp_state->create_info.attachmentCount;
        for (uint32_t i = 0; i < num_attachments; i++) {
            if (!RenderPassUsesAttachmentOnTile(rp_state->create_info, i) ||
                RenderPassUsesAttachmentAsResolve(rp_state->create_info, i)) {
                continue;
            }

            auto& attachment = rp_state->create_info.pAttachments[i];

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
                    "BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile", commandBuffer, loc,
                    "%s %s: Render pass was ended, but attachment #%u (format: %s, untouched aspects %s) "
                    "was never accessed by a pipeline or clear command. "
                    "On tile-based architectures, LOAD_OP_LOAD and STORE_OP_STORE consume bandwidth and should not be part of the "
                    "render pass if the attachments are not intended to be accessed.",
                    VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), i, string_VkFormat(attachment.format),
                    string_VkImageAspectFlags(untouched_aspects).c_str());
            }
        }
    }

    return skip;
}
