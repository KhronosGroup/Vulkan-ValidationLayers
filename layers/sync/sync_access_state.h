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

#pragma once
#include "sync/sync_common.h"
#include "sync/sync_barrier.h"
#include "containers/span.h"
#include <cstring>  // memcpy

namespace syncval {

class AccessState;
struct WriteState;
struct ReadState;
struct FirstAccess;
struct SemaphoreScope;
struct AccessContextStats;

// NOTE: the attachement read flag is put *only* in the access scope and not in the exect scope, since the ordering
//       rules apply only to this specific access for this stage, and not the stage as a whole. The ordering detection
//       also reflects this special case for read hazard detection (using access instead of exec scope)
constexpr VkPipelineStageFlags2 kColorAttachmentExecScope = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
const SyncAccessFlags kColorAttachmentAccessScope =
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_BIT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
constexpr VkPipelineStageFlags2 kDepthStencilAttachmentExecScope =
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
const SyncAccessFlags kDepthStencilAttachmentAccessScope =
    SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
constexpr VkPipelineStageFlags2 kRasterAttachmentExecScope = kDepthStencilAttachmentExecScope | kColorAttachmentExecScope;
const SyncAccessFlags kRasterAttachmentAccessScope = kDepthStencilAttachmentAccessScope | kColorAttachmentAccessScope;

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

struct SyncFlag {
    enum : uint32_t {
        kLoadOp = 0x01,
        kStoreOp = 0x02,
        kPresent = 0x04,
        kMarker = 0x08,
    };
};
using SyncFlags = uint32_t;

const char *string_SyncHazardVUID(SyncHazard hazard);

struct SyncHazardInfo {
    bool is_write = false;
    bool is_prior_write = false;
    bool is_racing_hazard = false;

    bool IsWrite() const { return is_write; }
    bool IsRead() const { return !is_write; }
    bool IsPriorWrite() const { return is_prior_write; }
    bool IsPriorRead() const { return !is_prior_write; }
    bool IsRacingHazard() const { return is_racing_hazard; }
};
SyncHazardInfo GetSyncHazardInfo(SyncHazard hazard);

class HazardResult {
  public:
    struct HazardState {
        std::unique_ptr<const AccessState> access_state;
        std::unique_ptr<const FirstAccess> recorded_access;
        SyncAccessIndex access_index = std::numeric_limits<SyncAccessIndex>::max();
        SyncAccessIndex prior_access_index;
        ResourceUsageTag tag = ResourceUsageTag();
        uint32_t handle_index = vvl::kNoIndex32;
        SyncHazard hazard = NONE;
        HazardState(const AccessState *access_state, const SyncAccessInfo &usage_info, SyncHazard hazard,
                    SyncAccessIndex prior_access_index, ResourceUsageTagEx tag_ex);
    };

    static HazardResult HazardVsPriorWrite(const AccessState *access_state, const SyncAccessInfo &usage_info, SyncHazard hazard,
                                           const WriteState &prior_write);
    static HazardResult HazardVsPriorRead(const AccessState *access_state, const SyncAccessInfo &usage_info, SyncHazard hazard,
                                          const ReadState &prior_read);

    void AddRecordedAccess(const FirstAccess &first_access);

