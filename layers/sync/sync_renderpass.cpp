/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_renderpass.h"
#include "sync/sync_validation.h"
#include "sync/sync_op.h"
#include "sync/sync_image.h"
#include "state_tracker/render_pass_state.h"

// Action for validating resolve operations
class ValidateResolveAction {
  public:
    ValidateResolveAction(VkRenderPass render_pass, uint32_t subpass, const AccessContext &context,
                          const CommandBufferAccessContext &cb_context, vvl::Func command)
        : render_pass_(render_pass),
          subpass_(subpass),
          context_(context),
          cb_context_(cb_context),
          command_(command),
          skip_(false) {}

    void operator()(const char *aspect_name, const char *resolve_action_name, uint32_t src_at, uint32_t dst_at,
                    const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                    SyncOrdering ordering_rule) {
        const HazardResult hazard = context_.DetectHazard(view_gen, gen_type, current_usage, ordering_rule);
        if (hazard.IsHazard()) {
            const Location loc(command_);

            std::stringstream ss;
            ss << view_gen.GetViewState()->Handle();
            ss << " (" << aspect_name << " " << resolve_action_name;
            ss << ", attachment " << src_at;
            ss << ", resolve attachment " << dst_at;
            ss << ", subpass " << subpass_ << ")";
            const std::string resource_description = ss.str();
            const auto error = cb_context_.GetSyncState().error_messages_.RenderPassResolveError(hazard, cb_context_, command_,
                                                                                                 resource_description);
            skip_ |= cb_context_.GetSyncState().SyncError(hazard.Hazard(), render_pass_, loc, error);
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const AccessContext &context_;
    const CommandBufferAccessContext &cb_context_;
    vvl::Func command_;
    bool skip_;
};

// Update action for resolve operations
class UpdateStateResolveAction {
  public:
    UpdateStateResolveAction(AccessContext &context, ResourceUsageTag tag) : context_(context), tag_(tag) {}
    void operator()(const char *, const char *, uint32_t, uint32_t, const AttachmentViewGen &view_gen,
                    AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage, SyncOrdering ordering_rule) {
        // Ignores validation only arguments...
        context_.UpdateAccessState(view_gen, gen_type, current_usage, ordering_rule, tag_);
    }

  private:
    AccessContext &context_;
    const ResourceUsageTag tag_;
};

void InitSubpassContexts(VkQueueFlags queue_flags, const vvl::RenderPass &rp_state, const AccessContext *external_context,
                         std::vector<AccessContext> &subpass_contexts) {
    const auto &create_info = rp_state.create_info;
    // Add this for all subpasses here so that they exsist during next subpass validation
    subpass_contexts.clear();
    subpass_contexts.reserve(create_info.subpassCount);
    for (uint32_t pass = 0; pass < create_info.subpassCount; pass++) {
        subpass_contexts.emplace_back(pass, queue_flags, rp_state.subpass_dependencies, subpass_contexts, external_context);
    }
}

static SyncAccessIndex GetLoadOpUsageIndex(VkAttachmentLoadOp load_op, syncval_state::AttachmentType type) {
    SyncAccessIndex access_index;
    if (load_op == VK_ATTACHMENT_LOAD_OP_NONE) {
        access_index = SYNC_ACCESS_INDEX_NONE;
    } else if (type == syncval_state::AttachmentType::kColor) {
        access_index = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                               : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        access_index = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                               : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
    return access_index;
}

static SyncAccessIndex GetStoreOpUsageIndex(VkAttachmentStoreOp store_op, syncval_state::AttachmentType type) {
    SyncAccessIndex access_index;
    if (store_op == VK_ATTACHMENT_STORE_OP_NONE) {
        access_index = SYNC_ACCESS_INDEX_NONE;
    } else if (type == syncval_state::AttachmentType::kColor) {
        access_index = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        access_index = SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
    return access_index;
}

static SyncAccessIndex ColorLoadUsage(VkAttachmentLoadOp load_op) {
    return GetLoadOpUsageIndex(load_op, syncval_state::AttachmentType::kColor);
}
static SyncAccessIndex DepthStencilLoadUsage(VkAttachmentLoadOp load_op) {
    return GetLoadOpUsageIndex(load_op, syncval_state::AttachmentType::kDepth);
}

// Caller must manage returned pointer
static AccessContext *CreateStoreResolveProxyContext(const AccessContext &context, const vvl::RenderPass &rp_state,
                                                     uint32_t subpass, const AttachmentViewGenVector &attachment_views) {
    auto *proxy = new AccessContext(context);
    RenderPassAccessContext::UpdateAttachmentResolveAccess(rp_state, attachment_views, subpass, kInvalidTag, *proxy);
    RenderPassAccessContext::UpdateAttachmentStoreAccess(rp_state, attachment_views, subpass, kInvalidTag, *proxy);
    return proxy;
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool RenderPassAccessContext::ValidateLayoutTransitions(const CommandBufferAccessContext &cb_context,
                                                        const AccessContext &access_context, const vvl::RenderPass &rp_state,
                                                        const VkRect2D &render_area, uint32_t subpass,
                                                        const AttachmentViewGenVector &attachment_views, vvl::Func command) {
    bool skip = false;
    // As validation methods are const and precede the record/update phase, for any tranistions from the immediately
    // previous subpass, we have to validate them against a copy of the AccessContext, with resolve operations applied, as
    // those affects have not been recorded yet.
    //
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> proxy_for_prev;
    AccessContext::TrackBack proxy_track_back;

    const auto &transitions = rp_state.subpass_transitions[subpass];
    for (const auto &transition : transitions) {
        const bool prev_needs_proxy = transition.prev_pass != VK_SUBPASS_EXTERNAL && (transition.prev_pass + 1 == subpass);

        const auto *track_back = access_context.GetTrackBackFromSubpass(transition.prev_pass);
        assert(track_back);
        if (prev_needs_proxy) {
            if (!proxy_for_prev) {
                proxy_for_prev.reset(
                    CreateStoreResolveProxyContext(*track_back->source_subpass, rp_state, transition.prev_pass, attachment_views));
                proxy_track_back = *track_back;
                proxy_track_back.source_subpass = proxy_for_prev.get();
            }
            track_back = &proxy_track_back;
        }
        auto hazard = access_context.DetectSubpassTransitionHazard(*track_back, attachment_views[transition.attachment]);
        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = cb_context.GetSyncState();
            const Location loc(command);

            const syncval_state::ImageViewState *attachment_view = attachment_views[transition.attachment].GetViewState();
            std::stringstream ss;
            ss << "in subpass " << subpass << " on attachment " << transition.attachment << " (";
            ss << sync_state.FormatHandle(attachment_view->Handle());
            ss << ", " << sync_state.FormatHandle(attachment_view->GetImageState()->Handle());
            ss << ", oldLayout " << string_VkImageLayout(transition.old_layout);
            ss << ", newLayout " << string_VkImageLayout(transition.new_layout);
            ss << ")";
            const std::string resource_description = ss.str();

            if (hazard.Tag() == kInvalidTag) {
                const auto error = sync_state.error_messages_.RenderPassLayoutTransitionVsStoreOrResolveError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout,
                    transition.prev_pass);
                skip |= sync_state.SyncError(hazard.Hazard(), rp_state.Handle(), loc, error);
            } else {
                const auto error = sync_state.error_messages_.RenderPassLayoutTransitionError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout);
                skip |= sync_state.SyncError(hazard.Hazard(), rp_state.Handle(), loc, error);
            }
        }
    }
    return skip;
}

bool RenderPassAccessContext::ValidateLoadOperation(const CommandBufferAccessContext &cb_context,
                                                    const AccessContext &access_context, const vvl::RenderPass &rp_state,
                                                    const VkRect2D &render_area, uint32_t subpass,
                                                    const AttachmentViewGenVector &attachment_views, vvl::Func command) {
    bool skip = false;
    const auto *attachment_ci = rp_state.create_info.pAttachments;

    for (uint32_t i = 0; i < rp_state.create_info.attachmentCount; i++) {
        if (subpass == rp_state.attachment_first_subpass[i]) {
            const auto &view_gen = attachment_views[i];
            if (!view_gen.IsValid()) continue;
            const auto &ci = attachment_ci[i];

            // Need check in the following way
            // 1) if the usage bit isn't in the dest_access_scope, and there is layout traniition for initial use, report hazard
            //    vs. transition
            // 2) if there isn't a layout transition, we need to look at the  external context with a "detect hazard" operation
            //    for each aspect loaded.

            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            const SyncAccessIndex load_index = has_depth ? DepthStencilLoadUsage(ci.loadOp) : ColorLoadUsage(ci.loadOp);
            const SyncAccessIndex stencil_load_index = has_stencil ? DepthStencilLoadUsage(ci.stencilLoadOp) : load_index;

            HazardResult hazard;
            const char *aspect = nullptr;

            bool checked_stencil = false;
            if (is_color && (load_index != SYNC_ACCESS_INDEX_NONE)) {
                hazard = access_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea, load_index,
                                                     SyncOrdering::kColorAttachment);
                aspect = "color";
            } else {
                if (has_depth && (load_index != SYNC_ACCESS_INDEX_NONE)) {
                    hazard = access_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea, load_index,
                                                         SyncOrdering::kDepthStencilAttachment);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && (stencil_load_index != SYNC_ACCESS_INDEX_NONE)) {
                    hazard = access_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                         stencil_load_index, SyncOrdering::kDepthStencilAttachment);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                const VkAttachmentLoadOp load_op = checked_stencil ? ci.stencilLoadOp : ci.loadOp;
                const SyncValidator &sync_state = cb_context.GetSyncState();
                const Location loc(command);

                std::stringstream ss;
                ss << "the " << aspect << " aspect of attachment " << i << " in subpass " << subpass;
                ss << " (" << sync_state.FormatHandle(view_gen.GetViewState()->Handle());
                ss << ", loadOp " << string_VkAttachmentLoadOp(load_op) << ")";
                const std::string resource_description = ss.str();

                if (hazard.Tag() == kInvalidTag) {  // Hazard vs. ILT
                    const auto error = sync_state.error_messages_.RenderPassLoadOpVsLayoutTransitionError(
                        hazard, cb_context, command, resource_description, load_op, is_color);
                    skip |= sync_state.SyncError(hazard.Hazard(), rp_state.Handle(), loc, error);
                } else {
                    const std::string error = sync_state.error_messages_.RenderPassLoadOpError(
                        hazard, cb_context, command, resource_description, subpass, i, load_op, is_color);
                    skip |= sync_state.SyncError(hazard.Hazard(), rp_state.Handle(), loc, error);
                }
            }
        }
    }
    return skip;
}

