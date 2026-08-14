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

#include "sync/sync_op.h"
#include "sync/sync_render_pass.h"
#include "sync/sync_command_buffer.h"
#include "sync/sync_event.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

#include "state_tracker/buffer_state.h"
#include "state_tracker/event_state.h"
#include "state_tracker/render_pass_state.h"

#include "utils/image_utils.h"
#include "utils/sync_utils.h"

using vvl::Func;

namespace syncval {

PipelineBarrierReplay::PipelineBarrierReplay(BarrierSet&& barrier_set) : barrier_set(std::move(barrier_set)) {}

SetEventReplay::SetEventReplay(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& src_exec_scope,
                               std::shared_ptr<const AccessContext>&& src_access_context, const Location& loc)
    : command(loc.function),
      event(std::move(event)),
      recorded_context(std::move(src_access_context)),
      src_exec_scope(src_exec_scope) {}

ResetEventReplay::ResetEventReplay(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc)
    : command(loc.function), event(std::move(event)), exec_scope(exec_scope) {}

WaitEventsReplay::WaitEventsReplay(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                                   const Location& loc)
    : command(loc.function), events(std::move(events)), barrier_sets(std::move(barrier_sets)) {}

void ApplyReplayAction(SyncEnvironment& env, const ReplayOperation& operation, AccessContext& access_context,
                       ResourceUsageTag exec_tag) {
    if (const auto* barrier = GetPipelineBarrierReplay(operation)) {
        ApplyBarrier(env, access_context, barrier->barrier_set, exec_tag);
    } else if (const auto* set_event = GetSetEventReplay(operation)) {
        // Build the event's first-scope context by merging the current replay destination
        // with the command-buffer context captured when the set-event command was recorded.
        // Keep the recorded context unchanged because the command buffer can be replayed multiple times.
        auto merged_context = std::make_shared<AccessContext>(*access_context.validator);
        merged_context->InitFrom(access_context);
        merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(env.queue_id, exec_tag), *set_event->recorded_context);
        merged_context->TrimAndClearFirstAccess();

        ApplyCmdSetEvent(env, set_event->event, set_event->src_exec_scope, merged_context, exec_tag, set_event->command);
    } else if (const auto* reset_event = GetResetEventReplay(operation)) {
        ApplyCmdResetEvent(env, reset_event->event, exec_tag, reset_event->command);
    } else if (const auto* wait_events = GetWaitEventsReplay(operation)) {
        ApplyCmdWaitEvents(env, access_context, wait_events->events, wait_events->barrier_sets, exec_tag, wait_events->command);
    }
}

static bool ValidateEventCommand(const SyncEnvironment& env, const ReplayOperation& operation,
                                 const AccessContext& destination_context, ResourceUsageTag exec_tag) {
    if (const auto* set_event = GetSetEventReplay(operation)) {
        return ValidateCmdSetEvent(env, set_event->event, set_event->src_exec_scope, exec_tag, Location(set_event->command));
    }
    if (const auto* reset_event = GetResetEventReplay(operation)) {
        return ValidateCmdResetEvent(env, reset_event->event, reset_event->exec_scope, exec_tag, Location(reset_event->command));
    }
    if (const auto* wait_events = GetWaitEventsReplay(operation)) {
        bool skip = false;
        Location location(wait_events->command);
        skip |= ValidateCmdWaitEvents(env, wait_events->events, exec_tag, location);
        skip |= DetectCmdWaitEventsImageBarrierHazard(env, destination_context, wait_events->events, wait_events->barrier_sets,
                                                      exec_tag, location);
        return skip;
    }
    return false;
}

ReplayState::ReplayState(CommandBufferContext& proxy_primary_cb_context, const CommandBufferContext& recorded_cb_context,
                         ResourceUsageTag base_tag, const Location& cb_loc)
    : env(proxy_primary_cb_context.GetSyncEnvironment()),
      destination_context(proxy_primary_cb_context.GetCbAccessContext()),
      recorded_cb_context(recorded_cb_context),
      base_tag(base_tag),
      cb_loc(cb_loc) {}

ReplayState::ReplayState(QueueBatchContext& batch_context, const CommandBufferContext& recorded_cb_context,
                         ResourceUsageTag base_tag, const Location& cb_loc)
    : env(batch_context.GetSyncEnvironment()),
      destination_context(batch_context.GetAccessContext()),
      recorded_cb_context(recorded_cb_context),
      base_tag(base_tag),
      cb_loc(cb_loc) {}

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const {
    bool skip = false;
    if (!first_use_range.non_empty()) {
        return skip;
    }
    const AccessContext& recorded_access_context = GetCurrentRecordedContext();
    const HazardResult hazard =
        recorded_access_context.DetectFirstUseHazard(env.queue_id, first_use_range, GetCurrentDestinationContext());
    if (hazard.IsHazard()) {
        LogObjectList objlist(env.handle, recorded_cb_context.GetCBState().Handle());
        const std::string error = env.validator.error_messages_.FirstUseError(env, hazard, recorded_cb_context, cb_loc.index);
        skip |= env.validator.SyncError(hazard.Hazard(), objlist, cb_loc, error);
    }
    return skip;
}

