/* Copyright (c) 2024-2026 The Khronos Group Inc.
 * Copyright (c) 2024-2026 Valve Corporation
 * Copyright (c) 2024-2026 LunarG, Inc.
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
#include <vulkan/vulkan_core.h>
#include <optional>

// TODO: temporary use EventMap, but it will be removed
#include "state_tracker/event_map.h"

class CoreChecks;
struct Location;

namespace vvl {
class CommandBuffer;
class Semaphore;
enum class Func;
}  // namespace vvl

// Tracks semaphore state changes during the validation phase of QueueSubmit commands.
// Semaphore state object (vvl::Semaphore) is updated later in the record phase.
struct SemaphoreSubmitState {
    const CoreChecks &core;
    const VkQueue queue;
    const VkQueueFlags queue_flags;

    // Track binary semaphore payload
    vvl::unordered_map<VkSemaphore, bool> binary_signaling_state;

    // Track semaphores that were temporary external and become internal after wait operation.
    vvl::unordered_set<VkSemaphore> internal_semaphores;

    // Track timeline operations
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_waits;

    SemaphoreSubmitState(const CoreChecks &core_, VkQueue q_, VkQueueFlags queue_flags_)
        : core(core_), queue(q_), queue_flags(queue_flags_) {}

    bool CanWaitBinary(const vvl::Semaphore &semaphore_state) const;
    bool CanSignalBinary(const vvl::Semaphore &semaphore_state, VkQueue &other_queue, vvl::Func &other_acquire_command) const;

    std::optional<uint64_t> CheckTimelineMaxDiff(const vvl::Semaphore &semaphore_state, uint64_t value, const char *&payload_type);
    std::optional<uint64_t> CheckTimelineSignalTooSmall(const vvl::Semaphore &semaphore_state, uint64_t value,
                                                        const char *&payload_type);

    VkQueue AnotherQueueWaits(const vvl::Semaphore &semaphore_state) const;

    bool ValidateBinaryWait(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state);
    bool ValidateTimelineWait(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);
    bool ValidateWaitSemaphore(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);


    bool ValidateBinarySignal(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state);
    bool ValidateTimelineSignal(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);
    bool ValidateSignalSemaphore(const Location &semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);
};

struct WaitEventSubmitInfo {
    std::vector<VkEvent> wait_events;
    VkPipelineStageFlags wait_src_stage_mask = VK_PIPELINE_STAGE_NONE;

    // Subset of waited events with known signaling state
    EventSignalingStateMap signaling_states;

    bool Validate(const CoreChecks& core, const vvl::CommandBuffer& cb_state, EventMap& submit_signaling_states,
                  const Location& loc) const;
};
