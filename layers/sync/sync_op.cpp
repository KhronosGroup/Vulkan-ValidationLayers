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
#include "sync/sync_access_context.h"
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

SyncOpPipelineBarrier::SyncOpPipelineBarrier(BarrierSet&& barrier_set) : barrier_set_(std::move(barrier_set)) {}

void SyncOpPipelineBarrier::ReplayRecord(SyncEnvironment& env, AccessContext& access_context,
                                         const ResourceUsageTag exec_tag) const {
    ApplyBarrier(env, access_context, barrier_set_, exec_tag);
}

bool SyncOpPipelineBarrier::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // The layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

SyncOpSetEvent::SyncOpSetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& src_exec_scope,
                               std::shared_ptr<const AccessContext>&& src_access_context, const Location& loc)
    : SyncOpBase(loc.function),
      event_(std::move(event)),
      recorded_context_(std::move(src_access_context)),
      src_exec_scope_(src_exec_scope) {}

bool SyncOpSetEvent::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return ValidateCmdSetEvent(replay.env, event_, src_exec_scope_, exec_tag, Location(command_));
}

void SyncOpSetEvent::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {
    // Create a copy of the current context, and merge in the state snapshot at record set event time
    // Note: we mustn't change the recorded context copy, as a given CB could be submitted more than once (in generaL)

    // Note: merged_context is a copy of the access_context, combined with the recorded context
    auto merged_context = std::make_shared<AccessContext>(*access_context.validator);
    merged_context->InitFrom(access_context);
    merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(env.queue_id, exec_tag), *recorded_context_);
    merged_context->TrimAndClearFirstAccess();  // Ensure the copy is minimal and normalized

    ApplyCmdSetEvent(env, event_, src_exec_scope_, merged_context, exec_tag, command_);
}

SyncOpResetEvent::SyncOpResetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc)
    : SyncOpBase(loc.function), event_(std::move(event)), exec_scope_(exec_scope) {}

bool SyncOpResetEvent::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return ValidateCmdResetEvent(replay.env, event_, exec_scope_, exec_tag, Location(command_));
}

void SyncOpResetEvent::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {
    ApplyCmdResetEvent(env, event_, exec_tag, command_);
}

SyncOpWaitEvents::SyncOpWaitEvents(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                                   const Location& loc)
    : SyncOpBase(loc.function), events_(std::move(events)), barrier_sets_(std::move(barrier_sets)) {}

void SyncOpWaitEvents::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {
    ApplyCmdWaitEvents(env, access_context, events_, barrier_sets_, exec_tag, command_);
}

bool SyncOpWaitEvents::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return ValidateCmdWaitEvents(replay.env, replay.GetReplayContext(), events_, barrier_sets_, exec_tag, Location(command_));
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(std::shared_ptr<const vvl::RenderPass>&& rp_state,
                                             std::vector<std::shared_ptr<const vvl::ImageView>>&& attachments,
                                             const RenderPassAccessContext* rp_context, const Location& loc)
    : SyncOpBase(loc.function), rp_state_(std::move(rp_state)), attachments_(std::move(attachments)), rp_context_(rp_context) {}

bool SyncOpBeginRenderPass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    replay.rp_replay.emplace(rp_context_, replay.replay_context, replay.env.queue_flags);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBeginRenderPass::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpNextSubpass::SyncOpNextSubpass(const Location& loc) : SyncOpBase(loc.function) {}

bool SyncOpNextSubpass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    bool skip = false;
    const uint32_t subpass_count = replay.rp_replay->rp_context->GetRenderPassState()->create_info.subpassCount;
    if (replay.rp_replay->current_subpass + 1 >= subpass_count) {
        return skip;
    }

    // Any store/resolve operations happen before the NextSubpass tag so we can advance to the next subpass state
    replay.rp_replay->current_subpass++;

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    skip |= replay.DetectFirstUseHazard(first_use_range);
    return skip;
}

void SyncOpNextSubpass::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpEndRenderPass::SyncOpEndRenderPass(const Location& loc) : SyncOpBase(loc.function) {}

bool SyncOpEndRenderPass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // The record_tag is the final layout transition. Any store/resolve operations happen before
    // the EndRenderPass tag so we can ignore them here.
    //
    // The final layout transition is recorded in command buffer context (not render pass context).
    // Do a render pass cleanup. This also switches replay to command buffer context where we can
    // validate layout transition.

    replay.replay_context.ResolveChildContexts(replay.rp_replay->GetSubpassContexts());
    replay.rp_replay.reset();

    // Validate final layout transition
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    bool skip = false;
    skip |= replay.DetectFirstUseHazard(first_use_range);

    return skip;
}

void SyncOpEndRenderPass::ReplayRecord(SyncEnvironment& env, AccessContext& access_context, ResourceUsageTag exec_tag) const {}

ReplayState::ReplayState(CommandBufferAccessContext& primary_cb_context, const CommandBufferAccessContext& recorded_context,
                         ResourceUsageTag base_tag, const Location& cb_loc)
    : env(primary_cb_context.GetSyncEnvironment()),
      replay_context(primary_cb_context.GetCurrentAccessContext()),
      recorded_context(recorded_context),
      base_tag(base_tag),
      cb_loc(cb_loc) {}

