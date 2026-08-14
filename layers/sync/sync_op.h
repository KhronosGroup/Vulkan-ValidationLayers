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
#include <variant>

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

struct PipelineBarrierReplay {
    PipelineBarrierReplay(BarrierSet&& barrier_set);
    BarrierSet barrier_set;
};

struct SetEventReplay {
    SetEventReplay(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& src_exec_scope,
                   std::shared_ptr<const AccessContext>&& src_access_context, const Location& loc);

    vvl::Func command = vvl::Func::Empty;
    std::shared_ptr<const vvl::Event> event;
    // Snapshot of the command buffer's access context at set event time
    std::shared_ptr<const AccessContext> recorded_context;
    SyncExecScope src_exec_scope;
};

struct ResetEventReplay {
    ResetEventReplay(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc);

    vvl::Func command = vvl::Func::Empty;
    std::shared_ptr<const vvl::Event> event;
    SyncExecScope exec_scope;
};

struct WaitEventsReplay {
    WaitEventsReplay(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                     const Location& loc);

    vvl::Func command = vvl::Func::Empty;
    std::vector<std::shared_ptr<const vvl::Event>> events;
    std::vector<BarrierSet> barrier_sets;
};

// Describes how replay changes its recorded and destination contexts.
// These changes are needed because each render pass subpass has its own access context
struct ReplayContextChange {
    enum class Type {
        kBeginRenderPass,
        kNextSubpass,
        kEndRenderPass,
    };

    ReplayContextChange(std::shared_ptr<const vvl::RenderPass>&& rp_state,
                        std::vector<std::shared_ptr<const vvl::ImageView>>&& attachments, const RenderPassAccessContext* rp_context)
        : type(Type::kBeginRenderPass),
          rp_state(std::move(rp_state)),
          attachments(std::move(attachments)),
          rp_context(rp_context) {}

    explicit ReplayContextChange(Type type) : type(type) {}

    Type type;

    // Keep these objects alive because RenderPassAccessContext stores pointers to them
    std::shared_ptr<const vvl::RenderPass> rp_state;
    std::vector<std::shared_ptr<const vvl::ImageView>> attachments;

    const RenderPassAccessContext* rp_context = nullptr;
};

// Describes either a synchronization action or a context change during replay
using ReplayOperation =
    std::variant<PipelineBarrierReplay, SetEventReplay, ResetEventReplay, WaitEventsReplay, ReplayContextChange>;

static inline const PipelineBarrierReplay* GetPipelineBarrierReplay(const ReplayOperation& operation) {
    return std::get_if<PipelineBarrierReplay>(&operation);
}

static inline const SetEventReplay* GetSetEventReplay(const ReplayOperation& operation) {
    return std::get_if<SetEventReplay>(&operation);
}

static inline const ResetEventReplay* GetResetEventReplay(const ReplayOperation& operation) {
    return std::get_if<ResetEventReplay>(&operation);
}

static inline const WaitEventsReplay* GetWaitEventsReplay(const ReplayOperation& operation) {
    return std::get_if<WaitEventsReplay>(&operation);
}

static inline const ReplayContextChange* GetReplayContextChange(const ReplayOperation& operation) {
    return std::get_if<ReplayContextChange>(&operation);
}

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

void ApplyReplayAction(SyncEnvironment& env, const ReplayOperation& operation, AccessContext& access_context,
                       ResourceUsageTag exec_tag);

struct ReplayEntry {
    // NOTE: templated Operation is a GCC workaround. Use Non-templated version with
    // ReplayOperation&& and std::move instead of std::forward when our GCC version supports this
    template <typename Operation>
    ReplayEntry(ResourceUsageTag tag, bool validate_layout_transition_first_use, Operation&& operation)
        : tag(tag),
          validate_layout_transition_first_use(validate_layout_transition_first_use),
          operation(std::forward<Operation>(operation)) {}

    ResourceUsageTag tag = 0;
    bool validate_layout_transition_first_use = false;
    ReplayOperation operation;
};

struct ReplayState {
    ReplayState(CommandBufferContext& proxy_primary_cb_context, const CommandBufferContext& recorded_cb_context,
                ResourceUsageTag base_tag, const Location& cb_loc);
    ReplayState(QueueBatchContext& batch_context, const CommandBufferContext& recorded_cb_context, ResourceUsageTag base_tag,
                const Location& cb_loc);

    bool ValidateFirstUse();
    bool DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const;
    void ApplyContextChange(const ReplayContextChange& change);

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
