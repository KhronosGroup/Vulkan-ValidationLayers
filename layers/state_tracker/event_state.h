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

#include "state_tracker/state_object.h"
#include "containers/custom_containers.h"
#include "generated/error_location_helper.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <optional>

struct EventSignalState {
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

    // Last command that changed the event signal state: Set/Set2/Reset/Reset2
    vvl::Func last_signaling_command = vvl::Func::Empty;

    // Return true if this state's effect on the event is known.
    // Without a reset or known prior state, we cannot tell whether a signal
    // in this state is a repeated signal, which the spec says is ignored
    //
    // NOTE: this implementation does not try to handle repeated unsignals, which
    // are also ignored. We do not currently keep data associated with unsignals,
    // (and for signals we store stage mask/dependency info). Update this if we
    // need to track associated unsignal data.
    bool HasKnownEffect(const EventSignalState* prior_state = nullptr) const;
};

struct EventWaitState {
    VkPipelineStageFlags2 barriers = VK_PIPELINE_STAGE_2_NONE;
    vvl::Func last_wait_command = vvl::Func::Empty;
};

using EventSignalStateMap = vvl::unordered_map<VkEvent, EventSignalState>;
using EventWaitStateMap = vvl::unordered_map<VkEvent, EventWaitState>;
using EventWaitCommandMap = vvl::unordered_map<VkEvent, vvl::Func>;

void UpdateEventSignalStates(EventSignalStateMap& accumulated_states, const EventSignalStateMap& recorded_states);

namespace vvl {

class Event : public StateObject {
  public:
    Event(VkEvent handle, const VkEventCreateInfo* create_info);
    VkEvent VkHandle() const { return handle_.Cast<VkEvent>(); }

    const VkEventCreateFlags flags;

#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_event_export;
#endif  // VK_USE_PLATFORM_METAL_EXT

    // Signaling state.
    // Gets updated at queue submission granularity or when signaled from the host.
    bool signaled = false;

    // Source stage specified by the "set event" command.
    // Gets updated at queue submission granularity.
    VkPipelineStageFlags2 signal_src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    std::optional<vku::safe_VkDependencyInfo> signal_dependency_info;

    // Queue that signaled this event. It's null if event was signaled from the host.
    VkQueue signaling_queue = VK_NULL_HANDLE;

    // Last command that changed the event signal state: Set/Set2/Reset/Reset2
    vvl::Func last_signaling_command = vvl::Func::Empty;
};

}  // namespace vvl
