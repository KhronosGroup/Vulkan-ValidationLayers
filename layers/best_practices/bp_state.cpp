/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "best_practices/bp_state.h"
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include "best_practices/best_practices_validation.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "utils/action_command_utils.h"

namespace bp_state {

static bool SparseMetaDataRequired(vvl::Image& img) {
    if (img.create_from_swapchain) {
        return false;
    }

    for (const auto& req : img.sparse_requirements) {
        if (req.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
            return true;
        }
    }
    return false;
}

ImageSubState::ImageSubState(vvl::Image& img) : vvl::ImageSubState(img), sparse_metadata_required(SparseMetaDataRequired(img)) {
    SetupUsages();
}

ImageSubState::Usage ImageSubState::UpdateUsage(uint32_t array_layer, uint32_t mip_level, IMAGE_SUBRESOURCE_USAGE_BP usage,
                                                uint32_t queue_family) {
    auto last_usage = usages_[array_layer][mip_level];
    usages_[array_layer][mip_level].type = usage;
    usages_[array_layer][mip_level].queue_family_index = queue_family;
    return last_usage;
}

ImageSubState::Usage ImageSubState::GetUsage(uint32_t array_layer, uint32_t mip_level) const {
    return usages_[array_layer][mip_level];
}

IMAGE_SUBRESOURCE_USAGE_BP ImageSubState::GetUsageType(uint32_t array_layer, uint32_t mip_level) const {
    return GetUsage(array_layer, mip_level).type;
}

uint32_t ImageSubState::GetLastQueueFamily(uint32_t array_layer, uint32_t mip_level) const {
    return GetUsage(array_layer, mip_level).queue_family_index;
}

void ImageSubState::SetupUsages() {
    usages_.resize(base.create_info.arrayLayers);
    for (auto& mip_vec : usages_) {
        mip_vec.resize(base.create_info.mipLevels, {IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED, VK_QUEUE_FAMILY_IGNORED});
    }
}

CommandBufferSubState::CommandBufferSubState(vvl::CommandBuffer& cb, BestPractices& validator)
    : vvl::CommandBufferSubState(cb), validator(validator) {}

void CommandBufferSubState::RecordExecuteCommand(vvl::CommandBuffer& secondary_command_buffer) {
    auto& secondary_sub_state = SubState(secondary_command_buffer);
    if (secondary_command_buffer.IsSecondary()) {
        render_pass_state.has_draw_cmd |= secondary_sub_state.render_pass_state.has_draw_cmd;
    }

    for (auto& function : secondary_sub_state.queue_submit_functions) {
        queue_submit_functions.push_back(function);
    }

    for (auto& early_clear : secondary_sub_state.render_pass_state.earlyClearAttachments) {
        if (validator.ClearAttachmentsIsFullClear(*this, uint32_t(early_clear.rects.size()), early_clear.rects.data())) {
            RecordAttachmentClearAttachments(early_clear.framebufferAttachment, early_clear.colorAttachment, early_clear.aspects,
                                             uint32_t(early_clear.rects.size()), early_clear.rects.data());
        } else {
            RecordAttachmentAccess(early_clear.framebufferAttachment, early_clear.aspects);
        }
    }

    for (auto& touch : secondary_sub_state.render_pass_state.touchesAttachments) {
        RecordAttachmentAccess(touch.framebufferAttachment, touch.aspects);
    }

    render_pass_state.numDrawCallsDepthEqualCompare += secondary_sub_state.render_pass_state.numDrawCallsDepthEqualCompare;
    render_pass_state.numDrawCallsDepthOnly += secondary_sub_state.render_pass_state.numDrawCallsDepthOnly;

    for (const auto& [event, secondary_info] : secondary_sub_state.event_signaling_state) {
        if (auto* primary_info = vvl::Find(event_signaling_state, event)) {
            primary_info->signaled = secondary_info.signaled;
        } else {
            event_signaling_state.emplace(event, secondary_info);
        }
    }
}

void CommandBufferSubState::RecordPushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags, uint32_t offset,
                                                uint32_t size, const void* values) {
    PushConstantData push_constant_data;
    push_constant_data.layout = layout;
    push_constant_data.stage_flags = stage_flags;
    push_constant_data.offset = offset;
    push_constant_data.values.resize(size);
    auto byte_values = static_cast<const std::byte*>(values);
    std::copy(byte_values, byte_values + size, push_constant_data.values.data());
    push_constant_data_chunks.emplace_back(push_constant_data);
}

