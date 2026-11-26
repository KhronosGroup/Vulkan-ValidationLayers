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

#include "gpuav/syncval/gpuav_syncval.h"

#include <memory>
#include <vector>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "generated/dispatch_functions.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/submission_reference.h"

namespace gpuav {
namespace syncval {

// Draw count readback state for a single command buffer recording
struct DrawCountsCbState {
    struct DrawCount {
        uint64_t command_number;
        VkBuffer buffer;
        VkDeviceSize offset;
        vko::BufferRange readback;
    };
    std::vector<DrawCount> draw_counts;
    size_t next_copy = 0;
    vvl::SubmissionReference latest_submission;
};

bool IsEnabled(const Validator& gpuav) {
    return gpuav.enabled[gpu_validation] && gpuav.enabled[sync_validation] && gpuav.syncval_settings.full_validation &&
           gpuav.syncval_settings.defer_full_validation;
}

void DrawCountBuffer(Validator& gpuav, CommandBufferSubState& cb_state, VkBuffer api_count_buffer,
                     VkDeviceSize api_count_buffer_offset) {
    if (!IsEnabled(gpuav)) {
        return;
    }
    // TODO: Support secondary command buffers
    if (!cb_state.base.IsPrimary()) {
        return;
    }
    // TODO: Support simultaneous use. Overlapping executions would write to the same readback slot
    if (cb_state.base.begin_info_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        return;
    }
    // TODO: A copy is not allowed between a suspended rendering instance and its resume.
    // Remove this check once CopyDrawCounts delays such copies until the resumed instance ends
    if (cb_state.base.active_render_pass && (cb_state.base.active_render_pass->GetRenderingFlags() & VK_RENDERING_SUSPENDING_BIT)) {
        return;
    }

    const vko::BufferRange count_range = cb_state.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(uint32_t));
    if (!count_range.Valid()) {
        return;
    }

    // We need the command number for this draw, but we're in pre-record and command_count
    // is updated in post-record. Add one to get this draw's command number
    const uint64_t command_number = cb_state.base.command_count + 1;

    DrawCountsCbState& state = cb_state.shared_resources_cache.GetOrCreate<DrawCountsCbState>();
    state.draw_counts.push_back({command_number, api_count_buffer, api_count_buffer_offset, count_range});
}

// Returns false if the command buffer has no draw counts to collect
static bool PrepareDrawCounts(CommandBufferSubState& cb_state, const vvl::SubmissionReference& submission) {
    if (DrawCountsCbState* state = cb_state.shared_resources_cache.TryGet<DrawCountsCbState>()) {
        state->latest_submission = submission;
        return true;
    }
    return false;
}

static void ReadDrawCounts(const CommandBufferSubState& cb_state, const vvl::SubmissionReference& submission,
                           vvl::unordered_map<uint64_t, uint32_t>& draw_counts) {
    const auto* state = cb_state.shared_resources_cache.TryGet<DrawCountsCbState>();
    if (!state) {
        return;
    }

    // GPU-AV reads results only when the last batch of a submit retires. The app can wait for
    // an earlier batch (e.g. host timeline wait) and reuse its command buffer before that.
    // This state then describes the newer submission, so skip reading the earlier counts
    if (state->latest_submission.queue != submission.queue || state->latest_submission.seq != submission.seq) {
        return;
    }

    for (const auto& draw : state->draw_counts) {
        draw_counts.emplace(draw.command_number, *static_cast<const uint32_t*>(draw.readback.offset_mapped_ptr));
    }
}

void CopyDrawCounts(CommandBufferSubState& cb_state) {
    auto* state = cb_state.shared_resources_cache.TryGet<DrawCountsCbState>();
    if (!state || state->next_copy == state->draw_counts.size()) {
        return;
    }

    VkMemoryBarrier count_barrier = vku::InitStructHelper();
    count_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    count_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                               &count_barrier, 0, nullptr, 0, nullptr);

    for (; state->next_copy < state->draw_counts.size(); state->next_copy++) {
        const auto& draw = state->draw_counts[state->next_copy];
        const VkBufferCopy copy_region = {draw.offset, draw.readback.offset, sizeof(uint32_t)};
        DispatchCmdCopyBuffer(cb_state.VkHandle(), draw.buffer, draw.readback.buffer, 1, &copy_region);
    }

    // Finish reading the counts before subsequent commands can overwrite them
    DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                               nullptr, 0, nullptr, 0, nullptr);
}

void PrepareData(CommandBufferSubState& cb_state, vvl::Queue& queue, vvl::QueueSubmission& submission, uint32_t cb_index) {
    if (!PrepareDrawCounts(cb_state, {&queue, submission.seq})) {
        return;
    }

    // Allocate syncval slots when the first command buffer needs collection
    auto& syncval_data = submission.gpuav_to_syncval_data;
    if (syncval_data.empty()) {
        syncval_data.resize(submission.cb_infos.size());
    }

    syncval_data[cb_index] = std::make_shared<vvl::SyncValInputData>();
}

void CollectData(const CommandBufferSubState& cb_state, const vvl::SubmissionReference& submission,
                 vvl::SyncValInputData& syncval_data) {
    const auto& gpuav = cb_state.gpuav_;
    if (gpuav.aborted_ || gpuav.device_state->dispatch_device_->is_device_lost) {
        return;
    }
    ReadDrawCounts(cb_state, submission, syncval_data.draw_counts);
}

}  // namespace syncval
}  // namespace gpuav
