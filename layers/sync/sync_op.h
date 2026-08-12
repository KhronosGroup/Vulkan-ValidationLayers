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

#include "sync/sync_access_context.h"
#include "sync/sync_barrier.h"
#include "error_message/error_location.h"
#include <optional>

namespace vvl {
class Event;
class ImageView;
class RenderPass;
}  // namespace vvl

namespace syncval {
class CommandBufferContext;
struct SyncEnvironment;
class QueueBatchContext;
class RenderPassAccessContext;
struct ReplayState;

class SyncOp {
  public:
    virtual ~SyncOp() = default;
    virtual bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const = 0;
    virtual void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const = 0;
};

class SyncOpPipelineBarrier : public SyncOp {
  public:
    SyncOpPipelineBarrier(BarrierSet&& barrier_set);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;

  private:
    BarrierSet barrier_set_;
};

class SyncOpSetEvent : public SyncOp {
  public:
    SyncOpSetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& src_exec_scope,
                   std::shared_ptr<const AccessContext>&& src_access_context, const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;

  private:
    vvl::Func command_ = vvl::Func::Empty;
    std::shared_ptr<const vvl::Event> event_;
    // Snapshot of the command buffer's access context at set event time
    std::shared_ptr<const AccessContext> recorded_context_;
    SyncExecScope src_exec_scope_;
};

class SyncOpResetEvent : public SyncOp {
  public:
    SyncOpResetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;

  private:
    vvl::Func command_ = vvl::Func::Empty;
    std::shared_ptr<const vvl::Event> event_;
    SyncExecScope exec_scope_;
};

class SyncOpWaitEvents : public SyncOp {
  public:
    SyncOpWaitEvents(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                     const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;

  private:
    vvl::Func command_ = vvl::Func::Empty;

    // TODO PHASE2 This is the wrong thing to use for "replay".. as the event state will have moved on since the record
    // TODO PHASE2 May need to capture by value w.r.t. "first use" or build up in calling/enqueue context through replay.
    std::vector<std::shared_ptr<const vvl::Event>> events_;

    std::vector<BarrierSet> barrier_sets_;
};

class SyncOpBeginRenderPass : public SyncOp {
  public:
    SyncOpBeginRenderPass(std::shared_ptr<const vvl::RenderPass>&& rp_state,
                          std::vector<std::shared_ptr<const vvl::ImageView>>&& attachments,
                          const RenderPassAccessContext* rp_context);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;

  protected:
    // Keep references to rp_state and attachments in case they are deleted.
    // The RenderPassAccessContext keeps only pointers to them.
    // TODO: make RenderPassAccessContext the owner of rp_state and attachments
    std::shared_ptr<const vvl::RenderPass> rp_state_;
    std::vector<std::shared_ptr<const vvl::ImageView>> attachments_;

    const RenderPassAccessContext* rp_context_ = nullptr;
};

class SyncOpNextSubpass : public SyncOp {
  public:
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;
};

class SyncOpEndRenderPass : public SyncOp {
  public:
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const override;
};

// Render pass state for submit-time replay. The accesses come from RenderPassAccessContext's
// recorded subpass contexts, and the subpass dependencies are applied to the accesses in the
// queue batch context (via subpass_contexts).
struct RenderPassReplayState {
    RenderPassReplayState(const RenderPassAccessContext* rp_context, const AccessContext& external_context,
                          VkQueueFlags queue_flags);
    AccessContext& GetCurrentDestinationContext();
    const AccessContext& GetCurrentDestinationContext() const;
    const AccessContext& GetCurrentRecordedContext() const;

    const RenderPassAccessContext* rp_context = nullptr;
    uint32_t current_subpass = 0;

    // Per-subpass contexts for replay. Unlike RenderPassAccessContext::subpass_contexts_ these hold
    // no recorded accesses (access maps are emtpy). All they store is the subpass dependencies
    std::unique_ptr<AccessContext[]> destination_subpass_contexts;
};

struct ReplayState {
    ReplayState(CommandBufferContext& proxy_primary_cb_context, const CommandBufferContext& recorded_cb_context,
                ResourceUsageTag base_tag, const Location& cb_loc);
    ReplayState(QueueBatchContext& batch_context, const CommandBufferContext& recorded_cb_context, ResourceUsageTag base_tag,
                const Location& cb_loc);

    bool ValidateFirstUse();
    bool DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const;

    AccessContext& GetCurrentDestinationContext();
    const AccessContext& GetCurrentDestinationContext() const;
    const AccessContext& GetCurrentRecordedContext() const;

    SyncEnvironment& env;
    AccessContext& destination_context;
    const CommandBufferContext& recorded_cb_context;
    std::optional<RenderPassReplayState> rp_replay;
    const ResourceUsageTag base_tag;
    const Location& cb_loc;
};

}  // namespace syncval
