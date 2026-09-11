/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync/sync_dynamic_rendering.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/pipeline_state.h"
#include <vulkan/utility/vk_format_utils.h>

namespace syncval {

constexpr SyncAccessIndex kResolveRead = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ;
constexpr SyncAccessIndex kResolveWrite = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
constexpr SyncOrdering kColorResolveOrder = SyncOrdering::kColorAttachment;

// Although depth resolve runs on the color attachment output stage and uses color accesses, depth accesses
// still participate in the ordering. That's why using raster and not only color attachment ordering
constexpr SyncOrdering kDepthStencilResolveOrder = SyncOrdering::kRaster;

constexpr SyncOrdering kStoreOrder = SyncOrdering::kRaster;

RenderingAttachment::RenderingAttachment(const SyncValidator& validator, const VkRenderingAttachmentInfo& info,
                                         const AttachmentType type)
    : type(type), load_op(info.loadOp), store_op(info.storeOp), resolve_mode(info.resolveMode) {
    view = validator.Get<vvl::ImageView>(info.imageView);
    if (view && info.resolveImageView != VK_NULL_HANDLE && resolve_mode != VK_RESOLVE_MODE_NONE) {
        resolve_view = validator.Get<vvl::ImageView>(info.resolveImageView);
    }
}

ImageRangeGen RenderingAttachment::GetRangeGen(const VkRect2D& render_area, uint32_t view_mask) const {
    // VkRenderingAttachmentInfo::imageView is allowed to be VK_NULL_HANDLE
    if (!view) {
        return {};
    }
    // Multiview is disabled: return range gen for render area
    if (view_mask == 0) {
        const VkOffset3D offset = CastTo3D(render_area.offset);
        const VkExtent3D extent = CastTo3D(render_area.extent);
        if (type == AttachmentType::kColor) {
            return MakeImageRangeGen(*view, offset, extent);
        } else if (type == AttachmentType::kDepth) {
            return MakeImageRangeGen(*view, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT);
        } else {
            return MakeImageRangeGen(*view, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT);
        }
    }
    // Initialize range gen based on view mask. The render area is not applied on purpose:
    // load/store/resolve operations can access the entire attachment subresource.
    // The non-multiview path keeps the original render-area model.
    if (type == AttachmentType::kColor) {
        return MakeImageRangeGen(*view, view_mask);
    } else if (type == AttachmentType::kDepth) {
        return MakeImageRangeGen(*view, view_mask, VK_IMAGE_ASPECT_DEPTH_BIT);
    } else {
        return MakeImageRangeGen(*view, view_mask, VK_IMAGE_ASPECT_STENCIL_BIT);
    }
}

ImageRangeGen RenderingAttachment::GetResolveRangeGen(const VkRect2D& render_area) const {
    const VkOffset3D offset = CastTo3D(render_area.offset);
    const VkExtent3D extent = CastTo3D(render_area.extent);
    if (type == AttachmentType::kColor) {
        return MakeImageRangeGen(*resolve_view, offset, extent);
    } else if (type == AttachmentType::kDepth) {
        // Select only the depth aspect
        return MakeImageRangeGen(*resolve_view, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT);
    } else {
        return MakeImageRangeGen(*resolve_view, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT);
    }
}

SyncAccessIndex RenderingAttachment::GetLoadUsage() const {
    if (load_op == VK_ATTACHMENT_LOAD_OP_NONE) {
        return SYNC_ACCESS_INDEX_NONE;
    } else if (type == AttachmentType::kColor) {
        return load_op == VK_ATTACHMENT_LOAD_OP_LOAD ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                     : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        return load_op == VK_ATTACHMENT_LOAD_OP_LOAD ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                     : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
}

SyncAccessIndex RenderingAttachment::GetStoreUsage() const {
    if (store_op == VK_ATTACHMENT_STORE_OP_NONE) {
        return SYNC_ACCESS_INDEX_NONE;
    } else if (type == AttachmentType::kColor) {
        return SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        return SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
}

SyncOrdering RenderingAttachment::GetOrdering() const {
    return type == AttachmentType::kColor ? SyncOrdering::kColorAttachment : SyncOrdering::kDepthStencilAttachment;
}

Location RenderingAttachment::GetLocation(uint32_t attachment_index, vvl::Func function) const {
    if (type == AttachmentType::kColor) {
        return Location(function, vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pColorAttachments, attachment_index);
    } else if (type == AttachmentType::kDepth) {
        return Location(function, vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pDepthAttachment);
    } else {
        assert(type == AttachmentType::kStencil);
        return Location(function, vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pStencilAttachment);
    }
}

bool RenderingAttachment::IsWriteable(bool depth_write, bool stencil_write) const {
    if (!view) {
        return false;
    }
    if (type == AttachmentType::kColor) {
        return true;
    } else if (type == AttachmentType::kDepth) {
        return depth_write && vkuFormatHasDepth(view->create_info.format);
    } else {
        assert(type == AttachmentType::kStencil);
        return stencil_write && vkuFormatHasStencil(view->create_info.format);
    }
}

std::vector<RenderingAttachment> CollectAttachments(const SyncValidator& validator, const VkRenderingInfo& rendering_info) {
    uint32_t attachment_count = rendering_info.colorAttachmentCount;
    attachment_count += rendering_info.pDepthAttachment ? 1 : 0;
    attachment_count += rendering_info.pStencilAttachment ? 1 : 0;

    std::vector<RenderingAttachment> attachments;
    attachments.reserve(attachment_count);

    for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; i++) {
        attachments.emplace_back(validator, rendering_info.pColorAttachments[i], AttachmentType::kColor);
    }
    if (rendering_info.pDepthAttachment) {
        attachments.emplace_back(validator, *rendering_info.pDepthAttachment, AttachmentType::kDepth);
    }
    if (rendering_info.pStencilAttachment) {
        attachments.emplace_back(validator, *rendering_info.pStencilAttachment, AttachmentType::kStencil);
    }
    return attachments;
}

void RenderingInstance::InitViewGens(std::vector<ImageRangeGen>& view_gen_storage) {
    view_gen_storage.clear();
    view_gen_storage.reserve(attachments.size());
    for (const RenderingAttachment& attachment : attachments) {
        view_gen_storage.emplace_back(attachment.GetRangeGen(render_area, view_mask));
    }
    view_gens = view_gen_storage;  // init span
}

const vvl::ImageView* RenderingInstance::GetClearAttachmentView(const VkClearAttachment& clear_attachment) const {
    if (clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (clear_attachment.colorAttachment < color_attachment_count) {
            return attachments[clear_attachment.colorAttachment].view.get();
        }
    } else if (clear_attachment.aspectMask & kDepthStencilAspects) {
        if (attachments.size() > color_attachment_count) {
            // If both depth and stencil attachments are defined they must both point to the same view
            return attachments.back().view.get();
        }
    }
    return nullptr;
}

bool RenderingInstance::ValidateBeginRendering(const SyncEnvironment& env, const AccessContext& access_context,
                                               const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                               const Location& loc, uint32_t render_pass_instance_id) const {
    assert(view_gens.size() == attachments.size());
    bool skip = false;
    const SyncValidator& validator = env.validator;

    if (flags & VK_RENDERING_RESUMING_BIT) {
        // Load operations do not happen when resuming
        return skip;
    }
    for (size_t i = 0; i < attachments.size(); i++) {
        const RenderingAttachment& attachment = attachments[i];
        const SyncAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) {
            continue;
        }
        const AttachmentAccess attachment_access = {AttachmentAccessType::LoadOp, attachment.GetOrdering(),
                                                    render_pass_instance_id};
        ImageRangeGen range_gen = view_gens[i];
        const HazardResult hazard = access_context.DetectAttachmentHazard(range_gen, load_index, attachment_access, env.queue_id);
        if (hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.view->Handle());

            std::ostringstream ss;
            ss << vvl::String(vvl::Field::pRenderingInfo) << ".";
            ss << attachment.GetLocation(uint32_t(i)).Fields();
            ss << " (" << validator.FormatHandle(attachment.view->Handle());
            ss << ", loadOp " << string_VkAttachmentLoadOp(attachment.load_op) << ")";
            std::string resource_description = ss.str();

            const std::string error = validator.error_messages_.BeginRenderingError(env, hazard, cb_context, replay_tag, loc,
                                                                                    resource_description, attachment.load_op);
            skip |= validator.SyncError(hazard.Hazard(), objlist, loc, error);
            if (skip) {
                break;
            }
        }
    }
    return skip;
}

