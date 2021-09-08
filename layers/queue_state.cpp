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
#include "state_tracker.h"
#include "cmd_buffer_state.h"

void ValidationStateTracker::RecordSubmitWaitSemaphore(CB_SUBMISSION &submission, VkQueue queue, VkSemaphore semaphore,
                                                       uint64_t value, uint64_t next_seq) {
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        if (semaphore_state->scope == kSyncScopeInternal) {
            SEMAPHORE_WAIT wait;
            wait.semaphore = semaphore;
            wait.type = semaphore_state->type;
            if (semaphore_state->type == VK_SEMAPHORE_TYPE_BINARY) {
                if (semaphore_state->signaler.first != VK_NULL_HANDLE) {
                    wait.queue = semaphore_state->signaler.first;
                    wait.seq = semaphore_state->signaler.second;
                    submission.waitSemaphores.emplace_back(std::move(wait));
                    semaphore_state->BeginUse();
                }
                semaphore_state->signaler.first = VK_NULL_HANDLE;
                semaphore_state->signaled = false;
            } else if (semaphore_state->payload < value) {
                wait.queue = queue;
                wait.seq = next_seq;
                wait.payload = value;
                submission.waitSemaphores.emplace_back(std::move(wait));
                semaphore_state->BeginUse();
            }
        } else {
            submission.externalSemaphores.push_back(semaphore);
            semaphore_state->BeginUse();
            if (semaphore_state->scope == kSyncScopeExternalTemporary) {
                semaphore_state->scope = kSyncScopeInternal;
            }
        }
    }
}

bool ValidationStateTracker::RecordSubmitSignalSemaphore(CB_SUBMISSION &submission, VkQueue queue, VkSemaphore semaphore,
                                                         uint64_t value, uint64_t next_seq) {
    bool retire_early = false;
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        if (semaphore_state->scope == kSyncScopeInternal) {
            SEMAPHORE_SIGNAL signal;
            signal.semaphore = semaphore;
            signal.seq = next_seq;
            if (semaphore_state->type == VK_SEMAPHORE_TYPE_BINARY) {
                semaphore_state->signaler.first = queue;
                semaphore_state->signaler.second = next_seq;
                semaphore_state->signaled = true;
            } else {
                signal.payload = value;
            }
            semaphore_state->BeginUse();
            submission.signalSemaphores.emplace_back(std::move(signal));
        } else {
            // Retire work up until this submit early, we will not see the wait that corresponds to this signal
            retire_early = true;
        }
    }
    return retire_early;
}

// Submit a fence to a queue, delimiting previous fences and previous untracked
// work by it.
static void SubmitFence(QUEUE_STATE *pQueue, FENCE_STATE *pFence, uint64_t submitCount) {
    pFence->state = FENCE_INFLIGHT;
    pFence->signaler.first = pQueue->Queue();
    pFence->signaler.second = pQueue->seq + pQueue->submissions.size() + submitCount;
}

uint64_t ValidationStateTracker::RecordSubmitFence(QUEUE_STATE *queue_state, VkFence fence, uint32_t submit_count) {
    auto fence_state = GetFenceState(fence);
    uint64_t early_retire_seq = 0;
    if (fence_state) {
        if (fence_state->scope == kSyncScopeInternal) {
            // Mark fence in use
            SubmitFence(queue_state, fence_state, std::max(1u, submit_count));
            if (!submit_count) {
                // If no submissions, but just dropping a fence on the end of the queue,
                // record an empty submission with just the fence, so we can determine
                // its completion.
                CB_SUBMISSION submission;
                submission.fence = fence;
                queue_state->submissions.emplace_back(std::move(submission));
            }
        } else {
            // Retire work up until this fence early, we will not see the wait that corresponds to this signal
            early_retire_seq = queue_state->seq + queue_state->submissions.size();
        }
    }
    return early_retire_seq;
}

