/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */

#include <algorithm>
#include <assert.h>
#include <string>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "sync/sync_utils.h"

bool CoreChecks::ValidateStageMaskHost(const Location &loc, VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;
    if ((stageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kHostStageMask);
        skip |= LogError(
            device, vuid,
            "%s stage mask must not include VK_PIPELINE_STAGE_HOST_BIT as the stage can't be invoked inside a command buffer.",
            loc.Message().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateFenceForSubmit(const FENCE_STATE *fence_state, const char *inflight_vuid, const char *retired_vuid,
                                        const char *func_name) const {
    bool skip = false;

    if (fence_state && fence_state->Scope() == kSyncScopeInternal) {
        switch (fence_state->State()) {
            case FENCE_INFLIGHT:
                skip |= LogError(fence_state->fence(), inflight_vuid, "%s: %s is already in use by another submission.", func_name,
                                 report_data->FormatHandle(fence_state->fence()).c_str());
                break;
            case FENCE_RETIRED:
                skip |= LogError(fence_state->fence(), retired_vuid,
                                 "%s: %s submitted in SIGNALED state.  Fences must be reset before being submitted", func_name,
                                 report_data->FormatHandle(fence_state->fence()).c_str());
                break;
            default:
                break;
        }
    }

    return skip;
}

bool SemaphoreSubmitState::ValidateBinaryWait(const core_error::Location &loc, VkQueue queue,
                                              const SEMAPHORE_STATE &semaphore_state) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    auto semaphore = semaphore_state.semaphore();
    if ((semaphore_state.Scope() == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
        VkQueue other_queue = AnotherQueueWaits(semaphore_state);
        if (other_queue) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kOtherQueueWaiting);
            const LogObjectList objlist(semaphore, queue, other_queue);
            skip |= core->LogError(objlist, vuid, "%s Queue %s is already waiting on semaphore (%s).", loc.Message().c_str(),
                                   core->report_data->FormatHandle(other_queue).c_str(),
                                   core->report_data->FormatHandle(semaphore).c_str());
        } else if (CannotWait(semaphore_state)) {
            auto error = IsExtEnabled(core->device_extensions.vk_khr_timeline_semaphore) ? SubmitError::kBinaryCannotBeSignalled
                                                                                         : SubmitError::kOldBinaryCannotBeSignalled;
            const auto &vuid = GetQueueSubmitVUID(loc, error);
            const LogObjectList objlist(semaphore, queue);
            skip |= core->LogError(
                objlist, semaphore_state.Scope() == kSyncScopeInternal ? vuid : kVUID_Core_DrawState_QueueForwardProgress,
                "%s Queue %s is waiting on semaphore (%s) that has no way to be signaled.", loc.Message().c_str(),
                core->report_data->FormatHandle(queue).c_str(), core->report_data->FormatHandle(semaphore).c_str());
        } else {
            signaled_semaphores.erase(semaphore);
            unsignaled_semaphores.insert(semaphore);
        }
    } else if (semaphore_state.Scope() == kSyncScopeExternalTemporary) {
        internal_semaphores.insert(semaphore);
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateWaitSemaphore(const core_error::Location &loc, VkSemaphore semaphore, uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;

    auto semaphore_state = core->Get<SEMAPHORE_STATE>(semaphore);
    if (!semaphore_state) {
        return skip;
    }
    switch (semaphore_state->type) {
        case VK_SEMAPHORE_TYPE_BINARY:
            skip = ValidateBinaryWait(loc, queue, *semaphore_state);
            break;
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(
                    semaphore, vuid, "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").",
                    loc.Message().c_str(), value, where.c_str(), core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            timeline_waits[semaphore] = value;
        } break;
        default:
            break;
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateSignalSemaphore(const core_error::Location &loc, VkSemaphore semaphore, uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;
    LogObjectList objlist(semaphore, queue);

    auto semaphore_state = core->Get<SEMAPHORE_STATE>(semaphore);
    if (!semaphore_state) {
        return skip;
    }
    switch (semaphore_state->type) {
        case VK_SEMAPHORE_TYPE_BINARY: {
            if ((semaphore_state->Scope() == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
                VkQueue other_queue = VK_NULL_HANDLE;
                if (CannotSignal(*semaphore_state, other_queue)) {
                    objlist.add(other_queue);
                    skip |= core->LogError(objlist, kVUID_Core_DrawState_QueueForwardProgress,
                                           "%s is signaling %s (%s) that was previously "
                                           "signaled by %s but has not since been waited on by any queue.",
                                           loc.Message().c_str(), core->report_data->FormatHandle(queue).c_str(),
                                           core->report_data->FormatHandle(semaphore).c_str(),
                                           core->report_data->FormatHandle(other_queue).c_str());
                } else {
                    unsignaled_semaphores.erase(semaphore);
                    signaled_semaphores.insert(semaphore);
                }
            }
            break;
        }
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            auto must_be_greater = [value](const SEMAPHORE_STATE::SemOp &op, bool is_pending) {
                if (!op.IsSignal()) {
                    return false;
                }
                // duplicate signal values are never allowed.
                if (value == op.payload) {
                    return true;
                }
                // exact value ordering cannot be determined until execution time
                return !is_pending && value < op.payload;
            };
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, must_be_greater)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemSmallValue);
                skip |= core->LogError(objlist, vuid,
                                       "At submit time, %s signal value (0x%" PRIx64
                                       ") in %s must be greater than %s timeline semaphore %s value (0x%" PRIx64 ")",
                                       loc.Message().c_str(), value, core->report_data->FormatHandle(queue).c_str(), where.c_str(),
                                       core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(
                    semaphore, vuid, "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").",
                    loc.Message().c_str(), value, where.c_str(), core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            timeline_signals[semaphore] = value;
            break;
        }
        default:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        if (submit.pWaitDstStageMask) {
            const LogObjectList objlist(semaphore, state.queue);
            auto loc = outer_loc.dot(Field::pWaitDstStageMask, i);
            skip |= ValidatePipelineStage(objlist, loc, state.queue_flags, submit.pWaitDstStageMask[i]);
            skip |= ValidateStageMaskHost(loc, submit.pWaitDstStageMask[i]);
        }
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pWaitSemaphores-03239",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pNext-03240",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but waitSemaphoreValueCount (%u) is different than "
                                 "waitSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->waitSemaphoreValueCount, submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(outer_loc.dot(Field::pWaitSemaphores, i), semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pWaitSemaphores-03239",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pNext-03241",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but signalSemaphoreValueCount (%u) is different than "
                                 "signalSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->signalSemaphoreValueCount, submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(loc, semaphore, value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo2KHR &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < submit.waitSemaphoreInfoCount; ++i) {
        const auto &wait_info = submit.pWaitSemaphoreInfos[i];
        Location loc = outer_loc.dot(Field::pWaitSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(wait_info.semaphore), loc.dot(Field::stageMask), state.queue_flags,
                                      wait_info.stageMask);
        skip |= ValidateStageMaskHost(loc.dot(Field::stageMask), wait_info.stageMask);
        skip |= state.ValidateWaitSemaphore(loc, wait_info.semaphore, wait_info.value);

        auto semaphore_state = Get<SEMAPHORE_STATE>(wait_info.semaphore);
        if (semaphore_state && semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            for (uint32_t sig_index = 0; sig_index < submit.signalSemaphoreInfoCount; sig_index++) {
                const auto &sig_info = submit.pSignalSemaphoreInfos[sig_index];
                if (wait_info.semaphore == sig_info.semaphore && wait_info.value >= sig_info.value) {
                    Location sig_loc = outer_loc.dot(Field::pSignalSemaphoreInfos, sig_index);
                    const LogObjectList objlist(wait_info.semaphore, state.queue);
                    skip |= LogError(wait_info.semaphore, "VUID-VkSubmitInfo2-semaphore-03881",
                                     "%s has value (%" PRIu64 ") but %s has value (%" PRIu64 ")", loc.Message().c_str(),
                                     wait_info.value, sig_loc.Message().c_str(), sig_info.value);
                }
            }
        }
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreInfoCount; ++i) {
        const auto &sem_info = submit.pSignalSemaphoreInfos[i];
        auto loc = outer_loc.dot(Field::pSignalSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(sem_info.semaphore), loc.dot(Field::stageMask), state.queue_flags,
                                      sem_info.stageMask);
        skip |= ValidateStageMaskHost(loc.dot(Field::stageMask), sem_info.stageMask);
        skip |= state.ValidateSignalSemaphore(loc, sem_info.semaphore, sem_info.value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkBindSparseInfo &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        const LogObjectList objlist(semaphore, state.queue);
        // NOTE: there are no stage masks in bind sparse submissions
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pWaitSemaphores-03246",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pNext-03247",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but waitSemaphoreValueCount (%u) is different than "
                                 "waitSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->waitSemaphoreValueCount, submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(outer_loc.dot(Field::pWaitSemaphores, i), semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pWaitSemaphores-03246",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pNext-03248",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but signalSemaphoreValueCount (%u) is different than "
                                 "signalSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->signalSemaphoreValueCount, submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(loc, semaphore, value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) const {
    bool skip = false;
    auto *sem_type_create_info = LvlFindInChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext);

    if (sem_type_create_info) {
        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE && !enabled_features.core12.timelineSemaphore) {
            skip |= LogError(device, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252",
                             "VkCreateSemaphore: timelineSemaphore feature is not enabled, can not create timeline semaphores");
        }

        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_BINARY && sem_type_create_info->initialValue != 0) {
            skip |= LogError(device, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279",
                             "vkCreateSemaphore: if semaphoreType is VK_SEMAPHORE_TYPE_BINARY, initialValue must be zero");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) const {
    return ValidateWaitSemaphores(device, pWaitInfo, timeout, "VkWaitSemaphores");
}

bool CoreChecks::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) const {
    return ValidateWaitSemaphores(device, pWaitInfo, timeout, "VkWaitSemaphoresKHR");
}

bool CoreChecks::ValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                        const char *apiName) const {
    bool skip = false;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pWaitInfo->pSemaphores[i]);
        if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError(pWaitInfo->pSemaphores[i], "VUID-VkSemaphoreWaitInfo-pSemaphores-03256",
                             "%s(): all semaphores in pWaitInfo must be timeline semaphores, but %s is not", apiName,
                             report_data->FormatHandle(pWaitInfo->pSemaphores[i]).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) const {
    auto fence_node = Get<FENCE_STATE>(fence);
    bool skip = false;
    if (fence_node) {
        if (fence_node->Scope() == kSyncScopeInternal && fence_node->State() == FENCE_INFLIGHT) {
            skip |= LogError(fence, "VUID-vkDestroyFence-fence-01120", "%s is in use.", report_data->FormatHandle(fence).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) const {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = Get<FENCE_STATE>(pFences[i]);
        if (fence_state && fence_state->Scope() == kSyncScopeInternal && fence_state->State() == FENCE_INFLIGHT) {
            skip |= LogError(pFences[i], "VUID-vkResetFences-pFences-01123", "%s is in use.",
                             report_data->FormatHandle(pFences[i]).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                 const VkAllocationCallbacks *pAllocator) const {
    auto sema_node = Get<SEMAPHORE_STATE>(semaphore);
    bool skip = false;
    if (sema_node) {
        skip |= ValidateObjectNotInUse(sema_node.get(), "vkDestroySemaphore", "VUID-vkDestroySemaphore-semaphore-01137");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) const {
    auto event_state = Get<EVENT_STATE>(event);
    bool skip = false;
    if (event_state) {
        skip |= ValidateObjectNotInUse(event_state.get(), "vkDestroyEvent", "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) const {
    auto sampler_state = Get<SAMPLER_STATE>(sampler);
    bool skip = false;
    if (sampler_state) {
        skip |= ValidateObjectNotInUse(sampler_state.get(), "vkDestroySampler", "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETEVENT, VK_TRUE, nullptr, nullptr);
    Location loc(Func::vkCmdSetEvent, Field::stageMask);
    const LogObjectList objlist(commandBuffer);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::ValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                      CMD_TYPE cmd_type) const {
    const LogObjectList objlist(commandBuffer, event);

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, cmd_type, enabled_features.core13.synchronization2,
                                         "VUID-vkCmdSetEvent2-synchronization2-03824", "synchronization2");
    Location loc(Func::vkCmdSetEvent2, Field::pDependencyInfo);
    if (pDependencyInfo->dependencyFlags != 0) {
        skip |= LogError(objlist, "VUID-vkCmdSetEvent2-dependencyFlags-03825", "%s (%s) must be 0",
                         loc.dot(Field::dependencyFlags).Message().c_str(),
                         string_VkDependencyFlags(pDependencyInfo->dependencyFlags).c_str());
    }
    skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfoKHR *pDependencyInfo) const {
    return ValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, CMD_SETEVENT2KHR);
}

bool CoreChecks::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                             const VkDependencyInfo *pDependencyInfo) const {
    return ValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, CMD_SETEVENT2);
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdResetEvent, Field::stageMask);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_RESETEVENT);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::ValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                        CMD_TYPE cmd_type) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdResetEvent2, Field::stageMask);

    bool skip = false;
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdResetEvent2-synchronization2-03829",
                         "vkCmdResetEvent2KHR(): Synchronization2 feature is not enabled");
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  VkPipelineStageFlags2KHR stageMask) const {
    return ValidateCmdResetEvent2(commandBuffer, event, stageMask, CMD_RESETEVENT2KHR);
}

bool CoreChecks::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               VkPipelineStageFlags2 stageMask) const {
    return ValidateCmdResetEvent2(commandBuffer, event, stageMask, CMD_RESETEVENT2);
}

// transient helper struct for checking parts of VUID 02285
struct RenderPassDepState {
    using Location = core_error::Location;
    using Func = core_error::Func;
    using Struct = core_error::Struct;
    using Field = core_error::Field;

    const CoreChecks *core;
    const std::string func_name;
    const std::string vuid;
    uint32_t active_subpass;
    const VkRenderPass rp_handle;
    const VkPipelineStageFlags2KHR disabled_features;
    const std::vector<uint32_t> &self_dependencies;
    const safe_VkSubpassDependency2 *dependencies;

    RenderPassDepState(const CoreChecks *c, const std::string &f, const std::string &v, uint32_t subpass, const VkRenderPass handle,
                       const DeviceFeatures &features, const std::vector<uint32_t> &self_deps,
                       const safe_VkSubpassDependency2 *deps)
        : core(c),
          func_name(f),
          vuid(v),
          active_subpass(subpass),
          rp_handle(handle),
          disabled_features(sync_utils::DisabledPipelineStages(features)),
          self_dependencies(self_deps),
          dependencies(deps) {}

    VkMemoryBarrier2KHR GetSubPassDepBarrier(const safe_VkSubpassDependency2 &dep) {
        VkMemoryBarrier2KHR result;

        const auto *barrier = LvlFindInChain<VkMemoryBarrier2KHR>(dep.pNext);
        if (barrier) {
            result = *barrier;
        } else {
            result.srcStageMask = dep.srcStageMask;
            result.dstStageMask = dep.dstStageMask;
            result.srcAccessMask = dep.srcAccessMask;
            result.dstAccessMask = dep.dstAccessMask;
        }
        return result;
    }

    bool ValidateStage(const Location &loc, VkPipelineStageFlags2KHR src_stage_mask, VkPipelineStageFlags2KHR dst_stage_mask) {
        // Look for matching mask in any self-dependency
        bool match = false;
        for (const auto self_dep_index : self_dependencies) {
            const auto sub_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            auto sub_src_stage_mask =
                sync_utils::ExpandPipelineStages(sub_dep.srcStageMask, sync_utils::kAllQueueTypes, disabled_features);
            auto sub_dst_stage_mask =
                sync_utils::ExpandPipelineStages(sub_dep.dstStageMask, sync_utils::kAllQueueTypes, disabled_features);
            match = ((sub_src_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                     (src_stage_mask == (sub_src_stage_mask & src_stage_mask))) &&
                    ((sub_dst_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                     (dst_stage_mask == (sub_dst_stage_mask & dst_stage_mask)));
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency srcAccessMask "
                           "for any self-dependency of subpass %d of %s for which dstAccessMask is also a subset. "
                           "Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::srcStageMask).Message().c_str(), src_stage_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency dstAccessMask "
                           "for any self-dependency of subpass %d of %s for which srcAccessMask is also a subset. "
                           "Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::dstStageMask).Message().c_str(), dst_stage_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
        }
        return !match;
    }

    bool ValidateAccess(const Location &loc, VkAccessFlags2KHR src_access_mask, VkAccessFlags2KHR dst_access_mask) {
        bool match = false;

        for (const auto self_dep_index : self_dependencies) {
            const auto sub_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            match = (src_access_mask == (sub_dep.srcAccessMask & src_access_mask)) &&
                    (dst_access_mask == (sub_dep.dstAccessMask & dst_access_mask));
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency "
                           "srcAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::srcAccessMask).Message().c_str(), src_access_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency "
                           "dstAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::dstAccessMask).Message().c_str(), dst_access_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
        }
        return !match;
    }

    bool ValidateDependencyFlag(VkDependencyFlags dependency_flags) {
        bool match = false;

        for (const auto self_dep_index : self_dependencies) {
            const auto &sub_dep = dependencies[self_dep_index];
            match = sub_dep.dependencyFlags == dependency_flags;
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s: dependencyFlags param (0x%X) does not equal VkSubpassDependency dependencyFlags value for any "
                           "self-dependency of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           func_name.c_str(), dependency_flags, active_subpass, core->report_data->FormatHandle(rp_handle).c_str(),
                           self_dep_ss.str().c_str());
        }
        return !match;
    }
};

// Validate VUs for Pipeline Barriers that are within a renderPass
// Pre: cb_state->activeRenderPass must be a pointer to valid renderPass state
bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                                    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                                    VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                                    const VkMemoryBarrier *mem_barriers, uint32_t buffer_mem_barrier_count,
                                                    const VkBufferMemoryBarrier *buffer_mem_barriers,
                                                    uint32_t image_mem_barrier_count,
                                                    const VkImageMemoryBarrier *image_barriers) const {
    bool skip = false;
    const auto &rp_state = cb_state->activeRenderPass;
    RenderPassDepState state(this, outer_loc.StringFunc().c_str(), "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                             cb_state->activeSubpass, rp_state->renderPass(), enabled_features,
                             rp_state->self_dependencies[cb_state->activeSubpass], rp_state->createInfo.pDependencies);
    if (state.self_dependencies.size() == 0) {
        skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                         "%s Barriers cannot be set during subpass %d of %s with no self-dependency specified.",
                         outer_loc.Message().c_str(), state.active_subpass, report_data->FormatHandle(state.rp_handle).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    skip |= state.ValidateStage(outer_loc, src_stage_mask, dst_stage_mask);

    if (0 != buffer_mem_barrier_count) {
        skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178",
                         "%s: bufferMemoryBarrierCount is non-zero (%d) for subpass %d of %s.", state.func_name.c_str(),
                         buffer_mem_barrier_count, state.active_subpass, report_data->FormatHandle(rp_state->renderPass()).c_str());
    }
    for (uint32_t i = 0; i < mem_barrier_count; ++i) {
        const auto &mem_barrier = mem_barriers[i];
        Location loc(outer_loc.function, Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= state.ValidateAccess(loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }

    for (uint32_t i = 0; i < image_mem_barrier_count; ++i) {
        const auto &img_barrier = image_barriers[i];
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        skip |= state.ValidateAccess(loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182",
                             "%s is %d and dstQueueFamilyIndex is %d but both must be VK_QUEUE_FAMILY_IGNORED.",
                             loc.dot(Field::srcQueueFamilyIndex).Message().c_str(), img_barrier.srcQueueFamilyIndex,
                             img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass, sub_desc,
                                                   state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(dependency_flags);
    return skip;
}

bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                                    const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;
    const auto &rp_state = cb_state->activeRenderPass;
    if (rp_state->UsesDynamicRendering()) {
        return skip;
    }
    RenderPassDepState state(this, outer_loc.StringFunc().c_str(), "VUID-vkCmdPipelineBarrier2-pDependencies-02285",
                             cb_state->activeSubpass, rp_state->renderPass(), enabled_features,
                             rp_state->self_dependencies[cb_state->activeSubpass], rp_state->createInfo.pDependencies);

    if (state.self_dependencies.size() == 0) {
        skip |= LogError(state.rp_handle, state.vuid,
                         "%s: Barriers cannot be set during subpass %d of %s with no self-dependency specified.",
                         state.func_name.c_str(), state.active_subpass, report_data->FormatHandle(rp_state->renderPass()).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pMemoryBarriers[i];
        Location loc(outer_loc.function, Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        skip |= state.ValidateStage(loc, mem_barrier.srcStageMask, mem_barrier.dstStageMask);
        skip |= state.ValidateAccess(loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }
    if (0 != dep_info->bufferMemoryBarrierCount) {
        skip |=
            LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier2-bufferMemoryBarrierCount-01178",
                     "%s: bufferMemoryBarrierCount is non-zero (%d) for subpass %d of %s.", state.func_name.c_str(),
                     dep_info->bufferMemoryBarrierCount, state.active_subpass, report_data->FormatHandle(state.rp_handle).c_str());
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const auto &img_barrier = dep_info->pImageMemoryBarriers[i];
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);

        skip |= state.ValidateStage(loc, img_barrier.srcStageMask, img_barrier.dstStageMask);
        skip |= state.ValidateAccess(loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-01182",
                             "%s is %d and dstQueueFamilyIndex is %d but both must be VK_QUEUE_FAMILY_IGNORED.",
                             loc.dot(Field::srcQueueFamilyIndex).Message().c_str(), img_barrier.srcQueueFamilyIndex,
                             img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass, sub_desc,
                                                   state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(dep_info->dependencyFlags);
    return skip;
}

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(const LogObjectList &objlist, const Location &loc,
                                                            VkQueueFlags queue_flags, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // these are always allowed.
    stage_mask &= ~(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR |
                    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR | VK_PIPELINE_STAGE_2_HOST_BIT_KHR);
    if (stage_mask == 0) {
        return skip;
    }

    static const std::map<VkPipelineStageFlags2KHR, VkQueueFlags> metaFlags{
        {VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
        {VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT},
        {VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
        {VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
    };

    for (const auto &entry : metaFlags) {
        if (((entry.first & stage_mask) != 0) && ((entry.second & queue_flags) == 0)) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(loc, entry.first);
            skip |= LogError(objlist, vuid,
                             "%s flag %s is not compatible with the queue family properties (%s) of this command buffer.",
                             loc.Message().c_str(), sync_utils::StringPipelineStageFlags(entry.first).c_str(),
                             string_VkQueueFlags(queue_flags).c_str());
        }
        stage_mask &= ~entry.first;
    }
    if (stage_mask == 0) {
        return skip;
    }

    auto supported_flags = sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR, queue_flags);

    auto bad_flags = stage_mask & ~supported_flags;

    // Lookup each bit in the stagemask and check for overlap between its table bits and queue_flags
    for (size_t i = 0; i < sizeof(bad_flags) * 8; i++) {
        VkPipelineStageFlags2KHR bit = (1ULL << i) & bad_flags;
        if (bit) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(loc, bit);
            skip |= LogError(
                objlist, vuid, "%s flag %s is not compatible with the queue family properties (%s) of this command buffer.",
                loc.Message().c_str(), sync_utils::StringPipelineStageFlags(bit).c_str(), string_VkQueueFlags(queue_flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStageFeatureEnables(const LogObjectList &objlist, const Location &loc,
                                                     VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    if (!enabled_features.core13.synchronization2 && stage_mask == 0) {
        const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(loc, 0);
        std::stringstream msg;
        msg << loc.Message() << " must not be 0 unless synchronization2 is enabled.";
        skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
    }

    auto disabled_stages = sync_utils::DisabledPipelineStages(enabled_features);
    auto bad_bits = stage_mask & disabled_stages;
    if (bad_bits == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_bits) * 8; i++) {
        VkPipelineStageFlags2KHR bit = 1ULL << i;
        if (bit & bad_bits) {
            const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(loc, bit);
            std::stringstream msg;
            msg << loc.Message() << " includes " << sync_utils::StringPipelineStageFlags(bit) << " when the device does not have "
                << sync_vuid_maps::kFeatureNameMap.at(bit) << " feature enabled.";

            skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStage(const LogObjectList &objlist, const Location &loc, VkQueueFlags queue_flags,
                                       VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    skip |= ValidateStageMasksAgainstQueueCapabilities(objlist, loc, queue_flags, stage_mask);
    skip |= ValidatePipelineStageFeatureEnables(objlist, loc, stage_mask);
    return skip;
}

bool CoreChecks::ValidateAccessMask(const LogObjectList &objlist, const Location &loc, VkQueueFlags queue_flags,
                                    VkAccessFlags2KHR access_mask, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // Early out if all commands set
    if ((stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) != 0) return skip;

    // or if only generic memory accesses are specified (or we got a 0 mask)
    access_mask &= ~(VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR);
    if (access_mask == 0) return skip;

    auto expanded_stages = sync_utils::ExpandPipelineStages(stage_mask, queue_flags);  // TODO:
    auto valid_accesses = sync_utils::CompatibleAccessMask(expanded_stages);
    auto bad_accesses = (access_mask & ~valid_accesses);
    if (bad_accesses == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_accesses) * 8; i++) {
        VkAccessFlags2KHR bit = (1ULL << i);
        if (bad_accesses & bit) {
            const auto &vuid = sync_vuid_maps::GetBadAccessFlagsVUID(loc, bit);
            std::stringstream msg;
            msg << loc.Message() << " bit " << sync_utils::StringAccessFlags(bit) << " is not supported by stage mask ("
                << sync_utils::StringPipelineStageFlags(stage_mask) << ").";
            skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateEventStageMask(const CMD_BUFFER_STATE &cb_state, size_t eventCount, size_t firstEventIndex,
                                        VkPipelineStageFlags2KHR sourceStageMask, EventToStageMap *localEventToStageMap) {
    bool skip = false;
    const ValidationStateTracker *state_data = cb_state.dev_data;
    VkPipelineStageFlags2KHR stage_mask = 0;
    const auto max_event = std::min((firstEventIndex + eventCount), cb_state.events.size());
    for (size_t event_index = firstEventIndex; event_index < max_event; ++event_index) {
        auto event = cb_state.events[event_index];
        auto event_data = localEventToStageMap->find(event);
        if (event_data != localEventToStageMap->end()) {
            stage_mask |= event_data->second;
        } else {
            auto global_event_data = state_data->Get<EVENT_STATE>(event);
            if (!global_event_data) {
                skip |= state_data->LogError(event, kVUID_Core_DrawState_InvalidEvent,
                                             "%s cannot be waited on if it has never been set.",
                                             state_data->report_data->FormatHandle(event).c_str());
            } else {
                stage_mask |= global_event_data->stageMask;
            }
        }
    }
    // TODO: Need to validate that host_bit is only set if set event is called
    // but set event can be called at any time.
    if (sourceStageMask != stage_mask && sourceStageMask != (stage_mask | VK_PIPELINE_STAGE_HOST_BIT)) {
        skip |= state_data->LogError(
            cb_state.commandBuffer(), "VUID-vkCmdWaitEvents-srcStageMask-parameter",
            "Submitting cmdbuffer with call to VkCmdWaitEvents using srcStageMask 0x%" PRIx64
            " which must be the bitwise OR of "
            "the stageMask parameters used in calls to vkCmdSetEvent and VK_PIPELINE_STAGE_HOST_BIT if used with "
            "vkSetEvent but instead is 0x%" PRIx64 ".",
            sourceStageMask, stage_mask);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount,
                                              const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    auto queue_flags = cb_state->GetQueueFlags();
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdWaitEvents);

    skip |= ValidatePipelineStage(objlist, loc.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, loc.dot(Field::dstStageMask), queue_flags, dstStageMask);

    skip |= ValidateCmd(*cb_state, CMD_WAITEVENTS);
    skip |= ValidateBarriers(loc.dot(Field::pDependencyInfo), cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount,
                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                             pImageMemoryBarriers);
    for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i) {
        if (pBufferMemoryBarriers[i].srcQueueFamilyIndex != pBufferMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803",
                             "vkCmdWaitEvents(): pBufferMemoryBarriers[%" PRIu32 "] has different srcQueueFamilyIndex (%" PRIu32
                             ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             i, pBufferMemoryBarriers[i].srcQueueFamilyIndex, pBufferMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        if (pImageMemoryBarriers[i].srcQueueFamilyIndex != pImageMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803",
                             "vkCmdWaitEvents(): pImageMemoryBarriers[%" PRIu32 "] has different srcQueueFamilyIndex (%" PRIu32
                             ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             i, pImageMemoryBarriers[i].srcQueueFamilyIndex, pImageMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                        const VkDependencyInfo *pDependencyInfos, CMD_TYPE cmd_type) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    const char *func_name = CommandTypeString(cmd_type);
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents2-synchronization2-03836",
                         "%s(): Synchronization2 feature is not enabled", func_name);
    }
    for (uint32_t i = 0; (i < eventCount) && !skip; i++) {
        const LogObjectList objlist(commandBuffer, pEvents[i]);
        Location loc(Func::vkCmdWaitEvents2, Field::pDependencyInfos, i);
        if (pDependencyInfos[i].dependencyFlags != 0) {
            skip |= LogError(objlist, "VUID-vkCmdWaitEvents2-dependencyFlags-03844", "%s (%s) must be 0.",
                             loc.dot(Field::dependencyFlags).Message().c_str(),
                             string_VkDependencyFlags(pDependencyInfos[i].dependencyFlags).c_str());
        }
        skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), &pDependencyInfos[i]);
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfoKHR *pDependencyInfos) const {
    return ValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2KHR);
}

bool CoreChecks::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               const VkDependencyInfo *pDependencyInfos) const {
    return ValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2);
}

void CORE_CMD_BUFFER_STATE::RecordWaitEvents(CMD_TYPE cmd_type, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags2KHR srcStageMask) {
    // CMD_BUFFER_STATE will add to the events vector.
    auto first_event_index = events.size();
    CMD_BUFFER_STATE::RecordWaitEvents(cmd_type, eventCount, pEvents, srcStageMask);
    auto event_added_count = events.size() - first_event_index;
    eventUpdates.emplace_back([event_added_count, first_event_index, srcStageMask](CMD_BUFFER_STATE &cb_state, bool do_validate,
                                                                                   EventToStageMap *localEventToStageMap) {
        if (!do_validate) return false;
        return CoreChecks::ValidateEventStageMask(cb_state, event_added_count, first_event_index, srcStageMask,
                                                  localEventToStageMap);
    });
}

void CoreChecks::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                            uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                            uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                            uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    StateTracker::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                             imageMemoryBarrierCount, pImageMemoryBarriers);
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, CMD_TYPE cmd_type) {
    // don't hold read lock during the base class method
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        TransitionImageLayouts(cb_state.get(), dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers);
    }
}

void CoreChecks::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                const VkDependencyInfoKHR *pDependencyInfos) {
    StateTracker::PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2KHR);
}

void CoreChecks::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             const VkDependencyInfo *pDependencyInfos) {
    StateTracker::PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2);
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdWaitEvents, cb_state.get(), bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                   pImageMemoryBarriers);
}

void CoreChecks::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfoKHR *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(Func::vkCmdWaitEvents2, cb_state.get(), dep_info);
    }
}

void CoreChecks::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              const VkDependencyInfo *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(Func::vkCmdWaitEvents2, cb_state.get(), dep_info);
    }
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                   uint32_t bufferMemoryBarrierCount,
                                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                   uint32_t imageMemoryBarrierCount,
                                                   const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    auto queue_flags = cb_state->GetQueueFlags();
    Location loc(Func::vkCmdPipelineBarrier);

    skip |= ValidatePipelineStage(objlist, loc.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, loc.dot(Field::dstStageMask), queue_flags, dstStageMask);
    skip |= ValidateCmd(*cb_state, CMD_PIPELINEBARRIER);
    if (cb_state->activeRenderPass && !cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= ValidateRenderPassPipelineBarriers(loc, cb_state.get(), srcStageMask, dstStageMask, dependencyFlags,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip = LogError(objlist, "VUID-vkCmdPipelineBarrier-dependencyFlags-01186",
                            "%s VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance",
                            loc.dot(Field::dependencyFlags).Message().c_str());
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier-None-06191",
                         "vkCmdPipelineBarrier(): a dynamic render pass instance is active.");
    }
    skip |= ValidateBarriers(loc, cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

bool CoreChecks::ValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                             CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    const char *func_name = CommandTypeString(cmd_type);

    Location loc(Func::vkCmdPipelineBarrier2, Field::pDependencyInfo);
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier2-synchronization2-03848",
                         "%s(): Synchronization2 feature is not enabled", func_name);
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers(loc, cb_state.get(), pDependencyInfo);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (pDependencyInfo->dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip = LogError(objlist, "VUID-vkCmdPipelineBarrier2-dependencyFlags-01186",
                            "%s VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance",
                            loc.dot(Field::dependencyFlags).Message().c_str());
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier2-None-06191",
                         "vkCmdPipelineBarrier(): a dynamic render pass instance is active.");
    }
    skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                       const VkDependencyInfoKHR *pDependencyInfo) const {
    return ValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, CMD_PIPELINEBARRIER2KHR);
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) const {
    return ValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, CMD_PIPELINEBARRIER2);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier *pImageMemoryBarriers) {
    StateTracker::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                  pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                  imageMemoryBarrierCount, pImageMemoryBarriers);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    RecordBarriers(Func::vkCmdPipelineBarrier, cb_state.get(), bufferMemoryBarrierCount, pBufferMemoryBarriers,
                   imageMemoryBarrierCount, pImageMemoryBarriers);
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo) {
    StateTracker::PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdPipelineBarrier2, cb_state.get(), *pDependencyInfo);
    TransitionImageLayouts(cb_state.get(), pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) {
    StateTracker::PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdPipelineBarrier2, cb_state.get(), *pDependencyInfo);
    TransitionImageLayouts(cb_state.get(), pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->write_in_use) {
            skip |=
                LogError(event, kVUID_Core_DrawState_QueueForwardProgress,
                         "vkSetEvent(): %s that is already in use by a command buffer.", report_data->FormatHandle(event).c_str());
        }
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkSetEvent-event-03941",
                             "vkSetEvent(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetEvent(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkResetEvent-event-03823",
                             "vkResetEvent(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEventStatus(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkGetEventStatus-event-03940",
                             "vkGetEventStatus(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo, const char *api_name) const {
    bool skip = false;
    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (!semaphore_state) {
        return skip;
    }
    if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-semaphore-03257",
                         "%s(): semaphore %s must be of VK_SEMAPHORE_TYPE_TIMELINE type.", api_name,
                         report_data->FormatHandle(pSignalInfo->semaphore).c_str());
        return skip;
    }

    const auto completed = semaphore_state->Completed();
    if (completed.payload >= pSignalInfo->value) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-value-03258",
                         "%s(): value (%" PRIu64 ") must be greater than current semaphore %s value (%" PRIu64 ").", api_name,
                         pSignalInfo->value, report_data->FormatHandle(pSignalInfo->semaphore).c_str(), completed.payload);
        return skip;
    }
    auto exceeds_pending = [pSignalInfo](const SEMAPHORE_STATE::SemOp &op, bool is_pending) {
        return is_pending && op.IsSignal() && pSignalInfo->value >= op.payload;
    };
    auto last_op = semaphore_state->LastOp(exceeds_pending);
    if (last_op) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-value-03259",
                         "%s(): value (%" PRIu64 ") must be less than value of any pending signal operation (%" PRIu64
                         ") for semaphore %s.",
                         api_name, pSignalInfo->value, last_op->payload, report_data->FormatHandle(pSignalInfo->semaphore).c_str());
        return skip;
    }

    uint64_t bad_value = 0;
    const char *where = nullptr;
    TimelineMaxDiffCheck exceeds_max_diff(pSignalInfo->value, phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
    last_op = semaphore_state->LastOp(exceeds_max_diff);
    if (last_op) {
        bad_value = last_op->payload;
        if (last_op->payload == semaphore_state->Completed().payload) {
            where = "current";
        } else {
            where = "pending";
        }
    }
    if (where) {
        Location loc(Func::vkSignalSemaphore, Struct::VkSemaphoreSignalInfo, Field::value);
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kTimelineSemMaxDiff);
        skip |=
            LogError(semaphore_state->Handle(), vuid,
                     "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s payload (%" PRIu64 ").", loc.Message().c_str(),
                     pSignalInfo->value, report_data->FormatHandle(semaphore_state->Handle()).c_str(), where, bad_value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) const {
    return ValidateSignalSemaphore(device, pSignalInfo, "vkSignalSemaphore");
}

bool CoreChecks::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) const {
    return ValidateSignalSemaphore(device, pSignalInfo, "vkSignalSemaphoreKHR");
}

bool CoreChecks::ValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                  const char *apiName) const {
    bool skip = false;
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError(semaphore, "VUID-vkGetSemaphoreCounterValue-semaphore-03255",
                         "%s(): semaphore %s must be of VK_SEMAPHORE_TYPE_TIMELINE type", apiName,
                         report_data->FormatHandle(semaphore).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue) const {
    return ValidateGetSemaphoreCounterValue(device, semaphore, pValue, "vkGetSemaphoreCounterValueKHR");
}
bool CoreChecks::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue) const {
    return ValidateGetSemaphoreCounterValue(device, semaphore, pValue, "vkGetSemaphoreCounterValue");
}