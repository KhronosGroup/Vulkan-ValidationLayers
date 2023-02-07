/*
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

#include <limits>
#include <memory>
#include <set>
#include <vulkan/vulkan.h>

#include "sync_validation_types.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"

class AccessContext;
class CommandBufferAccessContext;
class CommandExecutionContext;
struct PresentedImage;
class QueueBatchContext;
struct QueueSubmitCmdState;
class RenderPassAccessContext;
class ResourceAccessState;
struct ResourceFirstAccess;
class SyncEventsContext;
struct SyncEventState;
class SyncValidator;
namespace syncval_state {
class CommandBuffer;
class Swapchain;
}  // namespace syncval_state

using ImageRangeEncoder = subresource_adapter::ImageRangeEncoder;
using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

using QueueId = uint32_t;

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
    kNonAttachment = 0,
    kColorAttachment = 1,
    kDepthStencilAttachment = 2,
    kRaster = 3,
    kNumOrderings = 4,
};

// Useful Utilites for manipulating StageAccess parameters, suitable as base class to save typing
struct SyncStageAccess {
    static inline SyncStageAccessFlags FlagBit(SyncStageAccessIndex stage_access) {
        return syncStageAccessInfoByStageAccessIndex()[stage_access].stage_access_bit;
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
        return syncStageAccessInfoByStageAccessIndex()[stage_access_index].stage_mask;
    }
    static SyncStageAccessFlags AccessScopeByStage(VkPipelineStageFlags2KHR stages);
    static SyncStageAccessFlags AccessScopeByAccess(VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(VkPipelineStageFlags2KHR stages, VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(const SyncStageAccessFlags &stage_scope, VkAccessFlags2KHR accesses) {
        return stage_scope & AccessScopeByAccess(accesses);
    }
};

class AlternateResourceUsage {
  public:
    struct RecordBase;
    struct RecordBase {
        using Record = std::unique_ptr<RecordBase>;
        virtual Record MakeRecord() const = 0;
        virtual std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const = 0;
        virtual ~RecordBase() {}
    };

    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const AlternateResourceUsage &usage_)
            : sync_state(sync_state_), usage(usage_) {}
        const SyncValidator &sync_state;
        const AlternateResourceUsage &usage;
    };

    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); };

    std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const { return record_->Format(out, sync_state); };
    AlternateResourceUsage() = default;
    AlternateResourceUsage(const RecordBase &record) : record_(record.MakeRecord()) {}
    AlternateResourceUsage(const AlternateResourceUsage &other) : record_() {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        }
    }
    AlternateResourceUsage &operator=(const AlternateResourceUsage &other) {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        } else {
            record_.reset();
        }
        return *this;
    }

    operator bool() const { return bool(record_); }

  private:
    RecordBase::Record record_;
};

inline std::ostream &operator<<(std::ostream &out, const AlternateResourceUsage::FormatterState &formatter) {
    formatter.usage.Format(out, formatter.sync_state);
    return out;
}

template <typename State, typename T>
struct FormatterImpl {
    using That = T;
    friend T;
    const State &state;
    const That &that;

  private:
    // Only intended to be invoke with from That method
    FormatterImpl(const State &state_, const That &that_) : state(state_), that(that_) {}
};

struct NamedHandle {
    const static size_t kInvalidIndex = std::numeric_limits<size_t>::max();
    std::string name;
    VulkanTypedHandle handle;
    size_t index = kInvalidIndex;

    using FormatterState = FormatterImpl<SyncValidator, NamedHandle>;
    // NOTE: CRTP could DRY this
    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); }

    NamedHandle() = default;
    NamedHandle(const NamedHandle &other) = default;
    NamedHandle(NamedHandle &&other) = default;
    NamedHandle(const std::string &name_, const VulkanTypedHandle &handle_, size_t index_ = kInvalidIndex)
        : name(name_), handle(handle_), index(index_) {}
    NamedHandle(const char *name_, const VulkanTypedHandle &handle_, size_t index_ = kInvalidIndex)
        : name(name_), handle(handle_), index(index_) {}
    NamedHandle(const VulkanTypedHandle &handle_) : name(), handle(handle_) {}
    NamedHandle &operator=(const NamedHandle &other) = default;
    NamedHandle &operator=(NamedHandle &&other) = default;

    operator bool() const { return (handle.handle != 0U) && (handle.type != VulkanObjectType::kVulkanObjectTypeUnknown); }
    bool IsIndexed() const { return index != kInvalidIndex; }
};

struct ResourceCmdUsageRecord {
    using TagIndex = size_t;
    using Count = uint32_t;
    constexpr static TagIndex kMaxIndex = std::numeric_limits<TagIndex>::max();
    constexpr static Count kMaxCount = std::numeric_limits<Count>::max();

    enum class SubcommandType { kNone, kSubpassTransition, kLoadOp, kStoreOp, kResolveOp, kIndex };

    ResourceCmdUsageRecord() = default;
    ResourceCmdUsageRecord(CMD_TYPE command_, Count seq_num_, SubcommandType sub_type_, Count sub_command_,
                           const CMD_BUFFER_STATE *cb_state_, Count reset_count_)
        : command(command_),
          seq_num(seq_num_),
          sub_command_type(sub_type_),
          sub_command(sub_command_),
          cb_state(cb_state_),
          reset_count(reset_count_) {}

    // NamedHandle must be constructable from args
    template <class... Args>
    void AddHandle(Args &&...args) {
        handles.emplace_back(std::forward<Args>(args)...);
    }

    CMD_TYPE command = CMD_NONE;
    Count seq_num = 0U;
    SubcommandType sub_command_type = SubcommandType::kNone;
    Count sub_command = 0U;

    // This is somewhat repetitive, but it prevents the need for Exec/Submit time touchup, after which usage records can be
    // from different command buffers and resets.
    // plain pointer as a shared pointer is held by the context storing this record
    const CMD_BUFFER_STATE *cb_state = nullptr;
    Count reset_count;
    small_vector<NamedHandle, 1> handles;
};

struct ResourceUsageRecord : public ResourceCmdUsageRecord {
    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const ResourceUsageRecord &record_, const CMD_BUFFER_STATE *cb_state_)
            : sync_state(sync_state_), record(record_), ex_cb_state(cb_state_) {}
        const SyncValidator &sync_state;
        const ResourceUsageRecord &record;
        const CMD_BUFFER_STATE *ex_cb_state;
    };
    FormatterState Formatter(const SyncValidator &sync_state, const CMD_BUFFER_STATE *ex_cb_state) const {
        return FormatterState(sync_state, *this, ex_cb_state);
    }

    AlternateResourceUsage alt_usage;

    ResourceUsageRecord() = default;
    ResourceUsageRecord(CMD_TYPE command_, Count seq_num_, SubcommandType sub_type_, Count sub_command_,
                        const CMD_BUFFER_STATE *cb_state_, Count reset_count_)
        : ResourceCmdUsageRecord(command_, seq_num_, sub_type_, sub_command_, cb_state_, reset_count_) {}

    ResourceUsageRecord(const AlternateResourceUsage &other) : ResourceCmdUsageRecord(), alt_usage(other) {}
    ResourceUsageRecord(const ResourceUsageRecord &other) : ResourceCmdUsageRecord(other), alt_usage(other.alt_usage) {}
    ResourceUsageRecord &operator=(const ResourceUsageRecord &other) = default;
};

// The resource tag index is relative to the command buffer or queue in which it's found
using ResourceUsageTag = ResourceUsageRecord::TagIndex;
using ResourceUsageTagSet = std::set<ResourceUsageTag>;
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
    bool IsHazard() const { return NONE != hazard; }
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
};

enum class AccessAddressType : uint32_t { kLinear = 0, kIdealized = 1, kMaxType = 1, kTypeCount = kMaxType + 1 };

struct SemaphoreScope : SyncExecScope {
    SemaphoreScope(QueueId qid, const SyncExecScope &exec_scope) : SyncExecScope(exec_scope), queue(qid) {}
    SemaphoreScope() = default;
    QueueId queue;
};

struct AcquiredImage {
    std::shared_ptr<const IMAGE_STATE> image;
    AccessAddressType address_type;
    subresource_adapter::ImageRangeGenerator generator;
    ResourceUsageTag present_tag;
    ResourceUsageTag acquire_tag;
    bool Invalid() const { return BASE_NODE::Invalid(image); }

    AcquiredImage() = default;
    AcquiredImage(const PresentedImage &presented, ResourceUsageTag acq_tag);
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
               const SyncExecScope &exec_scope_);
        Signal(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state_, const PresentedImage &presented, ResourceUsageTag acq_tag);

        std::shared_ptr<const SEMAPHORE_STATE> sem_state;
        std::shared_ptr<QueueBatchContext> batch;
        // Use the SyncExecScope::valid_accesses for first access scope
        SemaphoreScope first_scope;

        // Swapchain specific support signal info
        // IFF swapchain_image is non-null
        //     batch is the batch of the last present for the acquired image
        //     The address_type, range_generator pair further limit the scope of the resolve operation, and the "barrier" will
        //     also be special case (updating "PRESENTED" write with "ACQUIRE" read, as well as setting the barrier)
        AcquiredImage acquired;

        // TODO add timeline semaphore support.
    };

    using SignalMap = vvl::unordered_map<VkSemaphore, std::shared_ptr<Signal>>;
    using iterator = SignalMap::iterator;
    using const_iterator = SignalMap::const_iterator;
    using mapped_type = SignalMap::mapped_type;
    iterator begin() { return signaled_.begin(); }
    const_iterator begin() const { return signaled_.begin(); }
    iterator end() { return signaled_.end(); }
    const_iterator end() const { return signaled_.end(); }

    bool SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, const std::shared_ptr<QueueBatchContext> &batch,
                         const VkSemaphoreSubmitInfo &signal_info);
    bool Insert(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, std::shared_ptr<Signal> &&signal);
    bool SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, const PresentedImage &presented,
                         ResourceUsageTag acq_tag);
    std::shared_ptr<const Signal> Unsignal(VkSemaphore);
    void Resolve(SignaledSemaphores &parent, std::shared_ptr<QueueBatchContext> &last_batch);
    SignaledSemaphores() : prev_(nullptr) {}
    SignaledSemaphores(const SignaledSemaphores &prev) : prev_(&prev) {}

  private:
    void Import(VkSemaphore sem, std::shared_ptr<Signal> &&move_from);
    void Reset();
    std::shared_ptr<const Signal> GetPrev(VkSemaphore sem) const;
    vvl::unordered_map<VkSemaphore, std::shared_ptr<Signal>> signaled_;
    const SignaledSemaphores *prev_;  // Allowing this type to act as a writable overlay
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
        bool operator==(const OrderingBarrier &rhs) const {
            return (exec_scope == rhs.exec_scope) && (access_scope == rhs.access_scope);
        }
    };
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

    HazardResult DetectHazard(SyncStageAccessIndex usage_index) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, SyncOrdering ordering_rule, QueueId queue_id) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, const OrderingBarrier &ordering, QueueId queue_id) const;
    HazardResult DetectHazard(const ResourceAccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range) const;

    HazardResult DetectAsyncHazard(SyncStageAccessIndex usage_index, ResourceUsageTag start_tag) const;
    HazardResult DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                   ResourceUsageTag start_tag) const;

    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, QueueId queue_id, VkPipelineStageFlags2KHR source_exec_scope,
                                     const SyncStageAccessFlags &source_access_scope) const;
    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, const ResourceAccessState &scope_state,
                                     VkPipelineStageFlags2KHR source_exec_scope, const SyncStageAccessFlags &source_access_scope,
                                     QueueId event_queue, ResourceUsageTag event_tag) const;

    void Update(SyncStageAccessIndex usage_index, SyncOrdering ordering_rule, ResourceUsageTag tag);
    void SetWrite(const SyncStageAccessFlags &usage_bit, ResourceUsageTag tag);
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

    bool HasPendingState() const {
        return (0 != pending_layout_transition) || pending_write_barriers.any() || (0 != pending_write_dep_chain);
    }
    bool HasWriteOp() const { return last_write != 0; }
    const SyncStageAccessFlags &LastWriteOp() const { return last_write; }
    ResourceUsageTag LastWriteTag() const { return write_tag; }
    bool operator==(const ResourceAccessState &rhs) const {
        const bool write_same = (read_execution_barriers == rhs.read_execution_barriers) &&
                                (input_attachment_read == rhs.input_attachment_read) && (write_barriers == rhs.write_barriers) &&
                                (write_dependency_chain == rhs.write_dependency_chain) && (last_write == rhs.last_write) &&
                                (write_tag == rhs.write_tag) && (write_queue == rhs.write_queue);

        const bool read_write_same = write_same && (last_reads == rhs.last_reads) && (last_read_stages == rhs.last_read_stages);

        const bool same = read_write_same && (first_accesses_ == rhs.first_accesses_) &&
                          (first_read_stages_ == rhs.first_read_stages_) &&
                          (first_write_layout_ordering_ == rhs.first_write_layout_ordering_);

        return same;
    }
    bool operator!=(const ResourceAccessState &rhs) const { return !(*this == rhs); }
    VkPipelineStageFlags2KHR GetReadBarriers(const SyncStageAccessFlags &usage) const;
    SyncStageAccessFlags GetWriteBarriers() const { return write_barriers; }
    bool InSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const {
        return ReadInSourceScopeOrChain(src_exec_scope) || WriteInSourceScopeOrChain(src_exec_scope, src_access_scope);
    }
    void SetQueueId(QueueId id);

    bool WriteInChain(VkPipelineStageFlags2KHR src_exec_scope) const;
    bool WriteInScope(const SyncStageAccessFlags &src_access_scope) const;
    bool WriteBarrierInScope(const SyncStageAccessFlags &src_access_scope) const;
    bool WriteInChainedScope(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope) const;
    bool WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope, SyncStageAccessFlags src_access_scope) const;
    bool WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                        SyncStageAccessFlags src_access_scope) const;
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
    bool IsWriteHazard(SyncStageAccessFlags usage) const { return (usage & ~write_barriers).any(); }
    bool IsRAWHazard(VkPipelineStageFlags2KHR usage_stage, const SyncStageAccessFlags &usage) const;

    // Apply ordering scope to write hazard detection
    bool IsOrderedWriteHazard(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope) const {
        // Must be neither in the access scope, nor in the chained access scope
        return !WriteInScope(src_access_scope) && !WriteInChainedScope(src_exec_scope, src_access_scope);
    }

    bool IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                              const SyncStageAccessFlags &src_access_scope) const {
        // Special rules for sequential ILT's
        if (last_write == SYNC_IMAGE_LAYOUT_TRANSITION_BIT) {
            if (queue_id == write_queue) {
                // In queue, they are implicitly ordered
                return false;
            } else {
                // In dep chain means that the ILT is *available*
                return !WriteInChain(src_exec_scope);
            }
        }
        // Otherwise treat as an ordinary write hazard check with ordering rules.
        return IsOrderedWriteHazard(src_exec_scope, src_access_scope);
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
    VkPipelineStageFlags2 GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering) const;

    void UpdateFirst(ResourceUsageTag tag, SyncStageAccessIndex usage_index, SyncOrdering ordering_rule);
    void TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering);

    static const OrderingBarrier &GetOrderingRules(SyncOrdering ordering_enum) {
        return kOrderingRules[static_cast<size_t>(ordering_enum)];
    }

    // TODO: Add a NONE (zero) enum to SyncStageAccessFlags for input_attachment_read and last_write

    // With reads, each must be "safe" relative to it's prior write, so we need only
    // save the most recent write operation (as anything *transitively* unsafe would arleady
    // be included
    SyncStageAccessFlags write_barriers;              // union of applicable barrier masks since last write
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

using ResourceAddress = VkDeviceSize;
using ResourceAccessRangeMap = sparse_container::range_map<ResourceAddress, ResourceAccessState>;
using ResourceAccessRange = typename ResourceAccessRangeMap::key_type;
using ResourceAccessRangeIndex = typename ResourceAccessRange::index_type;
using ResourceRangeMergeIterator = sparse_container::parallel_iterator<ResourceAccessRangeMap, const ResourceAccessRangeMap>;

struct FenceSyncState {
    std::shared_ptr<const FENCE_STATE> fence;
    ResourceUsageTag tag;
    QueueId queue_id;
    AcquiredImage acquired;  // Iff queue == invalid and acquired.image valid.
    FenceSyncState();
    FenceSyncState(const FenceSyncState &other) = default;
    FenceSyncState(FenceSyncState &&other) = default;
    FenceSyncState &operator=(const FenceSyncState &other) = default;
    FenceSyncState &operator=(FenceSyncState &&other) = default;

    FenceSyncState(const std::shared_ptr<const FENCE_STATE> &fence_, QueueId queue_id_, ResourceUsageTag tag_);
    FenceSyncState(const std::shared_ptr<const FENCE_STATE> &fence_, const PresentedImage &image, ResourceUsageTag tag_);
};

class AttachmentViewGen {
  public:
    enum Gen { kViewSubresource = 0, kRenderArea = 1, kDepthOnlyRenderArea = 2, kStencilOnlyRenderArea = 3, kGenSize = 4 };
    AttachmentViewGen(const IMAGE_VIEW_STATE *view_, const VkOffset3D &offset, const VkExtent3D &extent);
    AttachmentViewGen(const AttachmentViewGen &other) = default;
    AttachmentViewGen(AttachmentViewGen &&other) = default;
    AccessAddressType GetAddressType() const;
    const IMAGE_VIEW_STATE *GetViewState() const { return view_; }
    const std::optional<ImageRangeGen> &GetRangeGen(Gen type) const;
    bool IsValid() const { return gen_store_[Gen::kViewSubresource].has_value(); }
    Gen GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const;

  private:
    const IMAGE_VIEW_STATE *view_ = nullptr;
    VkImageAspectFlags view_mask_ = 0U;
    std::array<std::optional<ImageRangeGen>, Gen::kGenSize> gen_store_;
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

class SyncOpBase {
  public:
    SyncOpBase() : cmd_type_(CMD_NONE) {}
    SyncOpBase(CMD_TYPE cmd_type) : cmd_type_(cmd_type) {}
    virtual ~SyncOpBase() = default;

    const char *CmdName() const { return CommandTypeString(cmd_type_); }

    virtual bool Validate(const CommandBufferAccessContext &cb_context) const = 0;
    virtual ResourceUsageTag Record(CommandBufferAccessContext *cb_context) = 0;
    virtual bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const = 0;
    virtual void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const = 0;

  protected:
    // Only non-null and valid for SyncOps within a render pass instance  WIP -- think about how to manage for non RPI calls within
    // RPI and 2ndarys...
    uint32_t subpass_ = VK_SUBPASS_EXTERNAL;
    CMD_TYPE cmd_type_;
};

class SyncOpBarriers : public SyncOpBase {
  protected:
    template <typename Barriers, typename FunctorFactory>
    static void ApplyBarriers(const Barriers &barriers, const FunctorFactory &factory, QueueId queue_id, ResourceUsageTag tag,
                              AccessContext *context);
    template <typename Barriers, typename FunctorFactory>
    static void ApplyGlobalBarriers(const Barriers &barriers, const FunctorFactory &factory, QueueId queue_id, ResourceUsageTag tag,
                                    AccessContext *access_context);

    SyncOpBarriers(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkPipelineStageFlags srcStageMask,
                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                   const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                   const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                   const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpBarriers(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t event_count,
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
    SyncOpPipelineBarrier(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                          uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpPipelineBarrier(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          const VkDependencyInfoKHR &pDependencyInfo);
    ~SyncOpPipelineBarrier() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;
};

class SyncOpWaitEvents : public SyncOpBarriers {
  public:
    SyncOpWaitEvents(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                     const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                     const VkImageMemoryBarrier *pImageMemoryBarriers);

    SyncOpWaitEvents(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfo);
    ~SyncOpWaitEvents() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;

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
    SyncOpResetEvent(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                     VkPipelineStageFlags2KHR stageMask);
    ~SyncOpResetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    std::shared_ptr<const EVENT_STATE> event_;
    SyncExecScope exec_scope_;
};

class SyncOpSetEvent : public SyncOpBase {
  public:
    SyncOpSetEvent(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   VkPipelineStageFlags2KHR stageMask, const AccessContext *access_context);
    SyncOpSetEvent(CMD_TYPE cmd_type, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   const VkDependencyInfoKHR &dep_info, const AccessContext *access_context);
    ~SyncOpSetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void DoRecord(QueueId queue_id, ResourceUsageTag recorded_tag, const std::shared_ptr<const AccessContext> &access_context,
                  SyncEventsContext *events_context) const;
    std::shared_ptr<const EVENT_STATE> event_;
    // The Access context of the command buffer at record set event time.
    std::shared_ptr<const AccessContext> recorded_context_;
    SyncExecScope src_exec_scope_;
    // Note that the dep info is *not* dehandled, but retained for comparison with a future WaitEvents2
    std::shared_ptr<safe_VkDependencyInfo> dep_info_;
};

class SyncOpBeginRenderPass : public SyncOpBase {
  public:
    SyncOpBeginRenderPass(CMD_TYPE cmd_type, const SyncValidator &sync_state, const VkRenderPassBeginInfo *pRenderPassBegin,
                          const VkSubpassBeginInfo *pSubpassBeginInfo);
    ~SyncOpBeginRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;
    const RenderPassAccessContext *GetRenderPassAccessContext() const { return rp_context_; }

  protected:
    safe_VkRenderPassBeginInfo renderpass_begin_info_;
    safe_VkSubpassBeginInfo subpass_begin_info_;
    std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> shared_attachments_;
    std::vector<const IMAGE_VIEW_STATE *> attachments_;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state_;
    const RenderPassAccessContext *rp_context_;
};

class SyncOpNextSubpass : public SyncOpBase {
  public:
    SyncOpNextSubpass(CMD_TYPE cmd_type, const SyncValidator &sync_state, const VkSubpassBeginInfo *pSubpassBeginInfo,
                      const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpNextSubpass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;

  protected:
    safe_VkSubpassBeginInfo subpass_begin_info_;
    safe_VkSubpassEndInfo subpass_end_info_;
};

class SyncOpEndRenderPass : public SyncOpBase {
  public:
    SyncOpEndRenderPass(CMD_TYPE cmd_type, const SyncValidator &sync_state, const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpEndRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                        ResourceUsageTag base_tag, CommandExecutionContext &exec_context) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag tag) const override;

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

    template <typename Detector, typename RangeGen>
    HazardResult DetectHazard(AccessAddressType type, Detector &detector, RangeGen &range_gen, DetectOptions options) const;
    template <typename Detector, typename RangeGen>
    HazardResult DetectHazard(AccessAddressType type, Detector &detector, const RangeGen &range_gen, DetectOptions options) const {
        RangeGen mutable_gen(range_gen);
        return DetectHazard<Detector, RangeGen>(type, detector, mutable_gen, options);
    }

    HazardResult DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index, const ResourceAccessRange &range) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceLayers &subresource, const VkOffset3D &offset, const VkExtent3D &extent,
                              bool is_depth_sliced) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                              const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                              bool is_depth_sliced, DetectOptions options) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    HazardResult DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const;

    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, SyncOrdering ordering_rule,
                              const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                                          VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope,
                                          QueueId queue_id, const SyncEventState &sync_event, DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const AttachmentViewGen &attachment_view, const SyncBarrier &barrier,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                          const SyncStageAccessFlags &src_access_scope,
                                          const VkImageSubresourceRange &subresource_range, DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const SyncImageMemoryBarrier &image_barrier) const;
    HazardResult DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const;

    void RecordLayoutTransitions(const RENDER_PASS_STATE &rp_state, uint32_t subpass,
                                 const AttachmentViewGenVector &attachment_views, ResourceUsageTag tag);
    void RecordRenderpassAsyncContextTags();

    HazardResult DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                      const AccessContext &access_context) const;

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
    template <typename ResolveOp, typename RangeGenerator>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context, AccessAddressType address_type,
                            RangeGenerator range_gen, const ResourceAccessState *infill_state = nullptr,
                            bool recur_to_infill = false);

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
    void Trim();
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;

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
                                   CMD_TYPE cmd_type) const;
    bool ValidateLoadOperation(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                               const VkRect2D &render_area, uint32_t subpass, const AttachmentViewGenVector &attachment_views,
                               CMD_TYPE cmd_type) const;
    bool ValidateStoreOperation(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                                const VkRect2D &render_area, uint32_t subpass, const AttachmentViewGenVector &attachment_views,
                                CMD_TYPE cmd_type) const;
    bool ValidateResolveOperations(const CommandExecutionContext &ex_context, const RENDER_PASS_STATE &rp_state,
                                   const VkRect2D &render_area, const AttachmentViewGenVector &attachment_views, CMD_TYPE cmd_type,
                                   uint32_t subpass) const;

    void SetStartTag(ResourceUsageTag tag) { start_tag_ = tag; }
    template <typename Action>
    void ForAll(Action &&action);
    template <typename Action>
    void ConstForAll(Action &&action) const;
    template <typename Predicate>
    void EraseIf(Predicate &&pred);

    // For use during queue submit building up the QueueBatchContext AccessContext for validation, otherwise clear.
    void AddAsyncContext(const AccessContext *context, ResourceUsageTag tag);
    // For use during queue submit to avoid stale pointers;
    void ClearAsyncContext(const AccessContext *context) { async_.clear(); }

    struct AsyncReference {
        const AccessContext *context;
        // For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
        // unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)
        ResourceUsageTag tag;  // Start of open ended asynchronous range
        AsyncReference(const AccessContext &async_context, ResourceUsageTag async_tag) : context(&async_context), tag(async_tag) {}
    };

  private:
    template <typename Detector>
    HazardResult DetectHazard(AccessAddressType type, Detector &detector, const ResourceAccessRange &range,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectAsyncHazard(AccessAddressType type, const Detector &detector, const ResourceAccessRange &range,
                                   ResourceUsageTag async_tag) const;
    template <typename Detector>
    HazardResult DetectPreviousHazard(AccessAddressType type, Detector &detector, const ResourceAccessRange &range) const;
    void UpdateAccessState(AccessAddressType type, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                           const ResourceAccessRange &range, ResourceUsageTag tag);

    MapArray access_state_maps_;
    std::vector<TrackBack> prev_;
    std::vector<TrackBack *> prev_by_subpass_;
    std::vector<AsyncReference> async_;
    TrackBack *src_external_;
    TrackBack dst_external_;
    ResourceUsageTag start_tag_;
};

struct SyncEventState {
    enum IgnoreReason { NotIgnored = 0, ResetWaitRace, Reset2WaitRace, SetRace, MissingStageBits, SetVsWait2, MissingSetEvent };
    using EventPointer = std::shared_ptr<const EVENT_STATE>;
    using ScopeMap = ResourceAccessRangeMap;
    EventPointer event;
    CMD_TYPE last_command;              // Only Event commands are valid here.
    ResourceUsageTag last_command_tag;  // Needed to filter replay validation
    CMD_TYPE unsynchronized_set;
    VkPipelineStageFlags2KHR barriers;
    SyncExecScope scope;
    ResourceUsageTag first_scope_tag;
    bool destroyed;
    std::shared_ptr<const AccessContext> first_scope;

    SyncEventState()
        : event(),
          last_command(CMD_NONE),
          last_command_tag(0),
          unsynchronized_set(CMD_NONE),
          barriers(0U),
          scope(),
          first_scope_tag(),
          destroyed(true) {}

    SyncEventState(const SyncEventState &) = default;
    SyncEventState(SyncEventState &&) = default;

    SyncEventState(const SyncEventState::EventPointer &event_state) : SyncEventState() {
        event = event_state;
        destroyed = (event.get() == nullptr) || event_state->Destroyed();
    }

    void ResetFirstScope();
    const ScopeMap &FirstScope(AccessAddressType address_type) const { return first_scope->GetAccessStateMap(address_type); }
    IgnoreReason IsIgnoredByWait(CMD_TYPE cmd_type, VkPipelineStageFlags2KHR srcStageMask) const;
    bool HasBarrier(VkPipelineStageFlags2KHR stageMask, VkPipelineStageFlags2KHR exec_scope) const;
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;
};

class SyncEventsContext {
  public:
    using Map = vvl::unordered_map<const EVENT_STATE *, std::shared_ptr<SyncEventState>>;
    using iterator = Map::iterator;
    using const_iterator = Map::const_iterator;

    SyncEventState *GetFromShared(const SyncEventState::EventPointer &event_state) {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            if (!event_state.get()) return nullptr;

            const auto *event_plain_ptr = event_state.get();
            auto sync_state = std::make_shared<SyncEventState>(event_state);
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

    void ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst, ResourceUsageTag tag);
    void ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag);

    void Destroy(const EVENT_STATE *event_state) {
        auto sync_it = map_.find(event_state);
        if (sync_it != map_.end()) {
            sync_it->second->destroyed = true;
            map_.erase(sync_it);
        }
    }
    void Clear() { map_.clear(); }

    SyncEventsContext &DeepCopy(const SyncEventsContext &from);
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;

  private:
    Map map_;
};

class RenderPassAccessContext {
  public:
    static AttachmentViewGenVector CreateAttachmentViewGen(const VkRect2D &render_area,
                                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views);
    RenderPassAccessContext() : rp_state_(nullptr), render_area_(VkRect2D()), current_subpass_(0) {}
    RenderPassAccessContext(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area, VkQueueFlags queue_flags,
                            const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const AccessContext *external_context);

    bool ValidateDrawSubpassAttachment(const CommandExecutionContext &ex_context, const CMD_BUFFER_STATE &cmd_buffer,
                                       CMD_TYPE cmd_type) const;
    void RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd_buffer, ResourceUsageTag tag);
    bool ValidateNextSubpass(const CommandExecutionContext &ex_context, CMD_TYPE cmd_type) const;
    bool ValidateEndRenderPass(const CommandExecutionContext &ex_context, CMD_TYPE cmd_type) const;
    bool ValidateFinalSubpassLayoutTransitions(const CommandExecutionContext &ex_context, CMD_TYPE cmd_type) const;

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

  private:
    const RENDER_PASS_STATE *rp_state_;
    const VkRect2D render_area_;
    uint32_t current_subpass_;
    std::vector<AccessContext> subpass_contexts_;
    AttachmentViewGenVector attachment_views_;
};

// Command execution context is the base class for command buffer and queue contexts
// Preventing unintented leakage of subclass specific state, storing enough information
// for message logging.
// TODO: determine where to draw the design split for tag tracking (is there anything command to Queues and CB's)
class CommandExecutionContext {
  public:
    using AccessLog = std::vector<ResourceUsageRecord>;
    using CommandBufferSet = vvl::unordered_set<std::shared_ptr<const CMD_BUFFER_STATE>>;
    CommandExecutionContext() : sync_state_(nullptr) {}
    CommandExecutionContext(const SyncValidator *sync_validator) : sync_state_(sync_validator) {}
    virtual ~CommandExecutionContext() = default;
    virtual AccessContext *GetCurrentAccessContext() = 0;
    virtual SyncEventsContext *GetCurrentEventsContext() = 0;
    virtual const AccessContext *GetCurrentAccessContext() const = 0;
    virtual const SyncEventsContext *GetCurrentEventsContext() const = 0;
    virtual QueueId GetQueueId() const = 0;

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

    virtual HazardResult DetectFirstUseHazard(const ResourceUsageRange &tag_range) = 0;
    virtual void BeginRenderPassReplay(const SyncOpBeginRenderPass &begin_op, ResourceUsageTag tag) {
        assert("Must override if use by derived type is valid" == nullptr);
    }
    virtual void NextSubpassReplay() { assert("Must override if use by derived type is valid" == nullptr); }
    virtual void EndRenderPassReplay() { assert("Must override if use by derived type is valid" == nullptr); }

    bool ValidForSyncOps() const;

  protected:
    class ReplayGuard {
      public:
        ReplayGuard(CommandExecutionContext &exec_context, const CommandBufferAccessContext &recorded_context)
            : exec_context_(exec_context) {
            exec_context_.BeginCommandBufferReplay(recorded_context);
        }
        ~ReplayGuard() { exec_context_.EndCommandBufferReplay(); }

      private:
        CommandExecutionContext &exec_context_;
    };
    friend ReplayGuard;

    const SyncValidator *sync_state_;
    const CommandBufferAccessContext *current_replay_;

  private:
    // Only allow the replay guard to manage the begin/end
    void BeginCommandBufferReplay(const CommandBufferAccessContext &recorded) { current_replay_ = &recorded; }
    void EndCommandBufferReplay() { current_replay_ = nullptr; }
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
          access_log_(std::make_shared<AccessLog>()),
          cbs_referenced_(std::make_shared<CommandBufferSet>()),
          command_number_(0),
          subcommand_number_(0),
          reset_count_(0),
          cb_access_context_(),
          current_context_(&cb_access_context_),
          events_context_(),
          render_pass_contexts_(),
          current_renderpass_context_(),
          sync_ops_() {}
    CommandBufferAccessContext(SyncValidator &sync_validator, CMD_BUFFER_STATE *cb_state)
        : CommandBufferAccessContext(&sync_validator) {
        cb_state_ = cb_state;
    }

    struct AsProxyContext {};
    CommandBufferAccessContext(const CommandBufferAccessContext &real_context, AsProxyContext dummy);

    // NOTE: because this class is encapsulated in syncval_state::CommandBuffer, it isn't safe
    // to use shared_from_this from the constructor.
    void SetSelfReference() { cbs_referenced_->insert(cb_state_->shared_from_this()); }

    ~CommandBufferAccessContext() override = default;
    CommandExecutionContext &GetExecutionContext() { return *this; }
    const CommandExecutionContext &GetExecutionContext() const { return *this; }

    void Destroy() {
        // the cb self reference must be cleared or the command buffer reference count will never go to 0
        cbs_referenced_.reset();
        cb_state_ = nullptr;
    }

    void Reset() {
        access_log_ = std::make_shared<AccessLog>();
        cbs_referenced_ = std::make_shared<CommandBufferSet>();
        if (cb_state_) {
            cbs_referenced_->insert(cb_state_->shared_from_this());
        }
        sync_ops_.clear();
        command_number_ = 0;
        subcommand_number_ = 0;
        reset_count_++;
        command_handles_.clear();
        cb_access_context_.Reset();
        render_pass_contexts_.clear();
        current_context_ = &cb_access_context_;
        current_renderpass_context_ = nullptr;
        events_context_.Clear();
    }

    std::string FormatUsage(ResourceUsageTag tag) const override;
    std::string FormatUsage(const ResourceFirstAccess &access) const;  //  Only command buffers have "first usage"
    AccessContext *GetCurrentAccessContext() override { return current_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    QueueId GetQueueId() const override;

    RenderPassAccessContext *GetCurrentRenderPassContext() { return current_renderpass_context_; }
    const RenderPassAccessContext *GetCurrentRenderPassContext() const { return current_renderpass_context_; }
    ResourceUsageTag RecordBeginRenderPass(CMD_TYPE cmd_type, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views);

    bool ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, CMD_TYPE cmd_type) const;
    void RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, ResourceUsageTag tag);
    bool ValidateDrawVertex(uint32_t vertexCount, uint32_t firstVertex, CMD_TYPE cmd_type) const;
    void RecordDrawVertex(uint32_t vertexCount, uint32_t firstVertex, ResourceUsageTag tag);
    bool ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, CMD_TYPE cmd_type) const;
    void RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, ResourceUsageTag tag);
    bool ValidateDrawSubpassAttachment(CMD_TYPE cmd_type) const;
    void RecordDrawSubpassAttachment(ResourceUsageTag tag);
    ResourceUsageTag RecordNextSubpass(CMD_TYPE cmd_type);
    ResourceUsageTag RecordEndRenderPass(CMD_TYPE cmd_type);
    void RecordDestroyEvent(EVENT_STATE *event_state);

    bool ValidateFirstUse(CommandExecutionContext &exec_context, const char *func_name, uint32_t index) const;
    void RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_context);
    void ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    HazardResult DetectFirstUseHazard(const ResourceUsageRange &tag_range) override;

    const CMD_BUFFER_STATE *GetCommandBufferState() const { return cb_state_; }
    VkQueueFlags GetQueueFlags() const { return cb_state_ ? cb_state_->GetQueueFlags() : 0; }

    ResourceUsageTag NextSubcommandTag(CMD_TYPE command, ResourceUsageRecord::SubcommandType subcommand);
    ResourceUsageTag NextSubcommandTag(CMD_TYPE command, NamedHandle &&handle, ResourceUsageRecord::SubcommandType subcommand);

    ResourceUsageTag GetTagLimit() const override { return access_log_->size(); }
    VulkanTypedHandle Handle() const override {
        if (cb_state_) {
            return cb_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkCommandBuffer>(VK_NULL_HANDLE), kVulkanObjectTypeCommandBuffer);
    }

    ResourceUsageTag NextCommandTag(CMD_TYPE command, NamedHandle &&handle,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);

    ResourceUsageTag NextCommandTag(CMD_TYPE command,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);
    ResourceUsageTag NextIndexedCommandTag(CMD_TYPE command, uint32_t index);

    // NamedHandle must be constructable from args
    template <class... Args>
    void AddHandle(ResourceUsageTag tag, Args &&...args) {
        assert(tag < access_log_->size());
        if (tag < access_log_->size()) {
            (*access_log_)[tag].AddHandle(std::forward<Args>(args)...);
        }
    }

    std::shared_ptr<const CMD_BUFFER_STATE> GetCBStateShared() const { return cb_state_->shared_from_this(); }

    const CMD_BUFFER_STATE &GetCBState() const {
        assert(cb_state_);
        return *cb_state_;
    }
    CMD_BUFFER_STATE &GetCBState() {
        assert(cb_state_);
        return *cb_state_;
    }

    template <class T, class... Args>
    void RecordSyncOp(Args &&...args) {
        // T must be as derived from SyncOpBase or the compiler will flag the next line as an error.
        SyncOpPointer sync_op(std::make_shared<T>(std::forward<Args>(args)...));
        RecordSyncOp(std::move(sync_op));  // Call the non-template version
    }
    const AccessLog &GetAccessLog() const { return *access_log_; }
    std::shared_ptr<AccessLog> GetAccessLogShared() const { return access_log_; }
    std::shared_ptr<CommandBufferSet> GetCBReferencesShared() const { return cbs_referenced_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;
    const std::vector<SyncOpEntry> &GetSyncOps() const { return sync_ops_; };

  private:
    // As this is passing around a shared pointer to record, move to avoid needless atomics.
    void RecordSyncOp(SyncOpPointer &&sync_op);
    // Note: since every CommandBufferAccessContext is encapsulated in its CommandBuffer object,
    // a reference count is not needed here.
    CMD_BUFFER_STATE *cb_state_;

    std::shared_ptr<AccessLog> access_log_;
    std::shared_ptr<CommandBufferSet> cbs_referenced_;
    uint32_t command_number_;
    uint32_t subcommand_number_;
    uint32_t reset_count_;
    small_vector<NamedHandle, 1> command_handles_;

    AccessContext cb_access_context_;
    AccessContext *current_context_;
    SyncEventsContext events_context_;

    // Don't need the following for an active proxy cb context
    std::vector<std::unique_ptr<RenderPassAccessContext>> render_pass_contexts_;
    RenderPassAccessContext *current_renderpass_context_;
    std::vector<SyncOpEntry> sync_ops_;
};

namespace syncval_state {
class CommandBuffer : public CMD_BUFFER_STATE {
  public:
    CommandBufferAccessContext access_context;

    CommandBuffer(SyncValidator *dev, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                  const COMMAND_POOL_STATE *pool);
    ~CommandBuffer();

    void NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) override;

    void Destroy() override;
    void Reset() override;

  private:
    void ResetCBState();
};
}  // namespace syncval_state
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, syncval_state::CommandBuffer, CMD_BUFFER_STATE);

class QueueSyncState;

// Store references to ResourceUsageRecords with global tag range within a batch
class BatchAccessLog {
  public:
    struct BatchRecord {
        BatchRecord() = default;
        BatchRecord(const BatchRecord &other) = default;
        BatchRecord(BatchRecord &&other) = default;
        BatchRecord(const QueueSyncState &q, uint64_t submit, uint32_t batch)
            : queue(&q), submit_index(submit), batch_index(batch), cb_index(0), bias(0) {}
        BatchRecord &operator=(const BatchRecord &other) = default;
        const QueueSyncState *queue;
        uint64_t submit_index;
        uint32_t batch_index;
        uint32_t cb_index;
        ResourceUsageTag bias;
    };

    struct AccessRecord {
        const BatchRecord *batch;
        const ResourceUsageRecord *record;
        bool IsValid() const { return batch && record; }
    };

    struct CBSubmitLog {
      public:
        CBSubmitLog() = default;
        CBSubmitLog(const CBSubmitLog &batch) = default;
        CBSubmitLog(CBSubmitLog &&other) = default;
        CBSubmitLog &operator=(const CBSubmitLog &other) = default;
        CBSubmitLog &operator=(CBSubmitLog &&other) = default;
        CBSubmitLog(const BatchRecord &batch, std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs,
                    std::shared_ptr<const CommandExecutionContext::AccessLog> log)
            : batch_(batch), cbs_(cbs), log_(log) {}
        CBSubmitLog(const BatchRecord &batch, const CommandBufferAccessContext &cb)
            : CBSubmitLog(batch, cb.GetCBReferencesShared(), cb.GetAccessLogShared()) {}

        size_t Size() const { return log_->size(); }
        const BatchRecord &GetBatch() const { return batch_; }
        AccessRecord operator[](ResourceUsageTag tag) const;

      private:
        BatchRecord batch_;
        std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs_;
        std::shared_ptr<const CommandExecutionContext::AccessLog> log_;
    };

    ResourceUsageTag Import(const BatchRecord &batch, const CommandBufferAccessContext &cb_access);
    void Import(const BatchAccessLog &other);
    void Insert(const BatchRecord &batch, const ResourceUsageRange &range,
                std::shared_ptr<const CommandExecutionContext::AccessLog> log);

    void Trim(const ResourceUsageTagSet &used);
    // AccessRecord lookup is based on global tags
    AccessRecord operator[](ResourceUsageTag tag) const;
    BatchAccessLog() {}

  private:
    using CBSubmitLogRangeMap = sparse_container::range_map<ResourceUsageTag, CBSubmitLog>;
    CBSubmitLogRangeMap log_map_;
};

struct PresentedImageRecord {
    ResourceUsageTag tag;  // the global tag at presentation
    uint32_t image_index;
    uint32_t present_index;
    std::shared_ptr<const syncval_state::Swapchain> swapchain_state;
    std::shared_ptr<const IMAGE_STATE> image;
};

struct PresentedImage : public PresentedImageRecord {
    std::shared_ptr<QueueBatchContext> batch;
    subresource_adapter::ImageRangeGenerator range_gen;
    AccessAddressType address_type = AccessAddressType::kLinear;

    PresentedImage() = default;
    void UpdateMemoryAccess(SyncStageAccessIndex usage, ResourceUsageTag tag, AccessContext &access_context) const;
    PresentedImage(const SyncValidator &sync_state, const std::shared_ptr<QueueBatchContext> batch, VkSwapchainKHR swapchain,
                   uint32_t image_index, uint32_t present_index, ResourceUsageTag present_tag_);
    // For non-previsously presented images..
    PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index);

    bool Invalid() const { return BASE_NODE::Invalid(image); }
    void ExportToSwapchain(SyncValidator &);
    void SetImage(uint32_t at_index);
};
using PresentedImages = std::vector<PresentedImage>;

namespace syncval_state {
class Swapchain : public SWAPCHAIN_NODE {
  public:
    Swapchain(ValidationStateTracker *dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR swapchain);
    ~Swapchain() { Destroy(); }
    void RecordPresentedImage(PresentedImage &&presented_images);
    PresentedImage MovePresentedImage(uint32_t image_index);
    std::shared_ptr<const Swapchain> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Swapchain> shared_from_this() { return SharedFromThisImpl(this); }

  private:
    PresentedImages presented;  // Build this on demand
};
}  // namespace syncval_state
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSwapchainKHR, syncval_state::Swapchain, SWAPCHAIN_NODE);

class QueueBatchContext : public CommandExecutionContext {
  public:
    struct RenderPassReplayState {
        // A minimal subset of the functionality present in the RenderPassAccessContext. Since the accesses are recorded in the
        // first_use information of the recorded access contexts, s.t. all we need to support is the barrier/resolve operations
        RenderPassReplayState() { Reset(); }
        AccessContext *Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op_,
                             const AccessContext &external_context);
        AccessContext *Next();
        void End(AccessContext &external_context);

        const SyncOpBeginRenderPass *begin_op = nullptr;
        const AccessContext *replay_context = nullptr;
        uint32_t subpass = VK_SUBPASS_EXTERNAL;
        std::vector<AccessContext> subpass_contexts;
        void Reset() {
            begin_op = nullptr;
            replay_context = nullptr;
            subpass = VK_SUBPASS_EXTERNAL;
            subpass_contexts.clear();
        }
        operator bool() const { return begin_op != nullptr; }
    };

    class PresentResourceRecord : public AlternateResourceUsage::RecordBase {
      public:
        using Base_ = AlternateResourceUsage::RecordBase;
        Base_::Record MakeRecord() const override;
        ~PresentResourceRecord() override {}
        PresentResourceRecord(const PresentedImageRecord &presented) : presented_(presented) {}
        std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const override;

      private:
        PresentedImageRecord presented_;
    };

    class AcquireResourceRecord : public AlternateResourceUsage::RecordBase {
      public:
        using Base_ = AlternateResourceUsage::RecordBase;
        Base_::Record MakeRecord() const override;
        AcquireResourceRecord(const PresentedImage &presented, ResourceUsageTag tag, const char *func_name)
            : presented_(presented), acquire_tag_(tag), func_name_(func_name) {}
        std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const override;

      private:
        PresentedImage presented_;
        ResourceUsageTag acquire_tag_;
        std::string func_name_;
    };

    using ConstBatchSet = vvl::unordered_set<std::shared_ptr<const QueueBatchContext>>;
    using BatchSet = vvl::unordered_set<std::shared_ptr<QueueBatchContext>>;
    static constexpr bool TruePred(const std::shared_ptr<const QueueBatchContext> &) { return true; }
    struct CmdBufferEntry {
        uint32_t index = 0;
        std::shared_ptr<const syncval_state::CommandBuffer> cb;
        CmdBufferEntry(uint32_t index_, std::shared_ptr<const syncval_state::CommandBuffer> &&cb_)
            : index(index_), cb(std::move(cb_)) {}
    };

    using CommandBuffers = std::vector<CmdBufferEntry>;

    QueueBatchContext(const SyncValidator &sync_state, const QueueSyncState &queue_state, uint64_t submit_index,
                      uint32_t batch_index);
    QueueBatchContext(const SyncValidator &sync_state);
    QueueBatchContext() = delete;
    void Trim();

    std::string FormatUsage(ResourceUsageTag tag) const override;
    AccessContext *GetCurrentAccessContext() override { return current_access_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_access_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    const QueueSyncState *GetQueueSyncState() const { return queue_state_; }
    VkQueueFlags GetQueueFlags() const;
    QueueId GetQueueId() const override;

    void SetupBatchTags(const ResourceUsageRange &tag_range);
    void SetupBatchTags();
    void ResetEventsContext() { events_context_.Clear(); }
    ResourceUsageTag GetTagLimit() const override { return batch_.bias; }
    // begin is the tag bias  / .size() is the number of total records that should eventually be in access_log_
    ResourceUsageRange GetTagRange() const { return tag_range_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;

    void SetTagBias(ResourceUsageTag);
    // For Submit
    void SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const VkSubmitInfo2 &submit_info,
                            SignaledSemaphores &signaled_semaphores);
    void SetupCommandBufferInfo(const VkSubmitInfo2 &submit_info);
    bool DoQueueSubmitValidate(const SyncValidator &sync_state, QueueSubmitCmdState &cmd_state, const VkSubmitInfo2 &submit_info);
    void ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    // For Present
    void SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const VkPresentInfoKHR &present_info,
                            const PresentedImages &presented_images, SignaledSemaphores &signaled);
    bool DoQueuePresentValidate(const char *func_name, const PresentedImages &presented_images);
    void DoPresentOperations(const PresentedImages &presented_images);
    void LogPresentOperations(const PresentedImages &presented_images);

    // For Acquire
    void SetupAccessContext(const PresentedImage &presented);
    void DoAcquireOperation(const PresentedImage &presented);
    void LogAcquireOperation(const PresentedImage &presented, const char *func_name);

    VulkanTypedHandle Handle() const override;

    template <typename Predicate>
    void ApplyPredicatedWait(Predicate &predicate);
    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyAcquireWait(const AcquiredImage &acquired);

    HazardResult DetectFirstUseHazard(const ResourceUsageRange &tag_range) override;
    void BeginRenderPassReplay(const SyncOpBeginRenderPass &begin_op, ResourceUsageTag tag) override;
    void NextSubpassReplay() override;
    void EndRenderPassReplay() override;

    void Cleanup();

  private:
    void CommonSetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev,
                                  QueueBatchContext::ConstBatchSet &batches_resolved);
    std::shared_ptr<QueueBatchContext> ResolveOneWaitSemaphore(VkSemaphore sem, const PresentedImages &presented_images,
                                                               SignaledSemaphores &signaled);
    std::shared_ptr<QueueBatchContext> ResolveOneWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask,
                                                               SignaledSemaphores &signaled);

    void ImportSyncTags(const QueueBatchContext &from);
    const QueueSyncState *queue_state_ = nullptr;
    ResourceUsageRange tag_range_ = ResourceUsageRange(0, 0);  // Range of tags referenced by cbs_referenced

    AccessContext access_context_;
    AccessContext *current_access_context_;
    SyncEventsContext events_context_;
    BatchAccessLog batch_log_;
    std::vector<ResourceUsageTag> queue_sync_tag_;

    // Clear these after validation and import, not valid after.
    BatchAccessLog::BatchRecord batch_;  // Holds the cumulative tag bias, and command buffer counts for Import support.
    CommandBuffers command_buffers_;
    ConstBatchSet async_batches_;
    RenderPassReplayState rp_replay_;
};

class QueueSyncState {
  public:
    constexpr static QueueId kQueueIdBase = QueueId(0);
    constexpr static QueueId kQueueIdInvalid = ~kQueueIdBase;
    constexpr static QueueId kQueueAny = kQueueIdInvalid - 1;
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
    void UpdateLastBatch(std::shared_ptr<QueueBatchContext> &&last);
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

// The converter needs to be more complex than simply an array of VkSubmitInfo2 structures.
// In order to convert from Info->Info2, arrays of VkSemaphoreSubmitInfo and VkCommandBufferSubmitInfo
// structures must be created for the pWaitSemaphoreInfos, pCommandBufferInfos, and pSignalSemaphoreInfos
// which comprise the converted VkSubmitInfo information. The created VkSubmitInfo2 structure then references the storage
// of the arrays, which must have a lifespan longer than the conversion, s.t. the ensuing valdation/record operations
// can reference them.  The resulting VkSubmitInfo2 is then copied into an additional which takes the place of the pSubmits
// parameter.
struct SubmitInfoConverter {
    struct BatchStore {
        BatchStore(const VkSubmitInfo &info, VkQueueFlags queue_flags);

        static VkSemaphoreSubmitInfo WaitSemaphore(const VkSubmitInfo &info, uint32_t index);
        static VkCommandBufferSubmitInfo CommandBuffer(const VkSubmitInfo &info, uint32_t index);
        static VkSemaphoreSubmitInfo SignalSemaphore(const VkSubmitInfo &info, uint32_t index, VkQueueFlags queue_flags);

        std::vector<VkSemaphoreSubmitInfo> waits;
        std::vector<VkCommandBufferSubmitInfo> cbs;
        std::vector<VkSemaphoreSubmitInfo> signals;
        VkSubmitInfo2 info2;
    };

    SubmitInfoConverter(uint32_t count, const VkSubmitInfo *infos, VkQueueFlags queue_flags);

    std::vector<BatchStore> info_store;
    std::vector<VkSubmitInfo2> info2s;
};

class SyncValidator : public ValidationStateTracker, public SyncStageAccess {
  public:
    using StateTracker = ValidationStateTracker;
    SyncValidator() { container_type = LayerObjectTypeSyncValidation; }

    // Global tag range for submitted command buffers resource usage logs
    // Started the global tag count at 1 s.t. zero are invalid and ResourceUsageTag normalization can just zero them.
    mutable std::atomic<ResourceUsageTag> tag_limit_{1};  // This is reserved in Validation phase, thus mutable and atomic
    ResourceUsageRange ReserveGlobalTagRange(size_t tag_count) const;  // Note that the tag_limit_ is mutable this has side effects

    using QueueSyncStatesMap = vvl::unordered_map<VkQueue, std::shared_ptr<QueueSyncState>>;
    vvl::unordered_map<VkQueue, std::shared_ptr<QueueSyncState>> queue_sync_states_;
    QueueId queue_id_limit_ = QueueSyncState::kQueueIdBase;
    SignaledSemaphores signaled_semaphores_;

    using SignaledFences = vvl::unordered_map<VkFence, FenceSyncState>;
    using SignaledFence = SignaledFences::value_type;
    SignaledFences waitable_fences_;

    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyAcquireWait(const AcquiredImage &acquired);
    template <typename BatchOp>
    void ForAllQueueBatchContexts(BatchOp &&op);

    void UpdateFenceWaitInfo(VkFence fence, QueueId queue_id, ResourceUsageTag tag);
    void UpdateFenceWaitInfo(VkFence fence, const PresentedImage &image, ResourceUsageTag tag);
    void UpdateFenceWaitInfo(std::shared_ptr<const FENCE_STATE> &fence, FenceSyncState &&wait_info);

    void WaitForFence(VkFence fence);

    const QueueSyncState *GetQueueSyncState(VkQueue queue) const;
    QueueSyncState *GetQueueSyncState(VkQueue queue);
    std::shared_ptr<const QueueSyncState> GetQueueSyncStateShared(VkQueue queue) const;
    std::shared_ptr<QueueSyncState> GetQueueSyncStateShared(VkQueue queue);
    QueueId GetQueueIdLimit() const { return queue_id_limit_; }

    QueueBatchContext::BatchSet GetQueueBatchSnapshot();

    template <typename Predicate>
    QueueBatchContext::ConstBatchSet GetQueueLastBatchSnapshot(Predicate &&pred) const;
    QueueBatchContext::ConstBatchSet GetQueueLastBatchSnapshot() const {
        return GetQueueLastBatchSnapshot(QueueBatchContext::TruePred);
    };

    template <typename Predicate>
    QueueBatchContext::BatchSet GetQueueLastBatchSnapshot(Predicate &&pred);
    QueueBatchContext::BatchSet GetQueueLastBatchSnapshot() { return GetQueueLastBatchSnapshot(QueueBatchContext::TruePred); };

    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                           const COMMAND_POOL_STATE *cmd_pool) override;
    std::shared_ptr<SWAPCHAIN_NODE> CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                         VkSwapchainKHR swapchain) final;

    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd_type);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                              const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE command);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd_type);
    bool SupressedBoundDescriptorWAW(const HazardResult &hazard) const;

    void CreateDevice(const VkDeviceCreateInfo *pCreateInfo) override;

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd_type) const;

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

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                          const VkCopyBufferInfo2KHR *pCopyBufferInfos) const override;
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
                                         uint32_t imageMemoryBarrierCount,
                                         const VkImageMemoryBarrier *pImageMemoryBarriers) override;

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
                                const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd_type) const;
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

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd_type) const;
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
                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkBufferImageCopy *pRegions) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                            const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                      CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount,
                                             const VkBufferImageCopy *pRegions) const override;
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
                              CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                            CMD_TYPE cmd_type);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions,
                                   VkFilter filter) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) override;

    bool ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                CMD_TYPE cmd_type) const;
    void RecordIndirectBuffer(AccessContext &context, ResourceUsageTag tag, const VkDeviceSize struct_size, const VkBuffer buffer,
                              const VkDeviceSize offset, const uint32_t drawCount, uint32_t stride);

    bool ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                             VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, CMD_TYPE cmd_type) const;
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
                                      CMD_TYPE cmd_type) const;
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
                                             uint32_t stride, CMD_TYPE cmd_type) const;
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

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                            const VkResolveImageInfo2KHR *pResolveImageInfo) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                         const VkResolveImageInfo2 *pResolveImageInfo) const override;
    bool ValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                  CMD_TYPE cmd_type) const;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo) override;
    void RecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo, CMD_TYPE cmd_type);

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

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) const override;
    ResourceUsageRange SetupPresentInfo(const VkPresentInfoKHR &present_info, std::shared_ptr<QueueBatchContext> &batch,
                                        PresentedImages &presented_images) const;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) override;
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t *pImageIndex, VkResult result) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex,
                                            VkResult result) override;
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t *pImageIndex, VkResult result, const char *func_name);
    bool ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                             const char *func_name) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                    VkFence fence) const override;
    void RecordQueueSubmit(VkQueue queue, VkFence fence, VkResult result);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   VkResult result) override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                        VkFence fence) const override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       VkResult result) override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                     VkFence fence) const override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                    VkResult result) override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                     uint64_t timeout, VkResult result) override;
};
