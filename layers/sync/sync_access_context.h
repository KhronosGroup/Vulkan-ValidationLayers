/*
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

#pragma once

#include "sync/sync_common.h"
#include "sync/sync_access_map.h"
#include "sync/sync_access_state.h"

struct SubpassDependencyGraphNode;

namespace vvl {
class Bindable;
class Buffer;
class Event;
class Image;
class ImageView;
class VideoPictureResource;
class VideoSession;
}  // namespace vvl

namespace syncval {

bool SimpleBinding(const vvl::Bindable &bindable);
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer);

// ForEachEntryInRangesUntil -- Execute Action for each map entry in the generated ranges until it returns true
//
// Action is const w.r.t. map
// Action is allowed (assumed) to modify pos
// Action must not advance pos for ranges strictly < pos->first
// Action must handle range strictly less than pos->first correctly
// Action must handle pos == end correctly
// Action is assumed to only require invocation once per map entry
// RangeGen must be strictly monotonic
// Note: If Action invocations are heavyweight and inter-entry (gap) calls are not needed
//       add a template or function parameter to skip them. TBD.
template <typename RangeMap, typename RangeGen, typename Action>
bool ForEachEntryInRangesUntil(const RangeMap &map, RangeGen &range_gen, Action &action) {
    using RangeType = typename RangeGen::RangeType;
    using IndexType = typename RangeType::index_type;
    auto pos = map.LowerBound(*range_gen);
    const auto end = map.end();
    IndexType skip_limit = 0;
    for (; range_gen->non_empty() && pos != end; ++range_gen) {
        RangeType range = *range_gen;
        // See if a prev pos has covered this range
        if (range.end <= skip_limit) {
            // Since the map is const, we needn't call action on the same pos again
            continue;
        }

        //  If the current range was *partially* covered be a previous pos, trim, such that Action is only
        //  called once for a given range (and pos)
        if (range.begin < skip_limit) {
            range.begin = skip_limit;
        }

        // Now advance pos as needed to match range
        if (pos->first.strictly_less(range)) {
            ++pos;
            if (pos == end) break;
            if (pos->first.strictly_less(range)) {
                pos = map.LowerBound(range);
                if (pos == end) break;
            }
            assert(pos == map.LowerBound(range));
        }

        // If the range intersects pos->first, consider Action performed for that map entry, and
        // make sure not to call Action for this pos for any subsequent ranges
        skip_limit = range.end > pos->first.begin ? pos->first.end : 0U;

        // Action is allowed to alter pos but shouldn't do so if range is strictly < pos->first
        if (action(range, end, pos)) return true;
    }

    // Action needs to handle the "at end " condition (and can be useful for recursive actions)
    for (; range_gen->non_empty(); ++range_gen) {
        if (action(*range_gen, end, pos)) return true;
    }

    return false;
}

// This functor changes layout of access map as part of infill_update_range traversal:
//    * infills gaps within a specified input range (only for layout transition use case)
//    * if existing ranges intersect begin/end of the input range then existing ranges are split
//
// Besides layout changes this functor does not update access state.
// The subsequent functors that will run over the same input range will have a guarantee that access
// map structure won't be modified, so they can collect persistent references to access states.
//
// NOTE: notice, when this functor is used for not layout transtiion barrier it is not a noop: yes,
//  Infill operation is used only for layout transition, but infill_update_range can additionally
//  perform up to two splits if input range intersects access map existing ranges.
struct ApplyMarkupFunctor {
    ApplyMarkupFunctor(bool layout_transition) : layout_transition(layout_transition) {}

    using Iterator = AccessMap::iterator;
    Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const {
        if (!layout_transition) {
            return pos_hint;
        }
        auto inserted = accesses->Insert(pos_hint, range, AccessState{});
        return inserted;
    }
    void operator()(const Iterator &pos) const {}
    const bool layout_transition;
};

// This functor populates PendingBarriers with the results of independent barrier appication (pending barriers).
// After this functor finished its work then PendingBarriers::Apply() can be used to update the access states.
struct CollectBarriersFunctor {
    CollectBarriersFunctor(const BarrierScope &barrier_scope, const SyncBarrier &barrier, bool layout_transition,
                           uint32_t layout_transition_handle_index, PendingBarriers &pending_barriers)
        : barrier_scope(barrier_scope),
          barrier(barrier),
          layout_transition(layout_transition),
          layout_transition_handle_index(layout_transition_handle_index),
          pending_barriers(pending_barriers) {
        // Suppress layout transition during submit time application.
        // It add write access but this is necessary only during recording.
        if (barrier_scope.scope_queue != kQueueIdInvalid) {
            this->layout_transition = false;
            this->layout_transition_handle_index = vvl::kNoIndex32;
        }
    }

    using Iterator = AccessMap::iterator;
    Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const {
        assert(!layout_transition);  // MarkupFunctor infills gaps for layout transtion, so we should never get here in that case
        return pos_hint;
    }

    void operator()(const Iterator &pos) const {
        AccessState &access_state = pos->second;
        access_state.CollectPendingBarriers(barrier_scope, barrier, layout_transition, layout_transition_handle_index,
                                            pending_barriers);
    }

    const BarrierScope barrier_scope;
    const SyncBarrier barrier;
    bool layout_transition;
    uint32_t layout_transition_handle_index;
    PendingBarriers &pending_barriers;
};

struct QueueTagOffsetBarrierAction {
    QueueTagOffsetBarrierAction(QueueId qid, ResourceUsageTag offset) : queue_id(qid), tag_offset(offset) {}
    void operator()(AccessState *access) const {
        access->OffsetTag(tag_offset);
        access->SetQueueId(queue_id);
    };
    QueueId queue_id;
    ResourceUsageTag tag_offset;
};

struct ApplyTrackbackStackAction {
    explicit ApplyTrackbackStackAction(const std::vector<SyncBarrier> &barriers_,
                                       const AccessStateFunction *previous_barrier_ = nullptr)
        : barriers(barriers_), previous_barrier(previous_barrier_) {}
    void operator()(AccessState *access) const {
        assert(access);
        ApplyBarriers(*access, barriers);
        if (previous_barrier) {
            assert(bool(*previous_barrier));
            (*previous_barrier)(access);
        }
    }
    const std::vector<SyncBarrier> &barriers;
    const AccessStateFunction *previous_barrier;
};

class AccessContext;

struct SubpassBarrierTrackback {
    std::vector<SyncBarrier> barriers;
    const AccessContext *source_subpass = nullptr;
    SubpassBarrierTrackback() = default;
    SubpassBarrierTrackback(const AccessContext *source_subpass, VkQueueFlags queue_flags,
                            const std::vector<const VkSubpassDependency2 *> &subpass_dependencies)
        : source_subpass(source_subpass) {
        barriers.reserve(subpass_dependencies.size());
        for (const VkSubpassDependency2 *dependency : subpass_dependencies) {
            assert(dependency);
            barriers.emplace_back(queue_flags, *dependency);
        }
    }
};

class AttachmentViewGen {
  public:
    enum Gen { kViewSubresource = 0, kRenderArea = 1, kDepthOnlyRenderArea = 2, kStencilOnlyRenderArea = 3, kGenSize = 4 };
    AttachmentViewGen(const vvl::ImageView *image_view, const VkOffset3D &offset, const VkExtent3D &extent);
    AttachmentViewGen(const AttachmentViewGen &other) = default;
    AttachmentViewGen(AttachmentViewGen &&other) = default;
    const vvl::ImageView *GetViewState() const { return view_; }
    const std::optional<ImageRangeGen> &GetRangeGen(Gen type) const;
    bool IsValid() const { return gen_store_[Gen::kViewSubresource].has_value(); }
    Gen GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const;

  private:
    const vvl::ImageView *view_ = nullptr;
    std::array<std::optional<ImageRangeGen>, Gen::kGenSize> gen_store_;
};

using AttachmentViewGenVector = std::vector<AttachmentViewGen>;

// Provides ordering among all first accesses in the AccessContext.
// This accelerates the search of the first accesses that intersect a given tag range.
struct SortedFirstAccesses {
    void Init(const AccessMap &finalized_access_map);
    void Clear();

    // Access objects with first accesses that cover only single tag.
    // This is a separate case because it allows to quickly find a range
    // of such first accesses that belong to a given tag range.
    struct SingleTag {
        // The only tag referenced by the first accesses of the access object
        ResourceUsageTag tag{};
        const AccessMap::value_type *p_key_value = nullptr;
    };
    std::vector<SingleTag> sorted_single_tags;

    // Access objects with first accesses that cover more than one tag
    struct MultiTag {
        // range.begin: tag of the first first_access.
        // range.end: tag of the last first_access plus one.
        ResourceUsageRange range;
        const AccessMap::value_type *p_key_value = nullptr;
    };
    std::vector<MultiTag> sorted_multi_tags;

    // Ranged-for loop iterator helpers
    struct SingleTagRange {
        const std::vector<SingleTag> &sorted_single_tags;
        const ResourceUsageRange tag_range;
        std::vector<SingleTag>::const_iterator begin();
        std::vector<SingleTag>::const_iterator end();
    };
    SingleTagRange IterateSingleTagFirstAccesses(const ResourceUsageRange &tag_range) const;

    struct MultiTagRange {
        const std::vector<MultiTag> &sorted_multi_tags;
        const ResourceUsageRange tag_range;
        std::vector<MultiTag>::const_iterator begin();
        std::vector<MultiTag>::const_iterator end();
    };
    MultiTagRange IterateMultiTagFirstAccesses(const ResourceUsageRange &tag_range) const;
};

class AccessContext {
  public:
    using ScopeMap = AccessMap;
    enum DetectOptions : uint32_t {
        kDetectPrevious = 1U << 0,
        kDetectAsync = 1U << 1,
        kDetectAll = (kDetectPrevious | kDetectAsync)
    };

    HazardResult DetectHazard(const vvl::Buffer &buffer, SyncAccessIndex access_index, const AccessRange &range) const;
    HazardResult DetectHazard(const vvl::Image &image, SyncAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    HazardResult DetectHazard(const vvl::ImageView &image_view, SyncAccessIndex current_usage) const;
    HazardResult DetectHazard(const ImageRangeGen &ref_range_gen, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule = SyncOrdering::kOrderingNone, SyncFlags flags = 0) const;
    HazardResult DetectHazard(const vvl::ImageView &image_view, const VkOffset3D &offset, const VkExtent3D &extent,
                              SyncAccessIndex current_usage, SyncOrdering ordering_rule) const;
    HazardResult DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule, SyncFlags flags = 0) const;
    HazardResult DetectHazard(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                              SyncAccessIndex current_usage) const;
    HazardResult DetectHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                              const VkExtent3D &extent, bool is_depth_sliced, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule = SyncOrdering::kOrderingNone) const;

    HazardResult DetectImageBarrierHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                                          VkPipelineStageFlags2 src_exec_scope, const SyncAccessFlags &src_access_scope,
                                          QueueId queue_id, const ScopeMap &scope_map, ResourceUsageTag scope_tag,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const AttachmentViewGen &attachment_view, const SyncBarrier &barrier,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const vvl::Image &image, VkPipelineStageFlags2 src_exec_scope,
                                          const SyncAccessFlags &src_access_scope, const VkImageSubresourceRange &subresource_range,
                                          bool is_depth_sliced, DetectOptions options) const;
    HazardResult DetectSubpassTransitionHazard(const SubpassBarrierTrackback &track_back,
                                               const AttachmentViewGen &attach_view) const;

    HazardResult DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                      const AccessContext &access_context) const;
    HazardResult DetectMarkerHazard(const vvl::Buffer &buffer, const AccessRange &range) const;

    const SubpassBarrierTrackback &GetDstExternalTrackBack() const { return dst_external_; }

    void ResolvePreviousAccesses();
    void ResolveFromContext(const AccessContext &from);

    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context);
    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                            subresource_adapter::ImageRangeGenerator range_gen, bool infill = false, bool recur_to_infill = false);

    void UpdateAccessState(const vvl::Buffer &buffer, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const AccessRange &range, ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void UpdateAccessState(const vvl::Image &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag);
    void UpdateAccessState(const vvl::Image &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset, const VkExtent3D &extent,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                           SyncOrdering ordering_rule, ResourceUsageTag tag, SyncFlags flags = 0);
    void UpdateAccessState(const vvl::ImageView &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkOffset3D &offset, const VkExtent3D &extent, ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const vvl::ImageView &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void UpdateAccessState(ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void UpdateAccessState(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                           SyncAccessIndex current_usage, ResourceUsageTag tag);
    void ResolveChildContexts(vvl::span<AccessContext> subpass_contexts);

    void ImportAsyncContexts(const AccessContext &from);
    void ClearAsyncContexts() { async_.clear(); }

    AccessContext() = default;

    // Disable implicit copy operations and rely on explicit InitFrom.
    // AccessContext is a heavy object and there must be no possibility of an accidental copy.
    // Copy operations must be searchable (InitFrom function).
    AccessContext(const AccessContext &other) = delete;
    AccessContext &operator=(const AccessContext &) = delete;

    void InitFrom(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                  const AccessContext *contexts, const AccessContext *external_context);
    void InitFrom(const AccessContext &other);
    void Reset();

    void TrimAndClearFirstAccess();
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;

    const AccessMap &GetAccessMap() const { return access_state_map_; }
    const SubpassBarrierTrackback *GetTrackBackFromSubpass(uint32_t subpass) const {
        if (subpass == VK_SUBPASS_EXTERNAL) {
            return src_external_;
        } else {
            assert(subpass < prev_by_subpass_.size());
            return prev_by_subpass_[subpass];
        }
    }

    void SetStartTag(ResourceUsageTag tag) { start_tag_ = tag; }
    ResourceUsageTag StartTag() const { return start_tag_; }

    template <typename Predicate>
    void EraseIf(Predicate &&pred);

    // For use during queue submit building up the QueueBatchContext AccessContext for validation, otherwise clear.
    void AddAsyncContext(const AccessContext *context, ResourceUsageTag tag, QueueId queue_id);

    class AsyncReference {
      public:
        AsyncReference(const AccessContext &async_context, ResourceUsageTag async_tag, QueueId queue_id)
            : context_(&async_context), tag_(async_tag), queue_id_(queue_id) {}
        const AccessContext &Context() const { return *context_; }
        // For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
        // unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)
        ResourceUsageTag StartTag() const;
        QueueId GetQueueId() const { return queue_id_; }

      protected:
        const AccessContext *context_;
        ResourceUsageTag tag_;  // Start of open ended asynchronous range
        QueueId queue_id_;
    };

    template <typename Action>
    void UpdateMemoryAccessRangeState(Action &action, const AccessRange &range);

    template <typename Action, typename RangeGen>
    void UpdateMemoryAccessState(const Action &action, RangeGen &range_gen);

    // Called when all accesses are recorded. This can be used for preprocessing
    // or caching purposes. After finalization, it is save to keep persistent
    // references to individual accesses (until context is destroyed).
    void Finalize();

  private:
    struct UpdateMemoryAccessStateFunctor {
        using Iterator = AccessMap::iterator;
        Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const;
        void operator()(const Iterator &pos) const;
        UpdateMemoryAccessStateFunctor(const AccessContext &context_, SyncAccessIndex usage_, SyncOrdering ordering_rule_,
                                       ResourceUsageTagEx tag_ex, SyncFlags flags = 0)
            : context(context_), usage_info(GetAccessInfo(usage_)), ordering_rule(ordering_rule_), tag_ex(tag_ex), flags(flags) {}
        const AccessContext &context;
        const SyncAccessInfo &usage_info;
        const SyncOrdering ordering_rule;
        const ResourceUsageTagEx tag_ex;
        const SyncFlags flags;
    };

    // Follow the context previous to access the access state, supporting "lazy" import into the context. Not intended for
    // subpass layout transition, as the pending state handling is more complex (TODO: check if previous statement is
    // still true after pending barriers rework).
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(const AccessRange &range, AccessMap *descent_map, bool infill,
                               const AccessStateFunction *previous_barrier = nullptr) const;
    template <typename BarrierAction>
    void ResolvePreviousAccessStack(const AccessRange &range, AccessMap *descent_map, bool infill,
                                    const BarrierAction &previous_barrie) const;
    template <typename BarrierAction>
    void ResolveAccessRange(const AccessRange &range, BarrierAction &barrier_action, AccessMap *resolve_map, bool infill,
                            bool recur_to_infill = true) const;

    template <typename Detector>
    HazardResult DetectHazardRange(Detector &detector, const AccessRange &range, DetectOptions options) const;
    template <typename Detector, typename RangeGen>
    HazardResult DetectHazardGeneratedRanges(Detector &detector, RangeGen &range_gen, DetectOptions options) const;
    template <typename Detector, typename RangeGen>
    HazardResult DetectHazardGeneratedRanges(Detector &detector, const RangeGen &range_gen, DetectOptions options) const {
        RangeGen mutable_gen(range_gen);
        return DetectHazardGeneratedRanges<Detector, RangeGen>(detector, mutable_gen, options);
    }
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                              const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                              bool is_depth_sliced, DetectOptions options) const;

    template <typename Detector>
    HazardResult DetectHazardOneRange(Detector &detector, bool detect_prev, AccessMap::const_iterator &pos,
                                      const AccessMap::const_iterator &the_end, const AccessRange &range) const;

    template <typename Detector, typename RangeGen>
    HazardResult DetectAsyncHazard(const Detector &detector, const RangeGen &const_range_gen, ResourceUsageTag async_tag,
                                   QueueId async_queue_id) const;

    template <typename Detector>
    HazardResult DetectPreviousHazard(Detector &detector, const AccessRange &range) const;

  private:
    AccessMap access_state_map_;

    std::vector<SubpassBarrierTrackback> prev_;
    std::vector<SubpassBarrierTrackback *> prev_by_subpass_;

    // These contexts *must* have the same lifespan as this context, or be cleared, before the referenced contexts can expire
    std::vector<AsyncReference> async_;

    SubpassBarrierTrackback *src_external_ = nullptr;
    SubpassBarrierTrackback dst_external_;
    ResourceUsageTag start_tag_ = 0;

    // True if access map won't be modified anymore.
    // NOTE: In the current implementation we mark only command buffer contexts as finalized.
    // TODO: mark other contexts as finalized too if needed.
    bool finalized_ = false;

    // Provides ordering of the context's first accesses based on tag values.
    // Only available for finalized contexts.
    SortedFirstAccesses sorted_first_accesses_;
};

// The semantics of the InfillUpdateOps of infill_update_range are slightly different than for the UpdateMemoryAccessState Action
// operations, as this simplifies the generic traversal.  So we wrap them in a semantics Adapter to get the same effect.
template <typename Action>
struct ActionToOpsAdapter {
    void infill(AccessMap &accesses, const AccessMap::iterator &pos, const AccessRange &infill_range) const {
        // Combine Infill and update operations to make the generic implementation simpler
        AccessMap::iterator infill = action.Infill(&accesses, pos, infill_range);
        if (infill == accesses.end()) return;  // Allow action to 'pass' on filling in the blanks

        // Need to apply the action to the Infill.  'infill_update_range' expect ops.infill to be completely done with
        // the infill_range, where as Action::Infill assumes the caller will apply the action() logic to the infill_range
        for (; infill != pos; ++infill) {
            assert(infill != accesses.end());
            action(infill);
        }
    }
    void update(const AccessMap::iterator &pos) const { action(pos); }
    const Action &action;
};

template <typename Action>
void AccessContext::UpdateMemoryAccessRangeState(Action &action, const AccessRange &range) {
    assert(!finalized_);
    ActionToOpsAdapter<Action> ops{action};
    InfillUpdateRange(access_state_map_, range, ops);
}

template <typename Action, typename RangeGen>
void AccessContext::UpdateMemoryAccessState(const Action &action, RangeGen &range_gen) {
    assert(!finalized_);
    ActionToOpsAdapter<Action> ops{action};
    auto pos = access_state_map_.LowerBound(*range_gen);
    for (; range_gen->non_empty(); ++range_gen) {
        pos = InfillUpdateRange(access_state_map_, pos, *range_gen, ops);
    }
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector, typename RangeGen>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, const RangeGen &const_range_gen, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    using RangeType = typename RangeGen::RangeType;
    using ConstIterator = AccessMap::const_iterator;
    RangeGen range_gen(const_range_gen);

    HazardResult hazard;

    auto do_async_hazard_check = [&detector, async_tag, async_queue_id, &hazard](const RangeType &range, const ConstIterator &end,
                                                                                 ConstIterator &pos) {
        while (pos != end && pos->first.begin < range.end) {
            hazard = detector.DetectAsync(pos, async_tag, async_queue_id);
            if (hazard.IsHazard()) return true;
            ++pos;
        }
        return false;
    };

    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_async_hazard_check);

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectHazardOneRange(Detector &detector, bool detect_prev, AccessMap::const_iterator &pos,
                                                 const AccessMap::const_iterator &the_end, const AccessRange &range) const {
    HazardResult hazard;
    AccessRange gap = {range.begin, range.begin};

    while (pos != the_end && pos->first.begin < range.end) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(detector, gap);
                if (hazard.IsHazard()) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.IsHazard()) return hazard;
        ++pos;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(detector, gap);
        }
    }

    return hazard;
}

// A wrapper for a single range with the same semantics as other non-trivial range generators
template <typename KeyType>
class SingleRangeGenerator {
  public:
    using RangeType = KeyType;
    SingleRangeGenerator(const KeyType &range) : current_(range) {}
    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    SingleRangeGenerator &operator++() {
        current_ = KeyType();  // just one real range
        return *this;
    }

  private:
    SingleRangeGenerator() = default;
    KeyType current_;
};

template <typename Detector>
HazardResult AccessContext::DetectHazardRange(Detector &detector, const AccessRange &range, DetectOptions options) const {
    SingleRangeGenerator range_gen(range);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

template <typename BarrierAction>
void AccessContext::ResolveAccessRange(const AccessRange &range, BarrierAction &barrier_action, AccessMap *resolve_map, bool infill,
                                       bool recur_to_infill) const {
    if (!range.non_empty()) return;

    ParallelIterator current(*resolve_map, access_state_map_, range.begin);
    while (current->range.non_empty() && range.includes(current->range.begin)) {
        const auto current_range = current->range & range;
        if (current->pos_B.inside_lower_bound_range) {
            const auto &src_pos = current->pos_B.lower_bound;
            AccessState access(src_pos->second);  // intentional copy
            barrier_action(&access);
            if (current->pos_A.inside_lower_bound_range) {
                const auto trimmed = Split(current->pos_A.lower_bound, *resolve_map, current_range);
                trimmed->second.Resolve(access);
                current.InvalidateA(trimmed);
            } else {
                auto inserted = resolve_map->Insert(current->pos_A.lower_bound, current_range, access);
                current.InvalidateA(inserted);  // Update the parallel iterator to point at the insert segment
            }
        } else {
            // we have to descend to fill this gap
            if (recur_to_infill) {
                AccessRange recurrence_range = current_range;
                // The current context is empty for the current range, so recur to fill the gap.
                // Since we will be recurring back up the DAG, expand the gap descent to cover the full range for which B
                // is not valid, to minimize that recurrence
                if (current->pos_B.lower_bound == access_state_map_.end()) {
                    // Do the remainder here....
                    recurrence_range.end = range.end;
                } else {
                    // Recur only over the range until B becomes valid (within the limits of range).
                    recurrence_range.end = std::min(range.end, current->pos_B.lower_bound->first.begin);
                }
                ResolvePreviousAccessStack(recurrence_range, resolve_map, infill, barrier_action);

                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = recurrence_range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.InvalidateA();                         // Changes current->range
                current.Seek(seek_to);
            } else if (!current->pos_A.inside_lower_bound_range && infill) {
                // If we didn't find anything in the current range, and we aren't reccuring... we infill if required
                auto inserted = resolve_map->Insert(current->pos_A.lower_bound, current->range, AccessState{});
                current.InvalidateA(inserted);  // Update the parallel iterator to point at the correct segment after insert
            }
        }
        if (current->range.non_empty()) {
            ++current;
        }
    }

    // Infill if range goes passed both the current and resolve map prior contents
    if (recur_to_infill && (current->range.end < range.end)) {
        AccessRange trailing_fill_range = {current->range.end, range.end};
        ResolvePreviousAccessStack<BarrierAction>(trailing_fill_range, resolve_map, infill, barrier_action);
    }
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector, typename RangeGen>
HazardResult AccessContext::DetectHazardGeneratedRanges(Detector &detector, RangeGen &range_gen, DetectOptions options) const {
    HazardResult hazard;

    // Do this before range_gen is incremented s.t. the copies used will be correct
    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_ref : async_) {
            hazard = async_ref.Context().DetectAsyncHazard(detector, range_gen, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    using RangeType = typename RangeGen::RangeType;
    using ConstIterator = AccessMap::const_iterator;
    auto do_detect_hazard_range = [this, &detector, &hazard, detect_prev](const RangeType &range, const ConstIterator &end,
                                                                          ConstIterator &pos) {
        hazard = DetectHazardOneRange(detector, detect_prev, pos, end, range);
        return hazard.IsHazard();
    };

    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_detect_hazard_range);

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(Detector &detector, const AccessRange &range) const {
    AccessMap descent_map;
    ResolvePreviousAccess(range, &descent_map, false);

    for (auto prev = descent_map.begin(); prev != descent_map.end(); ++prev) {
        HazardResult hazard = detector.Detect(prev);
        if (hazard.IsHazard()) {
            return hazard;
        }
    }
    return {};
}

template <typename Predicate>
void AccessContext::EraseIf(Predicate &&pred) {
    assert(!finalized_);
    auto pos = access_state_map_.begin();
    while (pos != access_state_map_.end()) {
        if (pred(*pos)) {
            pos = access_state_map_.Erase(pos);
        } else {
            ++pos;
        }
    }
}

template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context) {
    assert(!finalized_);
    from_context.ResolveAccessRange(kFullRange, resolve_op, &access_state_map_, false, false);
}

// TODO: ImageRangeGenerator is a huge object. Here we make a copy. There is at least one place where it is
// already constructed on the stack and can be modified directly without copy. In other cases we need
// to copy. It's important to reduce size of ImageRangeGenerator (potentially for performance, not memory usage,
// the latter is a bounus).
template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                                       subresource_adapter::ImageRangeGenerator range_gen, bool infill, bool recur_to_infill) {
    assert(!finalized_);
    for (; range_gen->non_empty(); ++range_gen) {
        from_context.ResolveAccessRange(*range_gen, resolve_op, &access_state_map_, infill, recur_to_infill);
    }
}

template <typename BarrierAction>
void AccessContext::ResolvePreviousAccessStack(const AccessRange &range, AccessMap *descent_map, bool infill,
                                               const BarrierAction &previous_barrier) const {
    AccessStateFunction stacked_barrier(std::ref(previous_barrier));
    ResolvePreviousAccess(range, descent_map, infill, &stacked_barrier);
}

}  // namespace syncval
