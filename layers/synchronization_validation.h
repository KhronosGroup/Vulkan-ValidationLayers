/*
 * Copyright (c) 2019-2022 Valve Corporation
 * Copyright (c) 2019-2022 LunarG, Inc.
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
 * Author: Locke Lin <locke@lunarg.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */

#pragma once

#include <limits>
#include <memory>
#include <vulkan/vulkan.h>

#include "synchronization_validation_types.h"
#include "state_tracker.h"
#include "cmd_buffer_state.h"
#include "render_pass_state.h"

class AccessContext;
class CommandBufferAccessContext;
class CommandExecutionContext;
class ResourceAccessState;
struct ResourceFirstAccess;
class SyncValidator;

using ImageRangeEncoder = subresource_adapter::ImageRangeEncoder;
using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

enum SyncHazard {
    NONE = 0,
    READ_AFTER_WRITE,
    WRITE_AFTER_READ,
    WRITE_AFTER_WRITE,
    READ_RACING_WRITE,
    WRITE_RACING_WRITE,
    WRITE_RACING_READ,
};

enum class SyncOrdering : uint8_t {
    kNonAttachment = 0,
    kColorAttachment = 1,
    kDepthStencilAttachment = 2,
    kRaster = 3,
    kNumOrderings = 4,
};

// Useful Utilites for manipulating StageAccess parameters, suitable as base class to save typing
struct SyncStageAccess {
    static inline SyncStageAccessFlags FlagBit(SyncStageAccessIndex stage_access) {
        return syncStageAccessInfoByStageAccessIndex[stage_access].stage_access_bit;
    }
    static inline SyncStageAccessFlags Flags(SyncStageAccessIndex stage_access) {
        return static_cast<SyncStageAccessFlags>(FlagBit(stage_access));
    }

    static bool IsRead(const SyncStageAccessFlags &stage_access_bit) { return (stage_access_bit & syncStageAccessReadMask).any(); }
    static bool IsRead(SyncStageAccessIndex stage_access_index) { return IsRead(FlagBit(stage_access_index)); }

    static bool IsWrite(const SyncStageAccessFlags &stage_access_bit) {
        return (stage_access_bit & syncStageAccessWriteMask).any();
    }
    static bool HasWrite(const SyncStageAccessFlags &stage_access_mask) {
        return (stage_access_mask & syncStageAccessWriteMask).any();
    }
    static bool IsWrite(SyncStageAccessIndex stage_access_index) { return IsWrite(FlagBit(stage_access_index)); }
    static VkPipelineStageFlags2KHR PipelineStageBit(SyncStageAccessIndex stage_access_index) {
        return syncStageAccessInfoByStageAccessIndex[stage_access_index].stage_mask;
    }
    static SyncStageAccessFlags AccessScopeByStage(VkPipelineStageFlags2KHR stages);
    static SyncStageAccessFlags AccessScopeByAccess(VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(VkPipelineStageFlags2KHR stages, VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(const SyncStageAccessFlags &stage_scope, VkAccessFlags2KHR accesses) {
        return stage_scope & AccessScopeByAccess(accesses);
    }
};

struct ResourceUsageRecord {
    enum class SubcommandType { kNone, kSubpassTransition, kLoadOp, kStoreOp, kResolveOp, kIndex };

    using TagIndex = size_t;
    using Count = uint32_t;
    constexpr static TagIndex kMaxIndex = std::numeric_limits<TagIndex>::max();
    constexpr static Count kMaxCount = std::numeric_limits<Count>::max();
    CMD_TYPE command = CMD_NONE;
    Count seq_num = 0U;
    SubcommandType sub_command_type = SubcommandType::kNone;
    Count sub_command = 0U;

    // This is somewhat repetitive, but it prevents the need for Exec/Submit time touchup, after which usage records can be
    // from different command buffers and resets.
    const CMD_BUFFER_STATE *cb_state = nullptr;  // plain pointer as a shared pointer is held by the context storing this record
    Count reset_count;

    ResourceUsageRecord() = default;
    ResourceUsageRecord(CMD_TYPE command_, Count seq_num_, SubcommandType sub_type_, Count sub_command_,
                        const CMD_BUFFER_STATE *cb_state_, Count reset_count_)
        : command(command_),
          seq_num(seq_num_),
          sub_command_type(sub_type_),
          sub_command(sub_command_),
          cb_state(cb_state_),
          reset_count(reset_count_) {}
};

// The resource tag index is relative to the command buffer or queue in which it's found
using ResourceUsageTag = ResourceUsageRecord::TagIndex;
using ResourceUsageRange = sparse_container::range<ResourceUsageTag>;

struct HazardResult {
    std::unique_ptr<const ResourceAccessState> access_state;
    std::unique_ptr<const ResourceFirstAccess> recorded_access;
    SyncStageAccessIndex usage_index = std::numeric_limits<SyncStageAccessIndex>::max();
    SyncHazard hazard = NONE;
    SyncStageAccessFlags prior_access = 0U;  // TODO -- change to a NONE enum in ...Bits
    ResourceUsageTag tag = ResourceUsageTag();
    void Set(const ResourceAccessState *access_state_, SyncStageAccessIndex usage_index_, SyncHazard hazard_,
             const SyncStageAccessFlags &prior_, ResourceUsageTag tag_);
    void AddRecordedAccess(const ResourceFirstAccess &first_access);
};

struct SyncExecScope {
    VkPipelineStageFlags2KHR mask_param;  // the xxxStageMask parameter passed by the caller
    VkPipelineStageFlags2KHR
        expanded_mask;                    // all stage bits covered by any 'catch all bits' in the parameter (eg. ALL_GRAPHICS_BIT).
    VkPipelineStageFlags2KHR exec_scope;  // all earlier or later stages that would be affected by a barrier using this scope.
    SyncStageAccessFlags valid_accesses;  // all valid accesses that can be used with this scope.

    SyncExecScope() : mask_param(0), expanded_mask(0), exec_scope(0), valid_accesses(0) {}

    static SyncExecScope MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR src_stage_mask,
                                 const VkPipelineStageFlags2KHR disabled_feature_mask = 0);
    static SyncExecScope MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR src_stage_mask);
};

struct SyncBarrier {
    SyncExecScope src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    SyncExecScope dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
    SyncBarrier() = default;
    SyncBarrier(const SyncBarrier &other) = default;
    SyncBarrier &operator=(const SyncBarrier &) = default;

    SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst);
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
};

enum class AccessAddressType : uint32_t { kLinear = 0, kIdealized = 1, kMaxType = 1, kTypeCount = kMaxType + 1 };

struct SyncEventState {
    enum IgnoreReason { NotIgnored = 0, ResetWaitRace, Reset2WaitRace, SetRace, MissingStageBits, SetVsWait2 };
    using EventPointer = std::shared_ptr<const EVENT_STATE>;
    using ScopeMap = sparse_container::range_map<VkDeviceSize, bool>;
    EventPointer event;
    CMD_TYPE last_command;  // Only Event commands are valid here.
    ResourceUsageTag last_command_tag;  // Needed to filter replay validation
    CMD_TYPE unsynchronized_set;
    VkPipelineStageFlags2KHR barriers;
    SyncExecScope scope;
    ResourceUsageTag first_scope_tag;
    bool first_scope_set;
    bool destroyed;
    std::array<ScopeMap, static_cast<size_t>(AccessAddressType::kTypeCount)> first_scope;
    template <typename EventPointerType>
    SyncEventState(EventPointerType &&event_state)
        : event(std::forward<EventPointerType>(event_state)),
          last_command(CMD_NONE),
          last_command_tag(0),
          unsynchronized_set(CMD_NONE),
          barriers(0U),
          scope(),
          first_scope_tag(),
          first_scope_set(false),
          destroyed((event_state.get() == nullptr) || event_state->Destroyed()) {}
    SyncEventState() : SyncEventState(EventPointer()) {}
    void ResetFirstScope();
    const ScopeMap &FirstScope(AccessAddressType address_type) const { return first_scope[static_cast<size_t>(address_type)]; }
    IgnoreReason IsIgnoredByWait(CMD_TYPE cmd, VkPipelineStageFlags2KHR srcStageMask) const;
    bool HasBarrier(VkPipelineStageFlags2KHR stageMask, VkPipelineStageFlags2KHR exec_scope) const;
};