void CommandBufferSubState::ClearPushConstants() { push_constant_data_chunks.clear(); }

void CommandBufferSubState::RecordBeginRenderingCommon(const VkRenderPassBeginInfo* pRenderPassBegin,
                                                       const VkRenderingInfo* pRenderingInfo) {
    auto rp_state = base.active_render_pass.get();
    ASSERT_AND_RETURN(rp_state);

    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        vvl::ImageView* depth_image_view = nullptr;
        std::optional<VkAttachmentLoadOp> load_op;

        if (pRenderingInfo) {  // dynamic
            const auto depth_attachment = pRenderingInfo->pDepthAttachment;
            if (depth_attachment) {
                load_op.emplace(depth_attachment->loadOp);
                const auto depth_image_view_shared_ptr = base.dev_data.Get<vvl::ImageView>(depth_attachment->imageView);
                if (depth_image_view_shared_ptr) {
                    depth_image_view = depth_image_view_shared_ptr.get();
                }
            }

            for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
                const auto& color_attachment = pRenderingInfo->pColorAttachments[i];
                if (color_attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    if (auto image_view_state = base.dev_data.Get<vvl::ImageView>(color_attachment.imageView)) {
                        const VkFormat format = image_view_state->create_info.format;
                        validator.RecordClearColor(format, color_attachment.clearValue.color);
                    }
                }
            }

        } else if (pRenderPassBegin) {  // non-dynamic
            if (rp_state->create_info.pAttachments) {
                if (rp_state->create_info.subpassCount > 0) {
                    const auto depth_attachment = rp_state->create_info.pSubpasses[0].pDepthStencilAttachment;
                    if (depth_attachment) {
                        const uint32_t attachment_index = depth_attachment->attachment;
                        if (attachment_index != VK_ATTACHMENT_UNUSED) {
                            load_op.emplace(rp_state->create_info.pAttachments[attachment_index].loadOp);
                            depth_image_view = base.active_attachments[attachment_index].image_view;
                        }
                    }
                }
                for (uint32_t i = 0; i < pRenderPassBegin->clearValueCount; ++i) {
                    const auto& attachment = rp_state->create_info.pAttachments[i];
                    if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                        const auto& clear_color = pRenderPassBegin->pClearValues[i].color;
                        validator.RecordClearColor(attachment.format, clear_color);
                    }
                }
            }
        }
        if (depth_image_view && (depth_image_view->normalized_subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->VkHandle();
            RecordBindZcullScopeNV(depth_image, depth_image_view->normalized_subresource_range);
        } else {
            RecordUnbindZcullScopeNV();
        }
        if (load_op) {
            if (*load_op == VK_ATTACHMENT_LOAD_OP_CLEAR || *load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
                RecordResetScopeZcullDirectionNV();
            }
        }
    }

    // Spec states that after BeginRenderPass all resources should be rebound
    if (rp_state->has_multiview_enabled) {
        UnbindResources();
    }
}

void CommandBufferSubState::RecordBeginRendering(const VkRenderingInfo& rendering_info) {
    RecordBeginRenderingCommon(nullptr, &rendering_info);
}

