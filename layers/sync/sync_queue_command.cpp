/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync/sync_queue_command.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

namespace syncval {

bool WaitForFenceCommand::Execute(SyncValidator& validator) const {
    validator.WaitForFence(fence);
    return false;
}

bool WaitForSemaphoreCommand::Execute(SyncValidator& validator) const {
    validator.WaitForSemaphore(semaphore, value);
    return false;
}

bool SignalSemaphoreCommand::Execute(SyncValidator& validator) const {
    bool skip = false;

    SignalsUpdate signals_update(validator);
    std::vector<SignalInfo>& signals = signals_update.timeline_signals[semaphore];

    // Reject invalid signal
    if (!signals.empty() && value <= signals.back().timeline_value) {
        return skip;  // [core validation check]: strictly increasing signal values
    }

    signals.emplace_back(SignalInfo(value));
    skip |= validator.PropagateTimelineSignals(signals_update);

    if (!skip) {
        validator.ApplySignalsUpdate(signals_update, nullptr);
    }
    return skip;
}

bool HostResetEventCommand::Execute(SyncValidator& validator) const {
    if (auto event_state = validator.Get<vvl::Event>(event)) {
        const auto all_batches = validator.GetAllQueueBatchContexts();
        for (const auto& batch : all_batches) {
            SyncEventsContext& events_context = batch->GetEventsContext();
            if (SyncEventState* sync_event = events_context.GetFromShared(event_state)) {
                sync_event->last_command = vvl::Func::vkResetEvent;
                sync_event->unsynchronized_set = vvl::Func::Empty;
                sync_event->ResetFirstScope();
                sync_event->barriers = 0;
            }
        }
    }
    return false;
}

bool QueueWaitIdleCommand::Execute(SyncValidator& validator) const {
    const QueueId waited_queue = validator.GetQueueId(queue);
    if (waited_queue == kQueueIdInvalid) {
        return false;
    }
    const QueueState& queue_state = validator.GetQueueState(waited_queue);
    validator.ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex, queue_state.GetLastSynchronizedPresent(), {});

    // For each timeline, remove all signals signaled on the waited queue, except the last one.
    // The last signal is needed to represent the current timeline state
    validator.EnsureTimelineSignalsLimit(1, waited_queue);

    // Eliminate host waitable objects from the current queue
    vvl::EraseIf(validator.waitable_fences_, [waited_queue](const auto& sf) { return sf.second.queue_id == waited_queue; });
    for (auto& [semaphore, host_sync_state] : validator.host_waitable_semaphores_) {
        vvl::EraseIf(host_sync_state.sync_points,
                     [waited_queue](const auto& sync_point) { return sync_point.queue_id == waited_queue; });
    }
    return false;
}

bool DeviceWaitIdleCommand::Execute(SyncValidator& validator) const {
    const auto batches = validator.GetAllQueueBatchContexts();

    // Collect information about last synchronized present over all queues
    LastSynchronizedPresent global_last_synchronized_present;
    for (const auto& batch : batches) {
        global_last_synchronized_present.Merge(batch->last_synchronized_present);
    }

    // Device wait will preserve unsynchronized present operations
    for (const auto& batch : batches) {
        batch->ApplyDeviceWait(global_last_synchronized_present);
    }

    // For each timeline keep only the last signal per queue.
    // The last signal is needed to represent the current timeline state
    validator.EnsureTimelineSignalsLimit(1);

    // Cleanup fence waits associated with queues. Acquire fence waits are preserved
    auto is_queue_fence = [](const auto& waitable) { return waitable.second.queue_id != kQueueIdInvalid; };
    vvl::EraseIf(validator.waitable_fences_, is_queue_fence);
    for (auto& [semaphore, host_sync_state] : validator.host_waitable_semaphores_) {
        host_sync_state.sync_points.clear();
    }
    return false;
}

bool ResourceDestroyedCommand::Execute(SyncValidator& validator) const {
    for (const auto& batch : validator.GetAllQueueBatchContexts()) {
        for (const AccessRange& range : ranges) {
            batch->OnResourceDestroyed(range);
        }
        batch->Trim();
    }
    return false;
}

bool SemaphoreDestroyedCommand::Execute(SyncValidator& validator) const {
    if (auto it = validator.timeline_signals_.find(semaphore); it != validator.timeline_signals_.end()) {
        validator.stats.RemoveTimelineSignals((uint32_t)it->second.size());
        validator.timeline_signals_.erase(it);
    }
    validator.host_waitable_semaphores_.erase(semaphore);
    return false;
}

bool SwapchainDestroyedCommand::Execute(SyncValidator& validator) const {
    for (const auto& batch : validator.GetAllQueueBatchContexts()) {
        batch->last_synchronized_present.OnDestroySwapchain(swapchain);
    }
    return false;
}