class SyncEventsContext {
  public:
    using Map = layer_data::unordered_map<const EVENT_STATE *, std::shared_ptr<SyncEventState>>;
    using iterator = Map::iterator;
    using const_iterator = Map::const_iterator;

    SyncEventState *GetFromShared(const SyncEventState::EventPointer &event_state) {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            if (!event_state.get()) return nullptr;

            const auto *event_plain_ptr = event_state.get();
            auto sync_state = std::shared_ptr<SyncEventState>(new SyncEventState(event_state));
            auto insert_pair = map_.emplace(event_plain_ptr, sync_state);
            return insert_pair.first->second.get();
        }
        return find_it->second.get();
    }

    const SyncEventState *Get(const EVENT_STATE *event_state) const {
        const auto find_it = map_.find(event_state);
        if (find_it == map_.end()) {
            return nullptr;
        }
        return find_it->second.get();
    }
    const SyncEventState *Get(const SyncEventState::EventPointer &event_state) const { return Get(event_state.get()); }

    void ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst);

    // stl style naming for range-for support
    inline iterator begin() { return map_.begin(); }
    inline const_iterator begin() const { return map_.begin(); }
    inline iterator end() { return map_.end(); }
    inline const_iterator end() const { return map_.end(); }

    void Destroy(const EVENT_STATE *event_state) {
        auto sync_it = map_.find(event_state);
        if (sync_it != map_.end()) {
            sync_it->second->destroyed = true;
            map_.erase(sync_it);
        }
    }
    void Clear() { map_.clear(); }

  private:
    Map map_;
};

struct ResourceFirstAccess {
    ResourceUsageTag tag;
    SyncStageAccessIndex usage_index;
    SyncOrdering ordering_rule;
    ResourceFirstAccess(ResourceUsageTag tag_, SyncStageAccessIndex usage_index_, SyncOrdering ordering_rule_)
        : tag(tag_), usage_index(usage_index_), ordering_rule(ordering_rule_){};
    ResourceFirstAccess(const ResourceFirstAccess &other) = default;
    ResourceFirstAccess(ResourceFirstAccess &&other) = default;
    ResourceFirstAccess &operator=(const ResourceFirstAccess &rhs) = default;
    ResourceFirstAccess &operator=(ResourceFirstAccess &&rhs) = default;
    bool operator==(const ResourceFirstAccess &rhs) const {
        return (tag == rhs.tag) && (usage_index == rhs.usage_index) && (ordering_rule == rhs.ordering_rule);
    }
};

using QueueId = uint32_t;
class ResourceAccessState : public SyncStageAccess {
  protected:
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
    };
    using OrderingBarriers = std::array<OrderingBarrier, static_cast<size_t>(SyncOrdering::kNumOrderings)>;
    using FirstAccesses = small_vector<ResourceFirstAccess, 3>;

    // Mutliple read operations can be simlutaneously (and independently) synchronized,
    // given the only the second execution scope creates a dependency chain, we have to track each,
    // but only up to one per pipeline stage (as another read from the *same* stage become more recent,
    // and applicable one for hazard detection
    struct ReadState {
        VkPipelineStageFlags2KHR stage;  // The stage of this read
        SyncStageAccessFlags access;    // TODO: Change to FlagBits when we have a None bit enum
                                        // TODO: Revisit whether this needs to support multiple reads per stage
        VkPipelineStageFlags2KHR barriers;  // all applicable barriered stages
        VkPipelineStageFlags2KHR sync_stages;  // reads known to have happened after this
        ResourceUsageTag tag;
        QueueId queue;
        VkPipelineStageFlags2KHR pending_dep_chain;  // Should be zero except during barrier application
                                                     // Excluded from comparison
        ReadState() = default;
        ReadState(VkPipelineStageFlags2KHR stage_, SyncStageAccessFlags access_, VkPipelineStageFlags2KHR barriers_,
                  ResourceUsageTag tag_);
        bool operator==(const ReadState &rhs) const {
            bool same = (stage == rhs.stage) && (access == rhs.access) && (barriers == rhs.barriers) && (tag == rhs.tag);
            return same;
        }
        bool IsReadBarrierHazard(VkPipelineStageFlags2KHR src_exec_scope) const {
            // If the read stage is not in the src sync scope
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            return (src_exec_scope & (stage | barriers)) == 0;
        }

        bool operator!=(const ReadState &rhs) const { return !(*this == rhs); }
        void Set(VkPipelineStageFlags2KHR stage_, const SyncStageAccessFlags &access_, VkPipelineStageFlags2KHR barriers_,
                 ResourceUsageTag tag_);
        bool ReadInScopeOrChain(VkPipelineStageFlags2 exec_scope) const { return (exec_scope & (stage | barriers)) != 0; }
        bool ReadInEventScope(VkPipelineStageFlags2 exec_scope, ResourceUsageTag scope_tag) const {
            // If this read is the same one we included in the set event and in scope, then apply the execution barrier...
            // NOTE: That's not really correct... this read stage might *not* have been included in the setevent, and the barriers
            // representing the chain might have changed since then (that would be an odd usage), so as a first approximation
            // we'll assume the barriers *haven't* been changed since (if the tag hasn't), and while this could be a false
            // positive in the case of Set; SomeBarrier; Wait; we'll live with it until we can add more state to the first scope
            // capture (the specific write and read stages that *were* in scope at the moment of SetEvents.
            return (tag < scope_tag) && ReadInScopeOrChain(exec_scope);
        }
    };

  public:
    HazardResult DetectHazard(SyncStageAccessIndex usage_index) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, SyncOrdering ordering_rule) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, const OrderingBarrier &ordering) const;
    HazardResult DetectHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range) const;

    HazardResult DetectAsyncHazard(SyncStageAccessIndex usage_index, ResourceUsageTag start_tag) const;
    HazardResult DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                   ResourceUsageTag start_tag) const;

    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR source_exec_scope,
                                     const SyncStageAccessFlags &source_access_scope) const;
    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR source_exec_scope,
                                     const SyncStageAccessFlags &source_access_scope, ResourceUsageTag event_tag) const;

    void Update(SyncStageAccessIndex usage_index, SyncOrdering ordering_rule, ResourceUsageTag tag);
    void SetWrite(const SyncStageAccessFlags &usage_bit, ResourceUsageTag tag);
    void ClearWrite();
    void ClearRead();
    void Resolve(const ResourceAccessState &other);
    void ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition);
    void ApplyBarriersImmediate(const std::vector<SyncBarrier> &barriers);
    template <typename ScopeOps>
    void ApplyBarrier(ScopeOps &&scope, const SyncBarrier &barrier, bool layout_transition);
    void ApplyBarrier(const SyncBarrier &barrier, bool layout_transition);
    void ApplyPendingBarriers(ResourceUsageTag tag);

    struct QueueTagPredicate {
        QueueId queue;
        ResourceUsageTag tag;
        bool operator()(QueueId usage_queue, ResourceUsageTag usage_tag);
    };

    struct QueuePredicate {
        QueueId queue;
        QueuePredicate(QueueId queue_) : queue(queue_) {}
        bool operator()(QueueId usage_queue, ResourceUsageTag usage_tag);
    };
    struct TagPredicate {
        ResourceUsageTag tag;
        bool operator()(QueueId usage_queue, ResourceUsageTag usage_tag);
    };

    template <typename Pred>
    bool ApplyQueueTagWait(Pred &&);
    bool FirstAccessInTagRange(const ResourceUsageRange &tag_range) const;

    void OffsetTag(ResourceUsageTag offset);
    ResourceAccessState();

    bool HasPendingState() const {
        return (0 != pending_layout_transition) || pending_write_barriers.any() || (0 != pending_write_dep_chain);
    }
    bool HasWriteOp() const { return last_write != 0; }
    bool operator==(const ResourceAccessState &rhs) const {
        bool same = (write_barriers == rhs.write_barriers) && (write_dependency_chain == rhs.write_dependency_chain) &&
                    (last_reads == rhs.last_reads) && (last_read_stages == rhs.last_read_stages) && (write_tag == rhs.write_tag) &&
                    (input_attachment_read == rhs.input_attachment_read) &&
                    (read_execution_barriers == rhs.read_execution_barriers) && (first_accesses_ == rhs.first_accesses_);
        return same;
    }
    bool operator!=(const ResourceAccessState &rhs) const { return !(*this == rhs); }
    VkPipelineStageFlags2KHR GetReadBarriers(const SyncStageAccessFlags &usage) const;
    SyncStageAccessFlags GetWriteBarriers() const { return write_barriers; }
    bool InSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const {
        return ReadInSourceScopeOrChain(src_exec_scope) || WriteInSourceScopeOrChain(src_exec_scope, src_access_scope);
    }
    void SetQueueId(QueueId id);

    bool WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const;
    bool WriteInEventScope(const SyncStageAccessFlags &src_access_scope, ResourceUsageTag scope_tag) const;

    struct UntaggedScopeOps {
        bool WriteInScope(const SyncBarrier &barrier, const ResourceAccessState &access) const {
            return access.WriteInSourceScopeOrChain(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
        }
        bool ReadInScope(const SyncBarrier &barrier, const ReadState &read_state) const {
            return read_state.ReadInScopeOrChain(barrier.src_exec_scope.exec_scope);
        }
    };
    struct EventScopeOps {
        bool WriteInScope(const SyncBarrier &barrier, const ResourceAccessState &access) const {
            return access.WriteInEventScope(barrier.src_access_scope, scope_tag);
        }
        bool ReadInScope(const SyncBarrier &barrier, const ReadState &read_state) const {
            return read_state.ReadInEventScope(barrier.src_exec_scope.exec_scope, scope_tag);
        }
        EventScopeOps(ResourceUsageTag event_tag) : scope_tag(event_tag) {}
        ResourceUsageTag scope_tag;
    };

  private:
    static constexpr VkPipelineStageFlags2KHR kInvalidAttachmentStage = ~VkPipelineStageFlags2KHR(0);
    bool IsWriteHazard(SyncStageAccessFlags usage) const { return (usage & ~write_barriers).any(); }
    bool IsRAWHazard(VkPipelineStageFlags2KHR usage_stage, const SyncStageAccessFlags &usage) const;
    bool IsWriteBarrierHazard(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope) const {
        // If the previous write is *not* a layout transition
        // *AND* is *not* in the 1st access scope
        // *AND* the current barrier is not in the dependency chain
        // *AND* the there is no prior memory barrier for the previous write in the dependency chain
        // then the barrier access is unsafe (R/W after W)
        return (last_write != SYNC_IMAGE_LAYOUT_TRANSITION_BIT) && (last_write & src_access_scope).none() &&
               (((src_exec_scope & write_dependency_chain) == 0) || (write_barriers & src_access_scope).none());
    }
    bool ReadInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope) const {
        return (0 != (src_exec_scope & (last_read_stages | read_execution_barriers)));
    }

    static bool IsReadHazard(VkPipelineStageFlags2KHR stage_mask, const VkPipelineStageFlags2KHR barriers) {
        return stage_mask != (stage_mask & barriers);
    }

    bool IsReadHazard(VkPipelineStageFlags2KHR stage_mask, const ReadState &read_access) const {
        return IsReadHazard(stage_mask, read_access.barriers);
    }
    VkPipelineStageFlags2KHR GetOrderedStages(const OrderingBarrier &ordering) const;

    void UpdateFirst(ResourceUsageTag tag, SyncStageAccessIndex usage_index, SyncOrdering ordering_rule);
    void TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering);

    static const OrderingBarrier &GetOrderingRules(SyncOrdering ordering_enum) {
        return kOrderingRules[static_cast<size_t>(ordering_enum)];
    }

    // TODO: Add a NONE (zero) enum to SyncStageAccessFlags for input_attachment_read and last_write

    // With reads, each must be "safe" relative to it's prior write, so we need only
    // save the most recent write operation (as anything *transitively* unsafe would arleady
    // be included
    SyncStageAccessFlags write_barriers;          // union of applicable barrier masks since last write
    VkPipelineStageFlags2KHR write_dependency_chain;  // intiially zero, but accumulating the dstStages of barriers if they chain.
    ResourceUsageTag write_tag;
    QueueId write_queue;
    SyncStageAccessFlags last_write;  // only the most recent write

    // TODO Input Attachment cleanup for multiple reads in a given stage
    // Tracks whether the fragment shader read is input attachment read
    bool input_attachment_read;

    VkPipelineStageFlags2KHR last_read_stages;
    VkPipelineStageFlags2KHR read_execution_barriers;
    using ReadStates = small_vector<ReadState, 3, uint32_t>;
    ReadStates last_reads;

    // Pending execution state to support independent parallel barriers
    VkPipelineStageFlags2KHR pending_write_dep_chain;
    bool pending_layout_transition;
    SyncStageAccessFlags pending_write_barriers;
    OrderingBarrier pending_layout_ordering_;
    FirstAccesses first_accesses_;
    VkPipelineStageFlags2KHR first_read_stages_;
    OrderingBarrier first_write_layout_ordering_;

    static OrderingBarriers kOrderingRules;
};
using ResourceAccessStateFunction = std::function<void(ResourceAccessState *)>;
using ResourceAccessStateConstFunction = std::function<void(const ResourceAccessState &)>;

