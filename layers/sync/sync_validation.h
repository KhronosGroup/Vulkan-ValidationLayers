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

#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"

#include "sync/sync_common.h"
#include "sync/sync_access_context.h"
#include "sync/sync_renderpass.h"
#include "sync/sync_commandbuffer.h"

class AccessContext;
struct ClearAttachmentInfo;
class CommandBufferAccessContext;
class CommandExecutionContext;
struct PresentedImage;
class QueueBatchContext;
struct QueueSubmitCmdState;
class RenderPassAccessContext;
class ReplayState;
class ResourceAccessState;
class ResourceAccessWriteState;
struct ResourceFirstAccess;
class SyncEventsContext;
struct SyncEventState;
class SyncOpBase;
class SyncOpBeginRenderPass;
class SyncValidator;

VALSTATETRACK_DERIVED_STATE_OBJECT(VkImage, syncval_state::ImageState, IMAGE_STATE)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, syncval_state::ImageViewState, IMAGE_VIEW_STATE)

struct AcquiredImage {
    std::shared_ptr<const syncval_state::ImageState> image;
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
        Signal(const std::shared_ptr<const vvl::Semaphore> &sem_state_, const std::shared_ptr<QueueBatchContext> &batch_,
               const SyncExecScope &exec_scope_);
        Signal(const std::shared_ptr<const vvl::Semaphore> &sem_state_, const PresentedImage &presented, ResourceUsageTag acq_tag);

        std::shared_ptr<const vvl::Semaphore> sem_state;
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

    bool SignalSemaphore(const std::shared_ptr<const vvl::Semaphore> &sem_state, const std::shared_ptr<QueueBatchContext> &batch,
                         const VkSemaphoreSubmitInfo &signal_info);
    bool Insert(const std::shared_ptr<const vvl::Semaphore> &sem_state, std::shared_ptr<Signal> &&signal);
    bool SignalSemaphore(const std::shared_ptr<const vvl::Semaphore> &sem_state, const PresentedImage &presented,
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


// Command execution context is the base class for command buffer and queue contexts
// Preventing unintented leakage of subclass specific state, storing enough information
// for message logging.
// TODO: determine where to draw the design split for tag tracking (is there anything command to Queues and CB's)


// Allow keep track of the exec contexts replay state
class ReplayState {
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

    bool ValidateFirstUse();
    bool DetectFirstUseHazard(const ResourceUsageRange &first_use_range) const;

    ReplayState(CommandExecutionContext &exec_context, const CommandBufferAccessContext &recorded_context,
                const ErrorObject &error_object, uint32_t index);

    CommandExecutionContext &GetExecutionContext() const { return exec_context_; }
    ResourceUsageTag GetBaseTag() const { return base_tag_; }

    void BeginRenderPassReplaySetup(const SyncOpBeginRenderPass &begin_op) {
        exec_context_.BeginRenderPassReplaySetup(*this, begin_op);
    }
    void NextSubpassReplaySetup() { exec_context_.NextSubpassReplaySetup(*this); }
    void EndRenderPassReplayCleanup() { exec_context_.EndRenderPassReplayCleanup(*this); }

    AccessContext *ReplayStateRenderPassBegin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op,
                                              const AccessContext &external_context) {
        return rp_replay_.Begin(queue_flags, begin_op, external_context);
    }
    AccessContext *ReplayStateRenderPassNext() { return rp_replay_.Next(); }
    void ReplayStateRenderPassEnd(AccessContext &external_context) { rp_replay_.End(external_context); }

  protected:
    const AccessContext *GetRecordedAccessContext() const;

    CommandExecutionContext &exec_context_;
    const CommandBufferAccessContext &recorded_context_;
    const ErrorObject &error_obj_;
    const uint32_t index_;
    const ResourceUsageTag base_tag_;
    RenderPassReplayState rp_replay_;
};


VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, syncval_state::CommandBuffer, CMD_BUFFER_STATE)

class QueueSyncState;
class CommandBufferAccessContext;

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
        CBSubmitLog(const BatchRecord &batch, const CommandBufferAccessContext &cb);
        size_t Size() const { return log_->size(); }
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
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSwapchainKHR, syncval_state::Swapchain, SWAPCHAIN_NODE)

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
    void UpdateLastBatch(std::shared_ptr<QueueBatchContext> &&last);
    const vvl::Queue *GetQueueState() const { return queue_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }
    QueueId GetQueueId() const { return id_; }

    uint64_t ReserveSubmitId() const;  // Method is const but updates mutable sumbit_index atomically.

  private:
    mutable std::atomic<uint64_t> submit_index_;
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

class SyncValidator : public ValidationStateTracker, public SyncStageAccess {
  public:
    using ImageState = syncval_state::ImageState;
    using ImageViewState = syncval_state::ImageViewState;
    using StateTracker = ValidationStateTracker;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    SyncValidator() { container_type = LayerObjectTypeSyncValidation; }

    // Global tag range for submitted command buffers resource usage logs
    // Started the global tag count at 1 s.t. zero are invalid and ResourceUsageTag normalization can just zero them.
    mutable std::atomic<ResourceUsageTag> tag_limit_{1};  // This is reserved in Validation phase, thus mutable and atomic
    ResourceUsageRange ReserveGlobalTagRange(size_t tag_count) const;  // Note that the tag_limit_ is mutable this has side effects