void CommandBufferSubState::RecordBeginRenderPass(const VkRenderPassBeginInfo& render_pass_begin) {
    RecordBeginRenderingCommon(&render_pass_begin, nullptr);

    auto rp_state = base.active_render_pass.get();
    ASSERT_AND_RETURN(rp_state);

    // Check load ops
    for (uint32_t att = 0; att < rp_state->create_info.attachmentCount; att++) {
        const auto& attachment = rp_state->create_info.pAttachments[att];

        if (!RenderPassUsesAttachmentAsImageOnly(rp_state->create_info, att) &&
            !RenderPassUsesAttachmentOnTile(rp_state->create_info, att)) {
            continue;
        }

        // If renderpass doesn't load attachment, no need to validate image in queue
        if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_NONE) ||
            (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_NONE)) {
            continue;
        }

        IMAGE_SUBRESOURCE_USAGE_BP usage = IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED;

        if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ||
            (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD)) {
            usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE;
        } else if ((!vkuFormatIsStencilOnly(attachment.format) && attachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ||
                   (vkuFormatHasStencil(attachment.format) && attachment.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)) {
            usage = IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED;
        } else if (RenderPassUsesAttachmentAsImageOnly(rp_state->create_info, att)) {
            usage = IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS;
        }

        VkImageView image_view = VK_NULL_HANDLE;
        if (base.active_framebuffer) {
            if (base.active_framebuffer->create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(render_pass_begin.pNext);
                if (rpabi) {
                    image_view = rpabi->pAttachments[att];
                }
            } else {
                image_view = base.active_framebuffer->create_info.pAttachments[att];
            }
        }

        if (auto image_view_state = base.dev_data.Get<vvl::ImageView>(image_view)) {
            validator.QueueValidateImageView(queue_submit_functions, vvl::Func::vkCmdBeginRenderPass, *image_view_state, usage);
        }
    }

    // Check store ops
    for (uint32_t att = 0; att < rp_state->create_info.attachmentCount; att++) {
        const auto& attachment = rp_state->create_info.pAttachments[att];

        if (!RenderPassUsesAttachmentOnTile(rp_state->create_info, att)) {
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

        VkImageView image_view = VK_NULL_HANDLE;
        if (base.active_framebuffer) {
            if (base.active_framebuffer->create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
                const VkRenderPassAttachmentBeginInfo* rpabi =
                    vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(render_pass_begin.pNext);
                if (rpabi) {
                    image_view = rpabi->pAttachments[att];
                }
            } else {
                image_view = base.active_framebuffer->create_info.pAttachments[att];
            }
        }

        if (auto image_view_state = base.dev_data.Get<vvl::ImageView>(image_view)) {
            validator.QueueValidateImageView(queue_submit_functions_after_render_pass, vvl::Func::vkCmdEndRenderPass,
                                             *image_view_state, usage);
        }
    }

    // Reset state for the render pass
    {
        render_pass_state.touchesAttachments.clear();
        render_pass_state.earlyClearAttachments.clear();
        render_pass_state.numDrawCallsDepthOnly = 0;
        render_pass_state.numDrawCallsDepthEqualCompare = 0;
        render_pass_state.colorAttachment = false;
        render_pass_state.depthAttachment = false;
        render_pass_state.drawTouchAttachments = true;
        render_pass_state.has_draw_cmd = false;
        // Don't reset state related to pipeline state.

        // Reset NV state
        nv = {};

        // track depth / color attachment usage within the renderpass
        for (size_t i = 0; i < rp_state->create_info.subpassCount; i++) {
            // record if depth/color attachments are in use for this renderpass
            if (rp_state->create_info.pSubpasses[i].pDepthStencilAttachment != nullptr) render_pass_state.depthAttachment = true;

            if (rp_state->create_info.pSubpasses[i].colorAttachmentCount > 0) render_pass_state.colorAttachment = true;
        }
        // Spec states that after BeginRenderPass all resources should be rebound
        if (base.active_render_pass && base.active_render_pass->has_multiview_enabled) {
            UnbindResources();
        }
    }
}

void CommandBufferSubState::RecordNextSubpass() {
    if (!base.active_render_pass) {
        return;
    }

    // Spec states that after NextSubpass all resources should be rebound
    if (base.active_render_pass->has_multiview_enabled) {
        UnbindResources();
    }

    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        vvl::ImageView* depth_image_view = nullptr;

        const auto depth_attachment =
            base.active_render_pass->create_info.pSubpasses[base.GetActiveSubpass()].pDepthStencilAttachment;
        if (depth_attachment) {
            const uint32_t attachment_index = depth_attachment->attachment;
            if (attachment_index != VK_ATTACHMENT_UNUSED) {
                depth_image_view = base.active_attachments[attachment_index].image_view;
            }
        }
        if (depth_image_view && (depth_image_view->normalized_subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U) {
            const VkImage depth_image = depth_image_view->image_state->VkHandle();
            RecordBindZcullScopeNV(depth_image, depth_image_view->normalized_subresource_range);
        } else {
            RecordUnbindZcullScopeNV();
        }
    }
}