// Store operation validation can ignore resolve (before it) and layout tranistions after it.  The first is ignored
// because of the ordering guarantees w.r.t. sample access and that the resolve validation hasn't altered the state, because
// store is part of the same Next/End operation.
// The latter is handled in layout transistion validation directly
bool RenderPassAccessContext::ValidateStoreOperation(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    bool skip = false;
    const auto *attachment_ci = rp_state_->create_info.pAttachments;

    for (uint32_t i = 0; i < rp_state_->create_info.attachmentCount; i++) {
        if (current_subpass_ == rp_state_->attachment_last_subpass[i]) {
            const AttachmentViewGen &view_gen = attachment_views_[i];
            if (!view_gen.IsValid()) continue;
            const auto &ci = attachment_ci[i];

            // The spec states that "don't care" is an operation with VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // so we assume that an implementation is *free* to write in that case, meaning that for correctness
            // sake, we treat DONT_CARE as writing.
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE;
            if (!has_stencil && !store_op_stores) continue;

            HazardResult hazard;
            const char *aspect = nullptr;
            bool checked_stencil = false;
            if (is_color) {
                hazard = CurrentContext().DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                       SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster);
                aspect = "color";
            } else {
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE;
                if (has_depth && store_op_stores) {
                    hazard = CurrentContext().DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                           SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                           SyncOrdering::kRaster);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && stencil_op_stores) {
                    hazard = CurrentContext().DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                           SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                           SyncOrdering::kRaster);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                const SyncValidator &sync_state = cb_context.GetSyncState();
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const VkAttachmentStoreOp store_op = checked_stencil ? ci.stencilStoreOp : ci.storeOp;
                const Location loc(command);

                std::stringstream ss;
                ss << sync_state.FormatHandle(view_gen.GetViewState()->Handle());
                ss << " (subpass " << current_subpass_;
                ss << ", attachment " << i;
                ss << ", aspect " << aspect << " during store with " << op_type_string;
                ss << " " << string_VkAttachmentStoreOp(store_op) << ")";
                const std::string resource_description = ss.str();

                const std::string error =
                    sync_state.error_messages_.RenderPassStoreOpError(hazard, cb_context, command, resource_description, store_op);
                skip |= sync_state.SyncError(hazard.Hazard(), rp_state_->Handle(), loc, error);
            }
        }
    }
    return skip;
}

