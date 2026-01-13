/*
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

#pragma once

#include "sync/sync_common.h"
#include "sync/sync_access_map.h"
#include "sync/sync_access_state.h"
#include <optional>

struct SubpassDependencyGraphNode;

namespace vvl {
class Bindable;
class Buffer;
class Image;
class ImageView;
class VideoPictureResource;
class VideoSession;
}  // namespace vvl

namespace syncval {

class AccessContext;
class SyncValidator;

bool SimpleBinding(const vvl::Bindable &bindable);
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer);

// Execute Action for each map entry in the generated ranges until it returns true
//
// Action is const w.r.t. map
// Action is allowed (assumed) to modify pos
// Action must not advance pos for ranges strictly < pos->first
// Action must handle range strictly less than pos->first correctly
// Action must handle pos == end correctly
// Action is assumed to only require invocation once per map entry
// Note: If Action invocations are heavyweight and inter-entry (gap) calls are not needed
//       add a template or function parameter to skip them. TBD.
template <typename Action>
bool ForEachEntryInRangesUntil(const AccessMap &map, ImageRangeGen &range_gen, Action &action) {
    using RangeType = ImageRangeGen::RangeType;
    using IndexType = RangeType::index_type;
    auto pos = map.LowerBound((*range_gen).begin);
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
                pos = map.LowerBound(range.begin);
                if (pos == end) break;
            }
            assert(pos == map.LowerBound(range.begin));
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

// A single buffer barrier can be applied immediately to a memory range.
// Note that multiple barriers (of the same or different types) need to use
// the pending barriers functionality to ensure independent barrier application
struct ApplySingleBufferBarrierFunctor {
    ApplySingleBufferBarrierFunctor(const AccessContext &access_context, const BarrierScope &barrier_scope,
                                    const SyncBarrier &barrier);

    using Iterator = AccessMap::iterator;
    Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const;
    void operator()(const Iterator &pos) const;

    const AccessContext &access_context;
    const BarrierScope &barrier_scope;
    const SyncBarrier &barrier;
};

// A single image barrier can be applied immediately to a memory range.
// Note that multiple barriers (of the same or different types) need to use
// the pending barriers functionality to ensure independent barrier application
struct ApplySingleImageBarrierFunctor {
    ApplySingleImageBarrierFunctor(const AccessContext &access_context, const BarrierScope &barrier_scope,
                                   const SyncBarrier &barrier, bool layout_transition, uint32_t layout_transition_handle_index,
                                   ResourceUsageTag exec_tag);

    using Iterator = AccessMap::iterator;
    Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const;
    void operator()(const Iterator &pos) const;

    const AccessContext &access_context;
    const BarrierScope &barrier_scope;
    const SyncBarrier &barrier;
    const ResourceUsageTag exec_tag;
    bool layout_transition;
    uint32_t layout_transition_handle_index;
};

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
        auto inserted = accesses->Insert(pos_hint, range, AccessState::DefaultAccessState());
        return inserted;
    }
    void operator()(const Iterator &pos) const {}
    const bool layout_transition;
};

// This functor populates PendingBarriers with the results of independent barrier appication (pending barriers).
// After this functor finished its work then PendingBarriers::Apply() can be used to update the access states.
struct CollectBarriersFunctor {
    CollectBarriersFunctor(const AccessContext &access_context, const BarrierScope &barrier_scope, const SyncBarrier &barrier,
                           bool layout_transition, uint32_t layout_transition_handle_index, PendingBarriers &pending_barriers)
        : access_context(access_context),
          barrier_scope(barrier_scope),
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
    void operator()(const Iterator &pos) const;

    const AccessContext &access_context;
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

struct ApplySubpassTransitionBarriersAction {
    explicit ApplySubpassTransitionBarriersAction(const std::vector<SyncBarrier> &barriers, ResourceUsageTag layout_transition_tag)
        : barriers(barriers), layout_transition_tag(layout_transition_tag) {}
    void operator()(AccessState *access) const {
        assert(access);
        ApplyBarriers(*access, barriers, true, layout_transition_tag);
    }
    const std::vector<SyncBarrier> &barriers;
    const ResourceUsageTag layout_transition_tag;
};

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
    enum Gen {
        kViewSubresource = 0,        // Always available
        kRenderArea = 1,             // Always available
        kDepthOnlyRenderArea = 2,    // Only for formats with both depth and stencil to select depth
        kStencilOnlyRenderArea = 3,  // Only for formats with both depth and stencil to select stencil
        kGenSize = 4
    };
    AttachmentViewGen(const vvl::ImageView *image_view, const VkOffset3D &offset, const VkExtent3D &extent);
    const vvl::ImageView *GetViewState() const { return view_; }
    const std::optional<ImageRangeGen> &GetRangeGen(Gen type) const;
    Gen GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const;

  private:
    const vvl::ImageView *view_ = nullptr;
    std::array<std::optional<ImageRangeGen>, Gen::kGenSize> gen_store_;
};

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

    void ResolveFromContext(const AccessContext &from);

    // Non-lazy import of all accesses. WaitEvents needs this.
    void ResolvePreviousAccesses();

    // Resolves this subpass context from the subpass context defined by the layout transition dependency
    void ResolveFromSubpassContext(const ApplySubpassTransitionBarriersAction &subpass_transition_action,
                                   const AccessContext &from_context,
                                   subresource_adapter::ImageRangeGenerator attachment_range_gen);

    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context);

    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                            subresource_adapter::ImageRangeGenerator range_gen);

    void UpdateAccessState(const vvl::Buffer &buffer, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const AccessRange &range, ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void UpdateAccessState(ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex, SyncFlags flags = 0);

    void ResolveChildContexts(vvl::span<AccessContext> subpass_contexts);

    void ImportAsyncContexts(const AccessContext &from);
    void ClearAsyncContexts() { async_.clear(); }

    AccessContext() = default;
    AccessContext(const SyncValidator &validator) : validator(&validator) {}

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
    void UpdateMemoryAccessState(Action &action, const AccessRange &range);

    template <typename Action, typename RangeGen>
    void UpdateMemoryAccessState(const Action &action, RangeGen &range_gen);

    void RegisterGlobalBarrier(const SyncBarrier &barrier, QueueId queue_id);
    void ApplyGlobalBarriers(AccessState &access_state) const;
    uint32_t GetGlobalBarrierCount() const { return (uint32_t)global_barriers_.size(); }

    // Called when all accesses are recorded. This can be used for preprocessing
    // or caching purposes. After finalization, it is save to keep persistent
    // references to individual accesses (until context is destroyed).
    void Finalize();

  private:
    void ResetGlobalBarriers();

    // Follow the context previous to access the access state, supporting "lazy" import into the context. Not intended for
    // subpass layout transition, as the pending state handling is more complex (TODO: check if previous statement is
    // still true after pending barriers rework).
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(const AccessRange &range, AccessContext &descent_context, bool infill,
                               const AccessStateFunction &previous_barrier_action) const;
    void ResolveAccessRange(const AccessRange &range, const AccessStateFunction &barrier_action,
                            AccessContext &resolve_context) const;
    void ResolveAccessRangeRecursePrev(const AccessRange &range, const AccessStateFunction &barrier_action,
                                       AccessContext &resolve_context, bool infill) const;

    AccessMap::iterator InfillGapRecursePrev(const AccessRange &range, AccessMap::iterator pos_hint);
    AccessMap::iterator DoUpdateAccessState(AccessMap::iterator pos, const AccessRange &range, SyncAccessIndex access_index,
                                            SyncOrdering ordering_rule, ResourceUsageTagEx tag_ex, SyncFlags flags);

    template <typename Detector>
    HazardResult DetectHazardRange(Detector &detector, const AccessRange &range, DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazardGeneratedRanges(Detector &detector, ImageRangeGen &range_gen, DetectOptions options) const;
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

    template <typename Detector>
    HazardResult DetectAsyncHazard(const Detector &detector, const AccessRange &range, ResourceUsageTag async_tag,
                                   QueueId async_queue_id) const;
    template <typename Detector>
    HazardResult DetectAsyncHazard(const Detector &detector, ImageRangeGen &range_gen, ResourceUsageTag async_tag,
                                   QueueId async_queue_id) const;

    template <typename Detector>
    HazardResult DetectPreviousHazard(Detector &detector, const AccessRange &range) const;

  public:
    const SyncValidator *validator = nullptr;

  private:
    AccessMap access_state_map_;

    std::vector<SubpassBarrierTrackback> prev_;
    std::vector<SubpassBarrierTrackback *> prev_by_subpass_;

    // These contexts *must* have the same lifespan as this context, or be cleared, before the referenced contexts can expire
    std::vector<AsyncReference> async_;

    SubpassBarrierTrackback *src_external_ = nullptr;
    SubpassBarrierTrackback dst_external_;
    ResourceUsageTag start_tag_ = 0;

    // Global barriers are registered at the AccessContext level and applied to access states
    // lazily when the access state's barrier information is needed.
    // Global barriers track VkMemoryBarrier barriers and execution dependencies, including
    // those from image or buffer barriers.
    static constexpr uint32_t kMaxGlobaBarrierDefCount = 8;
    struct GlobalBarrierDef {
        SyncBarrier barrier;
        // The i-th bit indicates whether the source stage of this barrier
        // chains with the destination stage of the barrier for the i-th def.
        uint32_t chain_mask = 0;
    };
    QueueId global_barriers_queue_ = kQueueIdInvalid;
    GlobalBarrierDef global_barrier_defs_[kMaxGlobaBarrierDefCount];
    uint32_t global_barrier_def_count_ = 0;
    std::vector<uint32_t> global_barriers_;

    // True if access map won't be modified anymore.
    // NOTE: In the current implementation we mark only command buffer contexts as finalized.
    // TODO: mark other contexts as finalized too if needed.
    bool finalized_ = false;

    // Provides ordering of the context's first accesses based on tag values.
    // Only available for finalized contexts.
    SortedFirstAccesses sorted_first_accesses_;
};

// The semantics of the InfillUpdateOps of InfillUpdateRange are slightly different than for the
// UpdateMemoryAccessState Action operations, as this simplifies the generic traversal. So we wrap
// them in a semantics Adapter to get the same effect.
template <typename Action>
struct ActionToOpsAdapter {
    void infill(AccessMap &accesses, const AccessMap::iterator &pos, const AccessRange &infill_range) const {
        AccessMap::iterator infill = action.Infill(&accesses, pos, infill_range);

        // Need to apply the action to the Infill.
        // InfillUpdateRange expects ops.infill to be completely done with the infill_range,
        // where as Action::Infill assumes the caller will apply the action() logic to the infill_range
        for (; infill != pos; ++infill) {
            assert(infill != accesses.end());
            action(infill);
        }
    }
    void update(const AccessMap::iterator &pos) const { action(pos); }
    const Action &action;
};

template <typename Action>
void AccessContext::UpdateMemoryAccessState(Action &action, const AccessRange &range) {
    assert(!finalized_);
    ActionToOpsAdapter<Action> ops{action};
    InfillUpdateRange(access_state_map_, range, ops);
}

template <typename Action, typename RangeGen>
void AccessContext::UpdateMemoryAccessState(const Action &action, RangeGen &range_gen) {
    assert(!finalized_);
    ActionToOpsAdapter<Action> ops{action};
    auto pos = access_state_map_.LowerBound(range_gen->begin);
    for (; range_gen->non_empty(); ++range_gen) {
        pos = InfillUpdateRange(access_state_map_, pos, *range_gen, ops);
    }
}

template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, const AccessRange &range, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    HazardResult hazard;
    if (range.non_empty()) {
        auto pos = access_state_map_.LowerBound(range.begin);
        if (pos != access_state_map_.end() && pos->first.begin < range.end) {
            hazard = detector.DetectAsync(pos, async_tag, async_queue_id);
        }
    }
    return hazard;
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, ImageRangeGen &range_gen, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    using ConstIterator = AccessMap::const_iterator;

    HazardResult hazard;

    auto do_async_hazard_check = [&detector, async_tag, async_queue_id, &hazard](const ImageRangeGen::RangeType &range,
                                                                                 const ConstIterator &end, ConstIterator &pos) {
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

template <typename Detector>
HazardResult AccessContext::DetectHazardRange(Detector &detector, const AccessRange &range, DetectOptions options) const {
    HazardResult hazard;

    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_ref : async_) {
            hazard = async_ref.Context().DetectAsyncHazard(detector, range, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    if (range.non_empty()) {
        auto pos = access_state_map_.LowerBound(range.begin);
        hazard = DetectHazardOneRange(detector, detect_prev, pos, access_state_map_.end(), range);
    }
    return hazard;
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessContext::DetectHazardGeneratedRanges(Detector &detector, ImageRangeGen &range_gen, DetectOptions options) const {
    HazardResult hazard;

    // Do this before range_gen is incremented s.t. the copies used will be correct
    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_ref : async_) {
            ImageRangeGen range_gen_copy(range_gen);  // original range gen is needed later
            hazard = async_ref.Context().DetectAsyncHazard(detector, range_gen_copy, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    using ConstIterator = AccessMap::const_iterator;
    auto do_detect_hazard_range = [this, &detector, &hazard, detect_prev](const ImageRangeGen::RangeType &range,
                                                                          const ConstIterator &end, ConstIterator &pos) {
        hazard = DetectHazardOneRange(detector, detect_prev, pos, end, range);
        return hazard.IsHazard();
    };

    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_detect_hazard_range);

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(Detector &detector, const AccessRange &range) const {
    if (prev_.empty()) {
        return {};
    }
    AccessContext descent_context;
    for (const auto &prev_dep : prev_) {
        const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, nullptr);
        prev_dep.source_subpass->ResolveAccessRangeRecursePrev(range, barrier_action, descent_context, false);
    }
    AccessMap &descent_map = descent_context.access_state_map_;
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
    from_context.ResolveAccessRange(kFullRange, resolve_op, *this);
}

template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                                       subresource_adapter::ImageRangeGenerator range_gen) {
    assert(!finalized_);
    for (; range_gen->non_empty(); ++range_gen) {
        from_context.ResolveAccessRange(*range_gen, resolve_op, *this);
    }
}

}  // namespace syncval
