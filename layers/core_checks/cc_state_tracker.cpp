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
#include <vulkan/vulkan_core.h>
#include "core_validation.h"
#include "cc_sync_vuid_maps.h"
#include "error_message/error_strings.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/image_state.h"
#include "state_tracker/event_map.h"
#include "state_tracker/pipeline_state.h"

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

void CommandBufferSubState::Begin(const VkCommandBufferBeginInfo& begin_info) {
    if (begin_info.pInheritanceInfo && base.IsSecondary()) {
        // If we are a secondary command-buffer and inheriting.  Update the items we should inherit.
        if (begin_info.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
            // Check for VkCommandBufferInheritanceViewportScissorInfoNV (VK_NV_inherited_viewport_scissor)
            auto p_inherited_viewport_scissor_info =
                vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(begin_info.pInheritanceInfo->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && p_inherited_viewport_scissor_info->viewportScissor2D) {
                auto p_viewport_depths = p_inherited_viewport_scissor_info->pViewportDepths;
                viewport.inherited_depths.assign(p_viewport_depths,
                                                 p_viewport_depths + p_inherited_viewport_scissor_info->viewportDepthCount);
            }
        }
    }
}

void CommandBufferSubState::UpdateActionPipelineState(LastBound& last_bound, const vvl::Pipeline& pipeline_state) {
    // Update the consumed viewport/scissor count.
    {
        const auto* viewport_state = pipeline_state.ViewportState();
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        const auto has_dynamic_viewport_count = pipeline_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const auto has_dynamic_scissor_count = pipeline_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        const uint32_t pipeline_viewport_count =
            (has_dynamic_viewport_count || !viewport_state) ? 0 : viewport_state->viewportCount;
        const uint32_t pipeline_scissor_count = (has_dynamic_scissor_count || !viewport_state) ? 0 : viewport_state->scissorCount;

        // For each draw command D recorded to this command buffer, let
        //  * g_D be the graphics pipeline used
        //  * v_G be the viewportCount of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)
        //  * s_G be the scissorCount  of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT)
        // Then this value is max(0, max(v_G for all D in cb), max(s_G for all D in cb))
        used_viewport_scissor_count = std::max({used_viewport_scissor_count, pipeline_viewport_count, pipeline_scissor_count});
        viewport.used_dynamic_count |= pipeline_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        scissor.used_dynamic_count |= pipeline_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    }

    if (pipeline_state.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        base.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        base.SetActiveSubpassRasterizationSampleCount(base.dynamic_state_value.rasterization_samples);
    }

    if (last_bound.desc_set_pipeline_layout) {
        for (const auto& [set_index, binding_req_map] : pipeline_state.active_slots) {
            if (set_index >= last_bound.ds_slots.size()) {
                continue;
            }
            auto& ds_slot = last_bound.ds_slots[set_index];
            // Pull the set node
            auto& descriptor_set = ds_slot.ds_state;
            if (!descriptor_set) {
                continue;
            }

            // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor binding

            // We can skip updating the state if "nothing" has changed since the last validation.
            // See CoreChecks::ValidateActionState for more details.
            const bool need_update =  // Update if descriptor set (or contents) has changed
                ds_slot.validated_set != descriptor_set.get() ||
                ds_slot.validated_set_change_count != descriptor_set->GetChangeCount() ||
                (!base.dev_data.disabled[image_layout_validation] &&
                 ds_slot.validated_set_image_layout_change_count != base.image_layout_change_count);
            if (need_update) {
                if (!base.dev_data.disabled[command_buffer_state] && !descriptor_set->IsPushDescriptor()) {
                    base.AddChild(descriptor_set);
                }

                // Bind this set and its active descriptor resources to the command buffer
                descriptor_set->UpdateImageLayoutDrawStates(&base.dev_data, base, binding_req_map);

                ds_slot.validated_set = descriptor_set.get();
                ds_slot.validated_set_change_count = descriptor_set->GetChangeCount();
                ds_slot.validated_set_image_layout_change_count = base.image_layout_change_count;
            }
        }
    }
}

// Common logic after any draw/dispatch/traceRays
void CommandBufferSubState::RecordActionCommand(LastBound& last_bound, const Location&) {
    if (last_bound.pipeline_state) {
        UpdateActionPipelineState(last_bound, *last_bound.pipeline_state);
    }
}