bool IsImageLayoutDepthWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}

bool IsImageLayoutStencilWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
}

bool IsDepthAttachmentWriteable(const LastBound &last_bound_state, const VkFormat format, const VkImageLayout layout) {
    // PHASE1 TODO: These validation should be in core_checks.
    const bool depth_write_enable = last_bound_state.IsDepthWriteEnable();  // implicitly means DepthTestEnable is set
    return !vkuFormatIsStencilOnly(format) && depth_write_enable && IsImageLayoutDepthWritable(layout);
}

bool IsStencilAttachmentWriteable(const LastBound &last_bound_state, const VkFormat format, const VkImageLayout layout) {
    // PHASE1 TODO: It needs to check if stencil is writable.
    //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
    //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
    // PHASE1 TODO: These validation should be in core_checks.
    const bool stencil_test_enable = last_bound_state.IsStencilTestEnable();
    return !vkuFormatIsDepthOnly(format) && stencil_test_enable && IsImageLayoutStencilWritable(layout);
}

// Traverse the attachment resolves for this a specific subpass, and do action() to them.
// Used by both validation and record operations
//
// The signature for Action() reflect the needs of both uses.
template <typename Action>
void ResolveOperation(Action &action, const vvl::RenderPass &rp_state, const AttachmentViewGenVector &attachment_views,
                      uint32_t subpass) {
    const auto &rp_ci = rp_state.create_info;
    const auto *attachment_ci = rp_ci.pAttachments;
    const auto &subpass_ci = rp_ci.pSubpasses[subpass];

    // Color resolves -- require an inuse color attachment and a matching inuse resolve attachment
    const auto *color_attachments = subpass_ci.pColorAttachments;
    const auto *color_resolve = subpass_ci.pResolveAttachments;
    if (color_resolve && color_attachments) {
        for (uint32_t i = 0; i < subpass_ci.colorAttachmentCount; i++) {
            const auto &color_attach = color_attachments[i].attachment;
            const auto &resolve_attach = subpass_ci.pResolveAttachments[i].attachment;
            if ((color_attach != VK_ATTACHMENT_UNUSED) && (resolve_attach != VK_ATTACHMENT_UNUSED)) {
                action("color", "resolve read", color_attach, resolve_attach, attachment_views[color_attach],
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ,
                       SyncOrdering::kColorAttachment);
                action("color", "resolve write", color_attach, resolve_attach, attachment_views[resolve_attach],
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                       SyncOrdering::kColorAttachment);
            }
        }
    }

    // Depth stencil resolve only if the extension is present
    const auto ds_resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass_ci.pNext);
    if (ds_resolve && ds_resolve->pDepthStencilResolveAttachment &&
        (ds_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) && subpass_ci.pDepthStencilAttachment &&
        (subpass_ci.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
        const auto src_at = subpass_ci.pDepthStencilAttachment->attachment;
        const auto src_ci = attachment_ci[src_at];
        // The formats are required to match so we can pick either
        const bool resolve_depth = (ds_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) && vkuFormatHasDepth(src_ci.format);
        const bool resolve_stencil = (ds_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE) && vkuFormatHasStencil(src_ci.format);
        const auto dst_at = ds_resolve->pDepthStencilResolveAttachment->attachment;

        // Figure out which aspects are actually touched during resolve operations
        const char *aspect_string = nullptr;
        AttachmentViewGen::Gen gen_type = AttachmentViewGen::Gen::kRenderArea;
        if (resolve_depth && resolve_stencil) {
            aspect_string = "depth/stencil";
        } else if (resolve_depth) {
            // Validate depth only
            gen_type = AttachmentViewGen::Gen::kDepthOnlyRenderArea;
            aspect_string = "depth";
        } else if (resolve_stencil) {
            // Validate all stencil only
            gen_type = AttachmentViewGen::Gen::kStencilOnlyRenderArea;
            aspect_string = "stencil";
        }

        if (aspect_string) {
            action(aspect_string, "resolve read", src_at, dst_at, attachment_views[src_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, SyncOrdering::kRaster);
            action(aspect_string, "resolve write", src_at, dst_at, attachment_views[dst_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster);
        }
    }
}

bool RenderPassAccessContext::ValidateResolveOperations(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    ValidateResolveAction validate_action(rp_state_->VkHandle(), current_subpass_, CurrentContext(), cb_context, command);
    ResolveOperation(validate_action, *rp_state_, attachment_views_, current_subpass_);
    return validate_action.GetSkip();
}

void RenderPassAccessContext::UpdateAttachmentResolveAccess(const vvl::RenderPass &rp_state,
                                                            const AttachmentViewGenVector &attachment_views, uint32_t subpass,
                                                            const ResourceUsageTag tag, AccessContext access_context) {
    UpdateStateResolveAction update(access_context, tag);
    ResolveOperation(update, rp_state, attachment_views, subpass);
}

void RenderPassAccessContext::UpdateAttachmentStoreAccess(const vvl::RenderPass &rp_state,
                                                          const AttachmentViewGenVector &attachment_views, uint32_t subpass,
                                                          const ResourceUsageTag tag, AccessContext &access_context) {
    const auto *attachment_ci = rp_state.create_info.pAttachments;

    for (uint32_t i = 0; i < rp_state.create_info.attachmentCount; i++) {
        if (rp_state.attachment_last_subpass[i] == subpass) {
            const auto &view_gen = attachment_views[i];
            if (!view_gen.IsValid()) continue;  // UNUSED

            const auto &ci = attachment_ci[i];
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE;

            if (is_color && store_op_stores) {
                access_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                 SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster, tag);
            } else {
                if (has_depth && store_op_stores) {
                    access_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                     SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster,
                                                     tag);
                }
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE;
                if (has_stencil && stencil_op_stores) {
                    access_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                     SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster,
                                                     tag);
                }
            }
        }
    }
}

