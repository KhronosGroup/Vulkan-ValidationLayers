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

#include "state_tracker/submit_time_tracker.h"
#include "state_tracker/state_tracker.h"

bool vvl::SubmitTimeTracker::ProcessSubmissionBatch(const VkSubmitInfo& submit_info) const {
    // TODO: initialize from submmit_info or update interface to accept SubmissionBatch directly
    SubmissionBatch batch;

    bool skip = false;
    std::lock_guard lock(submit_time_mutex_);
    {
        skip |= validator_.ProcessSubmissionBatch(batch);
    }
    return skip;
}
