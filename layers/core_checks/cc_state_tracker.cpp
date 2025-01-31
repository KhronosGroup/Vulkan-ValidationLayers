/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "cc_state_tracker.h"
#include "core_validation.h"

void CoreChecks::Created(vvl::CommandBuffer& cb) {
    cb.SetSubState(container_type, std::make_unique<core::CommandBufferSubState>(cb));
}

void core::CommandBufferSubState::RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                                                   VkPipelineStageFlags2KHR srcStageMask) {
    // vvl::CommandBuffer will add to the events vector. TODO this is now incorrect
    auto first_event_index = base.events.size();
    auto event_added_count = eventCount;
    base.event_updates.emplace_back(
        [command, event_added_count, first_event_index, srcStageMask](
            vvl::CommandBuffer& cb_state, bool do_validate, EventMap& local_event_signal_info, VkQueue queue, const Location& loc) {
            if (!do_validate) return false;
            return CoreChecks::ValidateWaitEventsAtSubmit(command, cb_state, event_added_count, first_event_index, srcStageMask,
                                                          local_event_signal_info, queue, loc);
        });
}

void CoreChecks::Created(vvl::Queue& queue) {
    queue.SetSubState(container_type, std::make_unique<core::QueueSubState>(*this, queue));
}

core::QueueSubState::QueueSubState(Logger& logger, vvl::Queue& q) : vvl::QueueSubState(q), queue_submission_validator_(logger) {}

void core::QueueSubState::Retire(vvl::QueueSubmission& submission) { queue_submission_validator_.Validate(submission); }