void CommandBufferSubState::RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline& pipeline) {
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // Trash dynamic viewport/scissor state if pipeline defines static state and enabled rasterization.
        // akeley98 NOTE: There's a bit of an ambiguity in the spec, whether binding such a pipeline overwrites
        // the entire viewport (scissor) array, or only the subsection defined by the viewport (scissor) count.
        // I am taking the latter interpretation based on the implementation details of NVIDIA's Vulkan driver.
        const auto* viewport_state = pipeline.ViewportState();
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)) {
            viewport.trashed_count = true;
            if (viewport_state && (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT))) {
                viewport.trashed_mask |= (1u << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) {
            scissor.trashed_count = true;
            if (viewport_state && (!pipeline.IsDynamic(CB_DYNAMIC_STATE_SCISSOR))) {
                scissor.trashed_mask |= (1u << viewport_state->scissorCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
    }
}

void CommandBufferSubState::RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count) {
    uint32_t bits = ((1u << viewport_count) - 1u) << first_viewport;
    viewport.mask |= bits;
    viewport.trashed_mask &= ~bits;
}

void CommandBufferSubState::RecordSetViewportWithCount(uint32_t viewport_count) {
    uint32_t bits = (1u << viewport_count) - 1u;
    viewport.count_mask |= bits;
    viewport.trashed_mask &= ~bits;
    viewport.trashed_count = false;
}

void CommandBufferSubState::RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count) {
    uint32_t bits = ((1u << scissor_count) - 1u) << first_scissor;
    scissor.mask |= bits;
    scissor.trashed_mask &= ~bits;
}

void CommandBufferSubState::RecordSetScissorWithCount(uint32_t scissor_count) {
    uint32_t bits = (1u << scissor_count) - 1u;
    scissor.count_mask |= bits;
    scissor.trashed_mask &= ~bits;
    scissor.trashed_count = false;
}

void CommandBufferSubState::RecordSetEvent(vvl::Func, VkEvent event, VkPipelineStageFlags2 stage_mask,
                                           const VkDependencyInfo* dependency_info) {
    vku::safe_VkDependencyInfo safe_dependency_info = {};
    if (dependency_info) {
        safe_dependency_info.initialize(dependency_info);
    } else {
        // Set sType to invalid, so following code can check sType to see if the struct is valid
        safe_dependency_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }
    event_updates.emplace_back([event, stage_mask, safe_dependency_info](vvl::CommandBuffer&, bool do_validate,
                                                                         EventMap& local_event_signal_info, VkQueue,
                                                                         const Location& loc) {
        local_event_signal_info[event] = EventInfo{stage_mask, true, safe_dependency_info};
        return false;  // skip
    });
}

void CommandBufferSubState::RecordResetEvent(vvl::Func, VkEvent event, VkPipelineStageFlags2) {
    event_updates.emplace_back(
        [event](vvl::CommandBuffer&, bool do_validate, EventMap& local_event_signal_info, VkQueue, const Location& loc) {
            local_event_signal_info[event] = EventInfo{VK_PIPELINE_STAGE_2_NONE, false};
            return false;  // skip
        });
}

void CommandBufferSubState::RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                                             VkPipelineStageFlags2 src_stage_mask, const VkDependencyInfo* dependency_info) {
    // vvl::CommandBuffer will add to the events vector. TODO this is now incorrect
    auto first_event_index = base.events.size();
    auto event_added_count = eventCount;

    vku::safe_VkDependencyInfo safe_dependency_info = {};
    if (dependency_info) {
        safe_dependency_info.initialize(dependency_info);
    } else {
        // Set sType to invalid, so following code can check sType to see if the struct is valid
        safe_dependency_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    event_updates.emplace_back(
        [command, event_added_count, first_event_index, src_stage_mask, safe_dependency_info](
            vvl::CommandBuffer& cb_state, bool do_validate, EventMap& local_event_signal_info, VkQueue queue, const Location& loc) {
            if (!do_validate) return false;
            return CoreChecks::ValidateWaitEventsAtSubmit(command, cb_state, event_added_count, first_event_index, src_stage_mask,
                                                          safe_dependency_info, local_event_signal_info, queue, loc);
        });
}

static bool SetQueryState(const QueryObject& object, QueryState value, QueryMap* local_query_to_state_map) {
    (*local_query_to_state_map)[object] = value;
    return false;
}

static bool SetQueryStateMulti(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t perf_query_pass,
                               QueryState value, QueryMap* local_query_to_state_map) {
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryObject query_obj = {queryPool, firstQuery + i, perf_query_pass};
        (*local_query_to_state_map)[query_obj] = value;
    }
    return false;
}