using ResourceAccessRangeMap = sparse_container::range_map<VkDeviceSize, ResourceAccessState>;
using ResourceAccessRange = typename ResourceAccessRangeMap::key_type;
using ResourceAccessRangeIndex = typename ResourceAccessRange::index_type;
using ResourceRangeMergeIterator = sparse_container::parallel_iterator<ResourceAccessRangeMap, const ResourceAccessRangeMap>;

class AttachmentViewGen {
  public:
    enum Gen { kViewSubresource = 0, kRenderArea = 1, kDepthOnlyRenderArea = 2, kStencilOnlyRenderArea = 3, kGenSize = 4 };
    AttachmentViewGen(const IMAGE_VIEW_STATE *view_, const VkOffset3D &offset, const VkExtent3D &extent);
    AttachmentViewGen(const AttachmentViewGen &other) = default;
    AttachmentViewGen(AttachmentViewGen &&other) = default;
    AccessAddressType GetAddressType() const;
    const IMAGE_VIEW_STATE *GetViewState() const { return view_; }
    const ImageRangeGen *GetRangeGen(Gen type) const;
    bool IsValid() const { return gen_store_[Gen::kViewSubresource]; }
    Gen GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const;

  private:
    using RangeGenStore = layer_data::optional<ImageRangeGen>;
    const IMAGE_VIEW_STATE *view_ = nullptr;
    VkImageAspectFlags view_mask_ = 0U;
    std::array<RangeGenStore, Gen::kGenSize> gen_store_;
};

using AttachmentViewGenVector = std::vector<AttachmentViewGen>;

using SyncMemoryBarrier = SyncBarrier;
struct SyncBufferMemoryBarrier {
    using Buffer = std::shared_ptr<const BUFFER_STATE>;
    Buffer buffer;
    SyncBarrier barrier;
    ResourceAccessRange range;
    bool IsLayoutTransition() const { return false; }
    const ResourceAccessRange &Range() const { return range; };
    const BUFFER_STATE *GetState() const { return buffer.get(); }
    SyncBufferMemoryBarrier(const Buffer &buffer_, const SyncBarrier &barrier_, const ResourceAccessRange &range_)
        : buffer(buffer_), barrier(barrier_), range(range_) {}
    SyncBufferMemoryBarrier() = default;
};

struct SyncImageMemoryBarrier {
    using Image = std::shared_ptr<const IMAGE_STATE>;

    Image image;
    uint32_t index;
    SyncBarrier barrier;
    VkImageLayout old_layout;
    VkImageLayout new_layout;
    VkImageSubresourceRange range;

    bool IsLayoutTransition() const { return old_layout != new_layout; }
    const VkImageSubresourceRange &Range() const { return range; };
    const IMAGE_STATE *GetState() const { return image.get(); }
    SyncImageMemoryBarrier(const Image &image_, uint32_t index_, const SyncBarrier &barrier_, VkImageLayout old_layout_,
                           VkImageLayout new_layout_, const VkImageSubresourceRange &subresource_range_)
        : image(image_),
          index(index_),
          barrier(barrier_),
          old_layout(old_layout_),
          new_layout(new_layout_),
          range(subresource_range_) {}
    SyncImageMemoryBarrier() = default;
};