void RenderPassAccessContext::RecordLayoutTransitions(const vvl::RenderPass &rp_state, uint32_t subpass,
                                                      const AttachmentViewGenVector &attachment_views, const ResourceUsageTag tag,
                                                      AccessContext &access_context) {
    const auto &transitions = rp_state.subpass_transitions[subpass];
    const ResourceAccessState empty_infill;
    for (const auto &transition : transitions) {
        const auto prev_pass = transition.prev_pass;
        const auto &view_gen = attachment_views[transition.attachment];
        if (!view_gen.IsValid()) continue;

        const auto *trackback = access_context.GetTrackBackFromSubpass(prev_pass);
        assert(trackback);

        // Import the attachments into the current context
        const auto *prev_context = trackback->source_subpass;
        assert(prev_context);
        ApplySubpassTransitionBarriersAction barrier_action(trackback->barriers);
        const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kViewSubresource);
        assert(attachment_gen);

        access_context.ResolveFromContext(barrier_action, *prev_context, *attachment_gen, &empty_infill,
                                          true /* recur to infill */);
        assert(attachment_gen);
    }

    // If there were no transitions skip this global map walk
    if (transitions.size()) {
        ResolvePendingBarrierFunctor apply_pending_action(tag);
        access_context.ApplyToContext(apply_pending_action);
    }
}

