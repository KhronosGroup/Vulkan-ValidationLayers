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
#include "sync/sync_validation.h"

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

class SyncOpBase {
  public:
    SyncOpBase() : command_(vvl::Func::Empty) {}
    SyncOpBase(vvl::Func command) : command_(command) {}
    virtual ~SyncOpBase() = default;

    const char *CmdName() const { return vvl::String(command_); }

    virtual bool Validate(const CommandBufferAccessContext &cb_context) const = 0;
    virtual ResourceUsageTag Record(CommandBufferAccessContext *cb_context) = 0;
    virtual bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const = 0;
    virtual void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const = 0;

  protected:
    // Only non-null and valid for SyncOps within a render pass instance  WIP -- think about how to manage for non RPI calls within
    // RPI and 2ndarys...
    uint32_t subpass_ = VK_SUBPASS_EXTERNAL;
    vvl::Func command_;
};

class SyncOpBarriers : public SyncOpBase {
  protected:
    template <typename Barriers, typename FunctorFactory>
    static void ApplyBarriers(const Barriers &barriers, const FunctorFactory &factory, QueueId queue_id, ResourceUsageTag tag,
                              AccessContext *context);
    template <typename Barriers, typename FunctorFactory>
    static void ApplyGlobalBarriers(const Barriers &barriers, const FunctorFactory &factory, QueueId queue_id, ResourceUsageTag tag,
                                    AccessContext *access_context);

    SyncOpBarriers(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkPipelineStageFlags srcStageMask,
                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                   const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                   const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                   const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpBarriers(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t event_count,
                   const VkDependencyInfoKHR *pDependencyInfo);

    ~SyncOpBarriers() override = default;

  protected:
    struct BarrierSet {
        using ImageState = syncval_state::ImageState;
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
    SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                          uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          const VkDependencyInfoKHR &pDependencyInfo);
    ~SyncOpPipelineBarrier() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;
};

class SyncOpWaitEvents : public SyncOpBarriers {
  public:
    SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                     const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                     const VkImageMemoryBarrier *pImageMemoryBarriers);

    SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfo);
    ~SyncOpWaitEvents() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  protected:
    static const char *const kIgnored;
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void DoRecord(CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    // TODO PHASE2 This is the wrong thing to use for "replay".. as the event state will have moved on since the record
    // TODO PHASE2 May need to capture by value w.r.t. "first use" or build up in calling/enqueue context through replay.
    std::vector<std::shared_ptr<const EVENT_STATE>> events_;
    void MakeEventsList(const SyncValidator &sync_state, uint32_t event_count, const VkEvent *events);
};

class SyncOpResetEvent : public SyncOpBase {
  public:
    SyncOpResetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                     VkPipelineStageFlags2KHR stageMask);
    ~SyncOpResetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    std::shared_ptr<const EVENT_STATE> event_;
    SyncExecScope exec_scope_;
};

class SyncOpSetEvent : public SyncOpBase {
  public:
    SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   VkPipelineStageFlags2KHR stageMask, const AccessContext *access_context);
    SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   const VkDependencyInfoKHR &dep_info, const AccessContext *access_context);
    ~SyncOpSetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

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
    SyncOpBeginRenderPass(vvl::Func command, const SyncValidator &sync_state, const VkRenderPassBeginInfo *pRenderPassBegin,
                          const VkSubpassBeginInfo *pSubpassBeginInfo);
    ~SyncOpBeginRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;
    const RenderPassAccessContext *GetRenderPassAccessContext() const { return rp_context_; }

  protected:
    safe_VkRenderPassBeginInfo renderpass_begin_info_;
    safe_VkSubpassBeginInfo subpass_begin_info_;
    std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> shared_attachments_;
    std::vector<const syncval_state::ImageViewState *> attachments_;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state_;
    const RenderPassAccessContext *rp_context_;
};

class SyncOpNextSubpass : public SyncOpBase {
  public:
    SyncOpNextSubpass(vvl::Func command, const SyncValidator &sync_state, const VkSubpassBeginInfo *pSubpassBeginInfo,
                      const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpNextSubpass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  protected:
    safe_VkSubpassBeginInfo subpass_begin_info_;
    safe_VkSubpassEndInfo subpass_end_info_;
};

class SyncOpEndRenderPass : public SyncOpBase {
  public:
    SyncOpEndRenderPass(vvl::Func command, const SyncValidator &sync_state, const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpEndRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  protected:
    safe_VkSubpassEndInfo subpass_end_info_;
};
// The barrier operation for pipeline and subpass dependencies`
struct PipelineBarrierOp {
    SyncBarrier barrier;
    bool layout_transition;
    ResourceAccessState::QueueScopeOps scope;
    PipelineBarrierOp(QueueId queue_id, const SyncBarrier &barrier_, bool layout_transition_)
        : barrier(barrier_), layout_transition(layout_transition_), scope(queue_id) {
        if (queue_id != kQueueIdInvalid) {
            // This is a submit time application... supress layout transitions to not taint the QueueBatchContext write state
            layout_transition = false;
        }
    }

    PipelineBarrierOp(const PipelineBarrierOp &rhs)
        : barrier(rhs.barrier), layout_transition(rhs.layout_transition), scope(rhs.scope) {}

    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(scope, barrier, layout_transition); }
};

// Batch barrier ops don't modify in place, and thus don't need to hold pending state, and also are *never* layout transitions.
struct BatchBarrierOp : public PipelineBarrierOp {
    void operator()(ResourceAccessState *access_state) const {
        access_state->ApplyBarrier(scope, barrier, layout_transition);
        access_state->ApplyPendingBarriers(kInvalidTag);  // There can't be any need for this tag
    }
    BatchBarrierOp(QueueId queue_id, const SyncBarrier &barrier_) : PipelineBarrierOp(queue_id, barrier_, false) {}
};

// The barrier operation for wait events
struct WaitEventBarrierOp {
    ResourceAccessState::EventScopeOps scope_ops;
    SyncBarrier barrier;
    bool layout_transition;

    WaitEventBarrierOp(const QueueId scope_queue_, const ResourceUsageTag scope_tag_, const SyncBarrier &barrier_,
                       bool layout_transition_)
        : scope_ops(scope_queue_, scope_tag_), barrier(barrier_), layout_transition(layout_transition_) {
        if (scope_queue_ != kQueueIdInvalid) {
            // This is a submit time application... supress layout transitions to not taint the QueueBatchContext write state
            layout_transition = false;
        }
    }
    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(scope_ops, barrier, layout_transition); }
};