template <typename SubpassNode>
struct SubpassBarrierTrackback {
    std::vector<SyncBarrier> barriers;
    const SubpassNode *source_subpass = nullptr;
    SubpassBarrierTrackback() = default;
    SubpassBarrierTrackback(const SubpassBarrierTrackback &) = default;
    SubpassBarrierTrackback(const SubpassNode *source_subpass_, VkQueueFlags queue_flags_,
                            const std::vector<const VkSubpassDependency2 *> &subpass_dependencies_)
        : barriers(), source_subpass(source_subpass_) {
        barriers.reserve(subpass_dependencies_.size());
        for (const VkSubpassDependency2 *dependency : subpass_dependencies_) {
            assert(dependency);
            barriers.emplace_back(queue_flags_, *dependency);
        }
    }
    SubpassBarrierTrackback(const SubpassNode *source_subpass_, const SyncBarrier &barrier_)
        : barriers(1, barrier_), source_subpass(source_subpass_) {}
    SubpassBarrierTrackback &operator=(const SubpassBarrierTrackback &) = default;
};

struct ReplayTrackbackBarriersAction {
    struct TrackbackBarriers : public SubpassBarrierTrackback<ReplayTrackbackBarriersAction> {
        using Base = SubpassBarrierTrackback<ReplayTrackbackBarriersAction>;
        TrackbackBarriers(const ReplayTrackbackBarriersAction *source_subpass_, VkQueueFlags queue_flags_,
                          const std::vector<const VkSubpassDependency2 *> &subpass_dependencies_);
        void operator()(ResourceAccessState *access) const;
    };

    ReplayTrackbackBarriersAction() = default;
    ReplayTrackbackBarriersAction(VkQueueFlags queue_flags, const SubpassDependencyGraphNode &dependencies,
                                  const std::vector<ReplayTrackbackBarriersAction> &contexts);

    void operator()(ResourceAccessState *access) const;
    std::vector<TrackbackBarriers> trackback_barriers;
};

struct ReplayRenderpassContext {
    std::vector<ReplayTrackbackBarriersAction> subpass_contexts;
};

class SyncOpBase {
  public:
    using ReplayContextPtr = std::shared_ptr<ReplayRenderpassContext>;
    SyncOpBase() : cmd_(CMD_NONE) {}
    SyncOpBase(CMD_TYPE cmd) : cmd_(cmd) {}
    virtual ~SyncOpBase() = default;

    const char *CmdName() const { return CommandTypeString(cmd_); }
    const ReplayTrackbackBarriersAction *GetReplayTrackback() const;

    void SetReplayContext(uint32_t subpass, ReplayContextPtr &&replay);

    virtual bool Validate(const CommandBufferAccessContext &cb_context) const = 0;
    virtual ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const = 0;
    virtual bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const = 0;
    virtual void ReplayRecord(ResourceUsageTag tag, AccessContext *access_context, SyncEventsContext *events_context) const = 0;

  protected:
    // Only non-null and valid for SyncOps within a render pass instance  WIP -- think about how to manage for non RPI calls within
    // RPI and 2ndarys...
    ReplayContextPtr replay_context_;
    uint32_t subpass_ = VK_SUBPASS_EXTERNAL;
    CMD_TYPE cmd_;
};

class SyncOpBarriers : public SyncOpBase {
  protected:
    template <typename Barriers, typename FunctorFactory>
    static void ApplyBarriers(const Barriers &barriers, const FunctorFactory &factory, ResourceUsageTag tag,
                              AccessContext *context);
    template <typename Barriers, typename FunctorFactory>
    static void ApplyGlobalBarriers(const Barriers &barriers, const FunctorFactory &factory, ResourceUsageTag tag,
                                    AccessContext *access_context);

    SyncOpBarriers(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkPipelineStageFlags srcStageMask,
                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                   const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                   const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                   const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpBarriers(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t event_count,
                   const VkDependencyInfoKHR *pDependencyInfo);

    ~SyncOpBarriers() override = default;

  protected:
    struct BarrierSet {
        VkDependencyFlags dependency_flags;
        SyncExecScope src_exec_scope;
        SyncExecScope dst_exec_scope;
        std::vector<SyncMemoryBarrier> memory_barriers;
        std::vector<SyncBufferMemoryBarrier> buffer_memory_barriers;
        std::vector<SyncImageMemoryBarrier> image_memory_barriers;
        bool single_exec_scope;
        void MakeMemoryBarriers(const SyncExecScope &src, const SyncExecScope &dst, VkDependencyFlags dependencyFlags,
                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers);
        void MakeBufferMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                      VkDependencyFlags dependencyFlags, uint32_t bufferMemoryBarrierCount,
                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers);
        void MakeImageMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                     VkDependencyFlags dependencyFlags, uint32_t imageMemoryBarrierCount,
                                     const VkImageMemoryBarrier *pImageMemoryBarriers);
        void MakeMemoryBarriers(VkQueueFlags queue_flags, VkDependencyFlags dependency_flags, uint32_t barrier_count,
                                const VkMemoryBarrier2 *barriers);
        void MakeBufferMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, VkDependencyFlags dependency_flags,
                                      uint32_t barrier_count, const VkBufferMemoryBarrier2 *barriers);
        void MakeImageMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, VkDependencyFlags dependency_flags,
                                     uint32_t barrier_count, const VkImageMemoryBarrier2 *barriers);
    };
    std::vector<BarrierSet> barriers_;
};

class SyncOpPipelineBarrier : public SyncOpBarriers {
  public:
    SyncOpPipelineBarrier(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                          uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpPipelineBarrier(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          const VkDependencyInfoKHR &pDependencyInfo);
    ~SyncOpPipelineBarrier() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;
};

class SyncOpWaitEvents : public SyncOpBarriers {
  public:
    SyncOpWaitEvents(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                     const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                     const VkImageMemoryBarrier *pImageMemoryBarriers);

    SyncOpWaitEvents(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfo);
    ~SyncOpWaitEvents() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;

  protected:
    static const char *const kIgnored;
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    // TODO PHASE2 This is the wrong thing to use for "replay".. as the event state will have moved on since the record
    // TODO PHASE2 May need to capture by value w.r.t. "first use" or build up in calling/enqueue context through replay.
    std::vector<std::shared_ptr<const EVENT_STATE>> events_;
    void MakeEventsList(const SyncValidator &sync_state, uint32_t event_count, const VkEvent *events);
};

class SyncOpResetEvent : public SyncOpBase {
  public:
    SyncOpResetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                     VkPipelineStageFlags2KHR stageMask);
    ~SyncOpResetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    std::shared_ptr<const EVENT_STATE> event_;
    SyncExecScope exec_scope_;
};

class SyncOpSetEvent : public SyncOpBase {
  public:
    SyncOpSetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   VkPipelineStageFlags2KHR stageMask);
    SyncOpSetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   const VkDependencyInfoKHR &dep_info);
    ~SyncOpSetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;
    std::shared_ptr<const EVENT_STATE> event_;
    SyncExecScope src_exec_scope_;
    // Note that the dep info is *not* dehandled, but retained for comparison with a future WaitEvents2
    std::shared_ptr<safe_VkDependencyInfo> dep_info_;
};

class SyncOpBeginRenderPass : public SyncOpBase {
  public:
    SyncOpBeginRenderPass(CMD_TYPE cmd, const SyncValidator &sync_state, const VkRenderPassBeginInfo *pRenderPassBegin,
                          const VkSubpassBeginInfo *pSubpassBeginInfo);
    ~SyncOpBeginRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;

  protected:
    safe_VkRenderPassBeginInfo renderpass_begin_info_;
    safe_VkSubpassBeginInfo subpass_begin_info_;
    std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> shared_attachments_;
    std::vector<const IMAGE_VIEW_STATE *> attachments_;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state_;
    std::shared_ptr<ReplayRenderpassContext> replay_context_;  // Shared across all subpasses for same renderpass instance
};

class SyncOpNextSubpass : public SyncOpBase {
  public:
    SyncOpNextSubpass(CMD_TYPE cmd, const SyncValidator &sync_state, const VkSubpassBeginInfo *pSubpassBeginInfo,
                      const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpNextSubpass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;

  protected:
    safe_VkSubpassBeginInfo subpass_begin_info_;
    safe_VkSubpassEndInfo subpass_end_info_;
    std::shared_ptr<ReplayRenderpassContext> replay_context_;  // Shared across all subpasses for same renderpass instance
};

class SyncOpEndRenderPass : public SyncOpBase {
  public:
    SyncOpEndRenderPass(CMD_TYPE cmd, const SyncValidator &sync_state, const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpEndRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) const override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const override;
    void ReplayRecord(ResourceUsageTag recorded_tag, AccessContext *access_context,
                      SyncEventsContext *events_context) const override;