void CommandBufferSubState::RecordEndRenderingCommon(const vvl::RenderPass& rp_state) {
    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        std::optional<VkAttachmentStoreOp> store_op;

        if (rp_state.UsesDynamicRendering()) {
            const auto depth_attachment = rp_state.dynamic_rendering_begin_rendering_info.pDepthAttachment;
            if (depth_attachment) {
                store_op.emplace(depth_attachment->storeOp);
            }
        } else {
            if (rp_state.create_info.subpassCount > 0) {
                const uint32_t last_subpass = rp_state.create_info.subpassCount - 1;
                const auto depth_attachment = rp_state.create_info.pSubpasses[last_subpass].pDepthStencilAttachment;
                if (depth_attachment) {
                    const uint32_t attachment = depth_attachment->attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        store_op.emplace(rp_state.create_info.pAttachments[attachment].storeOp);
                    }
                }
            }
        }

        if (store_op) {
            if (*store_op == VK_ATTACHMENT_STORE_OP_DONT_CARE || *store_op == VK_ATTACHMENT_STORE_OP_NONE) {
                RecordResetScopeZcullDirectionNV();
            }
        }

        RecordUnbindZcullScopeNV();
    }
}

void CommandBufferSubState::RecordEndRendering(const VkRenderingEndInfoEXT*) {
    if (!base.active_render_pass) {
        return;
    }
    RecordEndRenderingCommon(*base.active_render_pass);
}

void CommandBufferSubState::RecordEndRenderPass() {
    if (!base.active_render_pass) {
        return;
    }
    RecordEndRenderingCommon(*base.active_render_pass);

    // Add Deferred Queue
    queue_submit_functions.insert(queue_submit_functions.end(), queue_submit_functions_after_render_pass.begin(),
                                  queue_submit_functions_after_render_pass.end());
    queue_submit_functions_after_render_pass.clear();
}

void CommandBufferSubState::RecordCopyImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout, VkImageLayout,
                                            uint32_t region_count, const VkImageCopy* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordCopyImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout, VkImageLayout,
                                             uint32_t region_count, const VkImageCopy2* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage(vvl::Image& dst_image_state, VkImageLayout, uint32_t region_count,
                                                    const VkBufferImageCopy* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                                     regions[i].imageSubresource);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage2(vvl::Image& dst_image_state, VkImageLayout, uint32_t region_count,
                                                     const VkBufferImageCopy2* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE,
                                     regions[i].imageSubresource);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer(vvl::Image& src_image_state, VkImageLayout, uint32_t region_count,
                                                    const VkBufferImageCopy* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                                     regions[i].imageSubresource);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer2(vvl::Image& src_image_state, VkImageLayout, uint32_t region_count,
                                                     const VkBufferImageCopy2* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ,
                                     regions[i].imageSubresource);
    }
}

void CommandBufferSubState::RecordBlitImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout, VkImageLayout,
                                            uint32_t region_count, const VkImageBlit* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordBlitImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout, VkImageLayout,
                                             uint32_t region_count, const VkImageBlit2* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordResolveImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, uint32_t region_count,
                                               const VkImageResolve* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordResolveImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, uint32_t region_count,
                                                const VkImageResolve2* regions, const Location& loc) {
    for (uint32_t i = 0; i < region_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, src_image_state, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ,
                                     regions[i].srcSubresource);
        validator.QueueValidateImage(queue_submit_functions, loc, dst_image_state, IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE,
                                     regions[i].dstSubresource);
    }
}

void CommandBufferSubState::RecordClearColorImage(vvl::Image& image_state, VkImageLayout image_layout,
                                                  const VkClearColorValue* color_values, uint32_t range_count,
                                                  const VkImageSubresourceRange* ranges, const Location& loc) {
    for (uint32_t i = 0; i < range_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, image_state, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, ranges[i]);
    }

    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        validator.RecordClearColor(image_state.create_info.format, *color_values);
    }
}