// TODO: SyncError reporting places in this function are not covered by the tests.
bool RenderPassAccessContext::ValidateDrawSubpassAttachment(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const vvl::CommandBuffer &cmd_buffer = cb_context.GetCBState();
    const auto &last_bound_state = cmd_buffer.lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;

    if (!pipe || pipe->RasterizationDisabled()) {
        return skip;
    }

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->create_info.pSubpasses[current_subpass_];
    const auto &current_context = CurrentContext();
    const SyncValidator &sync_state = cb_context.GetSyncState();

    auto report_atachment_hazard = [&sync_state, &cb_context, command](const HazardResult &hazard,
                                                                       const vvl::ImageView &attachment_view,
                                                                       std::string_view attachment_description) {
        const vvl::Image &attachment_image = *attachment_view.image_state;
        LogObjectList objlist(cb_context.GetCBState().Handle(), attachment_view.Handle(), attachment_image.Handle());
        const Location loc(command);

        std::stringstream ss;
        ss << attachment_description;
        ss << " (" << sync_state.FormatHandle(attachment_view.Handle());
        ss << ", " << sync_state.FormatHandle(attachment_image.Handle()) << ")";
        const std::string resource_description = ss.str();

        const std::string error =
            sync_state.error_messages_.RenderPassAttachmentError(hazard, cb_context, command, resource_description);
        return sync_state.SyncError(hazard.Hazard(), objlist, loc, error);
    };

    // Subpass's inputAttachment has been done in ValidateDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount ||
                subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            if (!view_gen.IsValid()) continue;
            HazardResult hazard =
                current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                             SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << "color attachment " << location << " in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, *view_gen.GetViewState(), attachment_description);
            }
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);

    if ((depth_stencil_attachment != VK_ATTACHMENT_UNUSED) && attachment_views_[depth_stencil_attachment].IsValid()) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const vvl::ImageView &view_state = *view_gen.GetViewState();
        const VkImageLayout ds_layout = subpass.pDepthStencilAttachment->layout;
        const VkFormat ds_format = view_state.create_info.format;
        const bool depth_write = IsDepthAttachmentWriteable(last_bound_state, ds_format, ds_layout);
        const bool stencil_write = IsStencilAttachmentWriteable(last_bound_state, ds_format, ds_layout);

        // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // const bool early_fragment_test = pipe->fragment_shader_state->early_fragment_test;
        if (depth_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << "depth aspect of depth-stencil attachment  in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, view_state, attachment_description);
            }
        }
        if (stencil_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << "stencil aspect of depth-stencil attachment  in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, view_state, attachment_description);
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordDrawSubpassAttachment(const vvl::CommandBuffer &cmd_buffer, const ResourceUsageTag tag) {
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cmd_buffer.lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->create_info.pSubpasses[current_subpass_];

    auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in RecordDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount ||
                subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            current_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                              SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment,
                                              tag);
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto *ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);
    if ((depth_stencil_attachment != VK_ATTACHMENT_UNUSED) && attachment_views_[depth_stencil_attachment].IsValid()) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const vvl::ImageView &view_state = *view_gen.GetViewState();
        bool depth_write = false, stencil_write = false;
        const bool has_depth = vkuFormatHasDepth(view_state.create_info.format);
        const bool has_stencil = vkuFormatHasStencil(view_state.create_info.format);

        const bool depth_write_enable = last_bound_state.IsDepthWriteEnable();  // implicitly means DepthTestEnable is set
        const bool stencil_test_enable = last_bound_state.IsStencilTestEnable();

        // PHASE1 TODO: These validation should be in core_checks.
        if (has_depth && depth_write_enable && IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (has_stencil && stencil_test_enable && IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        if (depth_write || stencil_write) {
            const auto ds_gentype = view_gen.GetDepthStencilRenderAreaGenType(depth_write, stencil_write);
            // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
            current_context.UpdateAccessState(view_gen, ds_gentype, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                              SyncOrdering::kDepthStencilAttachment, tag);
        }
    }
}