void RenderingInstance::RecordBeginRendering(AccessContext& access_context, uint32_t render_pass_instance_id, ResourceUsageTag tag,
                                             QueueId queue_id) const {
    assert(view_gens.size() == attachments.size());
    if ((flags & VK_RENDERING_RESUMING_BIT) != 0) {
        return;
    }
    for (size_t i = 0; i < attachments.size(); i++) {
        const RenderingAttachment& attachment = attachments[i];
        const SyncAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) {
            continue;
        }
        ImageRangeGen range_gen = view_gens[i];
        const AttachmentAccess attachment_access = {AttachmentAccessType::LoadOp, attachment.GetOrdering(),
                                                    render_pass_instance_id};
        access_context.UpdateAttachmentAccessState(range_gen, load_index, attachment_access, ResourceUsageTagEx{tag}, queue_id);
    }
}

bool RenderingInstance::ValidateEndRendering(const SyncEnvironment& env, const AccessContext& access_context,
                                             const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                             const Location& loc, uint32_t render_pass_instance_id) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    if ((flags & VK_RENDERING_SUSPENDING_BIT) != 0) {
        // Resolve/store operations do not happen when suspending
        return skip;
    }
    for (uint32_t i = 0; i < (uint32_t)attachments.size(); i++) {
        const RenderingAttachment& attachment = attachments[i];

        auto attachment_description = [&validator, &attachment, i](const auto& view, std::ostringstream& ss) {
            ss << vvl::String(vvl::Field::pRenderingInfo) << ".";
            ss << attachment.GetLocation(uint32_t(i)).Fields();
            ss << " (" << validator.FormatHandle(view->Handle());
        };

        // Validate resolve. Resolve view is available only when resolve operation is enabled
        if (attachment.resolve_view) {
            const bool is_color = attachment.type == AttachmentType::kColor;
            const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;

            const AttachmentAccess resolve_read_access = {AttachmentAccessType::ResolveRead, kResolveOrder,
                                                          render_pass_instance_id};
            ImageRangeGen view_gen = view_gens[i];
            HazardResult hazard = access_context.DetectAttachmentHazard(view_gen, kResolveRead, resolve_read_access, env.queue_id);
            if (hazard.IsHazard()) {
                const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.view->Handle());

                std::ostringstream ss;
                attachment_description(attachment.view, ss);
                ss << ", resolveMode " << string_VkResolveModeFlagBits(attachment.resolve_mode) << ")";
                const std::string resource_description = ss.str();

                const std::string error = validator.error_messages_.EndRenderingResolveError(
                    env, hazard, cb_context, replay_tag, loc, resource_description, attachment.resolve_mode, false);
                skip |= validator.SyncError(hazard.Hazard(), objlist, loc, error);
                if (skip) {
                    break;
                }
            }

            const AttachmentAccess resolve_write_access = {AttachmentAccessType::ResolveWrite, kResolveOrder,
                                                           render_pass_instance_id};
            ImageRangeGen resolve_gen = attachment.GetResolveRangeGen(render_area);
            hazard = access_context.DetectAttachmentHazard(resolve_gen, kResolveWrite, resolve_write_access, env.queue_id);
            if (hazard.IsHazard()) {
                const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.resolve_view->Handle());

                std::ostringstream ss;
                attachment_description(attachment.resolve_view, ss);
                ss << ", resolveMode " << string_VkResolveModeFlagBits(attachment.resolve_mode) << ")";
                const std::string resource_description = ss.str();

                const std::string error = validator.error_messages_.EndRenderingResolveError(
                    env, hazard, cb_context, replay_tag, loc, resource_description, attachment.resolve_mode, true);
                skip |= validator.SyncError(hazard.Hazard(), objlist, loc, error);
                if (skip) {
                    break;
                }
            }
        }

        // Validate store
        const SyncAccessIndex store_access = attachment.GetStoreUsage();
        if (store_access != SYNC_ACCESS_INDEX_NONE) {
            const AttachmentAccess attachment_access = {AttachmentAccessType::StoreOp, kStoreOrder, render_pass_instance_id};
            ImageRangeGen view_gen = view_gens[i];

            HazardResult hazard = access_context.DetectAttachmentHazard(view_gen, store_access, attachment_access, env.queue_id);
            if (hazard.IsHazard()) {
                const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.view->Handle());

                std::ostringstream ss;
                attachment_description(attachment.view, ss);
                ss << ", storeOp " << string_VkAttachmentStoreOp(attachment.store_op) << ")";
                const std::string resource_description = ss.str();

                const std::string error = validator.error_messages_.EndRenderingStoreError(
                    env, hazard, cb_context, replay_tag, loc, resource_description, attachment.store_op);
                skip |= validator.SyncError(hazard.Hazard(), objlist, loc, error);
                if (skip) {
                    break;
                }
            }
        }
    }
    return skip;
}