  protected:
    safe_VkSubpassEndInfo subpass_end_info_;
};

class AccessContext {
  public:
    enum DetectOptions : uint32_t {
        kDetectPrevious = 1U << 0,
        kDetectAsync = 1U << 1,
        kDetectAll = (kDetectPrevious | kDetectAsync)
    };
    struct AddressRange {
        AccessAddressType type;
        ResourceAccessRange range;
        AddressRange() = default;  // the explicit constructor below isn't needed in 20, but would delete the default.
        AddressRange(AccessAddressType type_, ResourceAccessRange range_) : type(type_), range(range_) {}
    };
    using MapArray = std::array<ResourceAccessRangeMap, static_cast<size_t>(AccessAddressType::kTypeCount)>;

    using TrackBack = SubpassBarrierTrackback<AccessContext>;

    HazardResult DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index, const ResourceAccessRange &range) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                              const VkExtent3D &extent) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                              const VkOffset3D &offset, const VkExtent3D &extent, DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                              DetectOptions options) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range) const;
    HazardResult DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const;

    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, SyncOrdering ordering_rule,
                              const VkOffset3D &offset, const VkExtent3D &extent) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                          const SyncStageAccessFlags &src_access_scope,
                                          const VkImageSubresourceRange &subresource_range, const SyncEventState &sync_event,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const AttachmentViewGen &attachment_view, const SyncBarrier &barrier,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                          const SyncStageAccessFlags &src_access_scope,
                                          const VkImageSubresourceRange &subresource_range, DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                          const SyncStageAccessFlags &src_stage_accesses,
                                          const VkImageMemoryBarrier &barrier) const;
    HazardResult DetectImageBarrierHazard(const SyncImageMemoryBarrier &image_barrier) const;
    HazardResult DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const;

    void RecordLayoutTransitions(const RENDER_PASS_STATE &rp_state, uint32_t subpass,
                                 const AttachmentViewGenVector &attachment_views, ResourceUsageTag tag);

    HazardResult DetectFirstUseHazard(const ResourceUsageRange &tag_range, const AccessContext &access_context,
                                      const ReplayTrackbackBarriersAction *replay_barrier) const;

    const TrackBack &GetDstExternalTrackBack() const { return dst_external_; }
    void Reset() {
        prev_.clear();
        prev_by_subpass_.clear();
        async_.clear();
        src_external_ = nullptr;
        dst_external_ = TrackBack();
        start_tag_ = ResourceUsageTag();
        for (auto &map : access_state_maps_) {
            map.clear();
        }
    }

    // Follow the context previous to access the access state, supporting "lazy" import into the context. Not intended for
    // subpass layout transition, as the pending state handling is more complex
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    template <typename BarrierAction>
    void ResolvePreviousAccessStack(AccessAddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                    const ResourceAccessState *infill_state, const BarrierAction &previous_barrie) const;
    void ResolvePreviousAccess(AccessAddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                               const ResourceAccessState *infill_state,
                               const ResourceAccessStateFunction *previous_barrier = nullptr) const;
    void ResolvePreviousAccesses();
    template <typename BarrierAction>
    void ResolveAccessRange(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, BarrierAction &barrier_action,
                            ResourceAccessRangeMap *descent_map, const ResourceAccessState *infill_state) const;
    template <typename BarrierAction>
    void ResolveAccessRange(AccessAddressType type, const ResourceAccessRange &range, BarrierAction &barrier_action,
                            ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                            bool recur_to_infill = true) const;
    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                            const ResourceAccessState *infill_state = nullptr, bool recur_to_infill = false);

    void UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const ResourceAccessRange &range, ResourceUsageTag tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset, const VkExtent3D &extent,
                           ResourceUsageTag tag);
    void UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncStageAccessIndex current_usage,
                           SyncOrdering ordering_rule, ResourceUsageTag tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceLayers &subresource, const VkOffset3D &offset, const VkExtent3D &extent,
                           ResourceUsageTag tag);
    void UpdateAttachmentResolveAccess(const RENDER_PASS_STATE &rp_state, const AttachmentViewGenVector &attachment_views,
                                       uint32_t subpass, ResourceUsageTag tag);
    void UpdateAttachmentStoreAccess(const RENDER_PASS_STATE &rp_state, const AttachmentViewGenVector &attachment_views,
                                     uint32_t subpass, ResourceUsageTag tag);

    void ResolveChildContexts(const std::vector<AccessContext> &contexts);

    void ImportAsyncContexts(const AccessContext &from);
    template <typename Action, typename RangeGen>
    void ApplyUpdateAction(AccessAddressType address_type, const Action &action, RangeGen *range_gen_arg);
    template <typename Action>
    void ApplyUpdateAction(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, const Action &action);
    template <typename Action>
    void ApplyToContext(const Action &barrier_action);
    static AccessAddressType ImageAddressType(const IMAGE_STATE &image);

    void DeleteAccess(const AddressRange &address);
    AccessContext(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                  const std::vector<AccessContext> &contexts, const AccessContext *external_context);

    AccessContext() { Reset(); }
    AccessContext(const AccessContext &copy_from) = default;

    ResourceAccessRangeMap &GetAccessStateMap(AccessAddressType type) { return access_state_maps_[static_cast<size_t>(type)]; }
    const ResourceAccessRangeMap &GetAccessStateMap(AccessAddressType type) const {
        return access_state_maps_[static_cast<size_t>(type)];
    }
    const TrackBack *GetTrackBackFromSubpass(uint32_t subpass) const {
        if (subpass == VK_SUBPASS_EXTERNAL) {
            return src_external_;
        } else {
            assert(subpass < prev_by_subpass_.size());
            return prev_by_subpass_[subpass];
        }
    }

    bool ValidateLayoutTransitions(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                                   const VkRect2D &render_area, uint32_t subpass, const AttachmentViewGenVector &attachment_views,
                                   const char *func_name) const;
    bool ValidateLoadOperation(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                               const VkRect2D &render_area, uint32_t subpass, const AttachmentViewGenVector &attachment_views,
                               const char *func_name) const;
    bool ValidateStoreOperation(const CommandExecutionContext &ex_context,

                                const RENDER_PASS_STATE &rp_state,

                                const VkRect2D &render_area, uint32_t subpass, const AttachmentViewGenVector &attachment_views,
                                const char *func_name) const;
    bool ValidateResolveOperations(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                                   const VkRect2D &render_area, const AttachmentViewGenVector &attachment_views,
                                   const char *func_name, uint32_t subpass) const;

    void SetStartTag(ResourceUsageTag tag) { start_tag_ = tag; }
    template <typename Action>
    void ForAll(Action &&action);

    // For use during queue submit building up the QueueBatchContext AccessContext
    TrackBack *AddTrackBack(const AccessContext *context, const SyncBarrier &barrier);
    void AddAsyncContext(const AccessContext *context);
    // For use during queue submit to avoid stale pointers;
    void ClearTrackBacks() { prev_.clear(); }
    void ClearAsyncContext(const AccessContext *context) { async_.clear(); }

  private:
    template <typename Detector>
    HazardResult DetectHazard(AccessAddressType type, const Detector &detector, const ResourceAccessRange &range,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectAsyncHazard(AccessAddressType type, const Detector &detector, const ResourceAccessRange &range) const;
    template <typename Detector>
    HazardResult DetectPreviousHazard(AccessAddressType type, const Detector &detector, const ResourceAccessRange &range) const;
    void UpdateAccessState(AccessAddressType type, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const ResourceAccessRange &range, ResourceUsageTag tag);

    MapArray access_state_maps_;
    std::vector<TrackBack> prev_;
    std::vector<TrackBack *> prev_by_subpass_;
    std::vector<const AccessContext *> async_;
    TrackBack *src_external_;
    TrackBack dst_external_;
    ResourceUsageTag start_tag_;
};

class RenderPassAccessContext {
  public:
    static AttachmentViewGenVector CreateAttachmentViewGen(const VkRect2D &render_area,
                                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views);
    RenderPassAccessContext() : rp_state_(nullptr), render_area_(VkRect2D()), current_subpass_(0) {}
    RenderPassAccessContext(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area, VkQueueFlags queue_flags,
                            const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const AccessContext *external_context);