void ValidationStateTracker::RetireWorkOnQueue(QUEUE_STATE *pQueue, uint64_t seq) {
    layer_data::unordered_map<VkQueue, uint64_t> other_queue_seqs;
    layer_data::unordered_map<VkSemaphore, uint64_t> timeline_semaphore_counters;

    // Roll this queue forward, one submission at a time.
    while (pQueue->seq < seq) {
        auto &submission = pQueue->submissions.front();

        for (auto &wait : submission.waitSemaphores) {
            auto semaphore_state = GetSemaphoreState(wait.semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
            }
            if (wait.type == VK_SEMAPHORE_TYPE_TIMELINE) {
                auto &last_counter = timeline_semaphore_counters[wait.semaphore];
                last_counter = std::max(last_counter, wait.payload);
            } else {
                auto &last_seq = other_queue_seqs[wait.queue];
                last_seq = std::max(last_seq, wait.seq);
            }
        }

        for (auto &signal : submission.signalSemaphores) {
            auto semaphore_state = GetSemaphoreState(signal.semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
                if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE && semaphore_state->payload < signal.payload) {
                    semaphore_state->payload = signal.payload;
                }
            }
        }

        for (auto &semaphore : submission.externalSemaphores) {
            auto semaphore_state = GetSemaphoreState(semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
            }
        }

        for (auto cb : submission.cbs) {
            auto cb_node = Get<CMD_BUFFER_STATE>(cb);
            if (!cb_node) {
                continue;
            }
            // First perform decrement on general case bound objects
            for (auto event : cb_node->writeEventsBeforeWait) {
                auto event_node = eventMap.find(event);
                if (event_node != eventMap.end()) {
                    event_node->second->write_in_use--;
                }
            }
            QueryMap local_query_to_state_map;
            VkQueryPool first_pool = VK_NULL_HANDLE;
            for (auto &function : cb_node->queryUpdates) {
                function(nullptr, /*do_validate*/ false, first_pool, submission.perf_submit_pass, &local_query_to_state_map);
            }

            for (const auto &query_state_pair : local_query_to_state_map) {
                if (query_state_pair.second == QUERYSTATE_ENDED) {
                    queryToStateMap[query_state_pair.first] = QUERYSTATE_AVAILABLE;
                }
            }
            if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                cb_node->EndUse();
            }
        }

        auto fence_state = GetFenceState(submission.fence);
        if (fence_state && fence_state->scope == kSyncScopeInternal) {
            fence_state->state = FENCE_RETIRED;
        }

        pQueue->submissions.pop_front();
        pQueue->seq++;
    }

    // Roll other queues forward to the highest seq we saw a wait for
    for (const auto &qs : other_queue_seqs) {
        RetireWorkOnQueue(GetQueueState(qs.first), qs.second);
    }
    for (const auto &sc : timeline_semaphore_counters) {
        RetireTimelineSemaphore(sc.first, sc.second);
    }
}

void ValidationStateTracker::RetireFence(VkFence fence) {
    auto fence_state = GetFenceState(fence);
    if (fence_state && fence_state->scope == kSyncScopeInternal) {
        if (fence_state->signaler.first != VK_NULL_HANDLE) {
            // Fence signaller is a queue -- use this as proof that prior operations on that queue have completed.
            RetireWorkOnQueue(GetQueueState(fence_state->signaler.first), fence_state->signaler.second);
        } else {
            // Fence signaller is the WSI. We're not tracking what the WSI op actually /was/ in CV yet, but we need to mark
            // the fence as retired.
            fence_state->state = FENCE_RETIRED;
        }
    }
}

void ValidationStateTracker::RetireTimelineSemaphore(VkSemaphore semaphore, uint64_t until_payload) {
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        for (const auto &pair : queueMap) {
            const auto &queue_state = pair.second;
            uint64_t max_seq = 0;
            for (const auto &submission : queue_state->submissions) {
                for (const auto &signal_semaphore : submission.signalSemaphores) {
                    if (signal_semaphore.semaphore == semaphore && signal_semaphore.payload <= until_payload) {
                        if (signal_semaphore.seq > max_seq) {
                            max_seq = signal_semaphore.seq;
                        }
                    }
                }
            }
            if (max_seq) {
                RetireWorkOnQueue(queue_state.get(), max_seq);
            }
        }
    }
}