void RenderingInstance::RecordEndRendering(AccessContext& access_context, uint32_t render_pass_instance_id, ResourceUsageTag tag,
                                           QueueId queue_id) const {
    if ((flags & VK_RENDERING_SUSPENDING_BIT) != 0) {
        return;
    }
    for (size_t i = 0; i < attachments.size(); i++) {
        const RenderingAttachment& attachment = attachments[i];
        if (attachment.resolve_view) {
            const bool is_color = attachment.type == AttachmentType::kColor;
            const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;

            const AttachmentAccess resolve_read_access = {AttachmentAccessType::ResolveRead, kResolveOrder,
                                                          render_pass_instance_id};
            ImageRangeGen view_gen = view_gens[i];
            access_context.UpdateAttachmentAccessState(view_gen, kResolveRead, resolve_read_access, ResourceUsageTagEx{tag},
                                                       queue_id);

            const AttachmentAccess resolve_write_access = {AttachmentAccessType::ResolveWrite, kResolveOrder,
                                                           render_pass_instance_id};
            ImageRangeGen resolve_gen = attachment.GetResolveRangeGen(render_area);
            access_context.UpdateAttachmentAccessState(resolve_gen, kResolveWrite, resolve_write_access, ResourceUsageTagEx{tag},
                                                       queue_id);
        }

        const SyncAccessIndex store_index = attachment.GetStoreUsage();
        if (store_index != SYNC_ACCESS_INDEX_NONE) {
            const AttachmentAccess attachment_access = {AttachmentAccessType::StoreOp, kStoreOrder, render_pass_instance_id};
            ImageRangeGen view_gen = view_gens[i];
            access_context.UpdateAttachmentAccessState(view_gen, store_index, attachment_access, ResourceUsageTagEx{tag}, queue_id);
        }
    }
}