void CommandBufferSubState::BeginQuery(const QueryObject& query_obj) {
    query_updates.emplace_back([query_obj](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                           uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        SetQueryState(QueryObject(query_obj, perf_query_pass), QUERYSTATE_RUNNING, local_query_to_state_map);
        return false;
    });
}

void CommandBufferSubState::EndQuery(const QueryObject& query_obj) {
    query_updates.emplace_back([query_obj](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                           uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        return SetQueryState(QueryObject(query_obj, perf_query_pass), QUERYSTATE_ENDED, local_query_to_state_map);
    });
}

void CommandBufferSubState::EndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    query_updates.emplace_back([queryPool, firstQuery, queryCount](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                                   uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        return SetQueryStateMulti(queryPool, firstQuery, queryCount, perf_query_pass, QUERYSTATE_ENDED, local_query_to_state_map);
    });
}

void CommandBufferSubState::ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    query_updates.emplace_back([queryPool, firstQuery, queryCount](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                                   uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        return SetQueryStateMulti(queryPool, firstQuery, queryCount, perf_query_pass, QUERYSTATE_RESET, local_query_to_state_map);
    });
}

void CommandBufferSubState::EnqueueUpdateVideoInlineQueries(const VkVideoInlineQueryInfoKHR& query_info) {
    query_updates.emplace_back([query_info](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                            uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            SetQueryState(QueryObject(query_info.queryPool, query_info.firstQuery + i), QUERYSTATE_ENDED, local_query_to_state_map);
        }
        return false;
    });
}

void CommandBufferSubState::Retire(uint32_t perf_submit_pass,
                                   const std::function<bool(const QueryObject&)>& is_query_updated_after) {
    QueryMap local_query_to_state_map;
    VkQueryPool first_pool = VK_NULL_HANDLE;
    for (auto& function : query_updates) {
        function(base, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
    }

    for (const auto& [query_object, query_state] : local_query_to_state_map) {
        if (query_state == QUERYSTATE_ENDED && !is_query_updated_after(query_object)) {
            auto query_pool_state = base.dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, QUERYSTATE_AVAILABLE);
        }
    }
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
    queue_submit_functions.clear();
    submit_validate_dynamic_rendering_barrier_subresources.clear();
    event_updates.clear();
    cmd_execute_commands_functions.clear();
    query_updates.clear();

    // Inherited Viewport/Scissor
    used_viewport_scissor_count = 0;
    viewport.mask = 0;
    viewport.count_mask = 0;
    viewport.trashed_mask = 0;
    viewport.trashed_count = false;
    viewport.used_dynamic_count = false;
    viewport.inherited_depths.clear();
    scissor.mask = 0;
    scissor.count_mask = 0;
    scissor.trashed_mask = 0;
    scissor.trashed_count = false;
    scissor.used_dynamic_count = false;
}

void CommandBufferSubState::ExecuteCommands(vvl::CommandBuffer& secondary_command_buffer) {
    auto& secondary_sub_state = SubState(secondary_command_buffer);
    if (secondary_command_buffer.IsSecondary()) {
        nesting_level = std::max(nesting_level, secondary_sub_state.nesting_level + 1);
    }

    for (auto& function : secondary_sub_state.event_updates) {
        event_updates.push_back(function);
    }

    for (auto& function : secondary_sub_state.queue_submit_functions) {
        queue_submit_functions.push_back(function);
    }

    // State is trashed after executing secondary command buffers.
    // Importantly, this function runs after CoreChecks::PreCallValidateCmdExecuteCommands.
    viewport.trashed_mask = vvl::MaxTypeValue(viewport.trashed_mask);
    viewport.trashed_count = true;
    scissor.trashed_mask = vvl::MaxTypeValue(scissor.trashed_mask);
    scissor.trashed_count = true;

    // Add a query update that runs all the query updates that happen in the sub command buffer.
    // This avoids locking ambiguity because primary command buffers are locked when these
    // callbacks run, but secondary command buffers are not.
    const VkCommandBuffer sub_command_buffer = secondary_command_buffer.VkHandle();
    query_updates.emplace_back([sub_command_buffer](vvl::CommandBuffer& cb_state_arg, bool do_validate,
                                                    VkQueryPool& first_perf_query_pool, uint32_t perf_query_pass,
                                                    QueryMap* local_query_to_state_map) {
        bool skip = false;
        auto secondary_cb_state_arg = cb_state_arg.dev_data.GetWrite<vvl::CommandBuffer>(sub_command_buffer);
        auto& secondary_sub_state_arg = SubState(*secondary_cb_state_arg);
        for (auto& function : secondary_sub_state_arg.query_updates) {
            skip |=
                function(*secondary_cb_state_arg, do_validate, first_perf_query_pool, perf_query_pass, local_query_to_state_map);
        }
        return skip;
    });
}