const syncval_state::ImageViewState *RenderPassAccessContext::GetClearAttachmentView(
    const VkClearAttachment &clear_attachment) const {
    const auto &subpass = rp_state_->create_info.pSubpasses[current_subpass_];
    uint32_t attachment_index = VK_ATTACHMENT_UNUSED;
    if (clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (clear_attachment.colorAttachment < subpass.colorAttachmentCount) {
            attachment_index = subpass.pColorAttachments[clear_attachment.colorAttachment].attachment;
        }
    } else if (clear_attachment.aspectMask & kDepthStencilAspects) {
        if (subpass.pDepthStencilAttachment) {
            attachment_index = subpass.pDepthStencilAttachment->attachment;
        }
    }
    // This catches both out of bounds attachment index and VK_ATTACHMENT_UNUSED special value.
    if (attachment_index >= rp_state_->create_info.attachmentCount) {
        return nullptr;
    }
    return attachment_views_[attachment_index].GetViewState();
}

bool RenderPassAccessContext::ValidateNextSubpass(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    // PHASE1 TODO: Add Validate Preserve attachments
    bool skip = false;
    skip |= ValidateResolveOperations(cb_context, command);
    skip |= ValidateStoreOperation(cb_context, command);

    const auto next_subpass = current_subpass_ + 1;
    if (next_subpass >= subpass_contexts_.size()) {
        return skip;
    }
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |= ValidateLayoutTransitions(cb_context, next_context, *rp_state_, render_area_, next_subpass, attachment_views_, command);
    if (!skip) {
        // To avoid complex (and buggy) duplication of the affect of layout transitions on load operations, we'll record them
        // on a copy of the (empty) next context.
        // Note: The resource access map should be empty so hopefully this copy isn't too horrible from a perf POV.
        AccessContext temp_context(next_context);
        RecordLayoutTransitions(*rp_state_, next_subpass, attachment_views_, kInvalidTag, temp_context);
        skip |= ValidateLoadOperation(cb_context, temp_context, *rp_state_, render_area_, next_subpass, attachment_views_, command);
    }
    return skip;
}
bool RenderPassAccessContext::ValidateEndRenderPass(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    // PHASE1 TODO: Validate Preserve
    bool skip = false;
    skip |= ValidateResolveOperations(cb_context, command);
    skip |= ValidateStoreOperation(cb_context, command);
    skip |= ValidateFinalSubpassLayoutTransitions(cb_context, command);
    return skip;
}

AccessContext *RenderPassAccessContext::CreateStoreResolveProxy() const {
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, current_subpass_, attachment_views_);
}

bool RenderPassAccessContext::ValidateFinalSubpassLayoutTransitions(const CommandBufferAccessContext &cb_context,
                                                                    vvl::Func command) const {
    bool skip = false;

    // As validation methods are const and precede the record/update phase, for any tranistions from the current (last)
    // subpass, we have to validate them against a copy of the current AccessContext, with resolve operations applied.
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> proxy_for_current;

    // Validate the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const auto &view_gen = attachment_views_[transition.attachment];
        const auto &trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(trackback.source_subpass);  // Transitions are given implicit transitions if the StateTracker is working correctly
        auto *context = trackback.source_subpass;

        if (transition.prev_pass == current_subpass_) {
            if (!proxy_for_current) {
                // We haven't recorded resolve ofor the current_subpass, so we need to copy current and update it *as if*
                proxy_for_current.reset(CreateStoreResolveProxy());
            }
            context = proxy_for_current.get();
        }

        // Use the merged barrier for the hazard check (safe since it just considers the src (first) scope.
        const SyncBarrier merged_barrier(trackback.barriers);
        auto hazard = context->DetectImageBarrierHazard(view_gen, merged_barrier, AccessContext::DetectOptions::kDetectPrevious);
        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = cb_context.GetSyncState();
            const Location loc(command);

            std::stringstream ss;
            ss << "on attachment " << transition.attachment << " (";
            ss << sync_state.FormatHandle(view_gen.GetViewState()->Handle());
            ss << ", " << sync_state.FormatHandle(view_gen.GetViewState()->GetImageState()->Handle());
            ss << ", oldLayout " << string_VkImageLayout(transition.old_layout);
            ss << ", newLayout " << string_VkImageLayout(transition.new_layout);
            ss << ")";
            const std::string resource_description = ss.str();

            if (hazard.Tag() == kInvalidTag) {  // Hazard vs. store/resolve
                const std::string error = sync_state.error_messages_.RenderPassFinalLayoutTransitionVsStoreOrResolveError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout,
                    transition.prev_pass);
                skip |= sync_state.SyncError(hazard.Hazard(), rp_state_->Handle(), loc, error);
            } else {
                const std::string error = sync_state.error_messages_.RenderPassFinalLayoutTransitionError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout);
                skip |= sync_state.SyncError(hazard.Hazard(), rp_state_->Handle(), loc, error);
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordLayoutTransitions(const ResourceUsageTag tag) {
    // Add layout transitions...
    RecordLayoutTransitions(*rp_state_, current_subpass_, attachment_views_, tag, subpass_contexts_[current_subpass_]);
}

