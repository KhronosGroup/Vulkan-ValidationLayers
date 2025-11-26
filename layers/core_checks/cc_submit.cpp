/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "cc_submit.h"
#include "core_checks/cc_sync_vuid_maps.h"
#include "core_checks/core_validation.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/queue_state.h"

static Location GetSignaledSemaphoreLocation(const Location& submit_loc, uint32_t index) {
    vvl::Field field = vvl::Field::Empty;
    if (submit_loc.function == vvl::Func::vkQueueSubmit || submit_loc.function == vvl::Func::vkQueueBindSparse) {
        field = vvl::Field::pSignalSemaphores;
    } else if (submit_loc.function == vvl::Func::vkQueueSubmit2 || submit_loc.function == vvl::Func::vkQueueSubmit2KHR) {
        field = vvl::Field::pSignalSemaphoreInfos;
    } else {
        assert(false && "Unhandled signaling function");
    }
    return submit_loc.dot(field, index);
}

static bool FindLayouts(const vvl::Image& image_state, std::vector<VkImageLayout>& layouts) {
    if (!image_state.layout_map) {
        return false;
    }
    const auto& layout_map = *image_state.layout_map;
    auto guard = image_state.LayoutMapReadLock();

    // TODO: Make this robust for >1 aspect mask. Now it will just say ignore potential errors in this case.
    if (layout_map.size() > image_state.create_info.arrayLayers * image_state.create_info.mipLevels) {
        return false;
    }

    for (const auto& entry : layout_map) {
        layouts.emplace_back(entry.second);
    }
    return true;
}

void QueueSubmissionValidator::Validate(const vvl::QueueSubmission& submission) const {
    // Ensure that timeline signals are monotonically increasing values
    for (uint32_t i = 0; i < (uint32_t)submission.signal_semaphores.size(); ++i) {
        const auto& signal = submission.signal_semaphores[i];
        const uint64_t current_payload = signal.semaphore->CompletedPayload();

        // Check only the case where the signal value is less than the current payload.
        // Equality (also invalid) is handled during QueueSubmit. We can do such an early
        // equality check because execution reordering of submits cannot change the result
        // of comparison of equal values. On the other hand, for not equal values the
        // result of comparison depends on the ordering of submits, which is only known at
        // execution time (here).
        const bool invlid_signal_value = signal.payload < current_payload;

        if (invlid_signal_value) {
            const Location signal_semaphore_loc = GetSignaledSemaphoreLocation(submission.loc.Get(), i);
            const auto& vuid = GetQueueSubmitVUID(signal_semaphore_loc, vvl::SubmitError::kTimelineSemSmallValue);
            core_checks.LogError(vuid, signal.semaphore->Handle(), signal_semaphore_loc,
                                 "(%s) signaled with value %" PRIu64 " which is smaller than the current value %" PRIu64,
                                 core_checks.FormatHandle(signal.semaphore->VkHandle()).c_str(), signal.payload, current_payload);
        }
    }

    // Validate image layouts on the command buffer boundaries
    {
        vvl::unordered_map<const vvl::Image*, ImageLayoutMap> local_image_layout_map;
        for (const vvl::CommandBufferSubmission& cb_submission : submission.cb_submissions) {
            auto cb_guard = cb_submission.cb->ReadLock();
            core_checks.ValidateCmdBufImageLayouts(submission.loc.Get(), *cb_submission.cb, local_image_layout_map);
        }
    }

    // Check that image being presented has correct layout
    // NOTE: Do separate check that swapchain and its images are not destroyed at this point.
    //       For example, you can destroy swapchain after it was used as the old swapchain.
    if (submission.swapchain && !submission.swapchain_image->Destroyed()) {
        std::vector<VkImageLayout> layouts;
        if (submission.swapchain_image && FindLayouts(*submission.swapchain_image, layouts)) {
            for (auto layout : layouts) {
                if (layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR) {
                    core_checks.LogError(
                        "VUID-VkPresentInfoKHR-pImageIndices-01430", submission.swapchain_image->Handle(), submission.loc.Get(),
                        "images passed to present must be in layout VK_IMAGE_LAYOUT_PRESENT_SRC_KHR or "
                        "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR but %s is in %s.",
                        core_checks.FormatHandle(submission.swapchain_image->Handle()).c_str(), string_VkImageLayout(layout));
                }
            }
        }
    }
}

void QueueSubmissionValidator::Update(vvl::QueueSubmission& submission) {
    for (vvl::CommandBufferSubmission& cb_submission : submission.cb_submissions) {
        auto cb_guard = cb_submission.cb->WriteLock();
        for (const vvl::CommandBuffer* secondary : cb_submission.cb->linked_command_buffers) {
            core_checks.UpdateCmdBufImageLayouts(*secondary);
        }
        core_checks.UpdateCmdBufImageLayouts(*cb_submission.cb);
    }
}
