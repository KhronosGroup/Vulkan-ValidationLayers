/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#include "queue_state.h"
#include "cmd_buffer_state.h"
#include "state_tracker.h"

uint64_t QUEUE_STATE::Submit(CB_SUBMISSION &&submission) {
    const uint64_t next_seq = seq + submissions.size() + 1;
    bool retire_early = false;

    for (auto &cb_node : submission.cbs) {
        for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
            secondary_cmd_buffer->IncrementResources();
        }
        cb_node->IncrementResources();
        // increment use count for all bound objects including secondary cbs
        cb_node->BeginUse();
        cb_node->Submit(submission.perf_submit_pass);
    }

    for (auto &wait : submission.wait_semaphores) {
        switch (wait.semaphore->scope) {
            case kSyncScopeInternal:
                if (wait.semaphore->type == VK_SEMAPHORE_TYPE_BINARY) {
                    wait.semaphore->signaled = false;
                }
                break;
            case kSyncScopeExternalTemporary:
                wait.semaphore->scope = kSyncScopeInternal;
                break;
            default:
                break;
        }
        wait.seq = next_seq;
        wait.semaphore->BeginUse();
    }

    for (auto &signal : submission.signal_semaphores) {
        if (signal.semaphore->scope == kSyncScopeInternal) {
            signal.semaphore->signaler.queue = this;
            signal.semaphore->signaler.seq = next_seq;
            if (signal.semaphore->type == VK_SEMAPHORE_TYPE_BINARY) {
                signal.semaphore->signaled = true;
            }
        } else {
            retire_early = true;
        }
        signal.seq = next_seq;
        signal.semaphore->BeginUse();
    }

    if (submission.fence) {
        if (submission.fence->scope == kSyncScopeInternal) {
            // Mark fence in use
            submission.fence->state = FENCE_INFLIGHT;
            submission.fence->signaler.queue = this;
            submission.fence->signaler.seq = next_seq;
        } else {
            retire_early = true;
        }
        submission.fence->BeginUse();
    }

    submissions.emplace_back(std::move(submission));
    return retire_early ? next_seq : 0;
}

void QUEUE_STATE::Retire(uint64_t next_seq) {
    layer_data::unordered_map<QUEUE_STATE *, uint64_t> other_queue_seqs;
    layer_data::unordered_map<std::shared_ptr<SEMAPHORE_STATE>, uint64_t> timeline_semaphore_counters;

    // Roll this queue forward, one submission at a time.
    while (seq < next_seq) {
        auto &submission = submissions.front();
        for (auto &wait : submission.wait_semaphores) {
            if (wait.semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE) {
                auto &last_counter = timeline_semaphore_counters[wait.semaphore];
                last_counter = std::max(last_counter, wait.payload);
                wait.semaphore->payload = std::max(wait.semaphore->payload, wait.payload);
            } else if (wait.semaphore->signaler.queue) {
                auto &last_seq = other_queue_seqs[wait.semaphore->signaler.queue];
                last_seq = std::max(last_seq, wait.semaphore->signaler.seq);
            }  // else this is an external semaphore
            wait.semaphore->EndUse();
        }

        for (auto &signal : submission.signal_semaphores) {
            if (signal.semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE && signal.semaphore->payload < signal.payload) {
                signal.semaphore->payload = signal.payload;
            }
            signal.semaphore->EndUse();
        }

        for (auto cb_node : submission.cbs) {
            for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
                secondary_cmd_buffer->Retire(submission.perf_submit_pass);
            }
            cb_node->Retire(submission.perf_submit_pass);
            cb_node->EndUse();
        }

        if (submission.fence) {
            if (submission.fence->scope == kSyncScopeInternal) {
                submission.fence->state = FENCE_RETIRED;
            }
            submission.fence->EndUse();
        }
        submissions.pop_front();
        seq++;
    }

    // Roll other queues forward to the highest seq we saw a wait for
    for (const auto &qs : other_queue_seqs) {
        qs.first->Retire(qs.second);
    }
    for (const auto &sc : timeline_semaphore_counters) {
        sc.first->Retire(sc.second);
    }
}

void FENCE_STATE::Retire() {
    if (scope == kSyncScopeInternal) {
        if (signaler.queue) {
            // Fence signaller is a queue -- use this as proof that prior operations on that queue have completed.
            signaler.queue->Retire(signaler.seq);
        } else {
            // Fence signaller is the WSI. We're not tracking what the WSI op actually /was/ in CV yet, but we need to mark
            // the fence as retired.
            state = FENCE_RETIRED;
        }
    } else {
        state = FENCE_RETIRED;
    }
}

void SEMAPHORE_STATE::Retire(uint64_t until_payload) {
    if (signaler.queue) {
        uint64_t max_seq = 0;
        for (const auto &submission : signaler.queue->submissions) {
            for (const auto &signal : submission.signal_semaphores) {
                if (signal.semaphore.get() == this) {
                    if (signal.payload <= until_payload && signal.seq > max_seq) {
                        max_seq = std::max(max_seq, signal.seq);
                    }
                }
            }
        }
        if (max_seq) {
            signaler.queue->Retire(max_seq);
        }
    }
}