    bool IsHazard() const { return state_.has_value() && NONE != state_->hazard; }
    bool IsWAWHazard() const;
    ResourceUsageTag Tag() const {
        assert(state_);
        return state_->tag;
    }
    ResourceUsageTagEx TagEx() const {
        assert(state_);
        return ResourceUsageTagEx{state_->tag, state_->handle_index};
    }
    SyncHazard Hazard() const {
        assert(state_);
        return state_->hazard;
    }
    const std::unique_ptr<const FirstAccess> &RecordedAccess() const {
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

struct FirstAccess {
    const SyncAccessInfo *usage_info;
    ResourceUsageTag tag;
    uint32_t handle_index;
    SyncOrdering ordering_rule;
    SyncFlags flags;

    FirstAccess(const SyncAccessInfo &usage_info, ResourceUsageTagEx tag_ex, SyncOrdering ordering_rule, SyncFlags flags)
        : usage_info(&usage_info), tag(tag_ex.tag), handle_index(tag_ex.handle_index), ordering_rule(ordering_rule), flags(flags) {}
    bool operator==(const FirstAccess &rhs) const {
        return (tag == rhs.tag) && (usage_info == rhs.usage_info) && (ordering_rule == rhs.ordering_rule) && flags == rhs.flags;
    }
    ResourceUsageTagEx TagEx() const { return {tag, handle_index}; }
};

using QueueId = uint32_t;

struct OrderingBarrier {
    VkPipelineStageFlags2 exec_scope = VK_PIPELINE_STAGE_2_NONE;
    SyncAccessFlags access_scope;

    bool operator==(const OrderingBarrier &rhs) const;
    size_t Hash() const;
};

const OrderingBarrier &GetOrderingRules(SyncOrdering ordering_enum);

using ResourceUsageTagSet = CachedInsertSet<ResourceUsageTag, 4>;

// Mutliple read operations can be simlutaneously (and independently) synchronized,
// given the only the second execution scope creates a dependency chain, we have to track each,
// but only up to one per pipeline stage (as another read from the *same* stage become more recent,
// and applicable one for hazard detection
struct ReadState {
    VkPipelineStageFlagBits2 stage;     // The stage of this read
    SyncAccessIndex access_index;       // TODO: Revisit whether this needs to support multiple reads per stage
    VkPipelineStageFlags2 barriers;     // all applicable barriered stages
    VkPipelineStageFlags2 sync_stages;  // reads known to have happened after this
    ResourceUsageTag tag;
    uint32_t handle_index;
    QueueId queue;

    void Set(VkPipelineStageFlagBits2 stage, SyncAccessIndex access_index, ResourceUsageTagEx tag_ex);

    ResourceUsageTagEx TagEx() const { return {tag, handle_index}; }
    bool operator==(const ReadState &rhs) const {
        return (stage == rhs.stage) && (access_index == rhs.access_index) && (barriers == rhs.barriers) &&
               (sync_stages == rhs.sync_stages) && (tag == rhs.tag) && (queue == rhs.queue);
    }

    bool IsReadBarrierHazard(QueueId barrier_queue, VkPipelineStageFlags2 src_exec_scope,
                             const SyncAccessFlags &src_access_scope) const {
        // Current implementation relies on TOP_OF_PIPE constant due to the fact that it's non-zero value
        // and AND-ing with it can create execution dependency when it's necessary. When NONE constant is
        // used, which equals to zero, then AND-ing with it always results in 0 which means "no barrier",
        // so it's not possible to use NONE internally in equivalent way to TOP_OF_PIPE.
        // Replace NONE with TOP_OF_PIPE in the scenarios where they are equivalent.
        //
        // If we update implementation to get rid of deprecated TOP_OF_PIPE/BOTTOM_OF_PIPE then we must
        // invert the condition below and exchange TOP_OF_PIPE and NONE roles, so deprecated stages would
        // not propagate into implementation internals.
        if (src_exec_scope == VK_PIPELINE_STAGE_2_NONE && src_access_scope.none()) {
            src_exec_scope = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        }

        // If the read stage is not in the src sync scope
        // *AND* not execution chained with an existing sync barrier (that's the or)
        // then the barrier access is unsafe (R/W after R)
        VkPipelineStageFlags2 queue_ordered_stage = (queue == barrier_queue) ? stage : VK_PIPELINE_STAGE_2_NONE;
        return (src_exec_scope & (queue_ordered_stage | barriers)) == 0;
    }
    bool ReadOrDependencyChainInSourceScope(QueueId queue, VkPipelineStageFlags2 src_exec_scope) const;
    bool InBarrierSourceScope(const BarrierScope &barrier_scope) const;
};

static_assert(std::is_trivially_copyable_v<ReadState>);

struct WriteState {
    SyncAccessIndex access_index;
    SyncFlags flags;

    // Union of applicable barrier masks since last write
    SyncAccessFlags barriers;

    // Accumulates the dstStages of barriers if they chain
    VkPipelineStageFlags2 dependency_chain;

    ResourceUsageTag tag;
    uint32_t handle_index;
    QueueId queue;

    void Set(SyncAccessIndex access_index, ResourceUsageTagEx tag_ex, SyncFlags flags);
    void SetQueueId(QueueId id);
    void MergeBarriers(const WriteState &other);

    bool operator==(const WriteState &rhs) const;

    bool DependencyChainInSourceScope(VkPipelineStageFlags2 src_exec_scope) const;
    bool WriteInSourceScope(const SyncAccessFlags &src_access_scope) const;
    bool WriteOrDependencyChainInSourceScope(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                                             const SyncAccessFlags &src_access_scope) const;
    bool InBarrierSourceScope(const BarrierScope &barrier_scope) const;

    bool IsWriteHazard(const SyncAccessInfo &usage_info) const;
    bool IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                              const SyncAccessFlags &src_access_scope) const;

    bool IsOrdered(const OrderingBarrier &ordering, QueueId queue_id) const;
    bool IsLoadOp() const { return flags & SyncFlag::kLoadOp; }
    bool IsStoreOp() const { return flags & SyncFlag::kStoreOp; }
    bool IsPresent() const { return flags & SyncFlag::kPresent; }

    ResourceUsageTagEx TagEx() const { return {tag, handle_index}; }
};

static_assert(std::is_trivially_copyable_v<WriteState>);

enum class PendingBarrierType : uint8_t { ReadAccessBarrier, WriteAccessBarrier, LayoutTransition };

struct PendingBarrierInfo {
    PendingBarrierType type;
    uint32_t index;  // indexes array determined by 'type'
    AccessState *access_state;
};

struct PendingReadBarrier {
    VkPipelineStageFlags2 barriers;
    uint32_t last_reads_index;  // indexes AccessState::last_reads
};

struct PendingWriteBarrier {
    SyncAccessFlags barriers;
    VkPipelineStageFlags2 dependency_chain;
};

struct PendingLayoutTransition {
    OrderingBarrier ordering;
    uint32_t handle_index;
};

// PendingBarriers stores the results of independent barrier applications, so the applied barriers
// do not interact (for example, they do not create execution dependencies between themselves).
// Apply() stores the final result in the access state. Independent barrier application is required
// by various sync APIs, such as vkCmdPipelineBarrier.
//
// A naive approach to applying a set of independent barriers is to apply them directly to the access
// state one at a time. This creates dependencies. PendingBarriers solves this by delaying updates to
// the access state until all barriers have been processed.
struct PendingBarriers {
    std::vector<PendingBarrierInfo> infos;
    std::vector<PendingReadBarrier> read_barriers;
    std::vector<PendingWriteBarrier> write_barriers;
    std::vector<PendingLayoutTransition> layout_transitions;

