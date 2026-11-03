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

    auto semaphore_state = validator.Get<vvl::Semaphore>(semaphore);
    if (!semaphore_state) {
        return skip;
    }

    SignalsUpdate signals_update(validator);
    std::vector<SignalInfo>& signals = signals_update.timeline_signals[semaphore];

    // Reject invalid signal
    if (!signals.empty() && value <= signals.back().timeline_value) {
        return skip;  // [core validation check]: strictly increasing signal values
    }

    signals.emplace_back(SignalInfo(semaphore_state, value));
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

    BatchContextPtr last_batch = queue_state.LastBatch();
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

    // Import the previous batch information
    if (last_batch && !vvl::Contains(resolved_batches, last_batch)) {
        batch->ResolveLastBatch(last_batch);
        resolved_batches.emplace_back(std::move(last_batch));
    }

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

bool IsQueueCommandReady(const QueueCommand& command) {
    return std::visit([](const auto& operation) { return operation.IsReady(); }, command);
}

bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator) {
    return std::visit([&validator](const auto& operation) { return operation.Execute(validator); }, command);
}

}  // namespace syncval