void CommandBufferSubState::RecordClearDepthStencilImage(vvl::Image& image_state, VkImageLayout image_layout,
                                                         const VkClearDepthStencilValue* depth_stencil_values, uint32_t range_count,
                                                         const VkImageSubresourceRange* ranges, const Location& loc) {
    for (uint32_t i = 0; i < range_count; i++) {
        validator.QueueValidateImage(queue_submit_functions, loc, image_state, IMAGE_SUBRESOURCE_USAGE_BP::CLEARED, ranges[i]);
    }
    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        for (uint32_t i = 0; i < range_count; i++) {
            // TODO - use state object
            RecordResetZcullDirectionNV(image_state.VkHandle(), ranges[i]);
        }
    }
}

void CommandBufferSubState::RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment* pAttachments,
                                                   uint32_t rect_count, const VkClearRect* pRects, const Location&) {
    auto* rp_state = base.active_render_pass.get();
    auto* fb_state = base.active_framebuffer.get();
    if (rect_count == 0 || !rp_state) {
        return;
    }
    if (!base.IsSecondary() && !fb_state && !rp_state->UsesDynamicRendering()) {
        return;
    }

    // If we have a rect which covers the entire frame buffer, we have a LOAD_OP_CLEAR-like command.
    const bool full_clear = validator.ClearAttachmentsIsFullClear(*this, rect_count, pRects);

    if (rp_state->UsesDynamicRendering()) {
        if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
            auto pColorAttachments = rp_state->dynamic_rendering_begin_rendering_info.pColorAttachments;

            for (uint32_t i = 0; i < attachment_count; i++) {
                auto& clear_attachment = pAttachments[i];

                if (clear_attachment.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    RecordResetScopeZcullDirectionNV();
                }
                if ((clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                    clear_attachment.colorAttachment != VK_ATTACHMENT_UNUSED && pColorAttachments) {
                    const auto& attachment = pColorAttachments[clear_attachment.colorAttachment];
                    if (attachment.imageView) {
                        if (auto image_view_state = base.dev_data.Get<vvl::ImageView>(attachment.imageView)) {
                            const VkFormat format = image_view_state->create_info.format;
                            validator.RecordClearColor(format, clear_attachment.clearValue.color);
                        }
                    }
                }
            }
        }

        // TODO: Implement other best practices for dynamic rendering

    } else {
        auto& subpass = rp_state->create_info.pSubpasses[base.GetActiveSubpass()];
        for (uint32_t i = 0; i < attachment_count; i++) {
            auto& attachment = pAttachments[i];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            VkImageAspectFlags aspects = attachment.aspectMask;

            if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
                if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
                    RecordResetScopeZcullDirectionNV();
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
                    RecordAttachmentClearAttachments(fb_attachment, attachment.colorAttachment, aspects, rect_count, pRects);
                } else {
                    RecordAttachmentAccess(fb_attachment, aspects);
                }
                if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
                    const VkFormat format = rp_state->create_info.pAttachments[fb_attachment].format;
                    validator.RecordClearColor(format, attachment.clearValue.color);
                }
            }
        }
    }
}

void CommandBufferSubState::Destroy() { ResetCBState(); }

void CommandBufferSubState::Reset(const Location&) { ResetCBState(); }

void CommandBufferSubState::ResetCBState() {
    num_submits = 0;
    small_indexed_draw_call_count = 0;
    queue_submit_functions.clear();
    queue_submit_functions_after_render_pass.clear();
    ClearPushConstants();
}

void CommandBufferSubState::RecordActionCommand(LastBound& last_bound, const Location& loc) {
    if (vvl::IsCommandDrawMesh(loc.function) || vvl::IsCommandDrawVertex(loc.function)) {
        render_pass_state.has_draw_cmd = true;
    }

    // Draw
    if (last_bound.bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
            RecordCmdDrawTypeNVIDIA();
        }

        if (render_pass_state.drawTouchAttachments) {
            for (auto& touch : render_pass_state.nextDrawTouchesAttachments) {
                RecordAttachmentAccess(touch.framebufferAttachment, touch.aspects);
            }
            // No need to touch the same attachments over and over.
            render_pass_state.drawTouchAttachments = false;
        }
    }

    validator.UpdateBoundDescriptorSets(*this, last_bound, loc);
}

void CommandBufferSubState::RecordSetEvent(vvl::Func, VkEvent event, VkPipelineStageFlags2, const VkDependencyInfo*) {
    if (auto* signaling_info = vvl::Find(event_signaling_state, event)) {
        signaling_info->signaled = true;
    } else {
        event_signaling_state.emplace(event, bp_state::CommandBufferSubState::SignalingInfo(true));
    }
}