    // Store result of barrier application as PendingBarriers state
    void AddReadBarrier(AccessState *access_state, uint32_t last_reads_index, const SyncBarrier &barrier);
    void AddWriteBarrier(AccessState *access_state, const SyncBarrier &barrier);
    void AddLayoutTransition(AccessState *access_state, const OrderingBarrier &layout_transition_ordering_barrier,
                             uint32_t layout_transition_handle_index);

    // Update accesss state with collected barriers
    void Apply(const ResourceUsageTag exec_tag);
};

class AccessState {
  public:
    AccessState() = default;
    ~AccessState();
    AccessState(const AccessState &other);
    AccessState &operator=(const AccessState &other);
    AccessState(AccessState &&other);
    AccessState &operator=(AccessState &&other);

    HazardResult DetectHazard(const SyncAccessInfo &usage_info) const;
    HazardResult DetectMarkerHazard() const;

    HazardResult DetectHazard(const SyncAccessInfo &usage_info, const OrderingBarrier &ordering, SyncFlags flags,
                              QueueId queue_id) const;
    HazardResult DetectHazard(const AccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range) const;

    HazardResult DetectAsyncHazard(const SyncAccessInfo &usage_info, ResourceUsageTag start_tag, QueueId queue_id) const;
    HazardResult DetectAsyncHazard(const AccessState &recorded_use, const ResourceUsageRange &tag_range,
                                   ResourceUsageTag start_tag, QueueId queue_id) const;

    HazardResult DetectBarrierHazard(const SyncAccessInfo &usage_info, QueueId queue_id, VkPipelineStageFlags2 source_exec_scope,
                                     const SyncAccessFlags &source_access_scope) const;
    HazardResult DetectBarrierHazard(const SyncAccessInfo &usage_info, const AccessState &scope_state,
                                     VkPipelineStageFlags2 source_exec_scope, const SyncAccessFlags &source_access_scope,
                                     QueueId event_queue, ResourceUsageTag event_tag) const;