bool RenderingInstance::ValidateDrawAttachments(const SyncEnvironment& env, const AccessContext& access_context,
                                                const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                                const Location& loc, uint32_t render_pass_instance_id,
                                                const vvl::Pipeline* pipeline, bool depth_write, bool stencil_write) const {
    bool skip = false;
    if (!pipeline || pipeline->RasterizationDisabled()) {
        return skip;
    }
    const auto& list = pipeline->fs_writable_output_location_list;
    const SyncValidator& validator = env.validator;

    for (const uint32_t output_location : list) {
        if (output_location >= color_attachment_count) {
            continue;
        }
        const auto& attachment = attachments[output_location];
        if (!attachment.IsWriteable(depth_write, stencil_write)) {
            continue;
        }
        const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kColorAttachment,
                                                 render_pass_instance_id, vvl::kNoIndex32};
        ImageRangeGen view_gen = view_gens[output_location];
        HazardResult hazard = access_context.DetectAttachmentHazard(view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                                    attachment_access, env.queue_id);

        if (hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.view->Handle());
            const Location attachment_loc = attachment.GetLocation(output_location, loc.function);
            const Location location = replay_tag == kInvalidTag ? attachment_loc.dot(vvl::Field::imageView) : loc;
            const std::string error = validator.error_messages_.DynamicRenderingAttachmentError(
                env, hazard, cb_context, replay_tag, loc, validator.FormatHandle(*attachment.view));
            skip |= validator.SyncError(hazard.Hazard(), objlist, location, error);
        }
    }

    // NOTE: these are the old todos, need to reevaluate which ones are needed
    // TODO:fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // TODO: Add layout based read/vs. write selection.
    // TODO: Read operations for both depth and stencil are possible in the future.
    // TODO: Add EARLY stage detection based on ExecutionMode.
    for (size_t i = color_attachment_count; i < attachments.size(); i++) {
        const RenderingAttachment& attachment = attachments[i];
        const bool writeable = attachment.IsWriteable(depth_write, stencil_write);

        if (writeable) {
            const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kDepthStencilAttachment,
                                                     render_pass_instance_id, vvl::kNoIndex32};
            ImageRangeGen view_gen = view_gens[i];
            HazardResult hazard = access_context.DetectAttachmentHazard(
                view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access, env.queue_id);

            if (hazard.IsHazard()) {
                const LogObjectList objlist = BaseObjectList(env, cb_context, attachment.view->Handle());
                const Location attachment_loc = attachment.GetLocation(uint32_t(i), loc.function);
                const Location location = replay_tag == kInvalidTag ? attachment_loc.dot(vvl::Field::imageView) : loc;
                const std::string error = validator.error_messages_.DynamicRenderingAttachmentError(
                    env, hazard, cb_context, replay_tag, loc, validator.FormatHandle(*attachment.view));
                skip |= validator.SyncError(hazard.Hazard(), objlist, location, error);
            }
        }
    }
    return skip;
}