void RenderPassAccessContext::RecordLoadOperations(const ResourceUsageTag tag) {
    const auto *attachment_ci = rp_state_->create_info.pAttachments;
    auto &subpass_context = subpass_contexts_[current_subpass_];

    for (uint32_t i = 0; i < rp_state_->create_info.attachmentCount; i++) {
        if (rp_state_->attachment_first_subpass[i] == current_subpass_) {
            const AttachmentViewGen &view_gen = attachment_views_[i];
            if (!view_gen.IsValid()) continue;  // UNUSED

            const auto &ci = attachment_ci[i];
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            if (is_color) {
                const SyncAccessIndex load_op = ColorLoadUsage(ci.loadOp);
                if (load_op != SYNC_ACCESS_INDEX_NONE) {
                    subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea, load_op,
                                                      SyncOrdering::kColorAttachment, tag);
                }
            } else {
                if (has_depth) {
                    const SyncAccessIndex load_op = DepthStencilLoadUsage(ci.loadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea, load_op,
                                                          SyncOrdering::kDepthStencilAttachment, tag);
                    }
                }
                if (has_stencil) {
                    const SyncAccessIndex load_op = DepthStencilLoadUsage(ci.stencilLoadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea, load_op,
                                                          SyncOrdering::kDepthStencilAttachment, tag);
                    }
                }
            }
        }
    }
}
AttachmentViewGenVector RenderPassAccessContext::CreateAttachmentViewGen(
    const VkRect2D &render_area, const std::vector<const syncval_state::ImageViewState *> &attachment_views) {
    AttachmentViewGenVector view_gens;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);
    view_gens.reserve(attachment_views.size());
    for (const auto *view : attachment_views) {
        view_gens.emplace_back(view, offset, extent);
    }
    return view_gens;
}
RenderPassAccessContext::RenderPassAccessContext(const vvl::RenderPass &rp_state, const VkRect2D &render_area,
                                                 VkQueueFlags queue_flags,
                                                 const std::vector<const syncval_state::ImageViewState *> &attachment_views,
                                                 const AccessContext *external_context)
    : rp_state_(&rp_state), render_area_(render_area), current_subpass_(0U), attachment_views_() {
    // Add this for all subpasses here so that they exist during next subpass validation
    InitSubpassContexts(queue_flags, rp_state, external_context, subpass_contexts_);
    attachment_views_ = CreateAttachmentViewGen(render_area, attachment_views);
}
void RenderPassAccessContext::RecordBeginRenderPass(const ResourceUsageTag barrier_tag, const ResourceUsageTag load_tag) {
    assert(0 == current_subpass_);
    AccessContext &current_context = subpass_contexts_[current_subpass_];
    current_context.SetStartTag(barrier_tag);

    RecordLayoutTransitions(barrier_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordNextSubpass(const ResourceUsageTag store_tag, const ResourceUsageTag barrier_tag,
                                                const ResourceUsageTag load_tag) {
    // Resolves are against *prior* subpass context and thus *before* the subpass increment
    UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, current_subpass_, store_tag, CurrentContext());
    UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, current_subpass_, store_tag, CurrentContext());

    if (current_subpass_ + 1 >= subpass_contexts_.size()) {
        return;
    }
    // Move to the next sub-command for the new subpass. The resolve and store are logically part of the previous
    // subpass, so their tag needs to be different from the layout and load operations below.
    current_subpass_++;
    AccessContext &current_context = subpass_contexts_[current_subpass_];
    current_context.SetStartTag(barrier_tag);

    RecordLayoutTransitions(barrier_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordEndRenderPass(AccessContext *external_context, const ResourceUsageTag store_tag,
                                                  const ResourceUsageTag barrier_tag) {
    // Add the resolve and store accesses
    UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, current_subpass_, store_tag, CurrentContext());
    UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, current_subpass_, store_tag, CurrentContext());

    // Export the accesses from the renderpass...
    external_context->ResolveChildContexts(subpass_contexts_);

    // Add the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    // Not that since *final* always comes from *one* subpass per view, we don't have to accumulate the barriers
    // TODO Aliasing we may need to reconsider barrier accumulation... though I don't know that it would be valid for aliasing
    //      that had mulitple final layout transistions from mulitple final subpasses.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const AttachmentViewGen &view_gen = attachment_views_[transition.attachment];
        const auto &last_trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(&subpass_contexts_[transition.prev_pass] == last_trackback.source_subpass);
        ApplyBarrierOpsFunctor<PipelineBarrierOp> barrier_action(true /* resolve */, last_trackback.barriers.size(), barrier_tag);
        for (const auto &barrier : last_trackback.barriers) {
            barrier_action.EmplaceBack(PipelineBarrierOp(kQueueIdInvalid, barrier, true));
        }
        external_context->ApplyUpdateAction(view_gen, AttachmentViewGen::Gen::kViewSubresource, barrier_action);
    }
}

void syncval_state::BeginRenderingCmdState::AddRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info) {
    info = std::make_unique<DynamicRenderingInfo>(state, rendering_info);
}

