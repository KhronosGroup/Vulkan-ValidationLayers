/* Copyright (c) 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
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

class ResourceAccessState;
class ResourceAccessWriteState;
struct ResourceFirstAccess;

// NOTE: the attachement read flag is put *only* in the access scope and not in the exect scope, since the ordering
//       rules apply only to this specific access for this stage, and not the stage as a whole. The ordering detection
//       also reflects this special case for read hazard detection (using access instead of exec scope)
constexpr VkPipelineStageFlags2KHR kColorAttachmentExecScope = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
const SyncStageAccessFlags kColorAttachmentAccessScope =
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_BIT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
constexpr VkPipelineStageFlags2KHR kDepthStencilAttachmentExecScope =
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR;
const SyncStageAccessFlags kDepthStencilAttachmentAccessScope =
    SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
constexpr VkPipelineStageFlags2KHR kRasterAttachmentExecScope = kDepthStencilAttachmentExecScope | kColorAttachmentExecScope;
const SyncStageAccessFlags kRasterAttachmentAccessScope = kDepthStencilAttachmentAccessScope | kColorAttachmentAccessScope;

enum SyncHazard {
    NONE = 0,
    READ_AFTER_WRITE,
    WRITE_AFTER_READ,
    WRITE_AFTER_WRITE,
    READ_RACING_WRITE,
    WRITE_RACING_WRITE,
    WRITE_RACING_READ,
    WRITE_AFTER_PRESENT,  // Once presented, an image may not be used until acquired
    READ_AFTER_PRESENT,
    PRESENT_AFTER_READ,  // Must be unreferenced and visible to present
    PRESENT_AFTER_WRITE,
};

enum class SyncOrdering : uint8_t {
    kOrderingNone = 0,
    kNonAttachment = kOrderingNone,
    kColorAttachment = 1,
    kDepthStencilAttachment = 2,
    kRaster = 3,
    kNumOrderings = 4,
};
const char *string_SyncHazard(SyncHazard hazard);
const char *string_SyncHazardVUID(SyncHazard hazard);

class HazardResult {
  public:
    struct HazardState {
        std::unique_ptr<const ResourceAccessState> access_state;
        std::unique_ptr<const ResourceFirstAccess> recorded_access;
        SyncStageAccessIndex usage_index = std::numeric_limits<SyncStageAccessIndex>::max();
        SyncStageAccessFlags prior_access;
        ResourceUsageTag tag = ResourceUsageTag();
        SyncHazard hazard = NONE;
        HazardState(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
                    const SyncStageAccessFlags &prior_, ResourceUsageTag tag_);
    };

    void Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
             const ResourceAccessWriteState &prior_write);
    void Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
             const SyncStageAccessFlags &prior_, ResourceUsageTag tag_);
    void AddRecordedAccess(const ResourceFirstAccess &first_access);

    bool IsHazard() const { return state_.has_value() && NONE != state_->hazard; }
    bool IsWAWHazard() const;
    ResourceUsageTag Tag() const {
        assert(state_);
        return state_->tag;
    }
    SyncHazard Hazard() const {
        assert(state_);
        return state_->hazard;
    }
    const std::unique_ptr<const ResourceFirstAccess> &RecordedAccess() const {
        assert(state_);
        return state_->recorded_access;
    }
    const HazardState &State() const {
        assert(state_);
        return state_.value();
    }

  private:
    std::optional<HazardState> state_;
};

struct SyncExecScope {
    VkPipelineStageFlags2KHR mask_param;  // the xxxStageMask parameter passed by the caller
    VkPipelineStageFlags2KHR
        expanded_mask;                    // all stage bits covered by any 'catch all bits' in the parameter (eg. ALL_GRAPHICS_BIT).
    VkPipelineStageFlags2KHR exec_scope;  // all earlier or later stages that would be affected by a barrier using this scope.
    SyncStageAccessFlags valid_accesses;  // all valid accesses that can be used with this scope.

    SyncExecScope() : mask_param(0), expanded_mask(0), exec_scope(0), valid_accesses(0) {}
    SyncExecScope(VkPipelineStageFlags2KHR mask_param_, VkPipelineStageFlags2KHR expanded_mask_,
                  VkPipelineStageFlags2KHR exec_scope_, const SyncStageAccessFlags &valid_accesses_)
        : mask_param(mask_param_), expanded_mask(expanded_mask_), exec_scope(exec_scope_), valid_accesses(valid_accesses_) {}

    static SyncExecScope MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR src_stage_mask,
                                 const VkPipelineStageFlags2KHR disabled_feature_mask = 0);
    static SyncExecScope MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR src_stage_mask);
};

struct SemaphoreScope : SyncExecScope {
    SemaphoreScope(QueueId qid, const SyncExecScope &exec_scope) : SyncExecScope(exec_scope), queue(qid) {}
    SemaphoreScope() = default;
    QueueId queue;
};

struct SyncBarrier {
    struct AllAccess {};
    SyncExecScope src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    SyncExecScope dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
    SyncBarrier() = default;
    SyncBarrier(const SyncBarrier &other) = default;
    SyncBarrier &operator=(const SyncBarrier &) = default;

    SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst);
    SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst, const AllAccess &);
    SyncBarrier(const SyncExecScope &src_exec, const SyncStageAccessFlags &src_access, const SyncExecScope &dst_exec,
                const SyncStageAccessFlags &dst_access)
        : src_exec_scope(src_exec), src_access_scope(src_access), dst_exec_scope(dst_exec), dst_access_scope(dst_access) {}

    template <typename Barrier>
    SyncBarrier(const Barrier &barrier, const SyncExecScope &src, const SyncExecScope &dst);

    SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &barrier);
    // template constructor for sync2 barriers
    template <typename Barrier>
    SyncBarrier(VkQueueFlags queue_flags, const Barrier &barrier);

    void Merge(const SyncBarrier &other) {
        // Note that after merge, only the exec_scope and access_scope fields are fully valid
        // TODO: Do we need to update any of the other fields?  Merging has limited application.
        src_exec_scope.exec_scope |= other.src_exec_scope.exec_scope;
        src_access_scope |= other.src_access_scope;
        dst_exec_scope.exec_scope |= other.dst_exec_scope.exec_scope;
        dst_access_scope |= other.dst_access_scope;
    }
    SyncBarrier(const std::vector<SyncBarrier> &barriers);
};

struct ResourceFirstAccess {
    ResourceUsageTag tag;
    const SyncStageAccessInfoType *usage_info;
    SyncOrdering ordering_rule;
    ResourceFirstAccess(ResourceUsageTag tag_, const SyncStageAccessInfoType &usage_info_, SyncOrdering ordering_rule_)
        : tag(tag_), usage_info(&usage_info_), ordering_rule(ordering_rule_){};
    ResourceFirstAccess(const ResourceFirstAccess &other) = default;
    ResourceFirstAccess(ResourceFirstAccess &&other) = default;
    ResourceFirstAccess &operator=(const ResourceFirstAccess &rhs) = default;
    ResourceFirstAccess &operator=(ResourceFirstAccess &&rhs) = default;
    bool operator==(const ResourceFirstAccess &rhs) const {
        return (tag == rhs.tag) && (usage_info == rhs.usage_info) && (ordering_rule == rhs.ordering_rule);
    }
};

using QueueId = uint32_t;
struct OrderingBarrier {
    VkPipelineStageFlags2KHR exec_scope;
    SyncStageAccessFlags access_scope;
    OrderingBarrier() = default;
    OrderingBarrier(const OrderingBarrier &) = default;
    OrderingBarrier(VkPipelineStageFlags2KHR es, SyncStageAccessFlags as) : exec_scope(es), access_scope(as) {}
    OrderingBarrier &operator=(const OrderingBarrier &) = default;
    OrderingBarrier &operator|=(const OrderingBarrier &rhs) {
        exec_scope |= rhs.exec_scope;
        access_scope |= rhs.access_scope;
        return *this;
    }
    bool operator==(const OrderingBarrier &rhs) const {
        return (exec_scope == rhs.exec_scope) && (access_scope == rhs.access_scope);
    }
};

using ResourceUsageTagSet = CachedInsertSet<ResourceUsageTag, 4>;

class ResourceAccessWriteState {
  public:
    bool operator==(const ResourceAccessWriteState &rhs) const {
        return (access_ == rhs.access_) && (barriers_ == rhs.barriers_) && (tag_ == rhs.tag_) && (queue_ == rhs.queue_) &&
               (dependency_chain_ == rhs.dependency_chain_);
    }
    bool WriteInChain(VkPipelineStageFlags2KHR src_exec_scope) const;
    bool WriteInScope(const SyncStageAccessFlags &src_access_scope) const;
    bool WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const;
    bool WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                        const SyncStageAccessFlags &src_access_scope) const;

    bool WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope,
                           QueueId scope_queue, ResourceUsageTag scope_tag) const;

    ResourceAccessWriteState(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag);
    ResourceAccessWriteState() = default;

    SyncStageAccessIndex Index() const { return access_->stage_access_index; }
    bool IsIndex(SyncStageAccessIndex usage_index) const { return Index() == usage_index; }
    bool IsQueue(QueueId other_queue) const { return queue_ == other_queue; }
    const SyncStageAccessInfoType &Access() const { return *access_; }
    const SyncStageAccessFlags &Barriers() const { return barriers_; }
    ResourceUsageTag Tag() const { return tag_; }
    bool IsWriteHazard(const SyncStageAccessInfoType &usage_info) const;
    bool IsOrdered(const OrderingBarrier &ordering, QueueId queue_id) const;

    bool IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                              const SyncStageAccessFlags &src_access_scope) const;

    void SetQueueId(QueueId id);
    void Set(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag);
    void MergeBarriers(const ResourceAccessWriteState &other);
    void OffsetTag(ResourceUsageTag offset) { tag_ += offset; }

    bool HasPendingState() const { return pending_barriers_.any() || (0 != pending_dep_chain_); }
    void ClearPending();
    void UpdatePendingBarriers(const SyncBarrier &barrier);
    void ApplyPendingBarriers();
    void UpdatePendingLayoutOrdering(const SyncBarrier &barrier);
    const OrderingBarrier &GetPendingLayoutOrdering() const { return pending_layout_ordering_; }

  private:
    const SyncStageAccessInfoType *access_;
    SyncStageAccessFlags barriers_;  // union of applicable barrier masks since last write
    ResourceUsageTag tag_;
    QueueId queue_;
    // intially zero, but accumulating the dstStages of barriers if they chain.
    VkPipelineStageFlags2KHR dependency_chain_;

    // Write specific layout state
    OrderingBarrier pending_layout_ordering_;
    VkPipelineStageFlags2KHR pending_dep_chain_;
    SyncStageAccessFlags pending_barriers_;

    friend ResourceAccessState;
};

class ResourceAccessState : public SyncStageAccess {
  protected:
    using OrderingBarriers = std::array<OrderingBarrier, static_cast<size_t>(SyncOrdering::kNumOrderings)>;
    using FirstAccesses = small_vector<ResourceFirstAccess, 3>;

  public:
    // Mutliple read operations can be simlutaneously (and independently) synchronized,
    // given the only the second execution scope creates a dependency chain, we have to track each,
    // but only up to one per pipeline stage (as another read from the *same* stage become more recent,
    // and applicable one for hazard detection
    struct ReadState {
        VkPipelineStageFlags2KHR stage;        // The stage of this read
        SyncStageAccessFlags access;           // TODO: Change to FlagBits when we have a None bit enum
                                               // TODO: Revisit whether this needs to support multiple reads per stage
        VkPipelineStageFlags2KHR barriers;     // all applicable barriered stages
        VkPipelineStageFlags2KHR sync_stages;  // reads known to have happened after this
        ResourceUsageTag tag;
        QueueId queue;
        VkPipelineStageFlags2KHR pending_dep_chain;  // Should be zero except during barrier application
                                                     // Excluded from comparison
        ReadState() = default;
        ReadState(VkPipelineStageFlags2KHR stage_, SyncStageAccessFlags access_, VkPipelineStageFlags2KHR barriers_,
                  ResourceUsageTag tag_);
        bool operator==(const ReadState &rhs) const {
            return (stage == rhs.stage) && (access == rhs.access) && (barriers == rhs.barriers) &&
                   (sync_stages == rhs.sync_stages) && (tag == rhs.tag) && (queue == rhs.queue) &&
                   (pending_dep_chain == rhs.pending_dep_chain);
        }
        void Normalize() { pending_dep_chain = VK_PIPELINE_STAGE_2_NONE; }
        bool IsReadBarrierHazard(VkPipelineStageFlags2KHR src_exec_scope) const {
            // If the read stage is not in the src sync scope
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            return (src_exec_scope & (stage | barriers)) == 0;
        }
        bool IsReadBarrierHazard(QueueId barrier_queue, VkPipelineStageFlags2KHR src_exec_scope) const {
            // If the read stage is not in the src sync scope
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            VkPipelineStageFlags2 queue_ordered_stage = (queue == barrier_queue) ? stage : VK_PIPELINE_STAGE_2_NONE;
            return (src_exec_scope & (queue_ordered_stage | barriers)) == 0;
        }

        bool operator!=(const ReadState &rhs) const { return !(*this == rhs); }
        void Set(VkPipelineStageFlags2KHR stage_, const SyncStageAccessFlags &access_, VkPipelineStageFlags2KHR barriers_,
                 ResourceUsageTag tag_);
        bool ReadInScopeOrChain(VkPipelineStageFlags2 exec_scope) const { return (exec_scope & (stage | barriers)) != 0; }
        bool ReadInQueueScopeOrChain(QueueId queue, VkPipelineStageFlags2 exec_scope) const;
        bool ReadInEventScope(VkPipelineStageFlags2 exec_scope, QueueId scope_queue, ResourceUsageTag scope_tag) const {
            // If this read is the same one we included in the set event and in scope, then apply the execution barrier...
            // NOTE: That's not really correct... this read stage might *not* have been included in the setevent, and the barriers
            // representing the chain might have changed since then (that would be an odd usage), so as a first approximation
            // we'll assume the barriers *haven't* been changed since (if the tag hasn't), and while this could be a false
            // positive in the case of Set; SomeBarrier; Wait; we'll live with it until we can add more state to the first scope
            // capture (the specific write and read stages that *were* in scope at the moment of SetEvents.
            return (tag < scope_tag) && ReadInQueueScopeOrChain(scope_queue, exec_scope);
        }
        void ApplyReadBarrier(VkPipelineStageFlags2KHR dst_scope) { pending_dep_chain |= dst_scope; }
        VkPipelineStageFlags2 ApplyPendingBarriers();
    };

    HazardResult DetectHazard(const SyncStageAccessInfoType &usage_info) const;
    HazardResult DetectHazard(const SyncStageAccessInfoType &usage_info, SyncOrdering ordering_rule, QueueId queue_id) const;
    HazardResult DetectHazard(const SyncStageAccessInfoType &usage_info, const OrderingBarrier &ordering, QueueId queue_id) const;
    HazardResult DetectHazard(const ResourceAccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range) const;

    HazardResult DetectAsyncHazard(const SyncStageAccessInfoType &usage_info, ResourceUsageTag start_tag) const;
    HazardResult DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                   ResourceUsageTag start_tag) const;

    HazardResult DetectBarrierHazard(const SyncStageAccessInfoType &usage_info, QueueId queue_id,
                                     VkPipelineStageFlags2KHR source_exec_scope,
                                     const SyncStageAccessFlags &source_access_scope) const;
    HazardResult DetectBarrierHazard(const SyncStageAccessInfoType &usage_info, const ResourceAccessState &scope_state,
                                     VkPipelineStageFlags2KHR source_exec_scope, const SyncStageAccessFlags &source_access_scope,
                                     QueueId event_queue, ResourceUsageTag event_tag) const;

    void Update(const SyncStageAccessInfoType &usage_info, SyncOrdering ordering_rule, ResourceUsageTag tag);
    void SetWrite(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag);
    void ClearWrite();
    void ClearRead();
    void ClearPending();
    void ClearFirstUse();
    void Resolve(const ResourceAccessState &other);
    void ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition);
    void ApplyBarriersImmediate(const std::vector<SyncBarrier> &barriers);
    template <typename ScopeOps>
    void ApplyBarrier(ScopeOps &&scope, const SyncBarrier &barrier, bool layout_transition);
    void ApplyPendingBarriers(ResourceUsageTag tag);
    void ApplySemaphore(const SemaphoreScope &signal, const SemaphoreScope wait);

    struct WaitQueueTagPredicate {
        QueueId queue;
        ResourceUsageTag tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const ResourceAccessState &access) const;  // Write access predicate
    };
    friend WaitQueueTagPredicate;

    struct WaitTagPredicate {
        ResourceUsageTag tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const ResourceAccessState &access) const;  // Write access predicate
    };
    friend WaitTagPredicate;

    struct WaitAcquirePredicate {
        ResourceUsageTag present_tag;
        ResourceUsageTag acquire_tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const ResourceAccessState &access) const;  // Write access predicate
    };
    friend WaitAcquirePredicate;

    template <typename Predicate>
    bool ApplyPredicatedWait(Predicate &predicate);

    bool FirstAccessInTagRange(const ResourceUsageRange &tag_range) const;

    void OffsetTag(ResourceUsageTag offset);
    ResourceAccessState();

    bool HasPendingState() const { return (0 != pending_layout_transition) || (last_write && last_write->HasPendingState()); }
    bool HasWriteOp() const { return last_write.has_value(); }
    SyncStageAccessIndex LastWriteOp() const { return last_write.has_value() ? last_write->Index() : SYNC_ACCESS_INDEX_NONE; }
    bool IsLastWriteOp(SyncStageAccessIndex usage_index) const { return LastWriteOp() == usage_index; }
    ResourceUsageTag LastWriteTag() const { return last_write.has_value() ? last_write->Tag() : ResourceUsageTag(0); }
    bool operator==(const ResourceAccessState &rhs) const {
        const bool write_same = (read_execution_barriers == rhs.read_execution_barriers) &&
                                (input_attachment_read == rhs.input_attachment_read) && (last_write == rhs.last_write);

        const bool read_write_same = write_same && (last_read_stages == rhs.last_read_stages) && (last_reads == rhs.last_reads);

        const bool same = read_write_same && (first_accesses_ == rhs.first_accesses_) &&
                          (first_read_stages_ == rhs.first_read_stages_) &&
                          (first_write_layout_ordering_ == rhs.first_write_layout_ordering_);

        return same;
    }
    bool operator!=(const ResourceAccessState &rhs) const { return !(*this == rhs); }
    VkPipelineStageFlags2KHR GetReadBarriers(const SyncStageAccessFlags &usage) const;
    SyncStageAccessFlags GetWriteBarriers() const {
        return last_write.has_value() ? last_write->Barriers() : SyncStageAccessFlags();
    }
    void SetQueueId(QueueId id);

    bool IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                              const SyncStageAccessFlags &src_access_scope) const;
    bool WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const;
    bool WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                        const SyncStageAccessFlags &src_access_scope) const;
    bool WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope,
                           QueueId scope_queue, ResourceUsageTag scope_tag) const;

    struct UntaggedScopeOps {
        bool WriteInScope(const SyncBarrier &barrier, const ResourceAccessState &access) const {
            return access.WriteInSourceScopeOrChain(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
        }
        bool ReadInScope(const SyncBarrier &barrier, const ReadState &read_state) const {
            return read_state.ReadInScopeOrChain(barrier.src_exec_scope.exec_scope);
        }
    };

    struct QueueScopeOps {
        bool WriteInScope(const SyncBarrier &barrier, const ResourceAccessState &access) const {
            return access.WriteInQueueSourceScopeOrChain(queue, barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
        }
        bool ReadInScope(const SyncBarrier &barrier, const ReadState &read_state) const {
            return read_state.ReadInQueueScopeOrChain(queue, barrier.src_exec_scope.exec_scope);
        }
        QueueScopeOps(QueueId scope_queue) : queue(scope_queue) {}
        QueueId queue;
    };

    struct EventScopeOps {
        bool WriteInScope(const SyncBarrier &barrier, const ResourceAccessState &access) const {
            return access.WriteInEventScope(barrier.src_exec_scope.exec_scope, barrier.src_access_scope, scope_queue, scope_tag);
        }
        bool ReadInScope(const SyncBarrier &barrier, const ReadState &read_state) const {
            return read_state.ReadInEventScope(barrier.src_exec_scope.exec_scope, scope_queue, scope_tag);
        }
        EventScopeOps(QueueId qid, ResourceUsageTag event_tag) : scope_queue(qid), scope_tag(event_tag) {}
        QueueId scope_queue;
        ResourceUsageTag scope_tag;
    };

    void Normalize();
    void GatherReferencedTags(ResourceUsageTagSet &used) const;

  private:
    static constexpr VkPipelineStageFlags2KHR kInvalidAttachmentStage = ~VkPipelineStageFlags2KHR(0);
    bool IsRAWHazard(const SyncStageAccessInfoType &usage_info) const;

    bool WriteInScope(const SyncStageAccessFlags &src_access_scope) const;
    // Apply ordering scope to write hazard detection

    bool ReadInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope) const {
        return (0 != (src_exec_scope & (last_read_stages | read_execution_barriers)));
    }

    static bool IsReadHazard(VkPipelineStageFlags2KHR stage_mask, const VkPipelineStageFlags2KHR barriers) {
        return stage_mask != (stage_mask & barriers);
    }

    bool IsReadHazard(VkPipelineStageFlags2KHR stage_mask, const ReadState &read_access) const {
        return IsReadHazard(stage_mask, read_access.barriers);
    }
    VkPipelineStageFlags2 GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering) const;

    void UpdateFirst(ResourceUsageTag tag, const SyncStageAccessInfoType &usage_info, SyncOrdering ordering_rule);
    void TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering);
    void MergePending(const ResourceAccessState &other);
    void MergeReads(const ResourceAccessState &other);

    static const OrderingBarrier &GetOrderingRules(SyncOrdering ordering_enum) {
        return kOrderingRules[static_cast<size_t>(ordering_enum)];
    }

    // TODO: Add a NONE (zero) enum to SyncStageAccessFlags for input_attachment_read and last_write

    // With reads, each must be "safe" relative to it's prior write, so we need only
    // save the most recent write operation (as anything *transitively* unsafe would arleady
    // be included
    // SyncStageAccessFlags write_barriers;              // union of applicable barrier masks since last write
    // VkPipelineStageFlags2KHR write_dependency_chain;  // intiially zero, but accumulating the dstStages of barriers if they
    // chain. ResourceUsageTag write_tag; QueueId write_queue;
    std::optional<ResourceAccessWriteState> last_write;  // only the most recent write

    VkPipelineStageFlags2KHR last_read_stages;
    VkPipelineStageFlags2KHR read_execution_barriers;
    using ReadStates = small_vector<ReadState, 3, uint32_t>;
    ReadStates last_reads;

    // TODO Input Attachment cleanup for multiple reads in a given stage
    // Tracks whether the fragment shader read is input attachment read
    bool input_attachment_read;

    // Not part of the write state, logically.  Can exist when !last_write
    // Pending execution state to support independent parallel barriers
    bool pending_layout_transition;

    FirstAccesses first_accesses_;
    VkPipelineStageFlags2KHR first_read_stages_;
    OrderingBarrier first_write_layout_ordering_;
    bool first_access_closed_;

    static OrderingBarriers kOrderingRules;
};
using ResourceAccessStateFunction = std::function<void(ResourceAccessState *)>;
using ResourceAccessRangeMap = sparse_container::range_map<ResourceAddress, ResourceAccessState>;
using ResourceRangeMergeIterator = sparse_container::parallel_iterator<ResourceAccessRangeMap, const ResourceAccessRangeMap>;

// Apply the memory barrier without updating the existing barriers.  The execution barrier
// changes the "chaining" state, but to keep barriers independent, we defer this until all barriers
// of the batch have been processed. Also, depending on whether layout transition happens, we'll either
// replace the current write barriers or add to them, so accumulate to pending as well.
template <typename ScopeOps>
void ResourceAccessState::ApplyBarrier(ScopeOps &&scope, const SyncBarrier &barrier, bool layout_transition) {
    // For independent barriers we need to track what the new barriers and dependency chain *will* be when we're done
    // applying the memory barriers
    // NOTE: We update the write barrier if the write is in the first access scope or if there is a layout
    //       transistion, under the theory of "most recent access".  If the resource acces  *isn't* safe
    //       vs. this layout transition DetectBarrierHazard should report it.  We treat the layout
    //       transistion *as* a write and in scope with the barrier (it's before visibility).
    if (layout_transition) {
        if (!last_write.has_value()) {
            last_write.emplace(UsageInfo(SYNC_ACCESS_INDEX_NONE), 0U);
        }
        last_write->UpdatePendingBarriers(barrier);
        last_write->UpdatePendingLayoutOrdering(barrier);
        pending_layout_transition = true;
    } else {
        if (scope.WriteInScope(barrier, *this)) {
            last_write->UpdatePendingBarriers(barrier);
        }

        if (!pending_layout_transition) {
            // Once we're dealing with a layout transition (which is modelled as a *write*) then the last reads/chains
            // don't need to be tracked as we're just going to clear them.
            VkPipelineStageFlags2 stages_in_scope = VK_PIPELINE_STAGE_2_NONE;

            for (auto &read_access : last_reads) {
                // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync
                // scope
                if (scope.ReadInScope(barrier, read_access)) {
                    // We'll apply the barrier in the next loop, because it's DRY'r to do it one place.
                    stages_in_scope |= read_access.stage;
                }
            }

            for (auto &read_access : last_reads) {
                if (0 != ((read_access.stage | read_access.sync_stages) & stages_in_scope)) {
                    // If this stage, or any stage known to be synchronized after it are in scope, apply the barrier to this
                    // read NOTE: Forwarding barriers to known prior stages changes the sync_stages from shallow to deep,
                    // because the
                    //       barriers used to determine sync_stages have been propagated to all known earlier stages
                    read_access.ApplyReadBarrier(barrier.dst_exec_scope.exec_scope);
                }
            }
        }
    }
}

// Return if the resulting state is "empty"
template <typename Predicate>
bool ResourceAccessState::ApplyPredicatedWait(Predicate &predicate) {
    VkPipelineStageFlags2KHR sync_reads = VK_PIPELINE_STAGE_2_NONE;

    // Use the predicate to build a mask of the read stages we are synchronizing
    // Use the sync_stages to also detect reads known to be before any synchronized reads (first pass)
    for (auto &read_access : last_reads) {
        if (predicate(read_access)) {
            // If we know this stage is before any stage we syncing, or if the predicate tells us that we are waited for..
            sync_reads |= read_access.stage;
        }
    }

    // Now that we know the reads directly in scopejust need to go over the list again to pick up the "known earlier" stages.
    // NOTE: sync_stages is "deep" catching all stages synchronized after it because we forward barriers
    uint32_t unsync_count = 0;
    for (auto &read_access : last_reads) {
        if (0 != ((read_access.stage | read_access.sync_stages) & sync_reads)) {
            // This is redundant in the "stage" case, but avoids a second branch to get an accurate count
            sync_reads |= read_access.stage;
        } else {
            ++unsync_count;
        }
    }

    if (unsync_count) {
        if (sync_reads) {
            // When have some remaining unsynchronized reads, we have to rewrite the last_reads array.
            ReadStates unsync_reads;
            unsync_reads.reserve(unsync_count);
            VkPipelineStageFlags2KHR unsync_read_stages = VK_PIPELINE_STAGE_2_NONE;
            for (auto &read_access : last_reads) {
                if (0 == (read_access.stage & sync_reads)) {
                    unsync_reads.emplace_back(read_access);
                    unsync_read_stages |= read_access.stage;
                }
            }
            last_read_stages = unsync_read_stages;
            last_reads = std::move(unsync_reads);
        }
    } else {
        // Nothing remains (or it was empty to begin with)
        ClearRead();
    }

    bool all_clear = last_reads.size() == 0;
    if (last_write.has_value()) {
        if (predicate(*this) || sync_reads) {
            // Clear any predicated write, or any the write from any any access with synchronized reads.
            // This could drop RAW detection, but only if the synchronized reads were RAW hazards, and given
            // MRR approach to reporting, this is consistent with other drops, especially since fixing the
            // RAW wit the sync_reads stages would preclude a subsequent RAW.
            ClearWrite();
        } else {
            all_clear = false;
        }
    }
    return all_clear;
}

template <typename Barrier>
SyncBarrier::SyncBarrier(const Barrier &barrier, const SyncExecScope &src, const SyncExecScope &dst)
    : src_exec_scope(src),
      src_access_scope(SyncStageAccess::AccessScope(src.valid_accesses, barrier.srcAccessMask)),
      dst_exec_scope(dst),
      dst_access_scope(SyncStageAccess::AccessScope(dst.valid_accesses, barrier.dstAccessMask)) {}

template <typename Barrier>
SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const Barrier &barrier) {
    auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
    src_exec_scope = src.exec_scope;
    src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, barrier.srcAccessMask);

    auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
    dst_exec_scope = dst.exec_scope;
    dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, barrier.dstAccessMask);
}
