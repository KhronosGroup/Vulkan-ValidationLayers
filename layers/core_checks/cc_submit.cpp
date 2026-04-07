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

void QueueSubmissionValidator::Validate(const vvl::QueueSubmission& submission) const {
    // Ensure that timeline signals are monotonically increasing values
    for (uint32_t i = 0; i < (uint32_t)submission.signal_semaphores.size(); ++i) {
        const auto& signal = submission.signal_semaphores[i];
        const uint64_t current_payload = signal.semaphore->CurrentPayload();

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
}