const syncval_state::DynamicRenderingInfo &syncval_state::BeginRenderingCmdState::GetRenderingInfo() const {
    assert(info);
    return *info;
}
syncval_state::DynamicRenderingInfo::DynamicRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info)
    : info(&rendering_info) {
    uint32_t attachment_count = info.colorAttachmentCount + (info.pDepthAttachment ? 1 : 0) + (info.pStencilAttachment ? 1 : 0);

    const VkOffset3D offset = CastTo3D(info.renderArea.offset);
    const VkExtent3D extent = CastTo3D(info.renderArea.extent);

    attachments.reserve(attachment_count);
    for (uint32_t i = 0; i < info.colorAttachmentCount; i++) {
        attachments.emplace_back(state, info.pColorAttachments[i], syncval_state::AttachmentType::kColor, offset, extent);
    }

    if (info.pDepthAttachment) {
        attachments.emplace_back(state, *info.pDepthAttachment, syncval_state::AttachmentType::kDepth, offset, extent);
    }

    if (info.pStencilAttachment) {
        attachments.emplace_back(state, *info.pStencilAttachment, syncval_state::AttachmentType::kStencil, offset, extent);
    }
}

const syncval_state::ImageViewState *syncval_state::DynamicRenderingInfo::GetClearAttachmentView(
    const VkClearAttachment &clear_attachment) const {
    const syncval_state::ImageViewState *attachment_view = nullptr;
    if (clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (clear_attachment.colorAttachment < info.colorAttachmentCount) {
            attachment_view = attachments[clear_attachment.colorAttachment].view.get();
        }
    } else if (clear_attachment.aspectMask & kDepthStencilAspects) {
        if (attachments.size() > info.colorAttachmentCount) {
            // If both depth and stencil attachments are defined they must both point to the same view
            attachment_view = attachments.back().view.get();
        }
    }
    return attachment_view;
}

syncval_state::DynamicRenderingInfo::Attachment::Attachment(const SyncValidator &state,
                                                            const vku::safe_VkRenderingAttachmentInfo &attachment_info,
                                                            AttachmentType type_, const VkOffset3D &offset,
                                                            const VkExtent3D &extent)
    : info(attachment_info), view(state.Get<ImageViewState>(attachment_info.imageView)), view_gen(), type(type_) {
    if (view) {
        if (type == AttachmentType::kColor) {
            view_gen = view->MakeImageRangeGen(offset, extent);
        } else if (type == AttachmentType::kDepth) {
            view_gen = view->MakeImageRangeGen(offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT);
        } else {
            view_gen = view->MakeImageRangeGen(offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT);
        }

        if (info.resolveImageView != VK_NULL_HANDLE && (info.resolveMode != VK_RESOLVE_MODE_NONE)) {
            resolve_view = state.Get<ImageViewState>(info.resolveImageView);
            if (resolve_view) {
                if (type == AttachmentType::kColor) {
                    resolve_gen.emplace(resolve_view->MakeImageRangeGen(offset, extent));
                } else if (type == AttachmentType::kDepth) {
                    // Only the depth aspect
                    resolve_gen.emplace(resolve_view->MakeImageRangeGen(offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT));
                } else {
                    resolve_gen.emplace(resolve_view->MakeImageRangeGen(offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT));
                }
            }
        }
    }
}

SyncAccessIndex syncval_state::DynamicRenderingInfo::Attachment::GetLoadUsage() const {
    return GetLoadOpUsageIndex(info.loadOp, type);
}

SyncAccessIndex syncval_state::DynamicRenderingInfo::Attachment::GetStoreUsage() const {
    return GetStoreOpUsageIndex(info.storeOp, type);
}

SyncOrdering syncval_state::DynamicRenderingInfo::Attachment::GetOrdering() const {
    return (type == AttachmentType::kColor) ? SyncOrdering::kColorAttachment : SyncOrdering::kDepthStencilAttachment;
}

Location syncval_state::DynamicRenderingInfo::Attachment::GetLocation(const Location &loc, uint32_t attachment_index) const {
    if (type == AttachmentType::kColor) {
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pColorAttachments, attachment_index);
    } else if (type == AttachmentType::kDepth) {
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pDepthAttachment);
    } else {
        assert(type == AttachmentType::kStencil);
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pStencilAttachment);
    }
}

bool syncval_state::DynamicRenderingInfo::Attachment::IsWriteable(const LastBound &last_bound_state) const {
    bool writeable = IsValid();
    if (writeable) {
        //  Depth and Stencil have additional criteria
        if (type == AttachmentType::kDepth) {
            writeable = last_bound_state.IsDepthWriteEnable() &&
                        IsDepthAttachmentWriteable(last_bound_state, view->create_info.format, info.imageLayout);
        } else if (type == AttachmentType::kStencil) {
            writeable = last_bound_state.IsStencilTestEnable() &&
                        IsStencilAttachmentWriteable(last_bound_state, view->create_info.format, info.imageLayout);
        }
    }
    return writeable;
}
