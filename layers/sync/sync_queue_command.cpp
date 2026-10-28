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
    for (auto& [semaphore, sync_points] : validator.host_waitable_semaphores_) {
        vvl::EraseIf(sync_points, [waited_queue](const auto& sync_point) { return sync_point.queue_id == waited_queue; });
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
    validator.host_waitable_semaphores_.clear();
    return false;
}

bool IsQueueCommandReady(const QueueCommand& command) {
    return std::visit([](const auto& operation) { return operation.IsReady(); }, command);
}

bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator) {
    return std::visit([&validator](const auto& operation) { return operation.Execute(validator); }, command);
}

}  // namespace syncval