QueuePresentCommand::QueuePresentCommand(SyncValidator& validator, QueueId queue_id, const VkPresentInfoKHR& present_info)
    : queue_id(queue_id) {
    assert(queue_id != kQueueIdInvalid);

    wait_semaphores.reserve(present_info.waitSemaphoreCount);
    for (VkSemaphore semaphore : vvl::make_span(present_info.pWaitSemaphores, present_info.waitSemaphoreCount)) {
        wait_semaphores.emplace_back(semaphore);
    }
    images.reserve(present_info.swapchainCount);
    for (uint32_t i = 0; i < present_info.swapchainCount; ++i) {
        auto swapchain = validator.Get<vvl::Swapchain>(present_info.pSwapchains[i]);
        if (!swapchain) {
            continue;
        }
        const uint32_t image_index = present_info.pImageIndices[i];
        auto image = swapchain->GetSwapChainImageShared(image_index);
        if (!image) {
            continue;
        }
        images.emplace_back(ImageInfo{std::move(swapchain), std::move(image), i, image_index});
    }
}

bool QueuePresentCommand::Execute(SyncValidator& validator) const {
    bool skip = false;
    QueueState& queue_state = validator.GetQueueState(queue_id);
    const uint64_t submit_id = queue_state.ReserveSubmitId();

    BatchContextPtr batch(std::make_shared<QueueBatchContext>(validator, queue_state));

    PresentedImages presented_images(images.size());
    for (uint32_t i = 0; i < images.size(); i++) {
        const ImageInfo& image_info = images[i];
        PresentedImage& presented = presented_images[i];

        // A tag per presented image. Acquire waits identify presents by the tag.
        // This local tag will be offset by the global tag
        presented.tag = i;

        presented.swapchain_index = image_info.swapchain_index;
        presented.image_index = image_info.image_index;
        presented.image = image_info.image;
        presented.batch = batch;
        presented.swapchain = image_info.swapchain->VkHandle();
        presented.range_gen = SubState(*image_info.image).MakeImageRangeGen(image_info.image->full_range, false);
    }

    SignalsUpdate signals_update(validator);
    auto resolved_batches = batch->ResolvePresentWaits(vvl::make_span(wait_semaphores), presented_images, signals_update);

    batch->ResolveLastBatch(queue_state.LastBatch(), resolved_batches);

    // Keep async batches alive while validation uses their contexts
    // through raw pointers (AsyncReference::context_)
    const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

    // Convert present tags to global range
    const ResourceUsageTag global_range_start = batch->SetupBatchTags(uint32_t(presented_images.size()));
    for (PresentedImage& presented : presented_images) {
        presented.tag += global_range_start;
    }

    skip |= batch->DoQueuePresentValidate(presented_images);
    batch->DoPresentOperations(presented_images);
    batch->LogPresentOperations(presented_images, submit_id);

    // Update state if there are no validation errors
    if (!skip) {
        validator.stats.UpdateAccessStats(validator);
        validator.stats.UpdateMemoryStats();
        queue_state.SetLastBatch(std::move(batch));
        validator.ApplySignalsUpdate(signals_update, queue_state.LastBatch());
        for (uint32_t i = 0; i < images.size(); ++i) {
            SubState(*images[i].swapchain).RecordPresentedImage(std::move(presented_images[i]));
        }
    }
    return skip;
}

bool AcquireNextImageCommand::Execute(SyncValidator& validator) const {
    // Consume the present record after earlier present commands have executed
    const PresentedImage presented = SubState(*swapchain).MovePresentedImage(image_index, image);

    // We create a queue-less QBC for the semaphore and fence to wait on.
    // NOTE: this is a heavyweight way to deal with the fact that all operation
    // logs live in the QueueBatchContext. Acquire doesn't happen on a queue,
    // but we need a place to put the acquire operation access record
    auto batch = std::make_shared<QueueBatchContext>(validator);
    batch->SetupAccessContext(presented);
    const ResourceUsageTag acquire_tag = batch->SetupBatchTags(1);
    batch->DoAcquireOperation(presented);
    batch->LogAcquireOperation(presented, command);

    if (semaphore != VK_NULL_HANDLE) {
        // Keep the existing signal if the semaphore is signaled twice.
        // NOTE: core validation reports the duplicate
        validator.binary_signals_.emplace(semaphore, SignalInfo(batch, presented, acquire_tag));
    }
    if (fence != VK_NULL_HANDLE) {
        FenceHostSyncPoint sync_point;
        sync_point.tag = acquire_tag;
        sync_point.acquired = AcquiredImage(presented, acquire_tag);
        validator.waitable_fences_[fence] = std::move(sync_point);
    }
    return false;
}