    void Update(const SyncAccessInfo &usage_info, SyncOrdering ordering_rule, ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void SetWrite(SyncAccessIndex access_index, ResourceUsageTagEx tag_ex, SyncFlags flags = 0);
    void ClearWrite();
    void ClearRead();
    void ClearFirstUse();
    void Resolve(const AccessState &other);

    // Apply a single barrier to the access state
    void ApplyBarrier(const BarrierScope &barrier_scope, const SyncBarrier &barrier, bool layout_transition = false,
                      uint32_t layout_transition_handle_index = vvl::kNoIndex32,
                      ResourceUsageTag layout_transition_tag = kInvalidTag);

    // Store the result of barrier application in PendingBarriers.
    // Does not update the access state (as ApplyBarrier does).
    // Used for applying multiple barriers independently.
    void CollectPendingBarriers(const BarrierScope &barrier_scope, const SyncBarrier &barrier, bool layout_transition,
                                uint32_t layout_transition_handle_index, PendingBarriers &pending_barriers);

    // Apply pending barriers to the access state.
    // Called after all barrier application results are collected in PendingBarriers.
    void ApplyPendingReadBarrier(const PendingReadBarrier &read_barrier, ResourceUsageTag tag);
    void ApplyPendingWriteBarrier(const PendingWriteBarrier &write_barrier);
    void ApplyPendingLayoutTransition(const PendingLayoutTransition &layout_transition, ResourceUsageTag layout_transition_tag);

    void ApplySemaphore(const SemaphoreScope &signal, const SemaphoreScope &wait);

    struct WaitQueueTagPredicate {
        QueueId queue;
        ResourceUsageTag tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const AccessState &access) const;  // Write access predicate
    };
    friend WaitQueueTagPredicate;

    struct WaitTagPredicate {
        ResourceUsageTag tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const AccessState &access) const;  // Write access predicate
    };
    friend WaitTagPredicate;

    struct WaitAcquirePredicate {
        ResourceUsageTag present_tag;
        ResourceUsageTag acquire_tag;
        bool operator()(const ReadState &read_access) const;       // Read access predicate
        bool operator()(const AccessState &access) const;  // Write access predicate
    };
    friend WaitAcquirePredicate;

    // Clear read/write accesses that satisfy the predicate
    // (predicate says which accesses should be considered synchronized).
    // Return true if all accesses were cleared and access state is empty
    template <typename Predicate>
    bool ClearPredicatedAccesses(Predicate &predicate);

    ResourceUsageRange GetFirstAccessRange() const;
    bool FirstAccessInTagRange(const ResourceUsageRange &tag_range) const;

    void OffsetTag(ResourceUsageTag offset);

    bool HasWriteOp() const { return last_write.has_value(); }
    bool IsLastWriteOp(SyncAccessIndex access_index) const {
        return last_write.has_value() && last_write->access_index == access_index;
    }
    ResourceUsageTag LastWriteTag() const { return last_write.has_value() ? last_write->tag : ResourceUsageTag(0); }
    const WriteState &LastWrite() const;
    bool operator==(const AccessState &rhs) const {
        const bool write_same = (read_execution_barriers == rhs.read_execution_barriers) &&
                                (input_attachment_read == rhs.input_attachment_read) && (last_write == rhs.last_write);

        bool read_same = (last_read_stages == rhs.last_read_stages) && (last_read_count == rhs.last_read_count);
        if (read_same) {
            for (uint32_t i = 0; i < last_read_count; i++) {
                read_same = read_same && (last_reads[i] == rhs.last_reads[i]);
            }
        }

        const bool read_write_same = write_same && read_same;

        const bool same = read_write_same && (first_accesses_ == rhs.first_accesses_) &&
                          (first_read_stages_ == rhs.first_read_stages_) &&
                          (first_write_layout_ordering_index == rhs.first_write_layout_ordering_index);

        return same;
    }
    bool operator!=(const AccessState &rhs) const { return !(*this == rhs); }
    VkPipelineStageFlags2 GetReadBarriers(SyncAccessIndex access_index) const;
    SyncAccessFlags GetWriteBarriers() const { return last_write.has_value() ? last_write->barriers : SyncAccessFlags(); }
    void SetQueueId(QueueId id);

    bool IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                              const SyncAccessFlags &src_access_scope) const;
    void Normalize();
    void GatherReferencedTags(ResourceUsageTagSet &used) const;

    void UpdateStats(AccessContextStats &stats) const;

  private:
    void CopySimpleMembers(const AccessState &other);
    bool IsRAWHazard(const SyncAccessInfo &usage_info) const;

