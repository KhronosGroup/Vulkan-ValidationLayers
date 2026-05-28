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

struct EventSignalingState {
    // Tracks how the event signaling state changes as command buffer recording progresses.
    // When recording is finished, this is the event state at the end of the command buffer
    bool signaled = false;

    // If this event was unsignaled at least once during command buffer recording
    bool was_reset = false;

    // Stage mask from CmdSetEvent.
    // It is NONE for CmdSetEvent2, or if the event is unsignaled
    VkPipelineStageFlags2 signal_src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // The dependency info from CmdSetEvent2.
    // It is no value for CmdSetEvent, or if the event is unsignaled
    std::optional<vku::safe_VkDependencyInfo> signal_dependency_info;

    // Return true if this state's effect on the event is known.
    // Without a reset or known prior state, we cannot tell whether a signal
    // in this state is a repeated signal, which the spec says is ignored
    //
    // NOTE: this implementation does not try to handle repeated unsignals, which
    // are also ignored. We do not currently keep data associated with unsignals,
    // (and for signals we store stage mask/dependency info). Update this if we
    // need to track associated unsignal data.
    bool HasKnownEffect(const EventSignalingState* prior_state = nullptr) const {
        // After a reset, later signal/reset commands define the event state
        if (was_reset) {
            return true;
        }
        // If the prior state ended unsignaled, this state cannot be a repeated
        // signal and defines the event state
        const bool is_prior_unsignaled = prior_state && !prior_state->signaled;
        return is_prior_unsignaled;
    }
};
using EventSignalingStateMap = vvl::unordered_map<VkEvent, EventSignalingState>;

inline void UpdateEventSignalingStates(EventSignalingStateMap& accumulated_states, const EventSignalingStateMap& recorded_states) {
    for (const auto& [event, recorded_state] : recorded_states) {
        EventSignalingState& accumulated_state = accumulated_states[event];

        const bool keep_accumulated_state = accumulated_state.signaled && !recorded_state.was_reset;
        if (keep_accumulated_state) {
            continue;
        }

        const bool was_reset = accumulated_state.was_reset;
        accumulated_state = recorded_state;
        accumulated_state.was_reset |= was_reset;
    }
}
