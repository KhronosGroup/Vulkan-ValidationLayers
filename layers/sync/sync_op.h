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
#include <vulkan/utility/vk_safe_struct.hpp>
#include "error_message/error_location.h"
#include <optional>

struct DeviceExtensions;

namespace vvl {
class Event;
class ImageView;
class RenderPass;
}  // namespace vvl

namespace syncval {

class CommandBufferAccessContext;
class CommandExecutionContext;
class QueueBatchContext;
class RenderPassAccessContext;
class SyncValidator;
struct ReplayState;

struct SyncEventState {
    using EventPointer = std::shared_ptr<const vvl::Event>;
    EventPointer event;
    vvl::Func last_command;             // Only Event commands are valid here.
    ResourceUsageTag last_command_tag;  // Needed to filter replay validation
    vvl::Func unsynchronized_set;
    VkPipelineStageFlags2 barriers;
    SyncExecScope scope;
    ResourceUsageTag first_scope_tag;
    std::shared_ptr<const AccessContext> first_scope;

    SyncEventState()
        : event(),
          last_command(vvl::Func::Empty),
          last_command_tag(0),
          unsynchronized_set(vvl::Func::Empty),
          barriers(0U),
          scope(),
          first_scope_tag() {}

    SyncEventState(const SyncEventState&) = default;
    SyncEventState(SyncEventState&&) = default;

    SyncEventState(const SyncEventState::EventPointer& event_state);

    void ResetFirstScope();
    const AccessContext::ScopeMap& FirstScope() const { return first_scope->GetAccessMap(); }
    bool HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope) const;
    void AddReferencedTags(ResourceUsageTagSet& referenced) const;
};

class SyncEventsContext {
  public:
    using Map = vvl::unordered_map<const vvl::Event*, std::shared_ptr<SyncEventState>>;
    using iterator = Map::iterator;
    using const_iterator = Map::const_iterator;

    SyncEventState* GetFromShared(const SyncEventState::EventPointer& event_state) {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            if (!event_state.get()) return nullptr;

            const auto* event_plain_ptr = event_state.get();
            auto sync_state = std::make_shared<SyncEventState>(event_state);
            auto insert_pair = map_.emplace(event_plain_ptr, sync_state);
            return insert_pair.first->second.get();
        }
        return find_it->second.get();
    }

    const SyncEventState* Get(const SyncEventState::EventPointer& event_state) const {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            return nullptr;
        }
        return find_it->second.get();
    }

    void ApplyBarrier(const SyncExecScope& src, const SyncExecScope& dst, ResourceUsageTag tag);
    void ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag);

    void Destroy(const vvl::Event* event_state) {
        auto sync_it = map_.find(event_state);
        if (sync_it != map_.end()) {
            map_.erase(sync_it);
        }
    }
    void Clear() { map_.clear(); }

    SyncEventsContext& DeepCopy(const SyncEventsContext& from);
    void AddReferencedTags(ResourceUsageTagSet& referenced) const;

  private:
    Map map_;
};

class SyncOpBase {
  public:
    SyncOpBase() = default;
    SyncOpBase(vvl::Func command) : command_(command) {}
    virtual ~SyncOpBase() = default;
    virtual bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const = 0;
    virtual void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const = 0;

  protected:
    vvl::Func command_ = vvl::Func::Empty;
};

class SyncOpPipelineBarrier : public SyncOpBase {
  public:
    SyncOpPipelineBarrier(BarrierSet&& barrier_set);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;

  private:
    BarrierSet barrier_set_;
};

class SyncOpSetEvent : public SyncOpBase {
  public:
    SyncOpSetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& src_exec_scope,
                   std::shared_ptr<const AccessContext>&& src_access_context, const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;

  private:
    std::shared_ptr<const vvl::Event> event_;
    // Snapshot of the command buffer's access context at set event time
    std::shared_ptr<const AccessContext> recorded_context_;
    SyncExecScope src_exec_scope_;
};

class SyncOpResetEvent : public SyncOpBase {
  public:
    SyncOpResetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;

  private:
    std::shared_ptr<const vvl::Event> event_;
    SyncExecScope exec_scope_;
};

class SyncOpWaitEvents : public SyncOpBase {
  public:
    SyncOpWaitEvents(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                     const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;

  private:
    // TODO PHASE2 This is the wrong thing to use for "replay".. as the event state will have moved on since the record
    // TODO PHASE2 May need to capture by value w.r.t. "first use" or build up in calling/enqueue context through replay.
    std::vector<std::shared_ptr<const vvl::Event>> events_;

    std::vector<BarrierSet> barrier_sets_;
};

class SyncOpBeginRenderPass : public SyncOpBase {
  public:
    SyncOpBeginRenderPass(std::shared_ptr<const vvl::RenderPass>&& rp_state,
                          std::vector<std::shared_ptr<const vvl::ImageView>>&& attachments,
                          const RenderPassAccessContext* rp_context, const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;
    const RenderPassAccessContext* GetRenderPassAccessContext() const { return rp_context_; }

  protected:
    // Keep references to rp_state and attachments in case they are deleted.
    // The RenderPassAccessContext keeps only pointers to them.
    // TODO: make RenderPassAccessContext the owner of rp_state and attachments
    std::shared_ptr<const vvl::RenderPass> rp_state_;
    std::vector<std::shared_ptr<const vvl::ImageView>> attachments_;

    const RenderPassAccessContext* rp_context_ = nullptr;
};

class SyncOpNextSubpass : public SyncOpBase {
  public:
    SyncOpNextSubpass(const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;
};

class SyncOpEndRenderPass : public SyncOpBase {
  public:
    SyncOpEndRenderPass(const Location& loc);
    bool ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const override;
};

// Render pass state for submit-time replay. The accesses come from RenderPassAccessContext's
// recorded subpass contexts, and the subpass dependencies are applied to the accesses in the
// queue batch context (via subpass_contexts).
struct RenderPassReplayState {
    RenderPassReplayState(const RenderPassAccessContext* rp_context, const AccessContext& external_context,
                          VkQueueFlags queue_flags);
    AccessContext& GetCurrentReplayContext();
    const AccessContext& GetCurrentRecordedContext() const;
    vvl::span<AccessContext> GetSubpassContexts();

    const RenderPassAccessContext* rp_context = nullptr;
    uint32_t current_subpass = 0;

    // Per-subpass contexts for replay. Unlike RenderPassAccessContext::subpass_contexts_ these hold
    // no recorded accesses (access maps are emtpy). All they store is the subpass dependencies
    std::unique_ptr<AccessContext[]> subpass_contexts;
};

struct ReplayState {
    ReplayState(CommandExecutionContext& exec_context, const CommandBufferAccessContext& recorded_context,
                ResourceUsageTag base_tag, const Location& cb_loc);

    bool ValidateFirstUse();
    bool DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const;
    QueueBatchContext& GetBatchContext();

    CommandExecutionContext& exec_context;
    const CommandBufferAccessContext& recorded_context;
    std::optional<RenderPassReplayState> rp_replay;
    const ResourceUsageTag base_tag;
    const Location& cb_loc;
};

}  // namespace syncval