    bool IsReadHazard(VkPipelineStageFlags2 stage_mask, const ReadState &read_access) const {
        return stage_mask != (stage_mask & read_access.barriers);
    }
    VkPipelineStageFlags2 GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering, SyncFlags flags) const;

    void UpdateFirst(ResourceUsageTagEx tag_ex, const SyncAccessInfo &usage_info, SyncOrdering ordering_rule, SyncFlags flags = 0);
    void TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering);

    bool HasReads() const { return last_read_count != 0; }
    vvl::span<ReadState> GetReads() { return vvl::make_span(last_reads, last_read_count); }
    vvl::span<ReadState> GetReads() const { return vvl::make_span(last_reads, last_read_count); }
    void AddRead(const ReadState &read);
    void MergeReads(const AccessState &other);
    void ClearReadStates();

    // The most recent write.
    // NOTE: For reads, each must be "safe" relative to its prior write, so we need
    // only to save the most recent write operation, since anything *transitively*
    // unsafe would already be included.
    std::optional<WriteState> last_write;

    uint32_t last_read_count = 0;

    // Points to the last_read_count read states
    ReadState *last_reads = nullptr;

    // The common case is a single read. In that case, last_reads points to this object
    // and last_read_count is 1.
    ReadState single_last_read;

    VkPipelineStageFlags2 last_read_stages = VK_PIPELINE_STAGE_2_NONE;
    VkPipelineStageFlags2 read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;

    // NOTE: Reserve capacity for 2 first accesses, more than that is not very common
    using FirstAccesses = small_vector<FirstAccess, 2>;
    FirstAccesses first_accesses_;
    VkPipelineStageFlags2 first_read_stages_ = VK_PIPELINE_STAGE_2_NONE;
    uint32_t first_write_layout_ordering_index = vvl::kNoIndex32;
    bool first_access_closed_ = false;

    // TODO Input Attachment cleanup for multiple reads in a given stage
    // Tracks whether the fragment shader read is input attachment read
    // TODO: Add a NONE (zero) enum to SyncStageAccessFlags for
    // input_attachment_read and last_write
    bool input_attachment_read = false;
};
using AccessStateFunction = std::function<void(AccessState *)>;

template <typename Predicate>
bool AccessState::ClearPredicatedAccesses(Predicate &predicate) {
    VkPipelineStageFlags2 sync_reads = VK_PIPELINE_STAGE_2_NONE;

    // Use the predicate to build a mask of the read stages we are synchronizing
    // Use the sync_stages to also detect reads known to be before any synchronized reads (first pass)
    for (const auto &read_access : GetReads()) {
        if (predicate(read_access)) {
            // If we know this stage is before any stage we syncing, or if the predicate tells us that we are waited for..
            sync_reads |= read_access.stage;
        }
    }

    // Now that we know the reads directly in scopejust need to go over the list again to pick up the "known earlier" stages.
    // NOTE: sync_stages is "deep" catching all stages synchronized after it because we forward barriers
    uint32_t unsync_count = 0;
    for (const auto &read_access : GetReads()) {
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
            small_vector<ReadState, 4> unsync_reads;
            unsync_reads.reserve(unsync_count);
            VkPipelineStageFlags2 unsync_read_stages = VK_PIPELINE_STAGE_2_NONE;
            for (auto &read_access : GetReads()) {
                if ((read_access.stage & sync_reads) == 0) {
                    unsync_reads.emplace_back(read_access);
                    unsync_read_stages |= read_access.stage;
                }
            }
            last_read_stages = unsync_read_stages;
            ClearReadStates();
            for (const ReadState &read : unsync_reads) {
                AddRead(read);
            }
        }
    } else {
        // Nothing remains (or it was empty to begin with)
        ClearRead();
    }

    bool all_clear = !HasReads();
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

// A helper function to apply multiple barriers.
// NOTE: That's for use cases when BarrierScope does not use queue id or tag (record time, not-event barriers).
// This can be extended if necessary to provide BarrierScope for each barrier.
void ApplyBarriers(AccessState &access_state, const std::vector<SyncBarrier> &barriers, bool layout_transition = false,
                   ResourceUsageTag layout_transition_tag = kInvalidTag);

// Global registry of layout transition ordering barriers
ThreadSafeLookupTable<OrderingBarrier> &GetLayoutOrderingBarrierLookup();

}  // namespace syncval

namespace std {
template <>
struct hash<syncval::OrderingBarrier> {
    size_t operator()(const syncval::OrderingBarrier &ordering_barrier) const { return ordering_barrier.Hash(); }
};
}  // namespace std