    vvl::unordered_map<VkQueue, std::shared_ptr<QueueSyncState>> queue_sync_states_;
    QueueId queue_id_limit_ = kQueueIdBase;
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
    void UpdateFenceWaitInfo(std::shared_ptr<const vvl::Fence> &fence, FenceSyncState &&wait_info);

    void WaitForFence(VkFence fence);

    void UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos);

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
    std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                  VkFormatFeatureFlags2KHR features) final;

    std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
                                                  uint32_t swapchain_index, VkFormatFeatureFlags2KHR features) final;
    std::shared_ptr<IMAGE_VIEW_STATE> CreateImageViewState(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv,
                                                           const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                                                           const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) final;

    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo, Func command);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                              const VkSubpassEndInfo *pSubpassEndInfo, Func command);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, Func command);
    bool SupressedBoundDescriptorWAW(const HazardResult &hazard) const;

    void CreateDevice(const VkDeviceCreateInfo *pCreateInfo) override;

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassBeginInfo *pSubpassBeginInfo,
                                               const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo,
                                            const ErrorObject &error_obj) const override;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy *pRegions, const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                          const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                       const ErrorObject &error_obj) const override;
    void RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo, Func command);
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                        const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions,
                                   const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                         const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                      const ErrorObject &error_obj) const override;

    void RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, Func command);
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                       const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                    const RecordObject &record_obj) override;

    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                           const ErrorObject &error_obj) const override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                         const RecordObject &record_obj) override;

    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                               const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                             const RecordObject &record_obj) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                          const RecordObject &record_obj) override;

    void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                          const RecordObject &record_obj) override;

    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                          VkSubpassContents contents, const RecordObject &record_obj) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) override;

    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                       const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                        const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject &record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) override;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                  const ErrorObject &error_obj) const;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                          const ErrorObject &error_obj) const override;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                         const RecordObject &record_obj) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                            const RecordObject &record_obj) override;

    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                          const ErrorObject &error_obj) const override;
    void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                        const RecordObject &record_obj) override;

    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                      const Location &loc) const;
    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                 const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                              const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, Func command);
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                               const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                            const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                      const Location &loc) const;
    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                             const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                 const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                              const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                    VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, Func command);
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                           const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                               const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                            const RecordObject &record_obj) override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                              const Location &loc) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                         const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                      const ErrorObject &error_obj) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                            Func command);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter,
                                   const RecordObject &record_obj) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                       const RecordObject &record_obj) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                    const RecordObject &record_obj) override;

    bool ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                const Location &loc) const;
    void RecordIndirectBuffer(AccessContext &context, ResourceUsageTag tag, const VkDeviceSize struct_size, const VkBuffer buffer,
                              const VkDeviceSize offset, const uint32_t drawCount, uint32_t stride);

    bool ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                             VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, const Location &loc) const;
    void RecordCountBuffer(AccessContext &context, ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset);

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                    const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                  const RecordObject &record_obj) override;

    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject &record_obj) override;

    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                       const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                      uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const ErrorObject &error_obj) const override;
    void RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride, Func command);
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject &error_obj) const override;
    void RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, Func command);
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue *pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges,
                                                  const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) override;

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects,
                                            const ErrorObject &error_obj) const override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects,
                                          const RecordObject &record_obj) override;

    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags,
                                                const ErrorObject &error_obj) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags, const RecordObject &record_obj) override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data, const RecordObject &record_obj) override;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions, const ErrorObject &error_obj) const override;

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve *pRegions, const RecordObject &record_obj) override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                            const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                         const ErrorObject &error_obj) const override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                          const RecordObject &record_obj) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                       const RecordObject &record_obj) override;

    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                      VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) override;

    bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                const ErrorObject &error_obj) const override;
    void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                              const RecordObject &record_obj) override;

    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const ErrorObject &error_obj) const override;
    void PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                   const RecordObject &record_obj) override;

    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo,
                                        const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                     const ErrorObject &error_obj) const override;
    void PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo,
                                       const RecordObject &record_obj) override;
    void PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                    const RecordObject &record_obj) override;

    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const ErrorObject &error_obj) const override;
    void PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                     const RecordObject &record_obj) override;

    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                          const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                       const ErrorObject &error_obj) const override;
    void PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                         const RecordObject &record_obj) override;
    void PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                      const RecordObject &record_obj) override;

    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                      const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                     VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                     const RecordObject &record_obj) override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                         const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) override;
    bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                                 VkDeviceSize dstOffset, uint32_t marker,
                                                 const ErrorObject &error_obj) const override;
    void PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkBuffer dstBuffer,
                                               VkDeviceSize dstOffset, uint32_t marker, const RecordObject &record_obj) override;
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                        const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                           const RecordObject &record_obj) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) override;

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                        const ErrorObject &error_obj) const override;
    ResourceUsageRange SetupPresentInfo(const VkPresentInfoKHR &present_info, std::shared_ptr<QueueBatchContext> &batch,
                                        PresentedImages &presented_images) const;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                       const RecordObject &record_obj) override;
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex,
                                            const RecordObject &record_obj) override;
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj);
    bool ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                             const ErrorObject &error_obj) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                    const ErrorObject &error_obj) const override;
    void RecordQueueSubmit(VkQueue queue, VkFence fence, const RecordObject &record_obj);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   const RecordObject &record_obj) override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       const RecordObject &record_obj) override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                     const ErrorObject &error_obj) const override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                    const RecordObject &record_obj) override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                     uint64_t timeout, const RecordObject &record_obj) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                             VkImage *pSwapchainImages, const RecordObject &record_obj) override;
};

