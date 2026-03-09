/* Copyright (c) 2019-2026 The Khronos Group Inc.
 * Copyright (c) 2019-2026 Valve Corporation
 * Copyright (c) 2019-2026 LunarG, Inc.
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

#include <vulkan/utility/vk_format_utils.h>
#include "sync/sync_renderpass.h"
#include "sync/sync_validation.h"
#include "sync/sync_op.h"
#include "sync/sync_image.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "utils/math_utils.h"

namespace syncval {

class ValidateResolveAction {
  public:
    ValidateResolveAction(VkRenderPass render_pass, uint32_t subpass, uint32_t view_mask, const AccessContext &context,
                          const CommandBufferAccessContext &cb_context, vvl::Func command)
        : render_pass_(render_pass),
          subpass_(subpass),
          view_mask_(view_mask),
          context_(context),
          cb_context_(cb_context),
          command_(command),
          skip_(false) {}

    void operator()(const char *aspect_name, const char *resolve_action_name, uint32_t src_at, uint32_t dst_at,
                    const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                    const AttachmentAccess &attachment_access) {
        const HazardResult hazard =
            context_.DetectAttachmentHazard(view_gen, gen_type, current_usage, attachment_access, view_mask_);
        if (hazard.IsHazard()) {
            const Location loc(command_);

            const SyncValidator &validator = cb_context_.GetSyncState();

            std::ostringstream ss;
            ss << validator.FormatHandle(view_gen.GetViewState()->Handle());
            ss << " (" << resolve_action_name << " of " << aspect_name << " multisample attachment " << src_at;
            ss << " in subpass " << subpass_ << " of " << validator.FormatHandle(render_pass_) << ")";
            const std::string resource_description = ss.str();
            const auto error =
                validator.error_messages_.RenderPassResolveError(hazard, cb_context_, command_, resource_description);
            skip_ |= validator.SyncError(hazard.Hazard(), render_pass_, loc, error);
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const uint32_t view_mask_;
    const AccessContext &context_;
    const CommandBufferAccessContext &cb_context_;
    vvl::Func command_;
    bool skip_;
};

class UpdateStateResolveAction {
  public:
    UpdateStateResolveAction(AccessContext &context, uint32_t view_mask, ResourceUsageTag tag)
        : context_(context), view_mask_(view_mask), tag_(tag) {}
    void operator()(const char *, const char *, uint32_t, uint32_t, const AttachmentViewGen &view_gen,
                    AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage, const AttachmentAccess &attachment_access) {
        context_.UpdateAttachmentAccessState(view_gen, gen_type, current_usage, attachment_access, ResourceUsageTagEx{tag_},
                                             view_mask_);
    }

  private:
    AccessContext &context_;
    const uint32_t view_mask_;
    const ResourceUsageTag tag_;
};

std::unique_ptr<AccessContext[]> InitSubpassContexts(VkQueueFlags queue_flags, const vvl::RenderPass &rp_state,
                                                     const AccessContext &external_context) {
    const uint32_t subpass_count = rp_state.create_info.subpassCount;
    auto subpass_contexts = std::make_unique<AccessContext[]>(subpass_count);
    // Add this for all subpasses here so that they exsist during next subpass validation
    for (uint32_t pass = 0; pass < subpass_count; pass++) {
        subpass_contexts[pass].validator = external_context.validator;
        subpass_contexts[pass].InitFrom(pass, queue_flags, rp_state.subpass_dependency_infos, subpass_contexts.get(),
                                        external_context);
    }
    return subpass_contexts;
}

static SyncAccessIndex GetLoadOpUsageIndex(VkAttachmentLoadOp load_op, AttachmentType type) {
    SyncAccessIndex access_index;
    if (load_op == VK_ATTACHMENT_LOAD_OP_NONE) {
        access_index = SYNC_ACCESS_INDEX_NONE;
    } else if (type == AttachmentType::kColor) {
        access_index = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                               : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        access_index = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                               : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
    return access_index;
}

static SyncAccessIndex GetStoreOpUsageIndex(VkAttachmentStoreOp store_op, AttachmentType type) {
    SyncAccessIndex access_index;
    if (store_op == VK_ATTACHMENT_STORE_OP_NONE) {
        access_index = SYNC_ACCESS_INDEX_NONE;
    } else if (type == AttachmentType::kColor) {
        access_index = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    } else {  // depth and stencil ops are the same
        access_index = SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    }
    return access_index;
}

static SyncAccessIndex ColorLoadUsage(VkAttachmentLoadOp load_op) {
    return GetLoadOpUsageIndex(load_op, AttachmentType::kColor);
}
static SyncAccessIndex DepthStencilLoadUsage(VkAttachmentLoadOp load_op) {
    return GetLoadOpUsageIndex(load_op, AttachmentType::kDepth);
}

static bool IsSubpassIncluded(uint32_t subpass, const vvl::RenderPass::SubpassPerView &subpass_per_view, uint32_t view_mask) {
    if (view_mask == 0) {
        return subpass_per_view[0] == subpass;
    }
    const auto view_indices = GetSetBitIndices(view_mask);
    for (uint8_t view_index : view_indices) {
        if (subpass_per_view[view_index] == subpass) {
            return true;
        }
    }
    return false;
}

// Caller must manage returned pointer
static AccessContext *CreateStoreResolveProxyContext(const AccessContext &context, const vvl::RenderPass &rp_state,
                                                     uint32_t render_pass_instance_id, uint32_t subpass, uint32_t view_mask,
                                                     const AttachmentViewGenVector &attachment_views) {
    auto *proxy = new AccessContext(*context.validator);
    proxy->InitFrom(context);
    RenderPassAccessContext::UpdateAttachmentResolveAccess(rp_state, attachment_views, render_pass_instance_id, subpass, view_mask,
                                                           kInvalidTag, *proxy);
    RenderPassAccessContext::UpdateAttachmentStoreAccess(rp_state, attachment_views, render_pass_instance_id, subpass, view_mask,
                                                         kInvalidTag, *proxy);
    return proxy;
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool RenderPassAccessContext::ValidateLayoutTransitions(const CommandBufferAccessContext &cb_context,
                                                        const AccessContext &access_context, const vvl::RenderPass &rp_state,
                                                        const VkRect2D &render_area, uint32_t render_pass_instance_id,
                                                        uint32_t subpass, uint32_t view_mask,
                                                        const AttachmentViewGenVector &attachment_views, vvl::Func command) {
    bool skip = false;
    // As validation methods are const and precede the record/update phase, for any tranistions from the immediately
    // previous subpass, we have to validate them against a copy of the AccessContext, with resolve operations applied, as
    // those affects have not been recorded yet.
    //
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> src_context_proxy;
    SubpassBarrier proxy_subpass_barrier;

    const auto &transitions = rp_state.subpass_transitions[subpass];
    for (const auto &transition : transitions) {
        const SubpassBarrier &subpass_barrier = access_context.GetSubpassBarrier(transition.src_subpass);
        const SubpassBarrier *p_subpass_barrier = &subpass_barrier;

        const bool src_context_needs_proxy =
            transition.src_subpass != VK_SUBPASS_EXTERNAL && (transition.src_subpass + 1 == subpass);

        if (src_context_needs_proxy) {
            if (!src_context_proxy) {
                // TODO: this looks wrong to create proxy once for all iterations.
                // Proxy depends on current iteration (transition.src_subpass), so it should be recreated
                // each time when needed. Write a test that exposes this and make a fix.
                src_context_proxy.reset(CreateStoreResolveProxyContext(*subpass_barrier.src_subpass_context, rp_state,
                                                                       render_pass_instance_id, transition.src_subpass, view_mask,
                                                                       attachment_views));
                proxy_subpass_barrier = subpass_barrier;
                proxy_subpass_barrier.src_subpass_context = src_context_proxy.get();
            }
            p_subpass_barrier = &proxy_subpass_barrier;
        }
        auto hazard = access_context.DetectSubpassTransitionHazard(*p_subpass_barrier, attachment_views[transition.attachment]);
        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = cb_context.GetSyncState();
            const Location loc(command);

            const vvl::ImageView *attachment_view = attachment_views[transition.attachment].GetViewState();
            std::ostringstream ss;
            ss << "in subpass " << subpass << " of " << sync_state.FormatHandle(rp_state.Handle());
            ss << " on attachment " << transition.attachment << " (";
            ss << sync_state.FormatHandle(attachment_view->Handle());
            ss << ", " << sync_state.FormatHandle(attachment_view->image_state->Handle());
            ss << ", oldLayout " << string_VkImageLayout(transition.old_layout);
            ss << ", newLayout " << string_VkImageLayout(transition.new_layout);
            ss << ")";
            const std::string resource_description = ss.str();

            if (hazard.Tag() == kInvalidTag) {
                const auto error = sync_state.error_messages_.RenderPassLayoutTransitionVsResolveError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout,
                    transition.src_subpass);
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
                                                    const VkRect2D &render_area, uint32_t render_pass_instance_id, uint32_t subpass,
                                                    uint32_t view_mask, const AttachmentViewGenVector &attachment_views,
                                                    vvl::Func command) {
    bool skip = false;

    AttachmentAccess attachment_access;
    attachment_access.type = AttachmentAccessType::LoadOp;
    attachment_access.render_pass_instance_id = render_pass_instance_id;
    attachment_access.subpass = subpass;

    for (uint32_t i = 0; i < rp_state.create_info.attachmentCount; i++) {
        if (IsSubpassIncluded(subpass, rp_state.attachment_first_subpass[i], view_mask)) {
            const auto &view_gen = attachment_views[i];
            const auto &ci = rp_state.create_info.pAttachments[i];

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
                attachment_access.ordering = SyncOrdering::kColorAttachment;
                hazard = access_context.DetectAttachmentHazard(view_gen, AttachmentViewGen::Gen::kRenderArea, load_index,
                                                               attachment_access, view_mask);
                aspect = "color";
            } else {
                if (has_depth && (load_index != SYNC_ACCESS_INDEX_NONE)) {
                    ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kDepthOnlyRenderArea);
                    attachment_access.ordering = SyncOrdering::kDepthStencilAttachment;
                    hazard = access_context.DetectAttachmentHazard(attachment_range_gen, load_index, attachment_access);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && (stencil_load_index != SYNC_ACCESS_INDEX_NONE)) {
                    ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kStencilOnlyRenderArea);
                    attachment_access.ordering = SyncOrdering::kDepthStencilAttachment;
                    hazard = access_context.DetectAttachmentHazard(attachment_range_gen, stencil_load_index, attachment_access);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                const VkAttachmentLoadOp load_op = checked_stencil ? ci.stencilLoadOp : ci.loadOp;
                const SyncValidator &sync_state = cb_context.GetSyncState();
                const Location loc(command);

                std::ostringstream ss;
                ss << "the " << aspect << " aspect of attachment " << i;
                ss << " (" << sync_state.FormatHandle(view_gen.GetViewState()->Handle()) << ")";
                ss << " in subpass " << subpass;
                ss << " of " << sync_state.FormatHandle(rp_state.Handle());
                ss << " (loadOp " << string_VkAttachmentLoadOp(load_op) << ")";
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

    const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kRaster, AttachmentAccessType::StoreOp);
    const uint32_t view_mask = rp_state_->create_info.pSubpasses[current_subpass_].viewMask;

    for (uint32_t i = 0; i < rp_state_->create_info.attachmentCount; i++) {
        if (IsSubpassIncluded(current_subpass_, rp_state_->attachment_last_subpass[i], view_mask)) {
            const AttachmentViewGen &view_gen = attachment_views_[i];
            const auto &ci = rp_state_->create_info.pAttachments[i];

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
                hazard = CurrentContext().DetectAttachmentHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                                 SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                                 attachment_access, view_mask);
                aspect = "color";
            } else {
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE;
                if (has_depth && store_op_stores) {
                    ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kDepthOnlyRenderArea);
                    hazard = CurrentContext().DetectAttachmentHazard(
                        attachment_range_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && stencil_op_stores) {
                    ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kStencilOnlyRenderArea);
                    hazard = CurrentContext().DetectAttachmentHazard(
                        attachment_range_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                const SyncValidator &sync_state = cb_context.GetSyncState();
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const VkAttachmentStoreOp store_op = checked_stencil ? ci.stencilStoreOp : ci.storeOp;
                const Location loc(command);

                std::ostringstream ss;
                ss << sync_state.FormatHandle(view_gen.GetViewState()->Handle());
                ss << " (subpass " << current_subpass_ << " of " << sync_state.FormatHandle(rp_state_->Handle());
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

static bool IsDepthAttachmentWriteable(const LastBound &last_bound_state, const VkFormat format) {
    const bool depth_write_enable = last_bound_state.IsDepthWriteEnable();
    return (vkuFormatIsDepthAndStencil(format) || vkuFormatIsDepthOnly(format)) && depth_write_enable;
}

static bool IsStencilAttachmentWriteable(const LastBound &last_bound_state, const VkFormat format) {
    if (!vkuFormatIsDepthAndStencil(format) && !vkuFormatIsStencilOnly(format)) {
        return false;
    }
    if (!last_bound_state.IsStencilTestEnable()) {
        return false;
    }
    auto is_writable = [&last_bound_state](const VkStencilOpState &ops) -> bool {
        if (ops.writeMask == 0) {
            return false;
        }

        // If compareOp is ALWAYS then failOp never runs (no writes possible)
        const bool ignore_fail_op = (ops.compareOp == VK_COMPARE_OP_ALWAYS);

        // If compareOp is NEVER then passOp never runs (no writes possible)
        const bool ignore_pass_op = (ops.compareOp == VK_COMPARE_OP_NEVER);

        // If depth test is not enabled then depthFailOp never runs (no writes possible)
        const bool ignore_depth_fail_op = !last_bound_state.IsDepthTestEnable();

        const bool is_read = (ops.failOp == VK_STENCIL_OP_KEEP || ignore_fail_op) &&
                             (ops.passOp == VK_STENCIL_OP_KEEP || ignore_pass_op) &&
                             (ops.depthFailOp == VK_STENCIL_OP_KEEP || ignore_depth_fail_op);
        return !is_read;
    };
    const VkStencilOpState front_ops = last_bound_state.GetStencilOpStateFront();
    const VkStencilOpState back_ops = last_bound_state.GetStencilOpStateBack();
    return is_writable(front_ops) || is_writable(back_ops);
}

// Traverse the attachment resolves for this a specific subpass, and do action() to them.
// Used by both validation and record operations
//
// The signature for Action() reflect the needs of both uses.
template <typename Action>
void ResolveOperation(Action &action, const vvl::RenderPass &rp_state, const AttachmentViewGenVector &attachment_views,
                      uint32_t render_pass_instance_id, uint32_t subpass) {
    const auto &rp_ci = rp_state.create_info;
    const auto *attachment_ci = rp_ci.pAttachments;
    const auto &subpass_ci = rp_ci.pSubpasses[subpass];

    AttachmentAccess attachment_access;
    attachment_access.render_pass_instance_id = render_pass_instance_id;
    attachment_access.subpass = subpass;

    // Color resolve requires an inuse color attachment and a matching inuse resolve attachment
    if (subpass_ci.pResolveAttachments && subpass_ci.pColorAttachments) {
        attachment_access.ordering = SyncOrdering::kColorAttachment;
        for (uint32_t i = 0; i < subpass_ci.colorAttachmentCount; i++) {
            const uint32_t color_attach = subpass_ci.pColorAttachments[i].attachment;
            const uint32_t resolve_attach = subpass_ci.pResolveAttachments[i].attachment;
            if (color_attach != VK_ATTACHMENT_UNUSED && resolve_attach != VK_ATTACHMENT_UNUSED) {
                attachment_access.type  = AttachmentAccessType::ResolveRead;
                action("color", "resolve read", color_attach, resolve_attach, attachment_views[color_attach],
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, attachment_access);

                attachment_access.type = AttachmentAccessType::ResolveWrite;
                action("color", "resolve write", color_attach, resolve_attach, attachment_views[resolve_attach],
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, attachment_access);
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
            attachment_access.ordering = SyncOrdering::kRaster;

            attachment_access.type = AttachmentAccessType::ResolveRead;
            action(aspect_string, "resolve read", src_at, dst_at, attachment_views[src_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, attachment_access);

            attachment_access.type = AttachmentAccessType::ResolveWrite;
            action(aspect_string, "resolve write", src_at, dst_at, attachment_views[dst_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, attachment_access);
        }
    }
}

bool RenderPassAccessContext::ValidateResolveOperations(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    const uint32_t view_mask = rp_state_->create_info.pSubpasses[current_subpass_].viewMask;
    ValidateResolveAction validate_action(rp_state_->VkHandle(), current_subpass_, view_mask, CurrentContext(), cb_context,
                                          command);
    ResolveOperation(validate_action, *rp_state_, attachment_views_, render_pass_instance_id_, current_subpass_);
    return validate_action.GetSkip();
}

void RenderPassAccessContext::UpdateAttachmentResolveAccess(const vvl::RenderPass &rp_state,
                                                            const AttachmentViewGenVector &attachment_views,
                                                            uint32_t render_pass_instance_id, uint32_t subpass, uint32_t view_mask,
                                                            const ResourceUsageTag tag, AccessContext &access_context) {
    UpdateStateResolveAction update(access_context, view_mask, tag);
    ResolveOperation(update, rp_state, attachment_views, render_pass_instance_id, subpass);
}

void RenderPassAccessContext::UpdateAttachmentStoreAccess(const vvl::RenderPass &rp_state,
                                                          const AttachmentViewGenVector &attachment_views,
                                                          uint32_t render_pass_instance_id, uint32_t subpass, uint32_t view_mask,
                                                          const ResourceUsageTag tag, AccessContext &access_context) {
    AttachmentAccess attachment_access;
    attachment_access.type = AttachmentAccessType::StoreOp;
    attachment_access.ordering = SyncOrdering::kRaster;
    attachment_access.render_pass_instance_id = render_pass_instance_id;
    attachment_access.subpass = subpass;

    for (uint32_t i = 0; i < rp_state.create_info.attachmentCount; i++) {
        if (IsSubpassIncluded(subpass, rp_state.attachment_last_subpass[i], view_mask)) {
            const auto &view_gen = attachment_views[i];

            const auto &ci = rp_state.create_info.pAttachments[i];
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE;

            if (is_color && store_op_stores) {
                access_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                           SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, attachment_access,
                                                           ResourceUsageTagEx{tag}, view_mask);
            } else {
                if (has_depth && store_op_stores) {
                    access_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               attachment_access, ResourceUsageTagEx{tag});
                }
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE;
                if (has_stencil && stencil_op_stores) {
                    access_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               attachment_access, ResourceUsageTagEx{tag});
                }
            }
        }
    }
}

void RenderPassAccessContext::RecordLayoutTransitions(const vvl::RenderPass &rp_state, uint32_t subpass,
                                                      const AttachmentViewGenVector &attachment_views, const ResourceUsageTag tag,
                                                      AccessContext &access_context) {
    const auto &transitions = rp_state.subpass_transitions[subpass];
    for (const auto &transition : transitions) {
        const auto &view_gen = attachment_views[transition.attachment];
        const SubpassBarrier &subpass_barrier = access_context.GetSubpassBarrier(transition.src_subpass);
        const AccessContext &src_subpass_context = *subpass_barrier.src_subpass_context;

        // Import the attachments into the current context
        ApplySubpassTransitionBarrierAction barrier_action(subpass_barrier, tag);
        ImageRangeGen attachment_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kViewSubresource);
        access_context.ResolveFromSubpassContext(barrier_action, src_subpass_context, attachment_gen);
    }
}

bool RenderPassAccessContext::ValidateDrawSubpassAttachment(const CommandBufferAccessContext &cb_context, vvl::Func command) const {
    bool skip = false;
    const vvl::CommandBuffer &cmd_buffer = cb_context.GetCBState();
    const auto &last_bound_state = cmd_buffer.GetLastBoundGraphics();
    const auto *pipe = last_bound_state.pipeline_state;

    if (!pipe || pipe->RasterizationDisabled()) {
        return skip;
    }

    const auto& list = pipe->fs_writable_output_location_list;
    const auto &subpass = rp_state_->create_info.pSubpasses[current_subpass_];
    const auto &current_context = CurrentContext();
    const SyncValidator &sync_state = cb_context.GetSyncState();

    auto report_atachment_hazard = [&sync_state, &cb_context, command](const HazardResult &hazard,
                                                                       const vvl::ImageView &attachment_view,
                                                                       std::string_view attachment_description) {
        const vvl::Image &attachment_image = *attachment_view.image_state;
        LogObjectList objlist(cb_context.GetCBState().Handle(), attachment_view.Handle(), attachment_image.Handle());
        const Location loc(command);

        std::ostringstream ss;
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
            const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kColorAttachment);
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            HazardResult hazard = current_context.DetectAttachmentHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                                         SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                                         attachment_access, subpass.viewMask);
            if (hazard.IsHazard()) {
                std::ostringstream ss;
                ss << "color attachment " << location << " in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, *view_gen.GetViewState(), attachment_description);
            }
        }
    }

    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);

    if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const vvl::ImageView &view_state = *view_gen.GetViewState();
        const VkFormat ds_format = view_state.create_info.format;
        const bool depth_write = IsDepthAttachmentWriteable(last_bound_state, ds_format);
        const bool stencil_write = IsStencilAttachmentWriteable(last_bound_state, ds_format);

        if (depth_write) {
            const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kDepthStencilAttachment);
            ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kDepthOnlyRenderArea);
            HazardResult hazard = current_context.DetectAttachmentHazard(
                attachment_range_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access);
            if (hazard.IsHazard()) {
                std::ostringstream ss;
                ss << "depth aspect of depth-stencil attachment  in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, view_state, attachment_description);
            }
        }
        if (stencil_write) {
            const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kDepthStencilAttachment);
            ImageRangeGen attachment_range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kStencilOnlyRenderArea);
            HazardResult hazard = current_context.DetectAttachmentHazard(
                attachment_range_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access);
            if (hazard.IsHazard()) {
                std::ostringstream ss;
                ss << "stencil aspect of depth-stencil attachment  in subpass " << cmd_buffer.GetActiveSubpass();
                const std::string attachment_description = ss.str();
                skip |= report_atachment_hazard(hazard, view_state, attachment_description);
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordDrawSubpassAttachment(const vvl::CommandBuffer &cmd_buffer, const ResourceUsageTag tag) {
    const auto &last_bound_state = cmd_buffer.GetLastBoundGraphics();
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return;

    const auto &list = pipe->fs_writable_output_location_list;
    const auto &subpass = rp_state_->create_info.pSubpasses[current_subpass_];

    auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in RecordDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount ||
                subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kColorAttachment);
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            current_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                                        SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, attachment_access,
                                                        ResourceUsageTagEx{tag}, subpass.viewMask);
        }
    }

    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto *ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);
    if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const vvl::ImageView &view_state = *view_gen.GetViewState();
        const VkFormat ds_format = view_state.create_info.format;
        const bool depth_write = IsDepthAttachmentWriteable(last_bound_state, ds_format);
        const bool stencil_write = IsStencilAttachmentWriteable(last_bound_state, ds_format);

        if (depth_write || stencil_write) {
            const AttachmentAccess attachment_access = GetAttachmentAccess(SyncOrdering::kDepthStencilAttachment);
            const auto ds_gentype = view_gen.GetDepthStencilRenderAreaGenType(depth_write, stencil_write);
            current_context.UpdateAttachmentAccessState(view_gen, ds_gentype,
                                                        SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, attachment_access,
                                                        ResourceUsageTagEx{tag});
        }
    }
}

const vvl::ImageView *RenderPassAccessContext::GetClearAttachmentView(const VkClearAttachment &clear_attachment) const {
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
    if (next_subpass >= rp_state_->create_info.subpassCount) {
        return skip;
    }
    const uint32_t next_subpass_view_mask = rp_state_->create_info.pSubpasses[next_subpass].viewMask;
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |= ValidateLayoutTransitions(cb_context, next_context, *rp_state_, render_area_, render_pass_instance_id_, next_subpass,
                                      next_subpass_view_mask, attachment_views_, command);
    if (!skip) {
        // To avoid complex (and buggy) duplication of the affect of layout transitions on load operations, we'll record them
        // on a copy of the (empty) next context.
        // Note: The resource access map should be empty so hopefully this copy isn't too horrible from a perf POV.
        AccessContext temp_context(cb_context.GetSyncState());
        temp_context.InitFrom(next_context);
        RecordLayoutTransitions(*rp_state_, next_subpass, attachment_views_, kInvalidTag, temp_context);
        skip |= ValidateLoadOperation(cb_context, temp_context, *rp_state_, render_area_, render_pass_instance_id_, next_subpass,
                                      next_subpass_view_mask, attachment_views_, command);
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
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, render_pass_instance_id_, current_subpass_,
                                          rp_state_->create_info.pSubpasses[current_subpass_].viewMask, attachment_views_);
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
        const SubpassBarrier &subpass_barrier = subpass_contexts_[transition.src_subpass].GetDstExternalSubpassBarrier();
        const AccessContext *context = subpass_barrier.src_subpass_context;

        if (transition.src_subpass == current_subpass_) {
            if (!proxy_for_current) {
                // We haven't recorded resolve ofor the current_subpass, so we need to copy current and update it *as if*
                proxy_for_current.reset(CreateStoreResolveProxy());
            }
            context = proxy_for_current.get();
        }

        // Use the merged barrier for the hazard check (safe since it just considers the src (first) scope.
        const SyncBarrier merged_barrier(subpass_barrier.barriers);
        auto hazard = context->DetectImageBarrierHazard(view_gen, merged_barrier, AccessContext::DetectOptions::kDetectPrevious);
        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = cb_context.GetSyncState();
            const Location loc(command);

            std::ostringstream ss;
            ss << "on attachment " << transition.attachment << " (";
            ss << sync_state.FormatHandle(view_gen.GetViewState()->Handle());
            ss << ", " << sync_state.FormatHandle(view_gen.GetViewState()->image_state->Handle());
            ss << ", oldLayout " << string_VkImageLayout(transition.old_layout);
            ss << ", newLayout " << string_VkImageLayout(transition.new_layout);
            ss << ")";
            const std::string resource_description = ss.str();

            if (hazard.Tag() == kInvalidTag) {  // Hazard vs. store/resolve
                const std::string error = sync_state.error_messages_.RenderPassFinalLayoutTransitionVsStoreOrResolveError(
                    hazard, cb_context, command, resource_description, transition.old_layout, transition.new_layout,
                    transition.src_subpass);
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
    RecordLayoutTransitions(*rp_state_, current_subpass_, attachment_views_, tag, CurrentContext());
}

void RenderPassAccessContext::RecordLoadOperations(const ResourceUsageTag tag) {
    const auto *attachment_ci = rp_state_->create_info.pAttachments;
    auto &subpass_context = CurrentContext();
    const uint32_t view_mask = rp_state_->create_info.pSubpasses[current_subpass_].viewMask;

    for (uint32_t i = 0; i < rp_state_->create_info.attachmentCount; i++) {
        if (IsSubpassIncluded(current_subpass_, rp_state_->attachment_first_subpass[i], view_mask)) {
            const AttachmentViewGen &view_gen = attachment_views_[i];

            const VkAttachmentDescription2 &ci = *attachment_ci[i].ptr();
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            if (is_color) {
                // TODO: LoadOp can access the entire attachment subresource, not only the render area.
                // The exception is when a feedback loop is enabled for the attachment, then only the render area is accessed.
                const SyncAccessIndex load_op_access = ColorLoadUsage(ci.loadOp);
                if (load_op_access != SYNC_ACCESS_INDEX_NONE) {
                    const AttachmentAccess attachment_access =
                        GetAttachmentAccess(SyncOrdering::kColorAttachment, AttachmentAccessType::LoadOp);
                    subpass_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea, load_op_access,
                                                                attachment_access, ResourceUsageTagEx{tag}, view_mask);
                }
            } else {
                // TODO: Update depth/stencil aspects separately only if separateDepthStencilAttachmentAccess is defined,
                // otherwise both should be updated.
                // Also LoadOp can access the entire attachment subresource, not only the render area.
                // The exception is when a feedback loop is enabled for the attachment, then only the render area is accessed.
                const AttachmentAccess attachment_access =
                    GetAttachmentAccess(SyncOrdering::kDepthStencilAttachment, AttachmentAccessType::LoadOp);
                if (has_depth) {
                    const SyncAccessIndex load_op = DepthStencilLoadUsage(ci.loadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea, load_op,
                                                                    attachment_access, ResourceUsageTagEx{tag});
                    }
                }
                if (has_stencil) {
                    const SyncAccessIndex load_op = DepthStencilLoadUsage(ci.stencilLoadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAttachmentAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                                    load_op, attachment_access, ResourceUsageTagEx{tag});
                    }
                }
            }
        }
    }
}

AttachmentViewGenVector RenderPassAccessContext::CreateAttachmentViewGen(
    const VkRect2D &render_area, const std::vector<const vvl::ImageView *> &attachment_views) {
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
                                                 const std::vector<const vvl::ImageView *> &attachment_views,
                                                 const AccessContext &external_context, uint32_t render_pass_instance_id)
    : rp_state_(&rp_state),
      render_area_(render_area),
      attachment_views_(CreateAttachmentViewGen(render_area, attachment_views)),
      subpass_contexts_(InitSubpassContexts(queue_flags, rp_state, external_context)),
      render_pass_instance_id_(render_pass_instance_id),
      current_subpass_(0) {}

void RenderPassAccessContext::RecordBeginRenderPass(const ResourceUsageTag barrier_tag, const ResourceUsageTag load_tag) {
    assert(0 == current_subpass_);
    AccessContext &current_context = CurrentContext();
    current_context.SetStartTag(barrier_tag);

    RecordLayoutTransitions(barrier_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordNextSubpass(ResourceUsageTag resolve_tag, const ResourceUsageTag store_tag,
                                                const ResourceUsageTag transition_tag, const ResourceUsageTag load_tag) {
    const uint32_t view_mask = rp_state_->create_info.pSubpasses[current_subpass_].viewMask;

    // Resolves are against *prior* subpass context and thus *before* the subpass increment
    UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, render_pass_instance_id_, current_subpass_, view_mask, resolve_tag,
                                  CurrentContext());
    UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, render_pass_instance_id_, current_subpass_, view_mask, store_tag,
                                CurrentContext());

    if (current_subpass_ + 1 >= rp_state_->create_info.subpassCount) {
        return;
    }
    // Move to the next sub-command for the new subpass. The resolve and store are logically part of the previous
    // subpass, so their tag needs to be different from the layout and load operations below.
    current_subpass_++;
    AccessContext &current_context = CurrentContext();
    current_context.SetStartTag(transition_tag);

    RecordLayoutTransitions(transition_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordEndRenderPass(AccessContext *external_context, const ResourceUsageTag store_tag,
                                                  const ResourceUsageTag transition_tag) {
    const uint32_t view_mask = rp_state_->create_info.pSubpasses[current_subpass_].viewMask;

    // Add the resolve and store accesses
    UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, render_pass_instance_id_, current_subpass_, view_mask, store_tag,
                                  CurrentContext());
    UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, render_pass_instance_id_, current_subpass_, view_mask, store_tag,
                                CurrentContext());

    // Export the accesses from the renderpass...
    external_context->ResolveChildContexts(GetSubpassContexts());

    // Add the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    // Not that since *final* always comes from *one* subpass per view, we don't have to accumulate the barriers
    // TODO Aliasing we may need to reconsider barrier accumulation... though I don't know that it would be valid for aliasing
    //      that had mulitple final layout transistions from mulitple final subpasses.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const AttachmentViewGen &view_gen = attachment_views_[transition.attachment];
        const SubpassBarrier &dst_external_barrier = subpass_contexts_[transition.src_subpass].GetDstExternalSubpassBarrier();
        assert(&subpass_contexts_[transition.src_subpass] == dst_external_barrier.src_subpass_context);

        ImageRangeGen range_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kViewSubresource);

        ImageRangeGen markup_range_gen = range_gen;  // second copy, preserve range_gen to use later
        ApplyMarkupFunctor markup_action(true);
        external_context->UpdateMemoryAccessState(markup_action, markup_range_gen);

        PendingBarriers pending_barriers;
        for (const auto &barrier : dst_external_barrier.barriers) {
            const BarrierScope barrier_scope(barrier);
            CollectBarriersFunctor collect_barriers(*external_context, barrier_scope, barrier, true, vvl::kNoIndex32,
                                                    pending_barriers);
            external_context->UpdateMemoryAccessState(collect_barriers, range_gen);
        }
        pending_barriers.Apply(transition_tag);
    }
}

AccessContext &RenderPassAccessContext::CurrentContext() {
    assert(current_subpass_ < rp_state_->create_info.subpassCount);
    return subpass_contexts_[current_subpass_];
}

const AccessContext &RenderPassAccessContext::CurrentContext() const {
    assert(current_subpass_ < rp_state_->create_info.subpassCount);
    return subpass_contexts_[current_subpass_];
}

vvl::span<const AccessContext> RenderPassAccessContext::GetSubpassContexts() const {
    return vvl::make_span<const AccessContext>(subpass_contexts_.get(), rp_state_->create_info.subpassCount);
}

vvl::span<AccessContext> RenderPassAccessContext::GetSubpassContexts() {
    return vvl::make_span<AccessContext>(subpass_contexts_.get(), rp_state_->create_info.subpassCount);
}

AttachmentAccess RenderPassAccessContext::GetAttachmentAccess(SyncOrdering ordering, AttachmentAccessType type) const {
    AttachmentAccess attachment_access;
    attachment_access.type = type;
    attachment_access.ordering = ordering;
    attachment_access.render_pass_instance_id = render_pass_instance_id_;
    attachment_access.subpass = current_subpass_;
    return attachment_access;
}

void BeginRenderingCmdState::AddRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info) {
    info = std::make_unique<DynamicRenderingInfo>(state, rendering_info);
}

const DynamicRenderingInfo &BeginRenderingCmdState::GetRenderingInfo() const {
    assert(info);
    return *info;
}
DynamicRenderingInfo::DynamicRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info)
    : info(&rendering_info) {
    uint32_t attachment_count = info.colorAttachmentCount + (info.pDepthAttachment ? 1 : 0) + (info.pStencilAttachment ? 1 : 0);

    const VkOffset3D offset = CastTo3D(info.renderArea.offset);
    const VkExtent3D extent = CastTo3D(info.renderArea.extent);

    attachments.reserve(attachment_count);
    for (uint32_t i = 0; i < info.colorAttachmentCount; i++) {
        attachments.emplace_back(state, info.pColorAttachments[i], AttachmentType::kColor, offset, extent);
    }

    if (info.pDepthAttachment) {
        attachments.emplace_back(state, *info.pDepthAttachment, AttachmentType::kDepth, offset, extent);
    }

    if (info.pStencilAttachment) {
        attachments.emplace_back(state, *info.pStencilAttachment, AttachmentType::kStencil, offset, extent);
    }
}

const vvl::ImageView *DynamicRenderingInfo::GetClearAttachmentView(const VkClearAttachment &clear_attachment) const {
    const vvl::ImageView *attachment_view = nullptr;
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

DynamicRenderingInfo::Attachment::Attachment(const SyncValidator &state, const vku::safe_VkRenderingAttachmentInfo &attachment_info,
                                             AttachmentType type_, const VkOffset3D &offset, const VkExtent3D &extent)
    : info(attachment_info), view(state.Get<vvl::ImageView>(attachment_info.imageView)), view_gen(), type(type_) {
    if (view) {
        if (type == AttachmentType::kColor) {
            view_gen = MakeImageRangeGen(*view, offset, extent);
        } else if (type == AttachmentType::kDepth) {
            view_gen = MakeImageRangeGen(*view, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT);
        } else {
            view_gen = MakeImageRangeGen(*view, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT);
        }

        if (info.resolveImageView != VK_NULL_HANDLE && (info.resolveMode != VK_RESOLVE_MODE_NONE)) {
            resolve_view = state.Get<vvl::ImageView>(info.resolveImageView);
            if (resolve_view) {
                if (type == AttachmentType::kColor) {
                    resolve_gen.emplace(MakeImageRangeGen(*resolve_view, offset, extent));
                } else if (type == AttachmentType::kDepth) {
                    // Only the depth aspect
                    resolve_gen.emplace(MakeImageRangeGen(*resolve_view, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT));
                } else {
                    resolve_gen.emplace(MakeImageRangeGen(*resolve_view, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT));
                }
            }
        }
    }
}

SyncAccessIndex DynamicRenderingInfo::Attachment::GetLoadUsage() const {
    return GetLoadOpUsageIndex(info.loadOp, type);
}

SyncAccessIndex DynamicRenderingInfo::Attachment::GetStoreUsage() const {
    return GetStoreOpUsageIndex(info.storeOp, type);
}

SyncOrdering DynamicRenderingInfo::Attachment::GetOrdering() const {
    return (type == AttachmentType::kColor) ? SyncOrdering::kColorAttachment : SyncOrdering::kDepthStencilAttachment;
}

Location DynamicRenderingInfo::Attachment::GetLocation(const Location &loc, uint32_t attachment_index) const {
    if (type == AttachmentType::kColor) {
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pColorAttachments, attachment_index);
    } else if (type == AttachmentType::kDepth) {
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pDepthAttachment);
    } else {
        assert(type == AttachmentType::kStencil);
        return loc.dot(vvl::Struct::VkRenderingAttachmentInfo, vvl::Field::pStencilAttachment);
    }
}

bool DynamicRenderingInfo::Attachment::IsWriteable(const LastBound &last_bound_state) const {
    bool writeable = IsValid();
    if (writeable) {
        //  Depth and Stencil have additional criteria
        if (type == AttachmentType::kDepth) {
            writeable =
                last_bound_state.IsDepthWriteEnable() && IsDepthAttachmentWriteable(last_bound_state, view->create_info.format);
        } else if (type == AttachmentType::kStencil) {
            writeable =
                last_bound_state.IsStencilTestEnable() && IsStencilAttachmentWriteable(last_bound_state, view->create_info.format);
        }
    }
    return writeable;
}

}  // namespace syncval
