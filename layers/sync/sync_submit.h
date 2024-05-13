/*
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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
#include "sync/sync_commandbuffer.h"
#include "state_tracker/queue_state.h"

struct PresentedImage;
class QueueBatchContext;
struct QueueSubmitCmdState;
class QueueSyncState;
class SyncValidator;

namespace vvl {
class Semaphore;
}  // namespace vvl

struct AcquiredImage {
    std::shared_ptr<const syncval_state::ImageState> image;
    subresource_adapter::ImageRangeGenerator generator;
    ResourceUsageTag present_tag;
    ResourceUsageTag acquire_tag;
    bool Invalid() const;

    AcquiredImage() = default;
    AcquiredImage(const PresentedImage &presented, ResourceUsageTag acq_tag);
};

// Information associated with a semaphore signal
struct SignalInfo {
    SignalInfo(const std::shared_ptr<QueueBatchContext> &batch, const SyncExecScope &exec_scope);
    SignalInfo(const PresentedImage &presented, ResourceUsageTag acquire_tag);

    // Batch from the first scope of the signal.
    std::shared_ptr<QueueBatchContext> batch;

    // Use the SyncExecScope::valid_accesses for first access scope
    SemaphoreScope first_scope;

    // Swapchain specific signal info.
    // Batch field is the batch of the last present for the acquired image.
    // The AcquiredImage further limits the scope of the resolve operation, and the "barrier" will also
    // be special case (updating "PRESENTED" write with "ACQUIRE" read, as well as setting the barrier).
    //
    // NOTE: shared_ptr is used here as a memory saver. AcquiredImage is 224 bytes at the time
    // of writing and is not used in the queue submit signals. If we optimize ImageRangeGenerator
    // memory usage then shared_ptr can be replaced by std::optional to avoid allocation.
    std::shared_ptr<AcquiredImage> acquired_image;
};

// Globally tracks signaled semaphores.
using SignaledSemaphores = vvl::unordered_map<VkSemaphore, SignalInfo>;

// The list of changes that should to be applied to SignaledSemaphores.
// These changes are collected during validation phase of QueueSubmit and are applied in the record phase.
struct SignaledSemaphoresUpdate {
    vvl::unordered_map<VkSemaphore, SignalInfo> signals_to_add;
    vvl::unordered_set<VkSemaphore> signals_to_remove;

    void OnSignal(const std::shared_ptr<QueueBatchContext> &batch, const VkSemaphoreSubmitInfo &signal_info);
    std::optional<SignalInfo> OnUnsignal(VkSemaphore semaphore);

    SignaledSemaphoresUpdate(const SyncValidator &sync_validator) : sync_validator_(sync_validator) {}

  private:
    const SyncValidator &sync_validator_;
};

struct FenceSyncState {
    std::shared_ptr<const vvl::Fence> fence;
    ResourceUsageTag tag;
    QueueId queue_id;
    AcquiredImage acquired;  // Iff queue == invalid and acquired.image valid.
    FenceSyncState();
    FenceSyncState(const FenceSyncState &other) = default;
    FenceSyncState(FenceSyncState &&other) = default;
    FenceSyncState &operator=(const FenceSyncState &other) = default;
    FenceSyncState &operator=(FenceSyncState &&other) = default;

    FenceSyncState(const std::shared_ptr<const vvl::Fence> &fence_, QueueId queue_id_, ResourceUsageTag tag_);
    FenceSyncState(const std::shared_ptr<const vvl::Fence> &fence_, const PresentedImage &image, ResourceUsageTag tag_);
};

struct PresentedImageRecord {
    ResourceUsageTag tag;  // the global tag at presentation
    uint32_t image_index;
    uint32_t present_index;
    std::weak_ptr<const syncval_state::Swapchain> swapchain_state;
    std::shared_ptr<const syncval_state::ImageState> image;
};

struct PresentedImage : public PresentedImageRecord {
    std::shared_ptr<QueueBatchContext> batch;
    subresource_adapter::ImageRangeGenerator range_gen;

    PresentedImage() = default;
    void UpdateMemoryAccess(SyncStageAccessIndex usage, ResourceUsageTag tag, AccessContext &access_context) const;
    PresentedImage(const SyncValidator &sync_state, const std::shared_ptr<QueueBatchContext> batch, VkSwapchainKHR swapchain,
                   uint32_t image_index, uint32_t present_index, ResourceUsageTag present_tag_);
    // For non-previsously presented images..
    PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index);
    bool Invalid() const;
    void ExportToSwapchain(SyncValidator &);
    void SetImage(uint32_t at_index);
};
using PresentedImages = std::vector<PresentedImage>;

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
        const DebugNameProvider *debug_name_provider;
        bool IsValid() const { return batch && record; }
    };

    struct CBSubmitLog : DebugNameProvider {
      public:
        CBSubmitLog() = default;
        CBSubmitLog(const CBSubmitLog &batch) = default;
        CBSubmitLog(CBSubmitLog &&other) = default;
        CBSubmitLog &operator=(const CBSubmitLog &other) = default;
        CBSubmitLog &operator=(CBSubmitLog &&other) = default;
        CBSubmitLog(const BatchRecord &batch, std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs,
                    std::shared_ptr<const CommandExecutionContext::AccessLog> log);
        CBSubmitLog(const BatchRecord &batch, const CommandBufferAccessContext &cb,
                    const std::vector<std::string> &initial_label_stack);
        size_t Size() const { return log_->size(); }
        AccessRecord operator[](ResourceUsageTag tag) const;

        // DebugNameProvider
        std::string GetDebugRegionName(const ResourceUsageRecord &record) const override;

      private:
        BatchRecord batch_;
        std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs_;
        std::shared_ptr<const CommandExecutionContext::AccessLog> log_;
        // label stack at the point when command buffer is submitted to the queue
        std::vector<std::string> initial_label_stack_;

        // TODO: remove this field and use (*cbs_)[0]->GetLabelCommands() directly
        // when timeline semaphore support is implemented.
        //
        // Until then, there is no guarantee command buffers stored in cbs_ are what
        // they are supposed to be when timeline semaphores are used (they can be reused
        // after wait on timeline semaphore). When this happens, validation might report
        // false positives (which is okay for unsupported feeature), but label code can crash.
        // Make a copy of label commands as a temporary protection measure.
        std::vector<vvl::CommandBuffer::LabelCommand> label_commands_;
    };

    ResourceUsageTag Import(const BatchRecord &batch, const CommandBufferAccessContext &cb_access,
                            const std::vector<std::string> &initial_label_stack);
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

class QueueBatchContext : public CommandExecutionContext {
  public:
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
        AcquireResourceRecord(const PresentedImageRecord &presented, ResourceUsageTag tag, vvl::Func command)
            : presented_(presented), acquire_tag_(tag), command_(command) {}
        std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const override;

      private:
        PresentedImageRecord presented_;
        ResourceUsageTag acquire_tag_;
        vvl::Func command_;
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
    const QueueSyncState *GetQueueSyncState() { return queue_state_; }
    VkQueueFlags GetQueueFlags() const;
    QueueId GetQueueId() const override;
    ExecutionType Type() const override { return kSubmitted; }

    void SetupBatchTags(const ResourceUsageRange &tag_range);
    void SetupBatchTags();
    void SetCurrentLabelStack(std::vector<std::string>* current_label_stack);
    void ResetEventsContext() { events_context_.Clear(); }
    ResourceUsageTag GetTagLimit() const override { return batch_.bias; }
    // begin is the tag bias  / .size() is the number of total records that should eventually be in access_log_
    ResourceUsageRange GetTagRange() const { return tag_range_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;

    void SetTagBias(ResourceUsageTag);
    // For Submit
    void SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const VkSubmitInfo2 &submit_info,
                            SignaledSemaphoresUpdate &signaled_semaphores_update);
    void SetupCommandBufferInfo(const VkSubmitInfo2 &submit_info);
    bool DoQueueSubmitValidate(const SyncValidator &sync_state, QueueSubmitCmdState &cmd_state, const VkSubmitInfo2 &submit_info);
    void ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    // For Present
    void SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const VkPresentInfoKHR &present_info,
                            const PresentedImages &presented_images, SignaledSemaphoresUpdate &signaled_semaphores_update);
    bool DoQueuePresentValidate(const Location &loc, const PresentedImages &presented_images);
    void DoPresentOperations(const PresentedImages &presented_images);
    void LogPresentOperations(const PresentedImages &presented_images);

    // For Acquire
    void SetupAccessContext(const PresentedImage &presented);
    void DoAcquireOperation(const PresentedImage &presented);
    void LogAcquireOperation(const PresentedImage &presented, vvl::Func command);

    VulkanTypedHandle Handle() const override;

    template <typename Predicate>
    void ApplyPredicatedWait(Predicate &predicate);
    void ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag);
    void ApplyAcquireWait(const AcquiredImage &acquired);

    void BeginRenderPassReplaySetup(ReplayState &replay, const SyncOpBeginRenderPass &begin_op) override;
    void NextSubpassReplaySetup(ReplayState &replay) override;
    void EndRenderPassReplayCleanup(ReplayState &replay) override;

    void ReplayLabelCommandsFromEmptyBatch();
    void Cleanup();

  private:
    void CommonSetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev,
                                  QueueBatchContext::ConstBatchSet &batches_resolved);
    std::shared_ptr<QueueBatchContext> ResolveOneWaitSemaphore(VkSemaphore sem, const PresentedImages &presented_images,
                                                               SignaledSemaphoresUpdate &signaled_semaphores_update);
    std::shared_ptr<QueueBatchContext> ResolveOneWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask,
                                                               SignaledSemaphoresUpdate &signaled_semaphores_update);

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
    std::vector<std::string> *current_label_stack_ = nullptr;
};

class QueueSyncState {
  public:
    QueueSyncState(const std::shared_ptr<vvl::Queue> &queue_state, VkQueueFlags queue_flags, QueueId id)
        : submit_index_(0), queue_state_(queue_state), last_batch_(), queue_flags_(queue_flags), id_(id) {}

    VulkanTypedHandle Handle() const {
        if (queue_state_) {
            return queue_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkQueue>(VK_NULL_HANDLE), kVulkanObjectTypeQueue);
    }
    std::shared_ptr<const QueueBatchContext> LastBatch() const { return last_batch_; }
    std::shared_ptr<QueueBatchContext> LastBatch() { return last_batch_; }
    void UpdateLastBatch();
    const vvl::Queue *GetQueueState() const { return queue_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }
    QueueId GetQueueId() const { return id_; }

    // Method is const but updates mutable sumbit_index atomically.
    uint64_t ReserveSubmitId() const;

    // Method is const but updates mutable pending_last_batch and relies on the queue external synchronization
    void SetPendingLastBatch(std::shared_ptr<QueueBatchContext> &&last) const;

    std::shared_ptr<QueueBatchContext> PendingLastBatch() const { return pending_last_batch_; }

  private:
    mutable std::atomic<uint64_t> submit_index_;
    mutable std::shared_ptr<QueueBatchContext> pending_last_batch_;
    std::shared_ptr<vvl::Queue> queue_state_;
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

struct QueueSubmitCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    const ErrorObject &error_obj;
    SignaledSemaphoresUpdate signaled_semaphores_update;
    QueueSubmitCmdState(const ErrorObject &error_obj, const SyncValidator &sync_validator)
        : error_obj(error_obj), signaled_semaphores_update(sync_validator) {}
};