void CommandBufferSubState::RecordResetEvent(vvl::Func, VkEvent event, VkPipelineStageFlags2) {
    if (auto* signaling_info = vvl::Find(event_signaling_state, event)) {
        signaling_info->signaled = false;
    } else {
        event_signaling_state.emplace(event, bp_state::CommandBufferSubState::SignalingInfo(false));
    }
}

static std::vector<bp_state::AttachmentInfo> GetAttachmentAccess(vvl::Pipeline& pipe_state) {
    std::vector<bp_state::AttachmentInfo> result;
    auto rp = pipe_state.RenderPassState();
    if (!rp || rp->UsesDynamicRendering()) {
        return result;
    }
    const auto& create_info = pipe_state.GraphicsCreateInfo();
    const auto& subpass = rp->create_info.pSubpasses[create_info.subpass];

    // NOTE: see PIPELINE_LAYOUT and vku::safe_VkGraphicsPipelineCreateInfo constructors. pColorBlendState and pDepthStencilState
    // are only non-null if they are enabled.
    if (create_info.pColorBlendState && !(pipe_state.ignore_color_attachments)) {
        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info.pColorBlendState->attachmentCount);
        for (uint32_t j = 0; j < num_color_attachments; j++) {
            if (create_info.pColorBlendState->pAttachments[j].colorWriteMask != 0) {
                uint32_t attachment = subpass.pColorAttachments[j].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    result.emplace_back(attachment, VK_IMAGE_ASPECT_COLOR_BIT);
                }
            }
        }
    }

    if (create_info.pDepthStencilState &&
        (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable ||
         create_info.pDepthStencilState->stencilTestEnable)) {
        uint32_t attachment = subpass.pDepthStencilAttachment ? subpass.pDepthStencilAttachment->attachment : VK_ATTACHMENT_UNUSED;
        if (attachment != VK_ATTACHMENT_UNUSED) {
            VkImageAspectFlags aspects = 0;
            if (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable) {
                aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (create_info.pDepthStencilState->stencilTestEnable) {
                aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            result.emplace_back(attachment, aspects);
        }
    }
    return result;
}

void CommandBufferSubState::RecordSetDepthTestStateNV(VkCompareOp new_depth_compare_op, bool new_depth_test_enable) {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    if (nv.depth_compare_op != new_depth_compare_op) {
        switch (new_depth_compare_op) {
            case VK_COMPARE_OP_LESS:
            case VK_COMPARE_OP_LESS_OR_EQUAL:
                nv.zcull_direction = ZcullDirection::Less;
                break;
            case VK_COMPARE_OP_GREATER:
            case VK_COMPARE_OP_GREATER_OR_EQUAL:
                nv.zcull_direction = ZcullDirection::Greater;
                break;
            default:
                // The other ops carry over the previous state.
                break;
        }
    }
    nv.depth_compare_op = new_depth_compare_op;
    nv.depth_test_enable = new_depth_test_enable;
}

void CommandBufferSubState::RecordSetDepthCompareOp(VkCompareOp depth_compare_op) {
    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestStateNV(depth_compare_op, nv.depth_test_enable);
    }
}

void CommandBufferSubState::RecordSetDepthTestEnable(VkBool32 depth_test_enable) {
    if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordSetDepthTestStateNV(nv.depth_compare_op, depth_test_enable);
    }
}

