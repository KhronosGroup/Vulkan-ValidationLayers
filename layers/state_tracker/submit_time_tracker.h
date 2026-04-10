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
#pragma once

#include "containers/custom_containers.h"
#include "containers/span.h"
#include "error_message/error_location.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace vvl {

class DeviceProxy;
class CommandBuffer;

// Batch blocked by a wait-before-signal dependency
struct UnresolvedBatch {
    UnresolvedBatch(const Location& submit_loc) : submit_loc_capture(submit_loc) {}

    LocationCapture submit_loc_capture;

    // Command buffers to validate when dependencies are resolved
    std::vector<std::shared_ptr<CommandBuffer>> command_buffers;

    // Timeline waits that block this batch
    std::vector<VkSemaphoreSubmitInfo> unresolved_timeline_waits;

    // Semaphores to signal once all waits are resolved
    std::vector<VkSemaphoreSubmitInfo> signals;
};

// Tracks submission batches and initiates submit time validation.
// Submit time validation is performed during submit calls such as QueueSubmit and QueuePresent.
// Validation that depends on actual completion of queue operations is not handled by this subsystem.
//
// Batches with no unresolved dependencies are validated immediately.
// Batches with pending timeline waits are stored until a later submit call resolves the wait.
// vkSignalSemaphore also triggers validation if it resolves a pending wait
class SubmitTimeTracker {
  public:
    SubmitTimeTracker(DeviceProxy& validator) : validator_(validator) {}
    void OnCreateTimelineSemaphore(VkSemaphore timeline, uint64_t initial_value);
    void OnDestroyTimelineSemaphore(VkSemaphore timeline);

    bool ProcessSubmitInfo(const VkSubmitInfo& submit_info, VkQueue queue, const Location& submit_loc) const;
    bool ProcessSubmitInfo(const VkSubmitInfo2& submit_info, VkQueue queue, const Location& submit_loc) const;
    bool ProcessSignalSemaphore(const VkSemaphoreSignalInfo& signal_info) const;
    bool ProcessPresent(const VkPresentInfoKHR& present_info, const Location& present_info_loc) const;

    std::optional<uint64_t> GetTimelineValue(VkSemaphore timeline) const;

  private:
    bool ProcessBatch(std::vector<std::shared_ptr<CommandBuffer>>&& command_buffers,
                      vvl::span<const VkSemaphoreSubmitInfo> wait_semaphores,
                      vvl::span<const VkSemaphoreSubmitInfo> signal_semaphores, VkQueue queue, const Location& submit_loc);
    bool ProcessSignal(VkSemaphore timeline, uint64_t signal_value);

    std::vector<VkSemaphoreSubmitInfo> GetUnresolvedTimelineWaits(vvl::span<const VkSemaphoreSubmitInfo> wait_semaphores);
    bool RegisterTimelineSignals(vvl::span<const VkSemaphoreSubmitInfo> signal_semaphores);
    bool PropagateTimelineSignals();
    bool CanBeResolved(const UnresolvedBatch& batch) const;
    bool UpdateTimelineValue(VkSemaphore timeline, uint64_t signal_value);

  private:
    // Submit time validation and state updates run under this mutex.
    // A batch submitted on one queue may be resolved by another due to wait-before-signal.
    // Serializing access to this state ensures the resolving queue validates the batch
    // immediately, rather than deferring validation until a later submit on the original queue
    mutable std::mutex mutex_;

    DeviceProxy& validator_;

    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals_;
    vvl::unordered_map<VkQueue, std::vector<UnresolvedBatch>> unresolved_batches_;
};

}  // namespace vvl
