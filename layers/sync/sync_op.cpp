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

void SyncOpPipelineBarrier::ReplayRecord(CommandExecutionContext& exec_context, const ResourceUsageTag exec_tag) const {
    ApplyBarrier(exec_context, barrier_set_, exec_tag);
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
    CommandExecutionContext& exec_context = replay.exec_context;
    const SyncValidator& validator = exec_context.GetSyncState();
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return validator.ValidateCmdSetEvent(exec_context, event_, src_exec_scope_, exec_tag, Location(command_));
}

void SyncOpSetEvent::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // Create a copy of the current context, and merge in the state snapshot at record set event time
    // Note: we mustn't change the recorded context copy, as a given CB could be submitted more than once (in generaL)

    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    // Note: merged_context is a copy of the access_context, combined with the recorded context
    auto merged_context = std::make_shared<AccessContext>(*access_context.validator);
    merged_context->InitFrom(access_context);
    merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(queue_id, exec_tag), *recorded_context_);
    merged_context->TrimAndClearFirstAccess();  // Ensure the copy is minimal and normalized

    const SyncValidator& validator = exec_context.GetSyncState();
    validator.ApplySetEvent(exec_context, event_, src_exec_scope_, merged_context, exec_tag, command_);
}

SyncOpResetEvent::SyncOpResetEvent(std::shared_ptr<const vvl::Event>&& event, const SyncExecScope& exec_scope, const Location& loc)
    : SyncOpBase(loc.function), event_(std::move(event)), exec_scope_(exec_scope) {}

bool SyncOpResetEvent::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    CommandExecutionContext& exec_context = replay.exec_context;
    const SyncValidator& validator = exec_context.GetSyncState();
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return validator.ValidateCmdResetEvent(exec_context, event_, exec_scope_, exec_tag, Location(command_));
}

void SyncOpResetEvent::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    const SyncValidator& validator = exec_context.GetSyncState();
    validator.ApplyResetEvent(exec_context, event_, exec_tag, command_);
}

SyncOpWaitEvents::SyncOpWaitEvents(std::vector<std::shared_ptr<const vvl::Event>>&& events, std::vector<BarrierSet>&& barrier_sets,
                                   const Location& loc)
    : SyncOpBase(loc.function), events_(std::move(events)), barrier_sets_(std::move(barrier_sets)) {}

void SyncOpWaitEvents::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    ApplyWaitEvents(exec_context, events_, barrier_sets_, exec_tag, command_);
}

bool SyncOpWaitEvents::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    CommandExecutionContext& exec_context = replay.exec_context;
    const SyncValidator& validator = exec_context.GetSyncState();
    const ResourceUsageTag exec_tag = replay.base_tag + recorded_tag;
    return validator.ValidateCmdWaitEvents(exec_context, events_, barrier_sets_, exec_tag, Location(command_));
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(std::shared_ptr<const vvl::RenderPass>&& rp_state,
                                             std::vector<std::shared_ptr<const vvl::ImageView>>&& attachments,
                                             const RenderPassAccessContext* rp_context, const Location& loc)
    : SyncOpBase(loc.function), rp_state_(std::move(rp_state)), attachments_(std::move(attachments)), rp_context_(rp_context) {}

bool SyncOpBeginRenderPass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // Render pass ops are not allowed in secondary command buffers, so batch context is valid
    QueueBatchContext& batch_context = *replay.batch_context;

    AccessContext* rp_access_context =
        replay.rp_replay.Begin(batch_context.GetQueueFlags(), *this, batch_context.GetAccessContext());
    batch_context.SetRenderPassAccessContext(*rp_access_context);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBeginRenderPass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpNextSubpass::SyncOpNextSubpass(const Location& loc) : SyncOpBase(loc.function) {}

