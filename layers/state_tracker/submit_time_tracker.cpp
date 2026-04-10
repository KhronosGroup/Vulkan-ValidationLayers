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

#include "state_tracker/submit_time_tracker.h"
#include "state_tracker/semaphore_state.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/wsi_state.h"
#include "containers/container_utils.h"
#include "utils/convert_utils.h"

namespace vvl {

void SubmitTimeTracker::OnCreateTimelineSemaphore(VkSemaphore timeline, uint64_t initial_value) {
    std::lock_guard lock(mutex_);
    timeline_signals_[timeline] = initial_value;
}

void SubmitTimeTracker::OnDestroyTimelineSemaphore(VkSemaphore timeline) {
    std::lock_guard lock(mutex_);
    timeline_signals_.erase(timeline);
}

bool SubmitTimeTracker::ProcessSubmitInfo(const VkSubmitInfo& submit_info, VkQueue queue, const Location& submit_loc) const {
    SubmitInfoConverter converter(submit_info);
    return ProcessSubmitInfo(converter.submit_info2, queue, submit_loc);
}

bool SubmitTimeTracker::ProcessSubmitInfo(const VkSubmitInfo2& submit_info, VkQueue queue, const Location& submit_loc) const {
    const auto wait_semaphores =
        vvl::span<const VkSemaphoreSubmitInfo>(submit_info.pWaitSemaphoreInfos, submit_info.waitSemaphoreInfoCount);
    const auto signal_semaphores =
        vvl::span<const VkSemaphoreSubmitInfo>(submit_info.pSignalSemaphoreInfos, submit_info.signalSemaphoreInfoCount);

    std::vector<std::shared_ptr<CommandBuffer>> command_buffers;
    command_buffers.reserve(submit_info.commandBufferInfoCount);
    for (const auto& cb_info : vvl::make_span(submit_info.pCommandBufferInfos, submit_info.commandBufferInfoCount)) {
        // If Get returns null, store it to preserve original indexing
        command_buffers.emplace_back(validator_.Get<CommandBuffer>(cb_info.commandBuffer));
    }

    std::lock_guard lock(mutex_);
    SubmitTimeTracker& this_tracker = *const_cast<SubmitTimeTracker*>(this);
    return this_tracker.ProcessBatch(std::move(command_buffers), wait_semaphores, signal_semaphores, queue, submit_loc);
}

bool SubmitTimeTracker::ProcessSignalSemaphore(const VkSemaphoreSignalInfo& signal_info) const {
    std::lock_guard lock(mutex_);
    SubmitTimeTracker& this_tracker = *const_cast<SubmitTimeTracker*>(this);
    return this_tracker.ProcessSignal(signal_info.semaphore, signal_info.value);
}

// NOTE: when swapchain learns how to work with timeline semaphores, this function should be
// reworked to check for resolving timeline signals similar to ProcessBatch.
// Current version assumes only binary semaphores are allowed (the only option as of April 2026)
bool SubmitTimeTracker::ProcessPresent(const VkPresentInfoKHR& present_info, const Location& present_info_loc) const {
    std::lock_guard lock(mutex_);
    bool skip = false;
    for (uint32_t i = 0; i < present_info.swapchainCount; i++) {
        if (auto swapchain_state = validator_.Get<Swapchain>(present_info.pSwapchains[i])) {
            const uint32_t image_index = present_info.pImageIndices[i];
            if (image_index >= swapchain_state->images.size()) {
                continue;  // invalid image index, reported elsewhere
            }
            skip |= validator_.ProcessPresentBatch(*swapchain_state->images[image_index].image_state, present_info_loc);
        }
    }
    return skip;
}

bool SubmitTimeTracker::ProcessBatch(std::vector<std::shared_ptr<CommandBuffer>>&& command_buffers,
                                     vvl::span<const VkSemaphoreSubmitInfo> wait_semaphores,
                                     vvl::span<const VkSemaphoreSubmitInfo> signal_semaphores, VkQueue queue,
                                     const Location& submit_loc) {
    bool skip = false;
    std::vector<UnresolvedBatch>& unresolved_batches = unresolved_batches_[queue];
    std::vector<VkSemaphoreSubmitInfo> unresolved_timeline_waits = GetUnresolvedTimelineWaits(wait_semaphores);

    // Add wait-before-signal batches to unresolved list and return
    const bool has_pending_waits = !unresolved_batches.empty() || !unresolved_timeline_waits.empty();
    if (has_pending_waits) {
        UnresolvedBatch batch(submit_loc);
        batch.command_buffers = std::move(command_buffers);
        batch.unresolved_timeline_waits = std::move(unresolved_timeline_waits);
        batch.signals.assign(signal_semaphores.begin(), signal_semaphores.end());
        unresolved_batches.emplace_back(std::move(batch));
        return skip;
    }

    skip |= validator_.ProcessSubmissionBatch(command_buffers, submit_loc);

    const bool new_timeline_signals = RegisterTimelineSignals(signal_semaphores);
    if (new_timeline_signals) {
        skip |= PropagateTimelineSignals();
    }
    return skip;
}

bool SubmitTimeTracker::ProcessSignal(VkSemaphore timeline, uint64_t signal_value) {
    bool skip = false;
    const bool new_timeline_signal = UpdateTimelineValue(timeline, signal_value);
    if (new_timeline_signal) {
        skip |= PropagateTimelineSignals();
    }
    return skip;
}

std::vector<VkSemaphoreSubmitInfo> SubmitTimeTracker::GetUnresolvedTimelineWaits(
    vvl::span<const VkSemaphoreSubmitInfo> wait_semaphores) {
    std::vector<VkSemaphoreSubmitInfo> unresolved;
    for (const auto& wait : wait_semaphores) {
        const std::optional<uint64_t> current_value = GetTimelineValue(wait.semaphore);
        if (!current_value.has_value()) {
            // Invalid or external semaphores should not block this batch
            continue;
        }
        if (wait.value > *current_value) {
            unresolved.emplace_back(wait);
        }
    }
    return unresolved;
}

bool SubmitTimeTracker::RegisterTimelineSignals(vvl::span<const VkSemaphoreSubmitInfo> signal_semaphores) {
    bool new_timeline_signals = false;
    for (const VkSemaphoreSubmitInfo& signal : signal_semaphores) {
        new_timeline_signals |= UpdateTimelineValue(signal.semaphore, signal.value);
    }
    return new_timeline_signals;
}

bool SubmitTimeTracker::PropagateTimelineSignals() {
    bool skip = false;

    // The caller ensures we just registered new timeline signals
    bool new_timeline_signals = true;

    // Each iteration attempts to resolve pending batches using current timeline value.
    // If a resolved batch generates new timeline signals, the loop runs again
    while (new_timeline_signals) {
        new_timeline_signals = false;

        for (auto& [queue, batches] : unresolved_batches_) {
            while (!batches.empty()) {
                UnresolvedBatch& batch = batches.front();
                if (!CanBeResolved(batch)) {
                    break;
                }
                skip |= validator_.ProcessSubmissionBatch(batch.command_buffers, batch.submit_loc_capture.Get());
                new_timeline_signals |= RegisterTimelineSignals(batch.signals);
                batches.erase(batches.begin());
            }
        }
    }
    return skip;
}

bool SubmitTimeTracker::CanBeResolved(const UnresolvedBatch& batch) const {
    for (const VkSemaphoreSubmitInfo& wait : batch.unresolved_timeline_waits) {
        const std::optional<uint64_t> current_value = GetTimelineValue(wait.semaphore);
        if (!current_value.has_value()) {
            // Invalid or external semaphores should not block this batch
            continue;
        }
        if (wait.value > current_value) {
            return false;
        }
    }
    return true;
}

std::optional<uint64_t> SubmitTimeTracker::GetTimelineValue(VkSemaphore timeline) const {
    auto semaphore_state = validator_.Get<Semaphore>(timeline);
    if (!semaphore_state || semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE ||
        semaphore_state->Scope() != Semaphore::kInternal) {
        // Used by the caller to detect invalid/non-timeline/external semaphores
        return {};
    }
    const uint64_t current_value = vvl::FindExisting(timeline_signals_, timeline);
    return current_value;
}

bool SubmitTimeTracker::UpdateTimelineValue(VkSemaphore timeline, uint64_t signal_value) {
    auto semaphore_state = validator_.Get<Semaphore>(timeline);
    if (!semaphore_state || semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        return false;
    }
    uint64_t& current_value = vvl::FindExisting(timeline_signals_, timeline);
    if (signal_value <= current_value) {
        return false;  // non-increasing signal, the error should be reported elsewhere
    }
    current_value = signal_value;
    return true;
}

}  // namespace vvl