    bool ValidateDrawSubpassAttachment(const CommandExecutionContext &ex_context, const CMD_BUFFER_STATE &cmd,
                                       const char *func_name) const;
    void RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd, ResourceUsageTag tag);
    bool ValidateNextSubpass(const CommandExecutionContext &ex_context, const char *command_name) const;
    bool ValidateEndRenderPass(const CommandExecutionContext &ex_context, const char *func_name) const;
    bool ValidateFinalSubpassLayoutTransitions(const CommandExecutionContext &ex_context, const char *func_name) const;

    void RecordLayoutTransitions(ResourceUsageTag tag);
    void RecordLoadOperations(ResourceUsageTag tag);
    void RecordBeginRenderPass(ResourceUsageTag tag, ResourceUsageTag load_tag);
    void RecordNextSubpass(ResourceUsageTag store_tag, ResourceUsageTag barrier_tag, ResourceUsageTag load_tag);
    void RecordEndRenderPass(AccessContext *external_context, ResourceUsageTag store_tag, ResourceUsageTag barrier_tag);

    AccessContext &CurrentContext() { return subpass_contexts_[current_subpass_]; }
    const AccessContext &CurrentContext() const { return subpass_contexts_[current_subpass_]; }
    const std::vector<AccessContext> &GetContexts() const { return subpass_contexts_; }
    uint32_t GetCurrentSubpass() const { return current_subpass_; }
    const RENDER_PASS_STATE *GetRenderPassState() const { return rp_state_; }
    AccessContext *CreateStoreResolveProxy() const;
    std::shared_ptr<ReplayRenderpassContext> GetReplayContext() const { return replay_context_; }

  private:
    const RENDER_PASS_STATE *rp_state_;
    const VkRect2D render_area_;
    uint32_t current_subpass_;
    std::vector<AccessContext> subpass_contexts_;
    AttachmentViewGenVector attachment_views_;
    std::shared_ptr<ReplayRenderpassContext> replay_context_;  // Each SyncOp for this renderpass instance will have a copy
};

// Command execution context is the base class for command buffer and queue contexts
// Preventing unintented leakage of subclass specific state, storing enough information
// for message logging.
// TODO: determine where to draw the design split for tag tracking (is there anything command to Queues and CB's)
class CommandExecutionContext {
  public:
    using AccessLog = std::vector<ResourceUsageRecord>;
    CommandExecutionContext() : sync_state_(nullptr) {}
    CommandExecutionContext(const SyncValidator *sync_validator) : sync_state_(sync_validator) {}
    virtual ~CommandExecutionContext() = default;
    virtual AccessContext *GetCurrentAccessContext() = 0;
    virtual SyncEventsContext *GetCurrentEventsContext() = 0;
    virtual const AccessContext *GetCurrentAccessContext() const = 0;
    virtual const SyncEventsContext *GetCurrentEventsContext() const = 0;

    const SyncValidator &GetSyncState() const {
        assert(sync_state_);
        return *sync_state_;
    }

    ResourceUsageRange ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context);
    std::string FormatHazard(const HazardResult &hazard) const;

    virtual ResourceUsageTag GetTagLimit() const = 0;
    virtual VulkanTypedHandle Handle() const = 0;
    virtual std::string FormatUsage(ResourceUsageTag tag) const = 0;
    virtual void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) = 0;

  protected:
    const SyncValidator *sync_state_;
};

class CommandBufferAccessContext : public CommandExecutionContext {
  public:
    using SyncOpPointer = std::shared_ptr<SyncOpBase>;
    struct SyncOpEntry {
        ResourceUsageTag tag;
        SyncOpPointer sync_op;
        SyncOpEntry(ResourceUsageTag tag_, SyncOpPointer &&sync_op_) : tag(tag_), sync_op(std::move(sync_op_)) {}
        SyncOpEntry() = default;
        SyncOpEntry(const SyncOpEntry &other) = default;
    };

    CommandBufferAccessContext(const SyncValidator *sync_validator = nullptr)
        : CommandExecutionContext(sync_validator),
          cb_state_(),
          queue_flags_(),
          destroyed_(false),
          access_log_(),
          cbs_referenced_(),
          command_number_(0),
          subcommand_number_(0),
          reset_count_(0),
          cb_access_context_(),
          current_context_(&cb_access_context_),
          events_context_(),
          render_pass_contexts_(),
          current_renderpass_context_(),
          sync_ops_() {}
    CommandBufferAccessContext(SyncValidator &sync_validator, std::shared_ptr<CMD_BUFFER_STATE> &cb_state, VkQueueFlags queue_flags)
        : CommandBufferAccessContext(&sync_validator) {
        cb_state_ = cb_state;
        queue_flags_ = queue_flags;
    }

    struct AsProxyContext {};
    CommandBufferAccessContext(const CommandBufferAccessContext &real_context, AsProxyContext dummy);

    ~CommandBufferAccessContext() override = default;
    CommandExecutionContext &GetExecutionContext() { return *this; }
    const CommandExecutionContext &GetExecutionContext() const { return *this; }

    void Reset() {
        access_log_.clear();
        cbs_referenced_.clear();
        sync_ops_.clear();
        command_number_ = 0;
        subcommand_number_ = 0;
        reset_count_++;
        cb_access_context_.Reset();
        render_pass_contexts_.clear();
        current_context_ = &cb_access_context_;
        current_renderpass_context_ = nullptr;
        events_context_.Clear();
    }
    void MarkDestroyed() { destroyed_ = true; }
    bool IsDestroyed() const { return destroyed_; }

    std::string FormatUsage(ResourceUsageTag tag) const override;
    std::string FormatUsage(const ResourceFirstAccess &access) const;  //  Only command buffers have "first usage"
    AccessContext *GetCurrentAccessContext() override { return current_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }

    RenderPassAccessContext *GetCurrentRenderPassContext() { return current_renderpass_context_; }
    const RenderPassAccessContext *GetCurrentRenderPassContext() const { return current_renderpass_context_; }
    ResourceUsageTag RecordBeginRenderPass(CMD_TYPE cmd, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views);
    void ApplyGlobalBarriersToEvents(const SyncExecScope &src, const SyncExecScope &dst);

    bool ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, const char *func_name) const;
    void RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, ResourceUsageTag tag);
    bool ValidateDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const char *func_name) const;
    void RecordDrawVertex(uint32_t vertexCount, uint32_t firstVertex, ResourceUsageTag tag);
    bool ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const char *func_name) const;
    void RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, ResourceUsageTag tag);
    bool ValidateDrawSubpassAttachment(const char *func_name) const;
    void RecordDrawSubpassAttachment(ResourceUsageTag tag);
    ResourceUsageTag RecordNextSubpass(CMD_TYPE cmd);
    ResourceUsageTag RecordEndRenderPass(CMD_TYPE cmd);
    void RecordDestroyEvent(VkEvent event);

    bool ValidateFirstUse(CommandExecutionContext *proxy_context, const char *func_name, uint32_t index) const;
    void RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_context, CMD_TYPE cmd);
    void ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    const CMD_BUFFER_STATE *GetCommandBufferState() const { return cb_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }

    ResourceUsageTag NextSubcommandTag(CMD_TYPE command, ResourceUsageRecord::SubcommandType subcommand);
    ResourceUsageTag GetTagLimit() const override { return access_log_.size(); }
    VulkanTypedHandle Handle() const override {
        if (cb_state_) {
            return cb_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkCommandBuffer>(VK_NULL_HANDLE), kVulkanObjectTypeCommandBuffer);
    }

    ResourceUsageTag NextCommandTag(CMD_TYPE command,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);
    ResourceUsageTag NextIndexedCommandTag(CMD_TYPE command, uint32_t index);

    std::shared_ptr<const CMD_BUFFER_STATE> GetCBStateShared() const { return cb_state_; }

    const CMD_BUFFER_STATE &GetCBState() const {
        assert(cb_state_);
        return *(cb_state_.get());
    }
    CMD_BUFFER_STATE &GetCBState() {
        assert(cb_state_);
        return *(cb_state_.get());
    }

    template <class T, class... Args>
    void RecordSyncOp(Args &&...args) {
        // T must be as derived from SyncOpBase or the compiler will flag the next line as an error.
        SyncOpPointer sync_op(std::make_shared<T>(std::forward<Args>(args)...));
        RecordSyncOp(std::move(sync_op));  // Call the non-template version
    }
    const AccessLog &GetAccessLog() const { return access_log_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;
    const std::vector<SyncOpEntry> &GetSyncOps() const { return sync_ops_; };

  private:
    // As this is passing around a shared pointer to record, move to avoid needless atomics.
    void RecordSyncOp(SyncOpPointer &&sync_op);
    std::shared_ptr<CMD_BUFFER_STATE> cb_state_;
    VkQueueFlags queue_flags_;
    bool destroyed_;

    AccessLog access_log_;
    layer_data::unordered_set<std::shared_ptr<const CMD_BUFFER_STATE>> cbs_referenced_;
    uint32_t command_number_;
    uint32_t subcommand_number_;
    uint32_t reset_count_;

    AccessContext cb_access_context_;
    AccessContext *current_context_;
    SyncEventsContext events_context_;

    // Don't need the following for an active proxy cb context
    std::vector<RenderPassAccessContext> render_pass_contexts_;
    RenderPassAccessContext *current_renderpass_context_;
    std::vector<SyncOpEntry> sync_ops_;
};