bool SyncOpNextSubpass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // Render pass ops are not allowed in secondary command buffers, so batch context is valid
    QueueBatchContext& batch_context = *replay.batch_context;

    // Any store/resolve operations happen before the NextSubpass tag so we can advance to the next subpass state
    AccessContext* rp_access_context = replay.rp_replay.Next();
    batch_context.SetRenderPassAccessContext(*rp_access_context);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpNextSubpass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
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

    // Render pass ops are not allowed in secondary command buffers, so batch context is valid
    QueueBatchContext& batch_context = *replay.batch_context;

    replay.rp_replay.End(batch_context.GetAccessContext());
    batch_context.SetOriginalAccessContext();

    // Validate final layout transition
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    bool skip = false;
    skip |= replay.DetectFirstUseHazard(first_use_range);

    return skip;
}

void SyncOpEndRenderPass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {}

ReplayState::ReplayState(QueueBatchContext& exec_context, const CommandBufferAccessContext& recorded_context,
                         const Location& cb_loc, ResourceUsageTag base_tag)
    : exec_context(exec_context),
      batch_context(&exec_context),
      recorded_context(recorded_context),
      cb_loc(cb_loc),
      base_tag(base_tag) {}

ReplayState::ReplayState(CommandBufferAccessContext& exec_context, const CommandBufferAccessContext& recorded_context,
                         const Location& cb_loc, ResourceUsageTag base_tag)
    : exec_context(exec_context), batch_context(nullptr), recorded_context(recorded_context), cb_loc(cb_loc), base_tag(base_tag) {}

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const {
    bool skip = false;
    if (!first_use_range.non_empty()) {
        return skip;
    }
    const AccessContext& recorded_access_context =
        rp_replay.replay_context ? *rp_replay.replay_context : recorded_context.GetCurrentAccessContext();
    const HazardResult hazard = recorded_access_context.DetectFirstUseHazard(exec_context.GetQueueId(), first_use_range,
                                                                             exec_context.GetCurrentAccessContext());
    if (hazard.IsHazard()) {
        const SyncValidator& sync_state = exec_context.GetSyncState();
        LogObjectList objlist(exec_context.Handle(), recorded_context.Handle());
        const std::string error = sync_state.error_messages_.FirstUseError(hazard, exec_context, recorded_context, cb_loc.index);
        skip |= sync_state.SyncError(hazard.Hazard(), objlist, cb_loc, error);
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
        sync_op.sync_op->ReplayRecord(exec_context, base_tag + sync_op.tag);

        // Advance past sync_op
        first_use_range.begin = sync_op.tag + 1;
    }

    // Validate first accesses after the last syncop
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);
    return skip;
}

AccessContext* RenderPassReplayState::Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass& begin_op_,
                                            const AccessContext& external_context) {
    const RenderPassAccessContext* rp_context = begin_op_.GetRenderPassAccessContext();
    assert(rp_context);

    begin_op = &begin_op_;
    replay_context = &rp_context->GetSubpassContexts()[0];
    subpass = 0;
    subpass_contexts = InitSubpassContexts(queue_flags, *rp_context->GetRenderPassState(), external_context);

    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (AccessContext& context : GetSubpassContexts()) {
        context.ClearAsyncContexts();
        context.ImportAsyncContexts(external_context);
    }

    return &subpass_contexts[0];
}

AccessContext* RenderPassReplayState::Next() {
    subpass++;

    const RenderPassAccessContext* rp_context = begin_op->GetRenderPassAccessContext();

    replay_context = &rp_context->GetSubpassContexts()[subpass];
    return &subpass_contexts[subpass];
}

void RenderPassReplayState::End(AccessContext& external_context) {
    external_context.ResolveChildContexts(GetSubpassContexts());
    *this = RenderPassReplayState{};
}

vvl::span<AccessContext> RenderPassReplayState::GetSubpassContexts() {
    return vvl::make_span(subpass_contexts.get(),
                          begin_op->GetRenderPassAccessContext()->GetRenderPassState()->create_info.subpassCount);
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
