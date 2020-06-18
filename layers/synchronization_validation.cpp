/* Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include <limits>
#include <vector>
#include <memory>
#include <bitset>
#include "synchronization_validation.h"

static const char *string_SyncHazardVUID(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "SYNC-HAZARD-NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "SYNC-HAZARD-READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "SYNC-HAZARD-WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "SYNC-HAZARD-WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "SYNC-HAZARD-READ-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "SYNC-HAZARD-WRITE-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "SYNC-HAZARD-WRITE-RACING-READ";
            break;
        default:
            assert(0);
    }
    return "SYNC-HAZARD-INVALID";
}

static const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONR";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "READ_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "WRITE_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "WRITE_RACING_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

static constexpr VkPipelineStageFlags kColorAttachmentExecScope = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
static constexpr SyncStageAccessFlags kColorAttachmentAccessScope =
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE_BIT;
static constexpr VkPipelineStageFlags kDepthStencilAttachmentExecScope =
    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
static constexpr SyncStageAccessFlags kDepthStencilAttachmentAccessScope =
    SyncStageAccessFlagBits::SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SyncStageAccessFlagBits::SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

static constexpr SyncOrderingBarrier kColorAttachmentRasterOrder = {kColorAttachmentExecScope, kColorAttachmentAccessScope};
static constexpr SyncOrderingBarrier kDepthStencilAttachmentRasterOrder = {kDepthStencilAttachmentExecScope,
                                                                           kDepthStencilAttachmentAccessScope};
static constexpr SyncOrderingBarrier kAttachmentRasterOrder = {kDepthStencilAttachmentExecScope | kColorAttachmentExecScope,
                                                               kDepthStencilAttachmentAccessScope | kColorAttachmentAccessScope};
// Sometimes we have an internal access conflict, and we using the kCurrentCommandTag to set and detect in temporary/proxy contexts
static const ResourceUsageTag kCurrentCommandTag(ResourceUsageTag::kMaxIndex);

inline VkDeviceSize GetRealWholeSize(VkDeviceSize offset, VkDeviceSize size, VkDeviceSize whole_size) {
    if (size == VK_WHOLE_SIZE) {
        return (whole_size - offset);
    }
    return size;
}

template <typename T>
static ResourceAccessRange MakeRange(const T &has_offset_and_size) {
    return ResourceAccessRange(has_offset_and_size.offset, (has_offset_and_size.offset + has_offset_and_size.size));
}

static ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size) { return ResourceAccessRange(start, (start + size)); }

// Expand the pipeline stage without regard to whether the are valid w.r.t. queue or extension
VkPipelineStageFlags ExpandPipelineStages(VkQueueFlags queue_flags, VkPipelineStageFlags stage_mask) {
    VkPipelineStageFlags expanded = stage_mask;
    if (VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        for (const auto &all_commands : syncAllCommandStagesByQueueFlags) {
            if (all_commands.first & queue_flags) {
                expanded |= all_commands.second;
            }
        }
    }
    if (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        expanded |= syncAllCommandStagesByQueueFlags.at(VK_QUEUE_GRAPHICS_BIT) & ~VK_PIPELINE_STAGE_HOST_BIT;
    }
    return expanded;
}

VkPipelineStageFlags RelatedPipelineStages(VkPipelineStageFlags stage_mask,
                                           std::map<VkPipelineStageFlagBits, VkPipelineStageFlags> &map) {
    VkPipelineStageFlags unscanned = stage_mask;
    VkPipelineStageFlags related = 0;
    for (const auto entry : map) {
        const auto stage = entry.first;
        if (stage & unscanned) {
            related = related | entry.second;
            unscanned = unscanned & ~stage;
            if (!unscanned) break;
        }
    }
    return related;
}

VkPipelineStageFlags WithEarlierPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages);
}

VkPipelineStageFlags WithLaterPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages);
}

static const ResourceAccessRange full_range(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());

// Class AccessContext stores the state of accesses specific to a Command, Subpass, or Queue
const std::array<AccessContext::AddressType, AccessContext::kAddressTypeCount> AccessContext::kAddressTypes = {
    AccessContext::AddressType::kLinearAddress, AccessContext::AddressType::kIdealizedAddress};

// Tranverse the attachment resolves for this a specific subpass, and do action() to them.
// Used by both validation and record operations
//
// The signature for Action() reflect the needs of both uses.
template <typename Action>
void ResolveOperation(Action &action, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                      const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass) {
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);
    const auto &rp_ci = rp_state.createInfo;
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
                       SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, kColorAttachmentRasterOrder, offset, extent, 0);
                action("color", "resolve write", color_attach, resolve_attach, attachment_views[resolve_attach],
                       SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, kColorAttachmentRasterOrder, offset, extent, 0);
            }
        }
    }

    // Depth stencil resolve only if the extension is present
    const auto ds_resolve = lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolve>(subpass_ci.pNext);
    if (ds_resolve && ds_resolve->pDepthStencilResolveAttachment &&
        (ds_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) && subpass_ci.pDepthStencilAttachment &&
        (subpass_ci.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
        const auto src_at = subpass_ci.pDepthStencilAttachment->attachment;
        const auto src_ci = attachment_ci[src_at];
        // The formats are required to match so we can pick either
        const bool resolve_depth = (ds_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasDepth(src_ci.format);
        const bool resolve_stencil = (ds_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasStencil(src_ci.format);
        const auto dst_at = ds_resolve->pDepthStencilResolveAttachment->attachment;
        VkImageAspectFlags aspect_mask = 0u;

        // Figure out which aspects are actually touched during resolve operations
        const char *aspect_string = nullptr;
        if (resolve_depth && resolve_stencil) {
            // Validate all aspects together
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            aspect_string = "depth/stencil";
        } else if (resolve_depth) {
            // Validate depth only
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
            aspect_string = "depth";
        } else if (resolve_stencil) {
            // Validate all stencil only
            aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
            aspect_string = "stencil";
        }

        if (aspect_mask) {
            action(aspect_string, "resolve read", src_at, dst_at, attachment_views[src_at],
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, kDepthStencilAttachmentRasterOrder, offset, extent,
                   aspect_mask);
            action(aspect_string, "resolve write", src_at, dst_at, attachment_views[dst_at],
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, kAttachmentRasterOrder, offset, extent, aspect_mask);
        }
    }
}

// Action for validating resolve operations
class ValidateResolveAction {
  public:
    ValidateResolveAction(VkRenderPass render_pass, uint32_t subpass, const AccessContext &context, const SyncValidator &sync_state,
                          const char *func_name)
        : render_pass_(render_pass),
          subpass_(subpass),
          context_(context),
          sync_state_(sync_state),
          func_name_(func_name),
          skip_(false) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const SyncOrderingBarrier &ordering,
                    const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask) {
        HazardResult hazard;
        hazard = context_.DetectHazard(view, current_usage, ordering, offset, extent, aspect_mask);
        if (hazard.hazard) {
            skip_ |= sync_state_.LogError(
                render_pass_, string_SyncHazardVUID(hazard.hazard),
                "%s: Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32 " to resolve attachment %" PRIu32 ".",
                func_name_, string_SyncHazard(hazard.hazard), subpass_, aspect_name, attachment_name, src_at, dst_at);
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const AccessContext &context_;
    const SyncValidator &sync_state_;
    const char *func_name_;
    bool skip_;
};

// Update action for resolve operations
class UpdateStateResolveAction {
  public:
    UpdateStateResolveAction(AccessContext &context, const ResourceUsageTag &tag) : context_(context), tag_(tag) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const SyncOrderingBarrier &ordering,
                    const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask) {
        // Ignores validation only arguments...
        context_.UpdateAccessState(view, current_usage, offset, extent, aspect_mask, tag_);
    }

  private:
    AccessContext &context_;
    const ResourceUsageTag &tag_;
};

AccessContext::AccessContext(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies,
                             const std::vector<AccessContext> &contexts, AccessContext *external_context) {
    Reset();
    const auto &subpass_dep = dependencies[subpass];
    prev_.reserve(subpass_dep.prev.size());
    prev_by_subpass_.resize(subpass, nullptr);  // Can't be more prevs than the subpass we're on
    for (const auto &prev_dep : subpass_dep.prev) {
        assert(prev_dep.dependency);
        const auto dep = *prev_dep.dependency;
        prev_.emplace_back(const_cast<AccessContext *>(&contexts[dep.srcSubpass]), queue_flags, dep);
        prev_by_subpass_[dep.srcSubpass] = &prev_.back();
    }

    async_.reserve(subpass_dep.async.size());
    for (const auto async_subpass : subpass_dep.async) {
        async_.emplace_back(const_cast<AccessContext *>(&contexts[async_subpass]));
    }
    if (subpass_dep.barrier_from_external) {
        src_external_ = TrackBack(external_context, queue_flags, *subpass_dep.barrier_from_external);
    } else {
        src_external_ = TrackBack();
    }
    if (subpass_dep.barrier_to_external) {
        dst_external_ = TrackBack(this, queue_flags, *subpass_dep.barrier_to_external);
    } else {
        dst_external_ = TrackBack();
    }
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(AddressType type, const Detector &detector,
                                                 const ResourceAccessRange &range) const {
    ResourceAccessRangeMap descent_map;
    ResolvePreviousAccess(type, range, &descent_map, nullptr);

    HazardResult hazard;
    for (auto prev = descent_map.begin(); prev != descent_map.end() && !hazard.hazard; ++prev) {
        hazard = detector.Detect(prev);
    }
    return hazard;
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessContext::DetectHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range,
                                         DetectOptions options) const {
    HazardResult hazard;

    if (static_cast<uint32_t>(options) | DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_context : async_) {
            hazard = async_context->DetectAsyncHazard(type, detector, range);
            if (hazard.hazard) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) | DetectOptions::kDetectPrevious) != 0;

    const auto &accesses = GetAccessStateMap(type);
    const auto from = accesses.lower_bound(range);
    const auto to = accesses.upper_bound(range);
    ResourceAccessRange gap = {range.begin, range.begin};

    for (auto pos = from; pos != to; ++pos) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(type, detector, gap);
                if (hazard.hazard) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.hazard) return hazard;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(type, detector, gap);
        }
    }

    return hazard;
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range) const {
    auto &accesses = GetAccessStateMap(type);
    const auto from = accesses.lower_bound(range);
    const auto to = accesses.upper_bound(range);

    HazardResult hazard;
    for (auto pos = from; pos != to && !hazard.hazard; ++pos) {
        hazard = detector.DetectAsync(pos);
    }

    return hazard;
}

static void ApplyGlobalBarrier(const SyncBarrier &barrier, ResourceAccessRangeMap *access_map) {
    for (auto &entry : *access_map) {
        entry.second.ApplyBarrier(barrier);
    }
}

// Returns the last resolved entry
static void ResolveMapToEntry(ResourceAccessRangeMap *dest, ResourceAccessRangeMap::iterator entry,
                              ResourceAccessRangeMap::const_iterator first, ResourceAccessRangeMap::const_iterator last,
                              const SyncBarrier *barrier) {
    auto at = entry;
    for (auto pos = first; pos != last; ++pos) {
        // Every member of the input iterator range must fit within the remaining portion of entry
        assert(at->first.includes(pos->first));
        assert(at != dest->end());
        // Trim up at to the same size as the entry to resolve
        at = sparse_container::split(at, *dest, pos->first);
        auto access = pos->second;
        if (barrier) {
            access.ApplyBarrier(*barrier);
        }
        at->second.Resolve(access);
        ++at;  // Go to the remaining unused section of entry
    }
}

void AccessContext::ResolveAccessRange(AddressType type, const ResourceAccessRange &range, const SyncBarrier *barrier,
                                       ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                                       bool recur_to_infill) const {
    ResourceRangeMergeIterator current(*resolve_map, GetAccessStateMap(type), range.begin);
    while (current->range.non_empty() && range.includes(current->range.begin)) {
        if (current->pos_B->valid) {
            const auto &src_pos = current->pos_B->lower_bound;
            auto access = src_pos->second;
            if (barrier) {
                access.ApplyBarrier(*barrier);
            }
            if (current->pos_A->valid) {
                current.trim_A();
                current->pos_A->lower_bound->second.Resolve(access);
            } else {
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current->range, access));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the insert segment
            }
        } else {
            // we have to descend to fill this gap
            if (recur_to_infill) {
                if (current->pos_A->valid) {
                    // Dest is valid, so we need to accumulate along the DAG and then resolve... in an N-to-1 resolve operation
                    ResourceAccessRangeMap gap_map;
                    ResolvePreviousAccess(type, current->range, &gap_map, infill_state);
                    ResolveMapToEntry(resolve_map, current->pos_A->lower_bound, gap_map.begin(), gap_map.end(), barrier);
                } else {
                    // There isn't anything in dest in current->range, so we can accumulate directly into it.
                    ResolvePreviousAccess(type, current->range, resolve_map, infill_state);
                    if (barrier) {
                        // Need to apply the barrier to the accesses we accumulated, noting that we haven't updated current
                        for (auto pos = resolve_map->lower_bound(current->range); pos != current->pos_A->lower_bound; ++pos) {
                            pos->second.ApplyBarrier(*barrier);
                        }
                    }
                }
                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = current->range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.invalidate_A();                       // Changes current->range
                current.seek(seek_to);
            } else if (!current->pos_A->valid && infill_state) {
                // If we didn't find anything in the current range, and we aren't reccuring... we infill if required
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current->range, *infill_state));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the correct segment after insert
            }
        }
        ++current;
    }
}

void AccessContext::ResolvePreviousAccess(AddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                          const ResourceAccessState *infill_state) const {
    if ((prev_.size() == 0) && (src_external_.context == nullptr)) {
        if (range.non_empty() && infill_state) {
            descent_map->insert(std::make_pair(range, *infill_state));
        }
    } else {
        // Look for something to fill the gap further along.
        for (const auto &prev_dep : prev_) {
            prev_dep.context->ResolveAccessRange(type, range, &prev_dep.barrier, descent_map, infill_state);
        }

        if (src_external_.context) {
            src_external_.context->ResolveAccessRange(type, range, &src_external_.barrier, descent_map, infill_state);
        }
    }
}

AccessContext::AddressType AccessContext::ImageAddressType(const IMAGE_STATE &image) {
    return (image.fragment_encoder->IsLinearImage()) ? AddressType::kLinearAddress : AddressType::kIdealizedAddress;
}

VkDeviceSize AccessContext::ResourceBaseAddress(const BINDABLE &bindable) {
    return bindable.binding.offset + bindable.binding.mem_state->fake_base_address;
}

static bool SimpleBinding(const BINDABLE &bindable) { return !bindable.sparse && bindable.binding.mem_state; }

static SyncStageAccessIndex ColorLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                                      : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    return stage_access;
}
static SyncStageAccessIndex DepthStencilLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                                      : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    return stage_access;
}

// Caller must manage returned pointer
static AccessContext *CreateStoreResolveProxyContext(const AccessContext &context, const RENDER_PASS_STATE &rp_state,
                                                     uint32_t subpass, const VkRect2D &render_area,
                                                     std::vector<const IMAGE_VIEW_STATE *> attachment_views) {
    auto *proxy = new AccessContext(context);
    proxy->UpdateAttachmentResolveAccess(rp_state, render_area, attachment_views, subpass, kCurrentCommandTag);
    proxy->UpdateAttachmentStoreAccess(rp_state, render_area, attachment_views, subpass, kCurrentCommandTag);
    return proxy;
}

void AccessContext::ResolvePreviousAccess(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range_arg,
                                          AddressType address_type, ResourceAccessRangeMap *descent_map,
                                          const ResourceAccessState *infill_state) const {
    if (!SimpleBinding(image_state)) return;

    auto subresource_range = NormalizeSubresourceRange(image_state.createInfo, subresource_range_arg);
    subresource_adapter::ImageRangeGenerator range_gen(*image_state.fragment_encoder.get(), subresource_range, {0, 0, 0},
                                                       image_state.createInfo.extent);
    const auto base_address = ResourceBaseAddress(image_state);
    for (; range_gen->non_empty(); ++range_gen) {
        ResolvePreviousAccess(address_type, (*range_gen + base_address), descent_map, infill_state);
    }
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool AccessContext::ValidateLayoutTransitions(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, uint32_t subpass,
                                              const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                              const char *func_name) const {
    bool skip = false;
    // As validation methods are const and precede the record/update phase, for any tranistions from the immediately
    // previous subpass, we have to validate them against a copy of the AccessContext, with resolve operations applied, as
    // those affects have not been recorded yet.
    //
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> proxy_for_prev;
    TrackBack proxy_track_back;

    const auto &transitions = rp_state.subpass_transitions[subpass];
    for (const auto &transition : transitions) {
        const bool prev_needs_proxy = transition.prev_pass != VK_SUBPASS_EXTERNAL && (transition.prev_pass + 1 == subpass);

        const auto *track_back = GetTrackBackFromSubpass(transition.prev_pass);
        if (prev_needs_proxy) {
            if (!proxy_for_prev) {
                proxy_for_prev.reset(CreateStoreResolveProxyContext(*track_back->context, rp_state, transition.prev_pass,
                                                                    render_area, attachment_views));
                proxy_track_back = *track_back;
                proxy_track_back.context = proxy_for_prev.get();
            }
            track_back = &proxy_track_back;
        }
        auto hazard = DetectSubpassTransitionHazard(*track_back, attachment_views[transition.attachment]);
        if (hazard.hazard) {
            skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                        "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " image layout transition.",
                                        func_name, string_SyncHazard(hazard.hazard), subpass, transition.attachment);
        }
    }
    return skip;
}

bool AccessContext::ValidateLoadOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                          const VkRect2D &render_area, uint32_t subpass,
                                          const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                          const char *func_name) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);
    const auto external_access_scope = src_external_.barrier.dst_access_scope;

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_first_subpass[i]) {
            if (attachment_views[i] == nullptr) continue;
            const IMAGE_VIEW_STATE &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;
            const auto &ci = attachment_ci[i];
            const bool is_transition = rp_state.attachment_first_is_transition[i];

            // Need check in the following way
            // 1) if the usage bit isn't in the dest_access_scope, and there is layout traniition for initial use, report hazard
            //    vs. transition
            // 2) if there isn't a layout transition, we need to look at the  external context with a "detect hazard" operation
            //    for each aspect loaded.

            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            const SyncStageAccessIndex load_index = has_depth ? DepthStencilLoadUsage(ci.loadOp) : ColorLoadUsage(ci.loadOp);
            const SyncStageAccessFlags load_mask = (has_depth || is_color) ? SyncStageAccess::Flags(load_index) : 0U;
            const SyncStageAccessIndex stencil_load_index = has_stencil ? DepthStencilLoadUsage(ci.stencilLoadOp) : load_index;
            const SyncStageAccessFlags stencil_mask = has_stencil ? SyncStageAccess::Flags(stencil_load_index) : 0U;

            HazardResult hazard;
            const char *aspect = nullptr;
            if (is_transition) {
                // For transition w
                SyncHazard transition_hazard = SyncHazard::NONE;
                bool checked_stencil = false;
                if (load_mask) {
                    if ((load_mask & external_access_scope) != load_mask) {
                        transition_hazard =
                            SyncStageAccess::HasWrite(load_mask) ? SyncHazard::WRITE_AFTER_WRITE : SyncHazard::READ_AFTER_WRITE;
                        aspect = is_color ? "color" : "depth";
                    }
                    if (!transition_hazard && stencil_mask) {
                        if ((stencil_mask & external_access_scope) != stencil_mask) {
                            transition_hazard = SyncStageAccess::HasWrite(stencil_mask) ? SyncHazard::WRITE_AFTER_WRITE
                                                                                        : SyncHazard::READ_AFTER_WRITE;
                            aspect = "stencil";
                            checked_stencil = true;
                        }
                    }
                }
                if (transition_hazard) {
                    // Hazard vs. ILT
                    auto load_op_string = string_VkAttachmentLoadOp(checked_stencil ? ci.stencilLoadOp : ci.loadOp);
                    skip |=
                        sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                            "%s: Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32
                                            " aspect %s during load with loadOp %s.",
                                            func_name, string_SyncHazard(transition_hazard), subpass, i, aspect, load_op_string);
                }
            } else {
                auto hazard_range = view.normalized_subresource_range;
                bool checked_stencil = false;
                if (is_color) {
                    hazard = DetectHazard(*image, load_index, view.normalized_subresource_range, offset, extent);
                    aspect = "color";
                } else {
                    if (has_depth) {
                        hazard_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                        hazard = DetectHazard(*image, load_index, hazard_range, offset, extent);
                        aspect = "depth";
                    }
                    if (!hazard.hazard && has_stencil) {
                        hazard_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                        hazard = DetectHazard(*image, stencil_load_index, hazard_range, offset, extent);
                        aspect = "stencil";
                        checked_stencil = true;
                    }
                }

                if (hazard.hazard) {
                    auto load_op_string = string_VkAttachmentLoadOp(checked_stencil ? ci.stencilLoadOp : ci.loadOp);
                    skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s.",
                                                func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, load_op_string);
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
bool AccessContext::ValidateStoreOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                           const VkRect2D &render_area, uint32_t subpass,
                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                           const char *func_name) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_last_subpass[i]) {
            if (attachment_views[i] == nullptr) continue;
            const IMAGE_VIEW_STATE &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;
            const auto &ci = attachment_ci[i];

            // The spec states that "don't care" is an operation with VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // so we assume that an implementation is *free* to write in that case, meaning that for correctness
            // sake, we treat DONT_CARE as writing.
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
            if (!has_stencil && !store_op_stores) continue;

            HazardResult hazard;
            const char *aspect = nullptr;
            bool checked_stencil = false;
            if (is_color) {
                hazard = DetectHazard(*image, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                      view.normalized_subresource_range, kAttachmentRasterOrder, offset, extent);
                aspect = "color";
            } else {
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
                auto hazard_range = view.normalized_subresource_range;
                if (has_depth && store_op_stores) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    hazard = DetectHazard(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, hazard_range,
                                          kAttachmentRasterOrder, offset, extent);
                    aspect = "depth";
                }
                if (!hazard.hazard && has_stencil && stencil_op_stores) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    hazard = DetectHazard(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, hazard_range,
                                          kAttachmentRasterOrder, offset, extent);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.hazard) {
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const char *const store_op_string = string_VkAttachmentStoreOp(checked_stencil ? ci.stencilStoreOp : ci.storeOp);
                skip |= sync_state.LogError(
                    rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " %s aspect during store with %s %s.", func_name,
                    string_SyncHazard(hazard.hazard), subpass, i, aspect, op_type_string, store_op_string);
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateResolveOperations(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area,
                                              const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const char *func_name,
                                              uint32_t subpass) const {
    ValidateResolveAction validate_action(rp_state.renderPass, subpass, *this, sync_state, func_name);
    ResolveOperation(validate_action, rp_state, render_area, attachment_views, subpass);
    return validate_action.GetSkip();
}

class HazardDetector {
    SyncStageAccessIndex usage_index_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const { return pos->second.DetectHazard(usage_index_); }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectAsyncHazard(usage_index_);
    }
    HazardDetector(SyncStageAccessIndex usage) : usage_index_(usage) {}
};

class HazardDetectorWithOrdering {
    const SyncStageAccessIndex usage_index_;
    const SyncOrderingBarrier &ordering_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(usage_index_, ordering_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectAsyncHazard(usage_index_);
    }
    HazardDetectorWithOrdering(SyncStageAccessIndex usage, const SyncOrderingBarrier &ordering)
        : usage_index_(usage), ordering_(ordering) {}
};

HazardResult AccessContext::DetectHazard(AddressType type, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    HazardDetector detector(usage_index);
    return DetectHazard(type, detector, range, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    return DetectHazard(AddressType::kLinearAddress, usage_index, range + ResourceBaseAddress(buffer));
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const IMAGE_STATE &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, DetectOptions options) const {
    if (!SimpleBinding(image)) return HazardResult();
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent);
    const auto address_type = ImageAddressType(image);
    const auto base_address = ResourceBaseAddress(image);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(address_type, detector, (*range_gen + base_address), options);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                         const VkExtent3D &extent) const {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    return DetectHazard(image, current_usage, subresource_range, offset, extent);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, const SyncOrderingBarrier &ordering,
                                         const VkOffset3D &offset, const VkExtent3D &extent) const {
    HazardDetectorWithOrdering detector(current_usage, ordering);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

// Some common code for looking at attachments, if there's anything wrong, we return no hazard, core validation
// should have reported the issue regarding an invalid attachment entry
HazardResult AccessContext::DetectHazard(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage,
                                         const SyncOrderingBarrier &ordering, const VkOffset3D &offset, const VkExtent3D &extent,
                                         VkImageAspectFlags aspect_mask) const {
    if (view != nullptr) {
        const IMAGE_STATE *image = view->image_state.get();
        if (image != nullptr) {
            auto *detect_range = &view->normalized_subresource_range;
            VkImageSubresourceRange masked_range;
            if (aspect_mask) {  // If present and non-zero, restrict the normalized range to aspects present in aspect_mask
                masked_range = view->normalized_subresource_range;
                masked_range.aspectMask = aspect_mask & masked_range.aspectMask;
                detect_range = &masked_range;
            }

            // NOTE: The range encoding code is not robust to invalid ranges, so we protect it from our change
            if (detect_range->aspectMask) {
                return DetectHazard(*image, current_usage, *detect_range, ordering, offset, extent);
            }
        }
    }
    return HazardResult();
}
class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_index_(usage_index), src_exec_scope_(src_exec_scope), src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(usage_index_, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_index_);
    }

  private:
    SyncStageAccessIndex usage_index_;
    VkPipelineStageFlags src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

HazardResult AccessContext::DetectBarrierHazard(AddressType type, SyncStageAccessIndex current_usage,
                                                VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                                const ResourceAccessRange &range, DetectOptions options) const {
    BarrierHazardDetector detector(current_usage, src_exec_scope, src_access_scope);
    return DetectHazard(type, detector, range, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                                     SyncStageAccessFlags src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    VkOffset3D zero_offset = {0, 0, 0};
    return DetectHazard(detector, image, subresource_range, zero_offset, image.createInfo.extent, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                                     SyncStageAccessFlags src_stage_accesses,
                                                     const VkImageMemoryBarrier &barrier) const {
    auto subresource_range = NormalizeSubresourceRange(image.createInfo, barrier.subresourceRange);
    const auto src_access_scope = SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask);
    return DetectImageBarrierHazard(image, src_exec_scope, src_access_scope, subresource_range, kDetectAll);
}

template <typename Flags, typename Map>
SyncStageAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncStageAccessFlags scope = 0;
    for (const auto &bit_scope : map) {
        if (flag_mask < bit_scope.first) break;

        if (flag_mask & bit_scope.first) {
            scope |= bit_scope.second;
        }
    }
    return scope;
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags stages) {
    return AccessScopeImpl(stages, syncStageAccessMaskByStageBit);
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags accesses) {
    return AccessScopeImpl(accesses, syncStageAccessMaskByAccessBit);
}

// Getting from stage mask and access mask to stage/acess masks is something we need to be good at...
SyncStageAccessFlags SyncStageAccess::AccessScope(VkPipelineStageFlags stages, VkAccessFlags accesses) {
    // The access scope is the intersection of all stage/access types possible for the enabled stages and the enables
    // accesses (after doing a couple factoring of common terms the union of stage/access intersections is the intersections
    // of the union of all stage/access types for all the stages and the same unions for the access mask...
    return AccessScopeByStage(stages) & AccessScopeByAccess(accesses);
}

template <typename Action>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const ResourceAccessRange &range, const Action &action) {
    // TODO: Optimization for operations that do a pure overwrite (i.e. WRITE usages which rewrite the state, vs READ usages
    //       that do incrementalupdates
    auto pos = accesses->lower_bound(range);
    if (pos == accesses->end() || !pos->first.intersects(range)) {
        // The range is empty, fill it with a default value.
        pos = action.Infill(accesses, pos, range);
    } else if (range.begin < pos->first.begin) {
        // Leading empty space, infill
        pos = action.Infill(accesses, pos, ResourceAccessRange(range.begin, pos->first.begin));
    } else if (pos->first.begin < range.begin) {
        // Trim the beginning if needed
        pos = accesses->split(pos, range.begin, sparse_container::split_op_keep_both());
        ++pos;
    }

    const auto the_end = accesses->end();
    while ((pos != the_end) && pos->first.intersects(range)) {
        if (pos->first.end > range.end) {
            pos = accesses->split(pos, range.end, sparse_container::split_op_keep_both());
        }

        pos = action(accesses, pos);
        if (pos == the_end) break;

        auto next = pos;
        ++next;
        if ((pos->first.end < range.end) && (next != the_end) && !next->first.is_subsequent_to(pos->first)) {
            // Need to infill if next is disjoint
            VkDeviceSize limit = (next == the_end) ? range.end : std::min(range.end, next->first.begin);
            ResourceAccessRange new_range(pos->first.end, limit);
            next = action.Infill(accesses, next, new_range);
        }
        pos = next;
    }
}

struct UpdateMemoryAccessStateFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const {
        // this is only called on gaps, and never returns a gap.
        ResourceAccessState default_state;
        context.ResolvePreviousAccess(type, range, accesses, &default_state);
        return accesses->lower_bound(range);
    }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.Update(usage, tag);
        return pos;
    }

    UpdateMemoryAccessStateFunctor(AccessContext::AddressType type_, const AccessContext &context_, SyncStageAccessIndex usage_,
                                   const ResourceUsageTag &tag_)
        : type(type_), context(context_), usage(usage_), tag(tag_) {}
    const AccessContext::AddressType type;
    const AccessContext &context;
    const SyncStageAccessIndex usage;
    const ResourceUsageTag &tag;
};

struct ApplyMemoryAccessBarrierFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.ApplyMemoryAccessBarrier(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
        return pos;
    }

    ApplyMemoryAccessBarrierFunctor(VkPipelineStageFlags src_exec_scope_, SyncStageAccessFlags src_access_scope_,
                                    VkPipelineStageFlags dst_exec_scope_, SyncStageAccessFlags dst_access_scope_)
        : src_exec_scope(src_exec_scope_),
          src_access_scope(src_access_scope_),
          dst_exec_scope(dst_exec_scope_),
          dst_access_scope(dst_access_scope_) {}

    VkPipelineStageFlags src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    VkPipelineStageFlags dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
};

struct ApplyGlobalBarrierFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.ApplyExecutionBarrier(src_exec_scope, dst_exec_scope);

        for (const auto &functor : barrier_functor) {
            functor(accesses, pos);
        }
        return pos;
    }

    ApplyGlobalBarrierFunctor(VkPipelineStageFlags src_exec_scope, VkPipelineStageFlags dst_exec_scope,
                              SyncStageAccessFlags src_stage_accesses, SyncStageAccessFlags dst_stage_accesses,
                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers)
        : src_exec_scope(src_exec_scope), dst_exec_scope(dst_exec_scope) {
        // Don't want to create this per tracked item, but don't want to loop through all tracked items per barrier...
        barrier_functor.reserve(memoryBarrierCount);
        for (uint32_t barrier_index = 0; barrier_index < memoryBarrierCount; barrier_index++) {
            const auto &barrier = pMemoryBarriers[barrier_index];
            barrier_functor.emplace_back(src_exec_scope, SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask),
                                         dst_exec_scope, SyncStageAccess::AccessScope(dst_stage_accesses, barrier.dstAccessMask));
        }
    }

    const VkPipelineStageFlags src_exec_scope;
    const VkPipelineStageFlags dst_exec_scope;
    std::vector<ApplyMemoryAccessBarrierFunctor> barrier_functor;
};

void AccessContext::UpdateAccessState(AddressType type, SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                                      const ResourceUsageTag &tag) {
    UpdateMemoryAccessStateFunctor action(type, *this, current_usage, tag);
    UpdateMemoryAccessState(&GetAccessStateMap(type), range, action);
}

void AccessContext::UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage,
                                      const ResourceAccessRange &range, const ResourceUsageTag &tag) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateAccessState(AddressType::kLinearAddress, current_usage, range + base_address, tag);
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag &tag) {
    if (!SimpleBinding(image)) return;
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent);
    const auto address_type = ImageAddressType(image);
    const auto base_address = ResourceBaseAddress(image);
    UpdateMemoryAccessStateFunctor action(address_type, *this, current_usage, tag);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(&GetAccessStateMap(address_type), (*range_gen + base_address), action);
    }
}
void AccessContext::UpdateAccessState(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const VkOffset3D &offset,
                                      const VkExtent3D &extent, VkImageAspectFlags aspect_mask, const ResourceUsageTag &tag) {
    if (view != nullptr) {
        const IMAGE_STATE *image = view->image_state.get();
        if (image != nullptr) {
            auto *update_range = &view->normalized_subresource_range;
            VkImageSubresourceRange masked_range;
            if (aspect_mask) {  // If present and non-zero, restrict the normalized range to aspects present in aspect_mask
                masked_range = view->normalized_subresource_range;
                masked_range.aspectMask = aspect_mask & masked_range.aspectMask;
                update_range = &masked_range;
            }
            UpdateAccessState(*image, current_usage, *update_range, offset, extent, tag);
        }
    }
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                      const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag &tag) {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    UpdateAccessState(image, current_usage, subresource_range, offset, extent, tag);
}

template <typename Action>
void AccessContext::UpdateMemoryAccess(const BUFFER_STATE &buffer, const ResourceAccessRange &range, const Action action) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateMemoryAccessState(&GetAccessStateMap(AddressType::kLinearAddress), (range + base_address), action);
}

template <typename Action>
void AccessContext::UpdateMemoryAccess(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                                       const Action action) {
    if (!SimpleBinding(image)) return;
    const auto address_type = ImageAddressType(image);
    auto *accesses = &GetAccessStateMap(address_type);

    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, {0, 0, 0},
                                                       image.createInfo.extent);

    const auto base_address = ResourceBaseAddress(image);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(accesses, (*range_gen + base_address), action);
    }
}

void AccessContext::UpdateAttachmentResolveAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                                  const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                                  const ResourceUsageTag &tag) {
    UpdateStateResolveAction update(*this, tag);
    ResolveOperation(update, rp_state, render_area, attachment_views, subpass);
}

void AccessContext::UpdateAttachmentStoreAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                                const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                                const ResourceUsageTag &tag) {
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (rp_state.attachment_last_subpass[i] == subpass) {
            if (attachment_views[i] == nullptr) continue;  // UNUSED
            const auto &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;

            const auto &ci = attachment_ci[i];
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;

            if (is_color && store_op_stores) {
                UpdateAccessState(*image, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, view.normalized_subresource_range,
                                  offset, extent, tag);
            } else {
                auto update_range = view.normalized_subresource_range;
                if (has_depth && store_op_stores) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    UpdateAccessState(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, update_range, offset, extent,
                                      tag);
                }
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
                if (has_stencil && stencil_op_stores) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    UpdateAccessState(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, update_range, offset, extent,
                                      tag);
                }
            }
        }
    }
}

template <typename Action>
void AccessContext::ApplyGlobalBarriers(const Action &barrier_action) {
    // Note: Barriers do *not* cross context boundaries, applying to accessess within.... (at least for renderpass subpasses)
    for (const auto address_type : kAddressTypes) {
        UpdateMemoryAccessState(&GetAccessStateMap(address_type), full_range, barrier_action);
    }
}

void AccessContext::ResolveChildContexts(const std::vector<AccessContext> &contexts) {
    for (uint32_t subpass_index = 0; subpass_index < contexts.size(); subpass_index++) {
        auto &context = contexts[subpass_index];
        for (const auto address_type : kAddressTypes) {
            context.ResolveAccessRange(address_type, full_range, &context.GetDstExternalTrackBack().barrier,
                                       &GetAccessStateMap(address_type), nullptr, false);
        }
    }
}

void AccessContext::ApplyImageBarrier(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                      SyncStageAccessFlags src_access_scope, VkPipelineStageFlags dst_exec_scope,
                                      SyncStageAccessFlags dst_access_scope, const VkImageSubresourceRange &subresource_range) {
    const ApplyMemoryAccessBarrierFunctor barrier_action(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
    UpdateMemoryAccess(image, subresource_range, barrier_action);
}

// Note: ImageBarriers do not operate at offset/extent resolution, only at the whole subreources level
void AccessContext::ApplyImageBarrier(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                      SyncStageAccessFlags src_access_scope, VkPipelineStageFlags dst_exec_scope,
                                      SyncStageAccessFlags dst_access_scope, const VkImageSubresourceRange &subresource_range,
                                      bool layout_transition, const ResourceUsageTag &tag) {
    if (layout_transition) {
        UpdateAccessState(image, SYNC_IMAGE_LAYOUT_TRANSITION, subresource_range, VkOffset3D{0, 0, 0}, image.createInfo.extent,
                          tag);
        ApplyImageBarrier(image, src_exec_scope, SYNC_IMAGE_LAYOUT_TRANSITION_BIT, dst_exec_scope, dst_access_scope,
                          subresource_range);
    } else {
        ApplyImageBarrier(image, src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope, subresource_range);
    }
}

// Note: ImageBarriers do not operate at offset/extent resolution, only at the whole subreources level
void AccessContext::ApplyImageBarrier(const IMAGE_STATE &image, const SyncBarrier &barrier,
                                      const VkImageSubresourceRange &subresource_range, bool layout_transition,
                                      const ResourceUsageTag &tag) {
    ApplyImageBarrier(image, barrier.src_exec_scope, barrier.src_access_scope, barrier.dst_exec_scope, barrier.dst_access_scope,
                      subresource_range, layout_transition, tag);
}

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const TrackBack &track_back, const IMAGE_VIEW_STATE *attach_view) const {
    if (!attach_view) return HazardResult();
    const auto image_state = attach_view->image_state.get();
    if (!image_state) return HazardResult();

    // We should never ask for a transition from a context we don't have
    assert(track_back.context);

    // Do the detection against the specific prior context independent of other contexts.  (Synchronous only)
    auto hazard = track_back.context->DetectImageBarrierHazard(*image_state, track_back.barrier.src_exec_scope,
                                                               track_back.barrier.src_access_scope,
                                                               attach_view->normalized_subresource_range, kDetectPrevious);
    if (!hazard.hazard) {
        // The Async hazard check is against the current context's async set.
        hazard = DetectImageBarrierHazard(*image_state, track_back.barrier.src_exec_scope, track_back.barrier.src_access_scope,
                                          attach_view->normalized_subresource_range, kDetectAsync);
    }
    return hazard;
}

// Class CommandBufferAccessContext: Keep track of resource access state information for a specific command buffer
bool CommandBufferAccessContext::ValidateBeginRenderPass(const RENDER_PASS_STATE &rp_state,

                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                         const char *func_name) const {
    // Check if any of the layout transitions are hazardous.... but we don't have the renderpass context to work with, so we
    bool skip = false;
    uint32_t subpass = 0;
    const auto &transitions = rp_state.subpass_transitions[subpass];
    if (transitions.size()) {
        const std::vector<AccessContext> empty_context_vector;
        // Create context we can use to validate against...
        AccessContext temp_context(subpass, queue_flags_, rp_state.subpass_dependencies, empty_context_vector,
                                   const_cast<AccessContext *>(&cb_access_context_));

        assert(pRenderPassBegin);
        if (nullptr == pRenderPassBegin) return skip;

        const auto fb_state = sync_state_->Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
        assert(fb_state);
        if (nullptr == fb_state) return skip;

        // Create a limited array of views (which we'll need to toss
        std::vector<const IMAGE_VIEW_STATE *> views;
        const auto count_attachment = GetFramebufferAttachments(*pRenderPassBegin, *fb_state);
        const auto attachment_count = count_attachment.first;
        const auto *attachments = count_attachment.second;
        views.resize(attachment_count, nullptr);
        for (const auto &transition : transitions) {
            assert(transition.attachment < attachment_count);
            views[transition.attachment] = sync_state_->Get<IMAGE_VIEW_STATE>(attachments[transition.attachment]);
        }

        skip |= temp_context.ValidateLayoutTransitions(*sync_state_, rp_state, pRenderPassBegin->renderArea, 0, views, func_name);
        skip |= temp_context.ValidateLoadOperation(*sync_state_, rp_state, pRenderPassBegin->renderArea, 0, views, func_name);
    }
    return skip;
}

bool CommandBufferAccessContext::ValidateNextSubpass(const char *func_name) const {
    bool skip = false;
    skip |=
        current_renderpass_context_->ValidateNextSubpass(*sync_state_, cb_state_->activeRenderPassBeginInfo.renderArea, func_name);

    return skip;
}

bool CommandBufferAccessContext::ValidateEndRenderpass(const char *func_name) const {
    // TODO: Things to add here.
    // Validate Preserve attachments
    bool skip = false;
    skip |= current_renderpass_context_->ValidateEndRenderPass(*sync_state_, cb_state_->activeRenderPassBeginInfo.renderArea,
                                                               func_name);

    return skip;
}

void CommandBufferAccessContext::RecordBeginRenderPass(const ResourceUsageTag &tag) {
    assert(sync_state_);
    if (!cb_state_) return;

    // Create an access context the current renderpass.
    render_pass_contexts_.emplace_back(&cb_access_context_);
    current_renderpass_context_ = &render_pass_contexts_.back();
    current_renderpass_context_->RecordBeginRenderPass(*sync_state_, *cb_state_, queue_flags_, tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
}

void CommandBufferAccessContext::RecordNextSubpass(const RENDER_PASS_STATE &rp_state, const ResourceUsageTag &tag) {
    assert(current_renderpass_context_);
    current_renderpass_context_->RecordNextSubpass(cb_state_->activeRenderPassBeginInfo.renderArea, tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
}

void CommandBufferAccessContext::RecordEndRenderPass(const RENDER_PASS_STATE &render_pass, const ResourceUsageTag &tag) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return;

    current_renderpass_context_->RecordEndRenderPass(cb_state_->activeRenderPassBeginInfo.renderArea, tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
}

bool RenderPassAccessContext::ValidateNextSubpass(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                  const char *func_name) const {
    // PHASE1 TODO: Add Validate Preserve attachments
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(sync_state, *rp_state_, render_area, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(sync_state, *rp_state_, render_area, current_subpass_, attachment_views_,
                                                    func_name);

    const auto next_subpass = current_subpass_ + 1;
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |= next_context.ValidateLayoutTransitions(sync_state, *rp_state_, render_area, next_subpass, attachment_views_, func_name);
    skip |= next_context.ValidateLoadOperation(sync_state, *rp_state_, render_area, next_subpass, attachment_views_, func_name);
    return skip;
}
bool RenderPassAccessContext::ValidateEndRenderPass(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                    const char *func_name) const {
    // PHASE1 TODO: Validate Preserve
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(sync_state, *rp_state_, render_area, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(sync_state, *rp_state_, render_area, current_subpass_, attachment_views_,
                                                    func_name);
    skip |= ValidateFinalSubpassLayoutTransitions(sync_state, render_area, func_name);
    return skip;
}

AccessContext *RenderPassAccessContext::CreateStoreResolveProxy(const VkRect2D &render_area) const {
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, current_subpass_, render_area, attachment_views_);
}

bool RenderPassAccessContext::ValidateFinalSubpassLayoutTransitions(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                                    const char *func_name) const {
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
        const auto &attach_view = attachment_views_[transition.attachment];
        const auto &trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(trackback.context);  // Transitions are given implicit transitions if the StateTracker is working correctly
        auto *context = trackback.context;

        if (transition.prev_pass == current_subpass_) {
            if (!proxy_for_current) {
                // We haven't recorded resolve ofor the current_subpass, so we need to copy current and update it *as if*
                proxy_for_current.reset(CreateStoreResolveProxy(render_area));
            }
            context = proxy_for_current.get();
        }

        auto hazard = context->DetectImageBarrierHazard(
            *attach_view->image_state, trackback.barrier.src_exec_scope, trackback.barrier.src_access_scope,
            attach_view->normalized_subresource_range, AccessContext::DetectOptions::kDetectPrevious);
        if (hazard.hazard) {
            skip |= sync_state.LogError(rp_state_->renderPass, string_SyncHazardVUID(hazard.hazard),
                                        "%s: Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                                        " final image layout transition.",
                                        func_name, string_SyncHazard(hazard.hazard), transition.prev_pass, transition.attachment);
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordLayoutTransitions(const ResourceUsageTag &tag) {
    // Add layout transitions...
    const auto &transitions = rp_state_->subpass_transitions[current_subpass_];
    auto &subpass_context = subpass_contexts_[current_subpass_];
    std::set<const IMAGE_VIEW_STATE *> view_seen;
    for (const auto &transition : transitions) {
        const auto attachment_view = attachment_views_[transition.attachment];
        if (!attachment_view) continue;
        const auto image = attachment_view->image_state.get();
        if (!image) continue;

        const auto *barrier = subpass_context.GetTrackBackFromSubpass(transition.prev_pass);
        auto insert_pair = view_seen.insert(attachment_view);
        if (insert_pair.second) {
            // We haven't recorded the transistion yet, so treat this as a normal barrier with transistion.
            subpass_context.ApplyImageBarrier(*image, barrier->barrier, attachment_view->normalized_subresource_range, true, tag);

        } else {
            // We've recorded the transition, but we need to added on the additional dest barriers, and rerecording the transition
            // would clear out the prior barrier flags, so apply this as a *non* transition barrier
            auto barrier_to_transition = barrier->barrier;
            barrier_to_transition.src_access_scope |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
            subpass_context.ApplyImageBarrier(*image, barrier->barrier, attachment_view->normalized_subresource_range, false, tag);
        }
    }
}

void RenderPassAccessContext::RecordLoadOperations(const VkRect2D &render_area, const ResourceUsageTag &tag) {
    const auto *attachment_ci = rp_state_->createInfo.pAttachments;
    auto &subpass_context = subpass_contexts_[current_subpass_];
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state_->createInfo.attachmentCount; i++) {
        if (rp_state_->attachment_first_subpass[i] == current_subpass_) {
            if (attachment_views_[i] == nullptr) continue;  // UNUSED
            const auto &view = *attachment_views_[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;

            const auto &ci = attachment_ci[i];
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            if (is_color) {
                subpass_context.UpdateAccessState(*image, ColorLoadUsage(ci.loadOp), view.normalized_subresource_range, offset,
                                                  extent, tag);
            } else {
                auto update_range = view.normalized_subresource_range;
                if (has_depth) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    subpass_context.UpdateAccessState(*image, DepthStencilLoadUsage(ci.loadOp), update_range, offset, extent, tag);
                }
                if (has_stencil) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    subpass_context.UpdateAccessState(*image, DepthStencilLoadUsage(ci.stencilLoadOp), update_range, offset, extent,
                                                      tag);
                }
            }
        }
    }
}

void RenderPassAccessContext::RecordBeginRenderPass(const SyncValidator &state, const CMD_BUFFER_STATE &cb_state,
                                                    VkQueueFlags queue_flags, const ResourceUsageTag &tag) {
    current_subpass_ = 0;
    rp_state_ = cb_state.activeRenderPass;
    subpass_contexts_.reserve(rp_state_->createInfo.subpassCount);
    // Add this for all subpasses here so that they exsist during next subpass validation
    for (uint32_t pass = 0; pass < rp_state_->createInfo.subpassCount; pass++) {
        subpass_contexts_.emplace_back(pass, queue_flags, rp_state_->subpass_dependencies, subpass_contexts_, external_context_);
    }
    attachment_views_ = state.GetCurrentAttachmentViews(cb_state);

    RecordLayoutTransitions(tag);
    RecordLoadOperations(cb_state.activeRenderPassBeginInfo.renderArea, tag);
}

void RenderPassAccessContext::RecordNextSubpass(const VkRect2D &render_area, const ResourceUsageTag &tag) {
    // Resolves are against *prior* subpass context and thus *before* the subpass increment
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);

    current_subpass_++;
    assert(current_subpass_ < subpass_contexts_.size());
    RecordLayoutTransitions(tag);
    RecordLoadOperations(render_area, tag);
}

void RenderPassAccessContext::RecordEndRenderPass(const VkRect2D &render_area, const ResourceUsageTag &tag) {
    // Add the resolve and store accesses
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);

    // Export the accesses from the renderpass...
    external_context_->ResolveChildContexts(subpass_contexts_);

    // Add the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const auto &attachment = attachment_views_[transition.attachment];
        const auto &last_trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(external_context_ == last_trackback.context);
        external_context_->ApplyImageBarrier(*attachment->image_state, last_trackback.barrier,
                                             attachment->normalized_subresource_range, true, tag);
    }
}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &barrier) {
    const auto src_stage_mask = ExpandPipelineStages(queue_flags, barrier.srcStageMask);
    src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    src_access_scope = SyncStageAccess::AccessScope(src_stage_mask, barrier.srcAccessMask);
    const auto dst_stage_mask = ExpandPipelineStages(queue_flags, barrier.dstStageMask);
    dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);
    dst_access_scope = SyncStageAccess::AccessScope(dst_stage_mask, barrier.dstAccessMask);
}

void ResourceAccessState::ApplyBarrier(const SyncBarrier &barrier) {
    ApplyExecutionBarrier(barrier.src_exec_scope, barrier.dst_exec_scope);
    ApplyMemoryAccessBarrier(barrier.src_exec_scope, barrier.src_access_scope, barrier.dst_exec_scope, barrier.dst_access_scope);
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    if (IsRead(usage)) {
        if (last_write && IsWriteHazard(usage)) {
            hazard.Set(READ_AFTER_WRITE, write_tag);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE usage states if any
        // Write-After-Write check -- if we have a previous write to test against
        if (last_write && IsWriteHazard(usage)) {
            hazard.Set(WRITE_AFTER_WRITE, write_tag);
        } else {
            // Look for casus belli for WAR
            const auto usage_stage = PipelineStageBit(usage_index);
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                if (IsReadHazard(usage_stage, last_reads[read_index])) {
                    hazard.Set(WRITE_AFTER_READ, last_reads[read_index].tag);
                    break;
                }
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index, const SyncOrderingBarrier &ordering) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    HazardResult hazard;
    const auto usage = FlagBit(usage_index);
    const bool write_is_ordered = (last_write & ordering.access_scope) == last_write;  // Is true if no write, and that's good.
    if (IsRead(usage)) {
        if (!write_is_ordered && IsWriteHazard(usage)) {
            hazard.Set(READ_AFTER_WRITE, write_tag);
        }
    } else {
        if (!write_is_ordered && IsWriteHazard(usage)) {
            hazard.Set(WRITE_AFTER_WRITE, write_tag);
        } else {
            const auto usage_stage = PipelineStageBit(usage_index);
            const auto unordered_reads = last_read_stages & ~ordering.exec_scope;
            if (unordered_reads) {
                // Look for any WAR hazards outside the ordered set of stages
                for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                    if (last_reads[read_index].stage & unordered_reads) {
                        if (IsReadHazard(usage_stage, last_reads[read_index])) {
                            hazard.Set(WRITE_AFTER_READ, last_reads[read_index].tag);
                            break;
                        }
                    }
                }
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    if (IsRead(usage)) {
        if (last_write != 0) {
            hazard.Set(READ_RACING_WRITE, write_tag);
        }
    } else {
        if (last_write != 0) {
            hazard.Set(WRITE_RACING_WRITE, write_tag);
        } else if (last_read_count > 0) {
            hazard.Set(WRITE_RACING_READ, last_reads[0].tag);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                                                      SyncStageAccessFlags src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    if (last_write) {
        // If the previous write is *not* in the 1st access scope
        // *AND* the current barrier is not in the dependency chain
        // *AND* the there is no prior memory barrier for the previous write in the dependency chain
        // then the barrier access is unsafe (R/W after W)
        if (((last_write & src_access_scope) == 0) && ((src_exec_scope & write_dependency_chain) == 0) && (write_barriers == 0)) {
            // TODO: Do we need a difference hazard name for this?
            hazard.Set(WRITE_AFTER_WRITE, write_tag);
        }
    }
    if (!hazard.hazard) {
        // Look at the reads if any
        for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
            const auto &read_access = last_reads[read_index];
            // If the read stage is not in the src sync sync
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            if ((src_exec_scope & (read_access.stage | read_access.barriers)) == 0) {
                hazard.Set(WRITE_AFTER_READ, read_access.tag);
                break;
            }
        }
    }
    return hazard;
}

// The logic behind resolves is the same as update, we assume that earlier hazards have be reported, and that no
// tranistive hazard can exists with a hazard between the earlier operations.  Yes, an early hazard can mask that another
// exists, but if you fix *that* hazard it either fixes or unmasks the subsequent ones.
void ResourceAccessState::Resolve(const ResourceAccessState &other) {
    if (write_tag.IsBefore(other.write_tag)) {
        // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent operation
        *this = other;
    } else if (!other.write_tag.IsBefore(write_tag)) {
        // This is the *equals* case for write operations, we merged the write barriers and the read state (but without the
        // dependency chaining logic or any stage expansion)
        write_barriers |= other.write_barriers;

        // Merge that read states
        for (uint32_t other_read_index = 0; other_read_index < other.last_read_count; other_read_index++) {
            auto &other_read = other.last_reads[other_read_index];
            if (last_read_stages & other_read.stage) {
                // Merge in the barriers for read stages that exist in *both* this and other
                // TODO: This is N^2 with stages... perhaps the ReadStates should be by stage index.
                for (uint32_t my_read_index = 0; my_read_index < last_read_count; my_read_index++) {
                    auto &my_read = last_reads[my_read_index];
                    if (other_read.stage == my_read.stage) {
                        if (my_read.tag.IsBefore(other_read.tag)) {
                            my_read.tag = other_read.tag;
                        }
                        my_read.barriers |= other_read.barriers;
                        break;
                    }
                }
            } else {
                // The other read stage doesn't exist in this, so add it.
                last_reads[last_read_count] = other_read;
                last_read_count++;
                last_read_stages |= other_read.stage;
            }
        }
    }  // the else clause would be that other write is before this write... in which case we supercede the other state and ignore
       // it.
}

void ResourceAccessState::Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto usage_bit = FlagBit(usage_index);
    if (IsRead(usage_index)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        const auto usage_stage = PipelineStageBit(usage_index);
        if (usage_stage & last_read_stages) {
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                ReadState &access = last_reads[read_index];
                if (access.stage == usage_stage) {
                    access.barriers = 0;
                    access.tag = tag;
                    break;
                }
            }
        } else {
            // We don't have this stage in the list yet...
            assert(last_read_count < last_reads.size());
            ReadState &access = last_reads[last_read_count++];
            access.stage = usage_stage;
            access.barriers = 0;
            access.tag = tag;
            last_read_stages |= usage_stage;
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        // Clobber last read and both sets of barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
        // if the last_reads/last_write were unsafe, we've reported them,
        // in either case the prior access is irrelevant, we can overwrite them as *this* write is now after them
        last_read_count = 0;
        last_read_stages = 0;

        write_barriers = 0;
        write_dependency_chain = 0;
        write_tag = tag;
        last_write = usage_bit;
    }
}

void ResourceAccessState::ApplyExecutionBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
    // Execution Barriers only protect read operations
    for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
        ReadState &access = last_reads[read_index];
        // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync scope
        if (srcStageMask & (access.stage | access.barriers)) {
            access.barriers |= dstStageMask;
        }
    }
    if (write_dependency_chain & srcStageMask) write_dependency_chain |= dstStageMask;
}

void ResourceAccessState::ApplyMemoryAccessBarrier(VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                                   VkPipelineStageFlags dst_exec_scope, SyncStageAccessFlags dst_access_scope) {
    // Assuming we've applied the execution side of this barrier, we update just the write
    // The || implements the "dependency chain" logic for this barrier
    if ((src_access_scope & last_write) || (write_dependency_chain & src_exec_scope)) {
        write_barriers |= dst_access_scope;
        write_dependency_chain |= dst_exec_scope;
    }
}

void SyncValidator::ResetCommandBufferCallback(VkCommandBuffer command_buffer) {
    auto *access_context = GetAccessContextNoInsert(command_buffer);
    if (access_context) {
        access_context->Reset();
    }
}

void SyncValidator::FreeCommandBufferCallback(VkCommandBuffer command_buffer) {
    auto access_found = cb_access_state.find(command_buffer);
    if (access_found != cb_access_state.end()) {
        access_found->second->Reset();
        cb_access_state.erase(access_found);
    }
}

void SyncValidator::ApplyGlobalBarriers(AccessContext *context, VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask, SyncStageAccessFlags src_access_scope,
                                        SyncStageAccessFlags dst_access_scope, uint32_t memoryBarrierCount,
                                        const VkMemoryBarrier *pMemoryBarriers) {
    // TODO: Implement this better (maybe some delayed/on-demand integration).
    ApplyGlobalBarrierFunctor barriers_functor(srcStageMask, dstStageMask, src_access_scope, dst_access_scope, memoryBarrierCount,
                                               pMemoryBarriers);
    context->ApplyGlobalBarriers(barriers_functor);
}

void SyncValidator::ApplyBufferBarriers(AccessContext *context, VkPipelineStageFlags src_exec_scope,
                                        SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                        SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                        const VkBufferMemoryBarrier *barriers) {
    for (uint32_t index = 0; index < barrier_count; index++) {
        auto barrier = barriers[index];
        const auto *buffer = Get<BUFFER_STATE>(barrier.buffer);
        if (!buffer) continue;
        barrier.size = GetRealWholeSize(barrier.offset, barrier.size, buffer->createInfo.size);
        ResourceAccessRange range = MakeRange(barrier);
        const auto src_access_scope = AccessScope(src_stage_accesses, barrier.srcAccessMask);
        const auto dst_access_scope = AccessScope(dst_stage_accesses, barrier.dstAccessMask);
        const ApplyMemoryAccessBarrierFunctor update_action(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
        context->UpdateMemoryAccess(*buffer, range, update_action);
    }
}

void SyncValidator::ApplyImageBarriers(AccessContext *context, VkPipelineStageFlags src_exec_scope,
                                       SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                       SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                       const VkImageMemoryBarrier *barriers, const ResourceUsageTag &tag) {
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        const auto *image = Get<IMAGE_STATE>(barrier.image);
        if (!image) continue;
        auto subresource_range = NormalizeSubresourceRange(image->createInfo, barrier.subresourceRange);
        bool layout_transition = barrier.oldLayout != barrier.newLayout;
        const auto src_access_scope = AccessScope(src_stage_accesses, barrier.srcAccessMask);
        const auto dst_access_scope = AccessScope(dst_stage_accesses, barrier.dstAccessMask);
        context->ApplyImageBarrier(*image, src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope, subresource_range,
                                   layout_transition, tag);
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.srcOffset, GetRealWholeSize(copy_region.srcOffset, copy_region.size, src_buffer->createInfo.size));
            auto hazard = context->DetectHazard(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // TODO -- add tag information to log msg when useful.
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for srcBuffer %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(srcBuffer).c_str(), region);
            }
        }
        if (dst_buffer && !skip) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.dstOffset, GetRealWholeSize(copy_region.dstOffset, copy_region.size, dst_buffer->createInfo.size));
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for dstBuffer %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(dstBuffer).c_str(), region);
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions) {
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    const auto tag = cb_context->NextCommandTag(CMD_COPYBUFFER);
    auto *context = cb_context->GetCurrentAccessContext();

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.srcOffset, GetRealWholeSize(copy_region.srcOffset, copy_region.size, src_buffer->createInfo.size));
            context->UpdateAccessState(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_buffer) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.dstOffset, GetRealWholeSize(copy_region.dstOffset, copy_region.size, dst_buffer->createInfo.size));
            context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for srcImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(srcImage).c_str(), region);
            }
        }

        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, dst_copy_extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for dstImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(dstImage).c_str(), region);
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource, copy_region.srcOffset,
                                       copy_region.extent, tag);
        }
        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource, copy_region.dstOffset,
                                       dst_copy_extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    // Validate Image Layout transitions
    for (uint32_t index = 0; index < imageMemoryBarrierCount; index++) {
        const auto &barrier = pImageMemoryBarriers[index];
        if (barrier.newLayout == barrier.oldLayout) continue;  // Only interested in layout transitions at this point.
        const auto *image_state = Get<IMAGE_STATE>(barrier.image);
        if (!image_state) continue;
        const auto hazard = context->DetectImageBarrierHazard(*image_state, src_exec_scope, src_stage_accesses, barrier);
        if (hazard.hazard) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            skip |= LogError(barrier.image, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdPipelineBarrier: Hazard %s for image barrier %" PRIu32 " %s", string_SyncHazard(hazard.hazard),
                             index, report_data->FormatHandle(barrier.image).c_str());
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                    uint32_t bufferMemoryBarrierCount,
                                                    const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                    uint32_t imageMemoryBarrierCount,
                                                    const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;
    const auto tag = cb_access_context->NextCommandTag(CMD_PIPELINEBARRIER);
    auto access_context = cb_access_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    const auto dst_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), dstStageMask);
    auto dst_stage_accesses = AccessScopeByStage(dst_stage_mask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    const auto dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);
    ApplyBufferBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                        bufferMemoryBarrierCount, pBufferMemoryBarriers);
    ApplyImageBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                       imageMemoryBarrierCount, pImageMemoryBarriers, tag);

    // Apply these last in-case there operation is a superset of the other two and would clean them up...
    ApplyGlobalBarriers(access_context, src_exec_scope, dst_exec_scope, src_stage_accesses, dst_stage_accesses, memoryBarrierCount,
                        pMemoryBarriers);
}

void SyncValidator::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    StateTracker::PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker
    // refactor would be messier without.
    // TODO: Find a good way to do this hooklessly.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeSyncValidation);
    SyncValidator *sync_device_state = static_cast<SyncValidator *>(validation_data);

    sync_device_state->SetCommandBufferResetCallback([sync_device_state](VkCommandBuffer command_buffer) -> void {
        sync_device_state->ResetCommandBufferCallback(command_buffer);
    });
    sync_device_state->SetCommandBufferFreeCallback([sync_device_state](VkCommandBuffer command_buffer) -> void {
        sync_device_state->FreeCommandBufferCallback(command_buffer);
    });
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR *pSubpassBeginInfo, const char *func_name) const {
    bool skip = false;
    const auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    auto cb_context = GetAccessContext(commandBuffer);

    if (rp_state && cb_context) {
        skip |= cb_context->ValidateBeginRenderPass(*rp_state, pRenderPassBegin, pSubpassBeginInfo, func_name);
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, "vkCmdBeginRenderPass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, "vkCmdBeginRenderPass2");
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, "vkCmdBeginRenderPass2KHR");
    return skip;
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     VkResult result) {
    // The state tracker sets up the command buffer state
    StateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    cb_access_context->Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE command) {
    auto cb_context = GetAccessContext(commandBuffer);
    if (cb_context) {
        cb_context->RecordBeginRenderPass(cb_context->NextCommandTag(command));
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, CMD_BEGINRENDERPASS);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                           const VkSubpassEndInfoKHR *pSubpassEndInfo, const char *func_name) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return skip;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return skip;

    skip |= cb_context->ValidateNextSubpass(func_name);

    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    skip |= ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, "vkCmdNextSubpass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                      const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, "vkCmdNextSubpass2KHR");
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, "vkCmdNextSubpass2");
    return skip;
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE command) {
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return;

    cb_context->RecordNextSubpass(*rp_state, cb_context->NextCommandTag(command));
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, CMD_NEXTSUBPASS);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo,
                                             const char *func_name) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return skip;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return skip;

    skip |= cb_context->ValidateEndRenderpass(func_name);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);
    skip |= ValidateCmdEndRenderPass(commandBuffer, nullptr, "vkEndRenderPass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                                                     const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, "vkEndRenderPass2");
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, "vkEndRenderPass2KHR");
    return skip;
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                           CMD_TYPE command) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return;

    const auto *rp_state = cb_state->activeRenderPass;
    if (!rp_state) return;

    cb_context->RecordEndRenderPass(*rp_state, cb_context->NextCommandTag(command));
}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer);
    RecordCmdEndRenderPass(commandBuffer, nullptr, CMD_ENDRENDERPASS);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
            auto hazard = context->DetectHazard(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // PHASE1 TODO -- add tag information to log msg when useful.
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBufferToImage: Hazard %s for srcBuffer %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region);
            }
        }
        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBufferToImage: Hazard %s for dstImage %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region);
            }
            if (skip) break;
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYBUFFERTOIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
            context->UpdateAccessState(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.imageSubresource,
                                       copy_region.imageOffset, copy_region.imageExtent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImageToBuffer: Hazard %s for srcImage %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region);
            }
        }
        if (dst_mem) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImageToBuffer: Hazard %s for dstBuffer %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region);
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYIMAGETOBUFFER);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.imageSubresource,
                                       copy_region.imageOffset, copy_region.imageExtent, tag);
        }
        if (dst_buffer) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
            context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit *pRegions, VkFilter filter) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkExtent3D extent = {static_cast<uint32_t>(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x),
                                 static_cast<uint32_t>(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y),
                                 static_cast<uint32_t>(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z)};
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, blit_region.srcSubresource,
                                                blit_region.srcOffsets[0], extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdBlitImage: Hazard %s for srcImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(srcImage).c_str(), region);
            }
        }

        if (dst_image) {
            VkExtent3D extent = {static_cast<uint32_t>(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x),
                                 static_cast<uint32_t>(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y),
                                 static_cast<uint32_t>(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z)};
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, blit_region.dstSubresource,
                                                blit_region.dstOffsets[0], extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdBlitImage: Hazard %s for dstImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(dstImage).c_str(), region);
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_BLITIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkExtent3D extent = {static_cast<uint32_t>(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x),
                                 static_cast<uint32_t>(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y),
                                 static_cast<uint32_t>(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z)};
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, blit_region.srcSubresource,
                                       blit_region.srcOffsets[0], extent, tag);
        }
        if (dst_image) {
            VkExtent3D extent = {static_cast<uint32_t>(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x),
                                 static_cast<uint32_t>(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y),
                                 static_cast<uint32_t>(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z)};
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, blit_region.dstSubresource,
                                       blit_region.dstOffsets[0], extent, tag);
        }
    }
}

bool SyncValidator::DetectDescriptorSetHazard(const CMD_BUFFER_STATE &cmd, VkPipelineBindPoint pipelineBindPoint,
                                              const char *function) const {
    bool skip = false;

    const auto last_bound_it = cmd.lastBound.find(pipelineBindPoint);
    if (last_bound_it == cmd.lastBound.cend()) {
        return skip;
    }
    auto const &state = last_bound_it->second;
    const auto *pPipe = state.pipeline_state;
    if (!pPipe) {
        return skip;
    }

    const auto *cb_access_context = GetAccessContext(cmd.commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &set_binding_pair : pPipe->active_slots) {
        uint32_t setIndex = set_binding_pair.first;
        cvdescriptorset::DescriptorSet *descriptor_set = state.per_set[setIndex].bound_descriptor_set;
        for (auto binding_pair : set_binding_pair.second) {
            auto binding = binding_pair.first;
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  binding_pair.first);
            const auto descriptor_type = binding_it.GetType();
            cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
            auto array_idx = 0;

            if (binding_it.IsVariableDescriptorCount()) {
                index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
            }
            for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
                uint32_t index = i - index_range.start;
                const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);
                if (descriptor->GetClass() == DescriptorClass::ImageSampler || descriptor->GetClass() == DescriptorClass::Image) {
                    const IMAGE_VIEW_STATE *img_view_state = nullptr;
                    if (descriptor->GetClass() == DescriptorClass::ImageSampler) {
                        img_view_state = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetImageViewState();
                    } else {
                        img_view_state = static_cast<const ImageDescriptor *>(descriptor)->GetImageViewState();
                    }
                    if (!img_view_state) continue;
                    SyncStageAccessIndex sync_index;
                    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                        sync_index = SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
                    } else {
                        sync_index = SYNC_VERTEX_SHADER_SHADER_READ;
                    }
                    const IMAGE_STATE *img_state = img_view_state->image_state.get();
                    auto hazard = context->DetectHazard(*img_state, sync_index, img_view_state->create_info.subresourceRange,
                                                        {0, 0, 0}, img_state->createInfo.extent);
                    if (hazard.hazard) {
                        skip |= LogError(img_view_state->image_view, string_SyncHazardVUID(hazard.hazard),
                                         "%s: Hazard %s for %s in %s and %s binding #%d index %d", function,
                                         string_SyncHazard(hazard.hazard),
                                         report_data->FormatHandle(img_view_state->image_view).c_str(),
                                         report_data->FormatHandle(cmd.commandBuffer).c_str(),
                                         report_data->FormatHandle(descriptor_set->GetSet()).c_str(), binding, index);
                    }
                } else if (descriptor->GetClass() == DescriptorClass::TexelBuffer) {
                    auto buf_view_state = static_cast<const TexelDescriptor *>(descriptor)->GetBufferViewState();
                    if (!buf_view_state) continue;
                    const BUFFER_STATE *buf_state = buf_view_state->buffer_state.get();
                    ResourceAccessRange range =
                        MakeRange(buf_view_state->create_info.offset,
                                  GetRealWholeSize(buf_view_state->create_info.offset, buf_view_state->create_info.range,
                                                   buf_state->createInfo.size));
                    auto hazard = context->DetectHazard(*buf_state, SYNC_VERTEX_SHADER_SHADER_READ, range);
                    if (hazard.hazard) {
                        skip |= LogError(buf_view_state->buffer_view, string_SyncHazardVUID(hazard.hazard),
                                         "%s: Hazard %s for %s in %s and %s binding #%d index %d", function,
                                         string_SyncHazard(hazard.hazard),
                                         report_data->FormatHandle(buf_view_state->buffer_view).c_str(),
                                         report_data->FormatHandle(cmd.commandBuffer).c_str(),
                                         report_data->FormatHandle(descriptor_set->GetSet()).c_str(), binding, index);
                    }
                } else if (descriptor->GetClass() == DescriptorClass::GeneralBuffer) {
                    auto buf_state = static_cast<const BufferDescriptor *>(descriptor)->GetBufferState();
                    if (!buf_state) continue;
                    ResourceAccessRange range = MakeRange(0, buf_state->createInfo.size);
                    SyncStageAccessIndex sync_index;
                    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        sync_index = SYNC_VERTEX_SHADER_UNIFORM_READ;
                    } else {
                        sync_index = SYNC_VERTEX_SHADER_SHADER_READ;
                    }
                    auto hazard = context->DetectHazard(*buf_state, sync_index, range);
                    if (hazard.hazard) {
                        skip |= LogError(buf_state->buffer, string_SyncHazardVUID(hazard.hazard),
                                         "%s: Hazard %s for %s in %s and %s binding #%d index %d", function,
                                         string_SyncHazard(hazard.hazard), report_data->FormatHandle(buf_state->buffer).c_str(),
                                         report_data->FormatHandle(cmd.commandBuffer).c_str(),
                                         report_data->FormatHandle(descriptor_set->GetSet()).c_str(), binding, index);
                    }
                }
            }
        }
    }
    return skip;
}

void SyncValidator::UpdateDescriptorSetAccessState(const CMD_BUFFER_STATE &cmd, CMD_TYPE command,
                                                   VkPipelineBindPoint pipelineBindPoint) {
    const auto last_bound_it = cmd.lastBound.find(pipelineBindPoint);
    if (last_bound_it == cmd.lastBound.cend()) {
        return;
    }
    auto const &state = last_bound_it->second;
    const auto *pPipe = state.pipeline_state;
    if (!pPipe) {
        return;
    }

    auto *cb_access_context = GetAccessContext(cmd.commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &set_binding_pair : pPipe->active_slots) {
        uint32_t setIndex = set_binding_pair.first;
        const cvdescriptorset::DescriptorSet *descriptor_set = state.per_set[setIndex].bound_descriptor_set;
        for (auto binding_pair : set_binding_pair.second) {
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  binding_pair.first);
            const auto descriptor_type = binding_it.GetType();
            cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
            auto array_idx = 0;

            if (binding_it.IsVariableDescriptorCount()) {
                index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
            }

            for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
                const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);
                if (descriptor->GetClass() == DescriptorClass::ImageSampler || descriptor->GetClass() == DescriptorClass::Image) {
                    const IMAGE_VIEW_STATE *img_view_state = nullptr;
                    if (descriptor->GetClass() == DescriptorClass::ImageSampler) {
                        img_view_state = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetImageViewState();
                    } else {
                        img_view_state = static_cast<const ImageDescriptor *>(descriptor)->GetImageViewState();
                    }
                    if (!img_view_state) continue;
                    SyncStageAccessIndex sync_index;
                    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                        sync_index = SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
                    } else {
                        sync_index = SYNC_VERTEX_SHADER_SHADER_READ;
                    }
                    const IMAGE_STATE *img_state = img_view_state->image_state.get();
                    context->UpdateAccessState(*img_state, sync_index, img_view_state->create_info.subresourceRange, {0, 0, 0},
                                               img_state->createInfo.extent, tag);

                } else if (descriptor->GetClass() == DescriptorClass::TexelBuffer) {
                    auto buf_view_state = static_cast<const TexelDescriptor *>(descriptor)->GetBufferViewState();
                    if (!buf_view_state) continue;
                    const BUFFER_STATE *buf_state = buf_view_state->buffer_state.get();
                    ResourceAccessRange range = MakeRange(buf_view_state->create_info.offset, buf_view_state->create_info.range);
                    context->UpdateAccessState(*buf_state, SYNC_VERTEX_SHADER_SHADER_READ, range, tag);

                } else if (descriptor->GetClass() == DescriptorClass::GeneralBuffer) {
                    auto buf_state = static_cast<const BufferDescriptor *>(descriptor)->GetBufferState();
                    if (!buf_state) continue;
                    SyncStageAccessIndex sync_index;
                    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        sync_index = SYNC_VERTEX_SHADER_UNIFORM_READ;
                    } else {
                        sync_index = SYNC_VERTEX_SHADER_SHADER_READ;
                    }
                    ResourceAccessRange range = MakeRange(0, buf_state->createInfo.size);
                    context->UpdateAccessState(*buf_state, sync_index, range, tag);
                }
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatch");
}

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DISPATCH, VK_PIPELINE_BIND_POINT_COMPUTE);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatchIndirect");
}

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DISPATCHINDIRECT, VK_PIPELINE_BIND_POINT_COMPUTE);
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDraw");
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAW, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexed");
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAWINDEXED, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirect");
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAWINDIRECT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirect");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAWINDEXEDINDIRECT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirectCount");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAWINDIRECTCOUNT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirectCountKHR");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirectCountAMD");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirectCount");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride) {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    UpdateDescriptorSetAccessState(*cb_state, CMD_DRAWINDEXEDINDIRECTCOUNT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirectCountKHR");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    const auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    return DetectDescriptorSetHazard(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirectCountAMD");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard =
                context->DetectHazard(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearColorImage: Hazard %s for %s, range index %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(image).c_str(), index);
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARCOLORIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent,
                                       tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard =
                context->DetectHazard(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearDepthStencilImage: Hazard %s for %s, range index %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(image).c_str(), index);
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARDEPTHSTENCILIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent,
                                       tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride,
                                                           VkQueryResultFlags flags) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // TODO: size is ERROR
        ResourceAccessRange range = MakeRange(dstOffset, dst_buffer->createInfo.size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |=
                LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard), "vkCmdCopyQueryPoolResults: Hazard %s for dstBuffer %s",
                         string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARDEPTHSTENCILIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // TODO: size is ERROR
        ResourceAccessRange range = MakeRange(dstOffset, dst_buffer->createInfo.size);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        ResourceAccessRange range = MakeRange(dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard), "vkCmdFillBuffer: Hazard %s for dstBuffer %s",
                             string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_FILLBUFFER);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        ResourceAccessRange range = MakeRange(dstOffset, size);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for srcImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(srcImage).c_str(), region);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for dstImage %s, region %" PRIu32, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(dstImage).c_str(), region);
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_RESOLVEIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                       resolve_region.srcOffset, resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                       resolve_region.dstOffset, resolve_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard), "vkCmdUpdateBuffer: Hazard %s for dstBuffer %s",
                             string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_UPDATEBUFFER);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}