ReplayState::ReplayState(QueueBatchContext& batch_context, const CommandBufferAccessContext& recorded_context,
                         ResourceUsageTag base_tag, const Location& cb_loc)
    : env(batch_context.GetSyncEnvironment()),
      replay_context(batch_context.GetAccessContext()),
      recorded_context(recorded_context),
      base_tag(base_tag),
      cb_loc(cb_loc) {}

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const {
    bool skip = false;
    if (!first_use_range.non_empty()) {
        return skip;
    }
    const AccessContext& recorded_access_context =
        rp_replay ? rp_replay->GetCurrentRecordedContext() : recorded_context.GetCbAccessContext();
    const HazardResult hazard = recorded_access_context.DetectFirstUseHazard(env.queue_id, first_use_range, GetReplayContext());
    if (hazard.IsHazard()) {
        LogObjectList objlist(env.handle, recorded_context.GetCBState().Handle());
        const std::string error = env.validator.error_messages_.FirstUseError(env, hazard, recorded_context, cb_loc.index);
        skip |= env.validator.SyncError(hazard.Hazard(), objlist, cb_loc, error);
    }
    return skip;
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

    for (const auto& sync_op : recorded_context.GetSyncOps()) {
        // Validate all first accesses until the next sync_op
        first_use_range.end = sync_op.tag;
        skip |= DetectFirstUseHazard(first_use_range);

        // Validate and record sync_ops that make memory accesses (for example, image layout transition)
        skip |= sync_op.sync_op->ReplayValidate(*this, sync_op.tag);
        sync_op.sync_op->ReplayRecord(env, GetReplayContext(), base_tag + sync_op.tag);

        // Advance past sync_op
        first_use_range.begin = sync_op.tag + 1;
    }

    // Validate first accesses after the last syncop
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);
    return skip;
}

const AccessContext& ReplayState::GetReplayContext() const {
    return rp_replay ? rp_replay->GetCurrentReplayContext() : replay_context;
}

AccessContext& ReplayState::GetReplayContext() { return rp_replay ? rp_replay->GetCurrentReplayContext() : replay_context; }

RenderPassReplayState::RenderPassReplayState(const RenderPassAccessContext* rp_context, const AccessContext& external_context,
                                             VkQueueFlags queue_flags)
    : rp_context(rp_context), current_subpass(0) {
    subpass_contexts = InitSubpassContexts(queue_flags, *rp_context->GetRenderPassState(), external_context);
    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (AccessContext& context : GetSubpassContexts()) {
        context.ClearAsyncContexts();
        context.ImportAsyncContexts(external_context);
    }
}

AccessContext& RenderPassReplayState::GetCurrentReplayContext() { return GetSubpassContexts()[current_subpass]; }
const AccessContext& RenderPassReplayState::GetCurrentReplayContext() const { return GetSubpassContexts()[current_subpass]; }

const AccessContext& RenderPassReplayState::GetCurrentRecordedContext() const {
    return rp_context->GetSubpassContexts()[current_subpass];
}

vvl::span<AccessContext> RenderPassReplayState::GetSubpassContexts() {
    return vvl::make_span(subpass_contexts.get(), rp_context->GetRenderPassState()->create_info.subpassCount);
}

vvl::span<const AccessContext> RenderPassReplayState::GetSubpassContexts() const {
    return vvl::make_span<const AccessContext>(subpass_contexts.get(), rp_context->GetRenderPassState()->create_info.subpassCount);
}

void SyncEventsContext::ApplyBarrier(const SyncExecScope& src, const SyncExecScope& dst, ResourceUsageTag tag) {
    const bool all_commands_bit = 0 != (src.stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    for (auto& event_pair : map_) {
        assert(event_pair.second);  // Shouldn't be storing empty
        auto& sync_event = *event_pair.second;
        // Events don't happen at a stage, so we need to check and store the unexpanded ALL_COMMANDS if set for inter-event-calls
        // But only if occuring before the tag
        if (((sync_event.barriers & src.exec_scope) || all_commands_bit) && (sync_event.last_command_tag <= tag)) {
            sync_event.barriers |= dst.exec_scope;
            sync_event.barriers |= dst.stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    }
}

void SyncEventsContext::ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag) {
    const SyncExecScope src_scope =
        SyncExecScope::MakeSrc(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_HOST_BIT);
    const SyncExecScope dst_scope = SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    ApplyBarrier(src_scope, dst_scope, tag);
}

SyncEventsContext& SyncEventsContext::DeepCopy(const SyncEventsContext& from) {
    // We need a deep copy of the const context to update during validation phase
    for (const auto& event : from.map_) {
        map_.emplace(event.first, std::make_shared<SyncEventState>(*event.second));
    }
    return *this;
}

void SyncEventsContext::AddReferencedTags(ResourceUsageTagSet& referenced) const {
    for (const auto& event : map_) {
        const std::shared_ptr<const SyncEventState>& event_state = event.second;
        if (event_state) {
            event_state->AddReferencedTags(referenced);
        }
    }
}

SyncEventState::SyncEventState(const SyncEventState::EventPointer& event_state) : SyncEventState() { event = event_state; }

void SyncEventState::ResetFirstScope() {
    first_scope.reset();
    scope = SyncExecScope();
    first_scope_tag = 0;
}

bool SyncEventState::HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope_arg) const {
    return (last_command == vvl::Func::Empty) || (stageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) || (barriers & exec_scope_arg) ||
           (barriers & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

void SyncEventState::AddReferencedTags(ResourceUsageTagSet& referenced) const {
    if (first_scope) {
        first_scope->AddReferencedTags(referenced);
    }
}

}  // namespace syncval
