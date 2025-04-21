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
#include "error_message/error_strings.h"
#include "state_tracker/image_state.h"
#include "state_tracker/event_map.h"
#include "sync/sync_vuid_maps.h"

// Location to add per-queue submit debug info if built with -D DEBUG_CAPTURE_KEYBOARD=ON
void CoreChecks::DebugCapture() {}

void CoreChecks::Created(vvl::CommandBuffer& cb) {
    cb.SetSubState(container_type, std::make_unique<core::CommandBufferSubState>(cb, *this));
}

void CoreChecks::Created(vvl::Queue& queue) {
    queue.SetSubState(container_type, std::make_unique<core::QueueSubState>(*this, queue));
}

namespace core {

CommandBufferSubState::CommandBufferSubState(vvl::CommandBuffer& cb, CoreChecks& validator)
    : vvl::CommandBufferSubState(cb), validator(validator) {
    ResetCBState();
}

void CommandBufferSubState::RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
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

void CommandBufferSubState::Reset(const Location& loc) { ResetCBState(); }

void CommandBufferSubState::Destroy() { ResetCBState(); }

void CommandBufferSubState::ResetCBState() {
    // QFO Tranfser
    qfo_transfer_image_barriers.Reset();
    qfo_transfer_buffer_barriers.Reset();

    // VK_EXT_nested_command_buffer
    nesting_level = 0;

    // Submit time validation
    submit_validate_dynamic_rendering_barrier_subresources.clear();
}

void CommandBufferSubState::ExecuteCommands(vvl::CommandBuffer& secondary_command_buffer) {
    if (secondary_command_buffer.IsSecondary()) {
        auto& secondary_sub_state = SubState(secondary_command_buffer);
        nesting_level = std::max(nesting_level, secondary_sub_state.nesting_level + 1);
    }
}

void CommandBufferSubState::SubmitTimeValidate() {
    for (const auto& [image, subresources] : submit_validate_dynamic_rendering_barrier_subresources) {
        const auto image_state = validator.Get<vvl::Image>(image);
        if (!image_state) {
            continue;
        }
        const auto global_layout_map = image_state->layout_range_map.get();
        ASSERT_AND_CONTINUE(global_layout_map);
        auto global_layout_map_guard = global_layout_map->ReadLock();

        for (const std::pair<VkImageSubresourceRange, vvl::LocationCapture>& entry : subresources) {
            const VkImageSubresourceRange& subresource = entry.first;
            const Location& barrier_loc = entry.second.Get();
            ImageLayoutRangeMap::RangeGenerator range_gen(image_state->subresource_encoder, subresource);
            global_layout_map->AnyInRange(range_gen, [this, &barrier_loc, &image_state](const ImageLayoutRangeMap::key_type& range,
                                                                                        const VkImageLayout& layout) {
                if (layout != VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ && layout != VK_IMAGE_LAYOUT_GENERAL) {
                    const auto& vuid = sync_vuid_maps::GetDynamicRenderingBarrierVUID(
                        barrier_loc, sync_vuid_maps::DynamicRenderingBarrierError::kImageLayout);
                    const LogObjectList objlist(base.Handle(), image_state->Handle());
                    const Location& image_loc = barrier_loc.dot(vvl::Field::image);
                    const VkImageSubresource subresource =
                        static_cast<VkImageSubresource>(image_state->subresource_encoder.Decode(range.begin));
                    return validator.LogError(vuid, objlist, image_loc, "(%s, %s) has layout %s.",
                                              validator.FormatHandle(image_state->Handle()).c_str(),
                                              string_VkImageSubresource(subresource).c_str(), string_VkImageLayout(layout));
                }
                return false;
            });
        }
    }
}

QueueSubState::QueueSubState(Logger& logger, vvl::Queue& q) : vvl::QueueSubState(q), queue_submission_validator_(logger) {}

void QueueSubState::PreSubmit(std::vector<vvl::QueueSubmission>& submissions) {
    for (const auto& submission : submissions) {
        for (auto& cb : submission.cb_submissions) {
            auto guard = cb.cb->ReadLock();
            CommandBufferSubState& cb_substate = SubState(*cb.cb);
            cb_substate.SubmitTimeValidate();
        }
    }
}

void QueueSubState::Retire(vvl::QueueSubmission& submission) { queue_submission_validator_.Validate(submission); }

}  // namespace core