void CommandBufferSubState::Submit(vvl::Queue& queue_state, uint32_t perf_submit_pass, const Location& loc) {
    for (auto& func : queue_submit_functions) {
        func(queue_state, base);
    }

    // Update vvl::Event with src_stage from the last recorded SetEvent.
    // Ultimately, it tracks the last SetEvent for the entire submission.
    {
        EventMap local_event_signal_info;
        for (const auto& function : event_updates) {
            function(base, /*do_validate*/ false, local_event_signal_info,
                     VK_NULL_HANDLE /* when do_validate is false then wait handler is inactive */, loc);
        }
        for (const auto& [event, info] : local_event_signal_info) {
            auto event_state = base.dev_data.Get<vvl::Event>(event);
            event_state->signaled = info.signal;
            event_state->dependency_info = info.dependency_info;
            event_state->signal_src_stage_mask = info.src_stage_mask;
            event_state->signaling_queue = queue_state.VkHandle();
        }
    }

    // Update vvl::QueryPool with a query state at the end of the command buffer.
    // Ultimately, it tracks the final query state for the entire submission.
    {
        VkQueryPool first_pool = VK_NULL_HANDLE;
        QueryMap local_query_to_state_map;
        for (auto& function : query_updates) {
            function(base, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
        }
        for (const auto& [query_object, query_state] : local_query_to_state_map) {
            auto query_pool_state = base.dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, query_state);
        }
    }
}

void CommandBufferSubState::SubmitTimeValidate() {
    for (const auto& [image, subresources] : submit_validate_dynamic_rendering_barrier_subresources) {
        const auto image_state = validator.Get<vvl::Image>(image);
        if (!image_state) {
            continue;
        }
        const auto global_layout_map = image_state->layout_map.get();
        ASSERT_AND_CONTINUE(global_layout_map);
        auto global_layout_map_guard = image_state->LayoutMapReadLock();

        for (const std::pair<VkImageSubresourceRange, vvl::LocationCapture>& entry : subresources) {
            const VkImageSubresourceRange& subresource = entry.first;
            const Location& barrier_loc = entry.second.Get();
            subresource_adapter::RangeGenerator range_gen(image_state->subresource_encoder, subresource);
            ForEachMatchingLayoutMapRange(
                *global_layout_map, std::move(range_gen),
                [this, &barrier_loc, &image_state](const ImageLayoutMap::key_type& range, const VkImageLayout& layout) {
                    if (layout != VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ && layout != VK_IMAGE_LAYOUT_GENERAL) {
                        const auto& vuid =
                            GetDynamicRenderingBarrierVUID(barrier_loc, vvl::DynamicRenderingBarrierError::kImageLayout);
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

void QueueSubState::Retire(vvl::QueueSubmission& submission) {
    queue_submission_validator_.Validate(submission);

    auto is_query_updated_after = [this](const QueryObject& query_object) {
        auto guard = base.Lock();
        bool first_queue_submission = true;
        for (const vvl::QueueSubmission& queue_submission : base.Submissions()) {
            // The current submission is still on the deque, so skip it
            if (first_queue_submission) {
                first_queue_submission = false;
                continue;
            }
            for (const vvl::CommandBufferSubmission& cb_submission : queue_submission.cb_submissions) {
                if (query_object.perf_pass != queue_submission.perf_submit_pass) {
                    continue;
                }
                if (cb_submission.cb->UpdatesQuery(query_object)) {
                    return true;
                }
            }
        }
        return false;
    };

    for (vvl::CommandBufferSubmission& cb_submission : submission.cb_submissions) {
        CommandBufferSubState& cb_sub_state = SubState(*cb_submission.cb);
        auto cb_guard = cb_sub_state.base.WriteLock();
        for (vvl::CommandBuffer* secondary_cmd_buffer : cb_submission.cb->linked_command_buffers) {
            CommandBufferSubState& secondary_sub_state = SubState(*secondary_cmd_buffer);
            auto secondary_guard = secondary_sub_state.base.WriteLock();
            secondary_sub_state.Retire(submission.perf_submit_pass, is_query_updated_after);
        }
        cb_sub_state.Retire(submission.perf_submit_pass, is_query_updated_after);
    }
}

}  // namespace core