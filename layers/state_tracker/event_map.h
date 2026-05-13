/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "vulkan/vulkan.h"
#include "containers/container_utils.h"
#include "containers/custom_containers.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <optional>
#include <vector>

// TODO: this is very similar to EventSignalingState (but signal field has different semantics).
// This will be reworked/removed soon.
struct EventInfo {
    VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE;
    bool signal = false;  // signal (SetEvent) or unsignal (ResetEvent)
    std::optional<vku::safe_VkDependencyInfo> dependency_info;
};
using EventMap = vvl::unordered_map<VkEvent, EventInfo>;

struct EventSignalingState {
    // Tracks how the event signaling state changes as command buffer recording progresses.
    // When recording is finished, this is the event state at the end of the command buffer
    bool signaled = false;

    // If signaled is true, this is the stage mask specified by the CmdSetEvent command.
    // If signaled is false, this is NONE
    VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // If signaled is true and the event was set by CmdSetEvent2, this is the dependency info from that command
    std::optional<vku::safe_VkDependencyInfo> dependency_info;

    EventSignalingState(bool signaled, VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE)
        : signaled(signaled), src_stage_mask(src_stage_mask) {}
};
using EventSignalingStateMap = vvl::unordered_map<VkEvent, EventSignalingState>;

// TODO: later this file will be renamed to event_state.h/cpp and this function will go into cpp
static inline void AddWaitEventSignalingStates(const std::vector<VkEvent>& wait_events,
                                               const EventSignalingStateMap& new_signaling_states,
                                               EventSignalingStateMap& current_signaling_states) {
    for (const auto& signaling_state : new_signaling_states) {
        const VkEvent event = signaling_state.first;
        const bool is_waited = vvl::Contains(wait_events, event);
        const bool is_signaled = vvl::Contains(current_signaling_states, event);
        if (is_waited && !is_signaled) {
            current_signaling_states.insert(signaling_state);
        }
    }
}