void CommandBufferSubState::RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline& pipeline) {
    // AMD best practice
    validator.PipelineUsedInFrame(pipeline.VkHandle());

    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        render_pass_state.nextDrawTouchesAttachments = GetAttachmentAccess(pipeline);
        render_pass_state.drawTouchAttachments = true;

        const auto* blend_state = pipeline.ColorBlendState();
        const auto* stencil_state = pipeline.DepthStencilState();

        if (blend_state && !(pipeline.ignore_color_attachments)) {
            // assume the pipeline is depth-only unless any of the attachments have color writes enabled
            render_pass_state.depthOnly = true;
            for (size_t i = 0; i < blend_state->attachmentCount; i++) {
                if (blend_state->pAttachments[i].colorWriteMask != 0) {
                    render_pass_state.depthOnly = false;
                }
            }
        }

        // check for depth value usage
        render_pass_state.depthEqualComparison = false;

        if (stencil_state && stencil_state->depthTestEnable) {
            switch (stencil_state->depthCompareOp) {
                case VK_COMPARE_OP_EQUAL:
                case VK_COMPARE_OP_GREATER_OR_EQUAL:
                case VK_COMPARE_OP_LESS_OR_EQUAL:
                    render_pass_state.depthEqualComparison = true;
                    break;
                default:
                    break;
            }
        }

        if (validator.VendorCheckEnabled(kBPVendorNVIDIA)) {
            using TessGeometryMeshState = bp_state::CommandBufferStateNV::TessGeometryMesh::State;
            auto& tgm = nv.tess_geometry_mesh;

            // Make sure the message is only signaled once per command buffer
            tgm.threshold_signaled = tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA;

            // Track pipeline switches with tessellation, geometry, and/or mesh shaders enabled, and disabled
            auto tgm_stages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                              VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
            auto new_tgm_state =
                (pipeline.active_shaders & tgm_stages) != 0 ? TessGeometryMeshState::Enabled : TessGeometryMeshState::Disabled;
            if (tgm.state != new_tgm_state && tgm.state != TessGeometryMeshState::Unknown) {
                tgm.num_switches++;
            }
            tgm.state = new_tgm_state;

            // Track depthTestEnable and depthCompareOp
            auto& pipeline_create_info = pipeline.GraphicsCreateInfo();
            auto depth_stencil_state = pipeline_create_info.pDepthStencilState;
            auto dynamic_state = pipeline_create_info.pDynamicState;
            if (depth_stencil_state && dynamic_state) {
                auto dynamic_state_begin = dynamic_state->pDynamicStates;
                auto dynamic_state_end = dynamic_state->pDynamicStates + dynamic_state->dynamicStateCount;

                const bool dynamic_depth_test_enable =
                    std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) != dynamic_state_end;
                const bool dynamic_depth_func =
                    std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) != dynamic_state_end;

                if (!dynamic_depth_test_enable) {
                    RecordSetDepthTestStateNV(nv.depth_compare_op, depth_stencil_state->depthTestEnable != VK_FALSE);
                }
                if (!dynamic_depth_func) {
                    RecordSetDepthTestStateNV(depth_stencil_state->depthCompareOp, nv.depth_test_enable);
                }
            }
        }
    }
}

void CommandBufferSubState::Submit(vvl::Queue& queue_state, uint32_t perf_submit_pass, const Location& loc) {
    for (auto& func : queue_submit_functions) {
        func(queue_state, base);
    }
}

void CommandBufferSubState::RecordAttachmentAccess(uint32_t attachment, VkImageAspectFlags aspects) {
    // Called when we have a partial clear attachment, or a normal draw call which accesses an attachment.
    auto itr =
        std::find_if(render_pass_state.touchesAttachments.begin(), render_pass_state.touchesAttachments.end(),
                     [attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == attachment; });

    if (itr != render_pass_state.touchesAttachments.end()) {
        itr->aspects |= aspects;
    } else {
        render_pass_state.touchesAttachments.emplace_back(attachment, aspects);
    }
}