void ReplayState::ApplyContextChange(const ReplayContextChange& change) {
    if (change.type == ReplayContextChange::Type::kBeginRenderPass) {
        rp_replay.emplace(change.rp_context, destination_context, env.queue_flags);
    } else if (change.type == ReplayContextChange::Type::kNextSubpass) {
        const uint32_t subpass_count = rp_replay->rp_context->GetRenderPassState()->create_info.subpassCount;
        // Store and resolve operations happen before the next-subpass tag.
        if (rp_replay->current_subpass + 1 < subpass_count) {
            rp_replay->current_subpass++;
        }
    } else if (change.type == ReplayContextChange::Type::kEndRenderPass) {
        const uint32_t subpass_count = rp_replay->rp_context->GetRenderPassState()->create_info.subpassCount;
        auto subpass_contexts = vvl::make_span(rp_replay->destination_subpass_contexts.get(), subpass_count);
        destination_context.ResolveChildContexts(subpass_contexts);
        rp_replay.reset();
    }
}

// Validate first-use hazards. The following describes how it works.
//
// The first access to a memory location can occur anywhere in the command buffer
// (not necessarily at the beginning), and first accesses to different resources
// may be interleaved with barriers. To validate each first access against accesses
// from previous submissions, we need to replay all barriers that occur before that
// specific first access.
//
// This defines the algorithm: replay barriers until we reach the next first access,
// validate that first access, then continue replaying barriers until the next first
// access, validate that one, and so on until we reach the end of the command buffer.
bool ReplayState::ValidateFirstUse() {
    bool skip = false;
    ResourceUsageRange first_use_range = {0, 0};

    for (const ReplayEntry& entry : recorded_cb_context.GetReplayEntries()) {
        first_use_range.end = entry.tag;
        skip |= DetectFirstUseHazard(first_use_range);

        const auto* context_change = GetReplayContextChange(entry.operation);
        if (context_change) {
            ApplyContextChange(*context_change);
        }

        if (entry.validate_layout_transition_first_use) {
            skip |= DetectFirstUseHazard({entry.tag, entry.tag + 1});
        }

        const bool replay_action = context_change == nullptr;
        if (replay_action) {
            const ResourceUsageTag exec_tag = base_tag + entry.tag;
            skip |= ValidateEventCommand(env, entry.operation, GetCurrentDestinationContext(), exec_tag);
            ApplyReplayAction(env, entry.operation, GetCurrentDestinationContext(), exec_tag);
        }

        first_use_range.begin = entry.tag + 1;
    }

    // Validate first accesses after the last replay entry.
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);

    return skip;
}

const AccessContext& ReplayState::GetCurrentDestinationContext() const {
    return rp_replay ? rp_replay->GetCurrentDestinationContext() : destination_context;
}

AccessContext& ReplayState::GetCurrentDestinationContext() {
    return rp_replay ? rp_replay->GetCurrentDestinationContext() : destination_context;
}

const AccessContext& ReplayState::GetCurrentRecordedContext() const {
    return rp_replay ? rp_replay->GetCurrentRecordedContext() : recorded_cb_context.GetCbAccessContext();
}

RenderPassReplayState::RenderPassReplayState(const RenderPassAccessContext* rp_context, const AccessContext& external_context,
                                             VkQueueFlags queue_flags)
    : rp_context(rp_context), current_subpass(0) {
    const vvl::RenderPass& render_pass = *rp_context->GetRenderPassState();
    const uint32_t subpass_count = render_pass.create_info.subpassCount;

    destination_subpass_contexts = InitSubpassContexts(queue_flags, render_pass, external_context);

    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (uint32_t i = 0; i < subpass_count; i++) {
        AccessContext& subpass_context = destination_subpass_contexts[i];
        subpass_context.ClearAsyncContexts();
        subpass_context.ImportAsyncContexts(external_context);
    }
}

AccessContext& RenderPassReplayState::GetCurrentDestinationContext() { return destination_subpass_contexts[current_subpass]; }

const AccessContext& RenderPassReplayState::GetCurrentDestinationContext() const {
    return destination_subpass_contexts[current_subpass];
}

const AccessContext& RenderPassReplayState::GetCurrentRecordedContext() const {
    return rp_context->GetSubpassContexts()[current_subpass];
}

}  // namespace syncval
