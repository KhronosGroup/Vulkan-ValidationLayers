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

#include "state_tracker/event_state.h"

bool EventSignalingState::HasKnownEffect(const EventSignalingState* prior_state) const {
    // After a reset, later signal/reset commands define the event state
    if (was_reset) {
        return true;
    }
    // If the prior state ended unsignaled, this state cannot be a repeated
    // signal and defines the event state
    const bool is_prior_unsignaled = prior_state && !prior_state->signaled;
    return is_prior_unsignaled;
}

void UpdateEventSignalingStates(EventSignalingStateMap& accumulated_states, const EventSignalingStateMap& recorded_states) {
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

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkEventCreateInfo* info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

namespace vvl {
Event::Event(VkEvent handle, const VkEventCreateInfo* create_info)
    : StateObject(handle, kVulkanObjectTypeEvent),
      flags(create_info->flags)
#ifdef VK_USE_PLATFORM_METAL_EXT
      ,
      metal_event_export(GetMetalExport(create_info))
#endif  // VK_USE_PLATFORM_METAL_EXT
{
}
}  // namespace vvl