void CommandBufferSubState::RecordAttachmentClearAttachments(uint32_t fb_attachment, uint32_t color_attachment,
                                                             VkImageAspectFlags aspects, uint32_t rectCount,
                                                             const VkClearRect* pRects) {
    // If we observe a full clear before any other access to a frame buffer attachment,
    // we have candidate for redundant clear attachments.
    auto itr =
        std::find_if(render_pass_state.touchesAttachments.begin(), render_pass_state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    uint32_t new_aspects = aspects;
    if (itr != render_pass_state.touchesAttachments.end()) {
        new_aspects = aspects & ~itr->aspects;
        itr->aspects |= aspects;
    } else {
        render_pass_state.touchesAttachments.emplace_back(fb_attachment, aspects);
    }

    if (new_aspects == 0) {
        return;
    }

    if (base.IsSecondary()) {
        // The first command might be a clear, but might not be the first in the render pass, defer any checks until
        // CmdExecuteCommands.
        render_pass_state.earlyClearAttachments.push_back(
            {fb_attachment, color_attachment, new_aspects, std::vector<VkClearRect>{pRects, pRects + rectCount}});
    }
}

void CommandBufferSubState::RecordBindZcullScopeNV(VkImage depth_attachment, const VkImageSubresourceRange& subresource_range) {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    if (depth_attachment == VK_NULL_HANDLE) {
        nv.zcull_scope = {};
        return;
    }

    assert((subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U);

    auto image_state = base.dev_data.Get<vvl::Image>(depth_attachment);
    ASSERT_AND_RETURN(image_state);

    const uint32_t mip_levels = image_state->create_info.mipLevels;
    const uint32_t array_layers = image_state->create_info.arrayLayers;

    auto& tree = nv.zcull_per_image[depth_attachment];
    if (tree.states.empty()) {
        tree.mip_levels = mip_levels;
        tree.array_layers = array_layers;
        tree.states.resize(array_layers * mip_levels);
    }

    nv.zcull_scope.image = depth_attachment;
    nv.zcull_scope.range = subresource_range;
    nv.zcull_scope.tree = &tree;
}

void CommandBufferSubState::RecordUnbindZcullScopeNV() {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));
    RecordBindZcullScopeNV(VK_NULL_HANDLE, VkImageSubresourceRange{});
}

void CommandBufferSubState::RecordResetScopeZcullDirectionNV() {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = nv.zcull_scope;
    RecordResetZcullDirectionNV(scope.image, scope.range);
}

template <typename Func>
static void ForEachSubresource(const vvl::Image& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layer_count =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t level_count =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layer_count; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < level_count; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

void CommandBufferSubState::RecordResetZcullDirectionNV(VkImage depth_image, const VkImageSubresourceRange& subresource_range) {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    RecordSetZcullDirectionNV(depth_image, subresource_range, ZcullDirection::Unknown);

    const auto image_it = nv.zcull_per_image.find(depth_image);
    if (image_it == nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = base.dev_data.Get<vvl::Image>(depth_image);
    ASSERT_AND_RETURN(image);

    ForEachSubresource(*image, subresource_range, [&tree](uint32_t layer, uint32_t level) {
        auto& subresource = tree.GetState(layer, level);
        subresource.num_less_draws = 0;
        subresource.num_greater_draws = 0;
    });
}

void CommandBufferSubState::RecordSetZcullDirectionNV(VkImage depth_image, const VkImageSubresourceRange& subresource_range,
                                                      ZcullDirection mode) {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    const auto image_it = nv.zcull_per_image.find(depth_image);
    if (image_it == nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = base.dev_data.Get<vvl::Image>(depth_image);
    ASSERT_AND_RETURN(image);

    ForEachSubresource(*image, subresource_range, [&tree, this](uint32_t layer, uint32_t level) {
        tree.GetState(layer, level).direction = nv.zcull_direction;
    });
}

void CommandBufferSubState::RecordSetScopeZcullDirectionNV(ZcullDirection mode) {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = nv.zcull_scope;
    RecordSetZcullDirectionNV(scope.image, scope.range, mode);
}

void CommandBufferSubState::RecordZcullDrawNV() {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    // Add one draw to each subresource depending on the current Z-cull direction
    auto& scope = nv.zcull_scope;

    auto image = base.dev_data.Get<vvl::Image>(scope.image);
    if (!image) return;

    ForEachSubresource(*image, scope.range, [&scope](uint32_t layer, uint32_t level) {
        auto& subresource = scope.tree->GetState(layer, level);

        switch (subresource.direction) {
            case ZcullDirection::Unknown:
                // Unreachable
                assert(false);
                break;
            case ZcullDirection::Less:
                ++subresource.num_less_draws;
                break;
            case ZcullDirection::Greater:
                ++subresource.num_greater_draws;
                break;
        }
    });
}

void CommandBufferSubState::RecordCmdDrawTypeNVIDIA() {
    assert(validator.VendorCheckEnabled(kBPVendorNVIDIA));

    if (nv.depth_test_enable && nv.zcull_direction != ZcullDirection::Unknown) {
        RecordSetScopeZcullDirectionNV(nv.zcull_direction);
        RecordZcullDrawNV();
    }
}

}  // namespace bp_state