void RenderingInstance::RecordDrawAttachments(AccessContext& access_context, uint32_t render_pass_instance_id,
                                              const vvl::Pipeline* pipeline, bool depth_write, bool stencil_write,
                                              ResourceUsageTag tag, QueueId queue_id) const {
    if (!pipeline || pipeline->RasterizationDisabled()) {
        return;
    }
    const auto& list = pipeline->fs_writable_output_location_list;

    for (const uint32_t output_location : list) {
        if (output_location >= color_attachment_count) {
            continue;
        }
        const auto& attachment = attachments[output_location];
        if (!attachment.IsWriteable(depth_write, stencil_write)) {
            continue;
        }
        const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kColorAttachment,
                                                 render_pass_instance_id, vvl::kNoIndex32};
        ImageRangeGen view_gen = view_gens[output_location];
        access_context.UpdateAttachmentAccessState(view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, attachment_access,
                                                   ResourceUsageTagEx{tag}, queue_id);
    }

    // NOTE: these are the old todos, need to reevaluate which ones are needed
    // TODO: fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // TODO: Add layout based read/vs. write selection.
    // TODO: Read operations for both depth and stencil are possible in the future.
    // TODO: Add EARLY stage detection based on ExecutionMode.
    const uint32_t attachment_count = static_cast<uint32_t>(attachments.size());
    for (uint32_t i = color_attachment_count; i < attachment_count; i++) {
        const RenderingAttachment& attachment = attachments[i];
        const bool writeable = attachment.IsWriteable(depth_write, stencil_write);

        if (writeable) {
            const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kDepthStencilAttachment,
                                                     render_pass_instance_id, vvl::kNoIndex32};
            ImageRangeGen view_gen = view_gens[i];
            access_context.UpdateAttachmentAccessState(view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                       attachment_access, ResourceUsageTagEx{tag}, queue_id);
        }
    }
}

}  // namespace syncval