class QueueSyncState;

// Store the ResourceUsageRecords for the global tag range.  The prev_ field allows for
// const Validation phase access from the cmd state "overlay" seamlessly.
class AccessLogger {
  public:
    struct BatchRecord {
        BatchRecord() = default;
        BatchRecord(const BatchRecord &other) = default;
        BatchRecord(BatchRecord &&other) = default;
        BatchRecord(const QueueSyncState *q, uint64_t submit, uint32_t batch)
            : queue(q), submit_index(submit), batch_index(batch) {}
        BatchRecord &operator=(const BatchRecord &other) = default;
        const QueueSyncState *queue;
        uint64_t submit_index;
        uint32_t batch_index;
    };

    struct AccessRecord {
        const BatchRecord *batch;
        const ResourceUsageRecord *record;
        bool IsValid() const { return batch && record; }
    };

    // BatchLog lookup is batch relative, thus the batch doesn't need to track it's offset
    class BatchLog {
      public:
        BatchLog() = default;
        BatchLog(const BatchLog &batch) = default;
        BatchLog(BatchLog &&other) = default;
        BatchLog &operator=(const BatchLog &other) = default;
        BatchLog &operator=(BatchLog &&other) = default;
        BatchLog(const BatchRecord &batch) : batch_(batch) {}

        size_t Size() const { return log_.size(); }
        const BatchRecord &GetBatch() const { return batch_; }
        AccessRecord operator[](size_t index) const;

        void Append(const CommandExecutionContext::AccessLog &other);

      private:
        BatchRecord batch_;
        layer_data::unordered_set<std::shared_ptr<const CMD_BUFFER_STATE>> cbs_referenced_;
        CommandExecutionContext::AccessLog log_;
    };

    using AccessLogRangeMap = sparse_container::range_map<ResourceUsageTag, BatchLog>;

    AccessLogger(const AccessLogger *prev = nullptr) : prev_(prev) {}
    // AccessLogger lookup is based on global tags
    AccessRecord operator[](ResourceUsageTag tag) const;
    BatchLog *AddBatch(const QueueSyncState *queue_state, uint64_t submit_id, uint32_t batch_id, const ResourceUsageRange &range);
    void MergeMove(AccessLogger &&child);
    void Reset();

  private:
    const AccessLogger *prev_;
    AccessLogRangeMap access_log_map_;
};

class SemaphoreSyncState;
class SignaledSemaphores;
// TODO need a map from fence to submbit batch id
class QueueBatchContext : public CommandExecutionContext {
  public:
    using ConstBatchSet = layer_data::unordered_set<std::shared_ptr<const QueueBatchContext>>;
    using BatchSet = layer_data::unordered_set<std::shared_ptr<QueueBatchContext>>;
    static constexpr bool TruePred(const std::shared_ptr<const QueueBatchContext> &) { return true; }
    struct CmdBufferEntry {
        uint32_t index = 0;
        std::shared_ptr<const CommandBufferAccessContext> cb;
        CmdBufferEntry(uint32_t index_, std::shared_ptr<const CommandBufferAccessContext> &&cb_)
            : index(index_), cb(std::move(cb_)) {}
    };

    // For Wait operations we have to
    struct QueueWormBase {
        std::vector<AccessContext::AddressRange> erase_list;
        bool erase_all = true;
    };

    struct QueueWaitWorm : QueueWormBase {
        ResourceAccessState::QueuePredicate predicate;
        QueueWaitWorm(QueueId queue_, ResourceUsageTag tag_ = ResourceUsageRecord::kMaxIndex);
        void operator()(AccessAddressType address_type, ResourceAccessRangeMap::value_type &access);
    };

    using CommandBuffers = std::vector<CmdBufferEntry>;

    std::string FormatUsage(ResourceUsageTag tag) const override;
    AccessContext *GetCurrentAccessContext() override { return &access_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return &access_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    const QueueSyncState *GetQueueSyncState() const { return queue_state_; }
    VkQueueFlags GetQueueFlags() const;

    void SetBatchLog(AccessLogger &loggger, uint64_t sumbit_id, uint32_t batch_id);
    void ResetAccessLog() {
        logger_ = nullptr;
        batch_log_ = nullptr;
    }
    ResourceUsageTag GetTagLimit() const override { return batch_log_->Size() + tag_range_.begin; }
    // begin is the tag bias  / .size() is the number of total records that should eventually be in access_log_
    ResourceUsageRange GetTagRange() const { return tag_range_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;

    void SetTagBias(ResourceUsageTag);
    CommandBuffers::const_iterator begin() const { return command_buffers_.cbegin(); }
    CommandBuffers::const_iterator end() const { return command_buffers_.cend(); }

    QueueBatchContext(const SyncValidator &sync_state, const QueueSyncState &queue_state);
    QueueBatchContext() = delete;

    template <typename BatchInfo>
    void Setup(const std::shared_ptr<const QueueBatchContext> &prev_batch, const BatchInfo &batch_info,
               SignaledSemaphores &signaled);

    void ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    VulkanTypedHandle Handle() const override;

    template <typename BatchInfo, typename Fn>
    static void ForEachWaitSemaphore(const BatchInfo &batch_info, Fn &&func);

    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyDeviceWait();

  private:
    using WaitBatchMap = layer_data::unordered_map<const QueueBatchContext *, AccessContext::TrackBack *>;

    // The BatchInfo is either the Submit or Submit2 version with traits allowing generic acces
    template <typename BatchInfo>
    class SubmitInfoAccessor {};
    template <typename BatchInfo>
    void SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const BatchInfo &batch_info,
                            SignaledSemaphores &signaled_semaphores);
    template <typename BatchInfo>
    void SetupCommandBufferInfo(const BatchInfo &batch_info);

    void WaitOneSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask, WaitBatchMap &batch_trackbacks,
                          SignaledSemaphores &signaled);

    const QueueSyncState *queue_state_ = nullptr;
    ResourceUsageRange tag_range_ = ResourceUsageRange(0, 0);  // Range of tags referenced by cbs_referenced

    AccessContext access_context_;
    SyncEventsContext events_context_;

    // Clear these after validation and import
    CommandBuffers command_buffers_;
    ConstBatchSet async_batches_;
    // When null use the global logger
    AccessLogger *logger_ = nullptr;
    AccessLogger::BatchLog *batch_log_ = nullptr;
};

class QueueSyncState {
  public:
    constexpr static QueueId kQueueIdBase = QueueId(0);
    constexpr static QueueId kQueueIdInvalid = ~kQueueIdBase;
    QueueSyncState(const std::shared_ptr<QUEUE_STATE> &queue_state, VkQueueFlags queue_flags, QueueId id)
        : submit_index_(0), queue_state_(queue_state), last_batch_(), queue_flags_(queue_flags), id_(id) {}

    VulkanTypedHandle Handle() const {
        if (queue_state_) {
            return queue_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkQueue>(VK_NULL_HANDLE), kVulkanObjectTypeQueue);
    }
    std::shared_ptr<const QueueBatchContext> LastBatch() const { return last_batch_; }
    std::shared_ptr<QueueBatchContext> LastBatch() { return last_batch_; }
    void SetLastBatch(std::shared_ptr<QueueBatchContext> &&last);
    QUEUE_STATE *GetQueueState() { return queue_state_.get(); }
    const QUEUE_STATE *GetQueueState() const { return queue_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }
    QueueId GetQueueId() const { return id_; }

    uint64_t ReserveSubmitId() const;  // Method is const but updates mutable sumbit_index atomically.

  private:
    mutable std::atomic<uint64_t> submit_index_;
    std::shared_ptr<QUEUE_STATE> queue_state_;
    std::shared_ptr<QueueBatchContext> last_batch_;
    const VkQueueFlags queue_flags_;
    QueueId id_;
};