QueueSubmitCommand::QueueSubmitCommand(SyncValidator& validator, QueueId queue_id, vvl::span<const VkSubmitInfo2> submits,
                                       VkFence fence, vvl::Func submit_func)
    : queue_id(queue_id),
      fence(fence),
      submit_func(submit_func),
      initial_label_stack(validator.GetQueueState(queue_id).GetQueue()->cmdbuf_label_stack) {
    batches.reserve(submits.size());
    for (const VkSubmitInfo2& submit : submits) {
        SubmitBatch& batch = batches.emplace_back();

        batch.waits.reserve(submit.waitSemaphoreInfoCount);
        for (const auto& wait : vvl::make_span(submit.pWaitSemaphoreInfos, submit.waitSemaphoreInfoCount)) {
            batch.waits.emplace_back(wait);
        }

        batch.command_buffers.reserve(submit.commandBufferInfoCount);
        for (const auto& cb_info : vvl::make_span(submit.pCommandBufferInfos, submit.commandBufferInfoCount)) {
            // Append even a null lookup result so later command buffers are reported at their original indices
            batch.command_buffers.emplace_back(validator.Get<vvl::CommandBuffer>(cb_info.commandBuffer));
        }

        batch.signals.reserve(submit.signalSemaphoreInfoCount);
        for (const auto& signal : vvl::make_span(submit.pSignalSemaphoreInfos, submit.signalSemaphoreInfoCount)) {
            batch.signals.emplace_back(signal);
        }
    }
}

bool QueueSubmitCommand::Execute(SyncValidator& validator) const {
    bool skip = false;

    QueueState& queue_state = validator.GetQueueState(queue_id);
    SignalsUpdate signals_update(validator);

    const uint64_t submit_id = queue_state.ReserveSubmitId();
    const Location submit_loc(submit_func);

    // Update label stack as we progress through batches and command buffers
    auto current_label_stack = initial_label_stack;

    BatchContextPtr last_batch = queue_state.LastBatch();
    bool has_unresolved_batches = !queue_state.UnresolvedBatches().empty();

    BatchContextPtr new_last_batch;
    std::vector<UnresolvedBatch> new_unresolved_batches;
    bool new_timeline_signals = false;

    for (uint32_t batch_idx = 0; batch_idx < batches.size(); batch_idx++) {
        const SubmitBatch& submit = batches[batch_idx];
        auto batch = std::make_shared<QueueBatchContext>(validator, queue_state);

        std::vector<VkSemaphoreSubmitInfo> unresolved_waits;
        auto resolved_batches = batch->ResolveSubmitWaits(submit.waits, unresolved_waits, signals_update);

        // Add unresolved batch
        if (has_unresolved_batches || !unresolved_waits.empty()) {
            UnresolvedBatch& unresolved_batch = new_unresolved_batches.emplace_back();
            unresolved_batch.batch = std::move(batch);
            unresolved_batch.submit_func = submit_func;
            unresolved_batch.submit_index = submit_id;
            unresolved_batch.batch_index = batch_idx;
            unresolved_batch.command_buffers.assign(submit.command_buffers.begin(), submit.command_buffers.end());
            unresolved_batch.unresolved_waits = std::move(unresolved_waits);
            unresolved_batch.resolved_dependencies = std::move(resolved_batches);
            unresolved_batch.signals.assign(submit.signals.begin(), submit.signals.end());
            unresolved_batch.label_stack = current_label_stack;

            has_unresolved_batches = true;
            validator.stats.AddUnresolvedBatch();
            continue;
        }
        new_last_batch = batch;

        batch->ResolveLastBatch(std::move(last_batch), resolved_batches);

        // Keep async batches alive while validation uses their contexts
        // through raw pointers (AsyncReference::context_)
        // TODO: All syncval tests pass when the return value is ignored. Write a regression test that fails/crashes in this case
        const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

        skip |= batch->ValidateSubmit(submit.command_buffers, submit_id, batch_idx, current_label_stack, submit_loc);

        new_timeline_signals |= signals_update.RegisterSignals(batch, submit.signals);

        // Unless the previous batch was referenced by a signal it will self destruct
        // in the record phase when the last batch is updated.
        last_batch = batch;
    }

    if (new_last_batch && !skip) {
        queue_state.SetLastBatch(std::move(new_last_batch));
    }
    if (!new_unresolved_batches.empty() && !skip) {
        vvl::Append(queue_state.UnresolvedBatches(), new_unresolved_batches);
    }

    // Check if timeline signals resolve existing wait-before-signal dependencies
    if (new_timeline_signals) {
        skip |= validator.PropagateTimelineSignals(signals_update);
    }

    if (!skip) {
        validator.stats.UpdateMemoryStats();
        validator.ApplySignalsUpdate(signals_update, queue_state.LastBatch());
        FenceHostSyncPoint sync_point;
        sync_point.queue_id = queue_id;
        sync_point.tag = validator.ReserveGlobalTagRange(1).begin;
        if (queue_state.LastBatch()) {
            sync_point.queue_sync_tags = queue_state.LastBatch()->GetQueueSyncTags();
        }
        if (fence != VK_NULL_HANDLE) {
            validator.waitable_fences_[fence] = std::move(sync_point);
        }
    }
    return skip;
}

bool IsQueueCommandReady(const QueueCommand& command) {
    return std::visit([](const auto& operation) { return operation.IsReady(); }, command);
}

bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator) {
    return std::visit([&validator](const auto& operation) { return operation.Execute(validator); }, command);
}

}  // namespace syncval