class SignaledSemaphores {
  public:
    // Is the record of a signaled semaphore, deleted when unsignaled
    struct Signal {
        Signal() = delete;
        Signal(const Signal &other) = default;
        Signal(Signal &&other) = default;
        Signal &operator=(const Signal &other) = default;
        Signal &operator=(Signal &&other) = default;
        Signal(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state_, const std::shared_ptr<QueueBatchContext> &batch_,
               const SyncExecScope &exec_scope_)
            : sem_state(sem_state_), batch(batch_), exec_scope(exec_scope_) {
            // Illegal to create a signal from no batch and an invalid semaphore... caller must assure validity
            assert(batch);
            assert(sem_state);
        }

        std::shared_ptr<const SEMAPHORE_STATE> sem_state;
        std::shared_ptr<QueueBatchContext> batch;
        // Use the SyncExecScope::valid_accesses for first access scope
        SyncExecScope exec_scope;
        // TODO add timeline semaphore support.
    };
    using SignalMap = layer_data::unordered_map<VkSemaphore, std::shared_ptr<Signal>>;
    using iterator = SignalMap::iterator;
    iterator begin() { return signaled_.begin(); }
    iterator end() { return signaled_.end(); }

    bool SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, const std::shared_ptr<QueueBatchContext> &batch,
                         const VkSemaphoreSubmitInfo &signal_info);
    std::shared_ptr<Signal> Unsignal(VkSemaphore);
    void Import(VkSemaphore sem, std::shared_ptr<Signal> &&move_from);
    void Reset();

  private:
    std::shared_ptr<Signal> GetPrev(VkSemaphore sem) const;
    layer_data::unordered_map<VkSemaphore, std::shared_ptr<Signal>> signaled_;
    const SignaledSemaphores *prev_;  // Allowing this type to act as a writable overlay
};

class SyncValidator : public ValidationStateTracker, public SyncStageAccess {
  public:
    using StateTracker = ValidationStateTracker;
    SyncValidator() { container_type = LayerObjectTypeSyncValidation; }
    virtual ~SyncValidator() { ResetCommandBufferCallbacks(); };

    // Global tag range for submitted command buffers resource usage logs
    mutable std::atomic<ResourceUsageTag> tag_limit_{0};  // This is reserved in Validation phase, thus mutable and atomic
    ResourceUsageRange ReserveGlobalTagRange(size_t tag_count) const;  // Note that the tag_limit_ is mutable this has side effects
    // This is a snapshot value only
    AccessLogger global_access_log_;

    layer_data::unordered_map<VkCommandBuffer, std::shared_ptr<CommandBufferAccessContext>> cb_access_state;

    using QueueSyncStatesMap = layer_data::unordered_map<VkQueue, std::shared_ptr<QueueSyncState>>;
    layer_data::unordered_map<VkQueue, std::shared_ptr<QueueSyncState>> queue_sync_states_;
    SignaledSemaphores signaled_semaphores_;

    const QueueSyncState *GetQueueSyncState(VkQueue queue) const;
    QueueSyncState *GetQueueSyncState(VkQueue queue);
    std::shared_ptr<const QueueSyncState> GetQueueSyncStateShared(VkQueue queue) const;
    std::shared_ptr<QueueSyncState> GetQueueSyncStateShared(VkQueue queue);

    template <typename Predicate>
    QueueBatchContext::ConstBatchSet GetQueueLastBatchSnapshot(Predicate &&pred) const;
    QueueBatchContext::ConstBatchSet GetQueueLastBatchSnapshot() const {
        return GetQueueLastBatchSnapshot(QueueBatchContext::TruePred);
    };

    template <typename Predicate>
    QueueBatchContext::BatchSet GetQueueLastBatchSnapshot(Predicate &&pred);
    QueueBatchContext::BatchSet GetQueueLastBatchSnapshot() { return GetQueueLastBatchSnapshot(QueueBatchContext::TruePred); };

    std::shared_ptr<CommandBufferAccessContext> AccessContextFactory(VkCommandBuffer command_buffer);
    CommandBufferAccessContext *GetAccessContext(VkCommandBuffer command_buffer);
    CommandBufferAccessContext *GetAccessContextNoInsert(VkCommandBuffer command_buffer);
    const CommandBufferAccessContext *GetAccessContext(VkCommandBuffer command_buffer) const;
    std::shared_ptr<CommandBufferAccessContext> GetAccessContextShared(VkCommandBuffer command_buffer);
    std::shared_ptr<const CommandBufferAccessContext> GetAccessContextShared(VkCommandBuffer command_buffer) const;

    void ResetCommandBufferCallback(VkCommandBuffer command_buffer);
    void FreeCommandBufferCallback(VkCommandBuffer command_buffer);
    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                              const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE command);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd);
    bool SupressedBoundDescriptorWAW(const HazardResult &hazard) const;

    void CreateDevice(const VkDeviceCreateInfo *pCreateInfo) override;

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents) const override;

    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassBeginInfo *pSubpassBeginInfo) const override;

    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo) const override;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions) const override;

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy *pRegions) override;

    void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) override;
    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) const override;
    bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) const override;
    bool ValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos, CMD_TYPE cmd_type) const;

    void RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos, CMD_TYPE cmd_type);
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions) const override;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions) override;

    bool ValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) const override;
    bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) const override;

    void RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, CMD_TYPE cmd_type);
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) override;

    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier *pImageMemoryBarriers) const override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) override;

    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                               const VkDependencyInfoKHR *pDependencyInfo) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) const override;
    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) override;

    void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                          VkResult result) override;

    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                          VkSubpassContents contents) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const VkSubpassBeginInfo *pSubpassBeginInfo) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo) override;

    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd) const;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                        const VkSubpassEndInfo *pSubpassEndInfo) const override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo) const override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo) override;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd) const;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) const override;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) override;

    template <typename RegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                      CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkBufferImageCopy *pRegions) const override;
    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const override;
    bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                              const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) const override;

    template <typename RegionType>
    void RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                    CMD_TYPE cmd_type);
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                            const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                      CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) const override;
    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const override;
    bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                              const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) const override;

    template <typename RegionType>
    void RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                    VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, CMD_TYPE cmd_type);
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                            const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                              const char *apiName) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                            ResourceUsageTag tag);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions,
                                   VkFilter filter) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) override;

    bool ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                const char *function) const;
    void RecordIndirectBuffer(AccessContext &context, ResourceUsageTag tag, const VkDeviceSize struct_size, const VkBuffer buffer,
                              const VkDeviceSize offset, const uint32_t drawCount, uint32_t stride);

    bool ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                             VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, const char *function) const;
    void RecordCountBuffer(AccessContext &context, ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset);

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;

    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;

    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance) override;

    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;

    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                      uint32_t stride) override;

    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride) override;

    bool ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                      const char *function) const;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) const override;
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, CMD_TYPE cmd_type);
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;

    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const char *function) const;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const override;
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, CMD_TYPE cmd_type);
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges) const override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue *pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange *pRanges) override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges) const override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges) override;

    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags) override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data) const override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data) override;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions) const override;

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve *pRegions) override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) const override;
    bool ValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo, CMD_TYPE cmd_type) const;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;
    void RecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo, CMD_TYPE cmd_type);

    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void *pData) const override;
    void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                      VkDeviceSize dataSize, const void *pData) override;

    bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const override;
    void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) override;

    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    void PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;

    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                        const VkDependencyInfoKHR *pDependencyInfo) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                     const VkDependencyInfo *pDependencyInfo) const override;
    void PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                       const VkDependencyInfoKHR *pDependencyInfo) override;
    void PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo) override;

    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    void PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;

    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                          VkPipelineStageFlags2KHR stageMask) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                       VkPipelineStageFlags2 stageMask) const override;
    void PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask) override;
    void PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) override;

    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier *pImageMemoryBarriers) const override;
    void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                     VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos) const override;
    void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                         const VkDependencyInfoKHR *pDependencyInfos) override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos) const override;
    void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos) override;
    bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                                 VkDeviceSize dstOffset, uint32_t marker) const override;
    void PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                               VkDeviceSize dstOffset, uint32_t marker) override;
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer *pCommandBuffers) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) override;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                    VkFence fence) const override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   VkResult result) override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                        VkFence fence) const override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       VkResult result) override;
};
