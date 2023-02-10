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
 */

#include <algorithm>
#include <assert.h>
#include <string>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "sync/sync_utils.h"

ReadLockGuard CoreChecks::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard CoreChecks::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

extern template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                        const VkImageMemoryBarrier *barrier);
extern template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                        const VkImageMemoryBarrier2KHR *barrier);
extern template bool CoreChecks::UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE *cb_state, const Location &loc,
                                                                   const VkImageMemoryBarrier &img_barrier,
                                                                   const CommandBufferImageLayoutMap &current_map,
                                                                   CommandBufferImageLayoutMap &layout_updates) const;
extern template bool CoreChecks::UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE *cb_state, const Location &loc,
                                                                   const VkImageMemoryBarrier2KHR &img_barrier,
                                                                   const CommandBufferImageLayoutMap &current_map,
                                                                   CommandBufferImageLayoutMap &layout_updates) const;

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

bool CoreChecks::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkFence *pFence) const {
    bool skip = false;
    auto fence_export_info = LvlFindInChain<VkExportFenceCreateInfo>(pCreateInfo->pNext);
    if (fence_export_info && fence_export_info->handleTypes != 0) {
        auto external_properties = LvlInitStruct<VkExternalFenceProperties>();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalFenceHandleTypeFlagBits flag) {
            auto external_info = LvlInitStruct<VkPhysicalDeviceExternalFenceInfo>();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalFenceProperties(physical_device, &external_info, &external_properties);
            if ((external_properties.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError(device, "VUID-VkExportFenceCreateInfo-handleTypes-01446",
                                 "vkCreateFence(): VkFenceCreateInfo pNext chain contains VkExportFenceCreateInfo with the %s flag "
                                 "set, which does not support VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalFenceHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalFenceHandleTypeFlagBits>(fence_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (fence_export_info->handleTypes & external_properties.compatibleHandleTypes) != fence_export_info->handleTypes) {
            skip |= LogError(device, "VUID-VkExportFenceCreateInfo-handleTypes-01446",
                             "vkCreateFence(): VkFenceCreateInfo pNext chain contains VkExportFenceCreateInfo with handleTypes "
                             "flags (%s) that are not reported as compatible by vkGetPhysicalDeviceExternalFenceProperties.",
                             string_VkExternalFenceHandleTypeFlags(fence_export_info->handleTypes).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) const {
    bool skip = false;
    auto sem_type_create_info = LvlFindInChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext);

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

    auto sem_export_info = LvlFindInChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    if (sem_export_info && sem_export_info->handleTypes != 0) {
        auto external_properties = LvlInitStruct<VkExternalSemaphoreProperties>();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalSemaphoreHandleTypeFlagBits flag) {
            auto external_info = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfo>();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalSemaphoreProperties(physical_device, &external_info, &external_properties);
            if ((external_properties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError(device, "VUID-VkExportSemaphoreCreateInfo-handleTypes-01124",
                                 "vkCreateSemaphore(): VkSemaphoreCreateInfo pNext chain contains VkExportSemaphoreCreateInfo with "
                                 "the %s flag set, which does not support VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalSemaphoreHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalSemaphoreHandleTypeFlagBits>(sem_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (sem_export_info->handleTypes & external_properties.compatibleHandleTypes) != sem_export_info->handleTypes) {
            skip |= LogError(
                device, "VUID-VkExportSemaphoreCreateInfo-handleTypes-01124",
                "vkCreateSemaphore(): VkSemaphoreCreateInfo pNext chain contains VkExportSemaphoreCreateInfo with "
                "handleTypes flags (%s) that are not reported as compatible by vkGetPhysicalDeviceExternalSemaphoreProperties.",
                string_VkExternalSemaphoreHandleTypeFlags(sem_export_info->handleTypes).c_str());
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

// VkSubpassDependency validation happens when vkCreateRenderPass() is called.
// Dependencies between subpasses can only use pipeline stages compatible with VK_QUEUE_GRAPHICS_BIT,
// for external subpasses we don't have a yet command buffer so we have to assume all of them are valid.
static inline VkQueueFlags SubpassToQueueFlags(uint32_t subpass) {
    return subpass == VK_SUBPASS_EXTERNAL ? sync_utils::kAllQueueTypes : static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT);
}

bool CoreChecks::ValidateSubpassDependency(const LogObjectList &objects, const Location &in_loc,
                                           const VkSubpassDependency2 &dependency) const {
    bool skip = false;
    Location loc = in_loc;
    VkMemoryBarrier2KHR converted_barrier;
    const auto *mem_barrier = LvlFindInChain<VkMemoryBarrier2KHR>(dependency.pNext);

    if (mem_barrier && enabled_features.core13.synchronization2) {
        if (dependency.srcAccessMask != 0) {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcAccessMask",
                             "%s is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                             loc.dot(Field::srcAccessMask).Message().c_str());
        }
        if (dependency.dstAccessMask != 0) {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstAccessMask",
                             "%s dstAccessMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                             loc.dot(Field::dstAccessMask).Message().c_str());
        }
        if (dependency.srcStageMask != 0) {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask",
                             "%s srcStageMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                             loc.dot(Field::srcStageMask).Message().c_str());
        }
        if (dependency.dstStageMask != 0) {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask",
                             "%s dstStageMask is non-zero when a VkMemoryBarrier2KHR is present in pNext.",
                             loc.dot(Field::dstStageMask).Message().c_str());
        }
        loc = in_loc.dot(Field::pNext);
        converted_barrier = *mem_barrier;
    } else {
        if (mem_barrier) {
            skip |= LogError(objects, "UNASSIGNED-CoreChecks-VkSubpassDependency2-pNext",
                             "%s a VkMemoryBarrier2KHR is present in pNext but synchronization2 is not enabled.",
                             loc.Message().c_str());
        }
        // use the subpass dependency flags, upconverted into wider synchronization2 fields.
        converted_barrier.srcStageMask = dependency.srcStageMask;
        converted_barrier.dstStageMask = dependency.dstStageMask;
        converted_barrier.srcAccessMask = dependency.srcAccessMask;
        converted_barrier.dstAccessMask = dependency.dstAccessMask;
    }
    auto src_queue_flags = SubpassToQueueFlags(dependency.srcSubpass);
    skip |= ValidatePipelineStage(objects, loc.dot(Field::srcStageMask), src_queue_flags, converted_barrier.srcStageMask);
    skip |= ValidateAccessMask(objects, loc.dot(Field::srcAccessMask), src_queue_flags, converted_barrier.srcAccessMask,
                               converted_barrier.srcStageMask);

    auto dst_queue_flags = SubpassToQueueFlags(dependency.dstSubpass);
    skip |= ValidatePipelineStage(objects, loc.dot(Field::dstStageMask), dst_queue_flags, converted_barrier.dstStageMask);
    skip |= ValidateAccessMask(objects, loc.dot(Field::dstAccessMask), dst_queue_flags, converted_barrier.dstAccessMask,
                               converted_barrier.dstStageMask);
    return skip;
}

// Verify an ImageMemoryBarrier's old/new ImageLayouts are compatible with the Image's ImageUsageFlags.
bool CoreChecks::ValidateBarrierLayoutToImageUsage(const Location &loc, VkImage image, VkImageLayout layout,
                                                   VkImageUsageFlags usage_flags) const {
    bool skip = false;
    bool is_error = false;
    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0);
            break;
        // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            // alias VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
            is_error = ((usage_flags & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            is_error = ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0);
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            is_error = ((usage_flags & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) == 0);
            is_error |= ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0);
            is_error |= ((usage_flags & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR) == 0);
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            is_error = ((usage_flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) == 0);
            break;
        default:
            // Other VkImageLayout values do not have VUs defined in this context.
            break;
    }

    if (is_error) {
        const auto &vuid = sync_vuid_maps::GetBadImageLayoutVUID(loc, layout);

        skip |=
            LogError(image, vuid, "%s Image barrier Layout=%s is not compatible with %s usage flags 0x%" PRIx32 ".",
                     loc.Message().c_str(), string_VkImageLayout(layout), report_data->FormatHandle(image).c_str(), usage_flags);
    }
    return skip;
}

// Verify image barriers are compatible with the images they reference.
template <typename ImageBarrier>
bool CoreChecks::ValidateBarriersToImages(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                          uint32_t imageMemoryBarrierCount, const ImageBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    // Scoreboard for duplicate layout transition barriers within the list
    // Pointers retained in the scoreboard only have the lifetime of *this* call (i.e. within the scope of the API call)
    const CommandBufferImageLayoutMap &current_map = cb_state->GetImageSubresourceLayoutMap();
    CommandBufferImageLayoutMap layout_updates;

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        auto loc = outer_loc.dot(Field::pImageMemoryBarriers, i);
        const auto &img_barrier = pImageMemoryBarriers[i];

        auto image_state = Get<IMAGE_STATE>(img_barrier.image);
        if (!image_state) {
            continue;
        }

        if ((img_barrier.srcQueueFamilyIndex != img_barrier.dstQueueFamilyIndex) ||
            (img_barrier.oldLayout != img_barrier.newLayout)) {
            VkImageUsageFlags usage_flags = image_state->createInfo.usage;
            skip |=
                ValidateBarrierLayoutToImageUsage(loc.dot(Field::oldLayout), img_barrier.image, img_barrier.oldLayout, usage_flags);
            skip |=
                ValidateBarrierLayoutToImageUsage(loc.dot(Field::newLayout), img_barrier.image, img_barrier.newLayout, usage_flags);
        }

        // Make sure layout is able to be transitioned, currently only presented shared presentable images are locked
        if (image_state->layout_locked) {
            // TODO: Add unique id for error when available
            skip |= LogError(
                img_barrier.image, "VUID-Undefined",
                "%s Attempting to transition shared presentable %s"
                " from layout %s to layout %s, but image has already been presented and cannot have its layout transitioned.",
                loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
        }

        const VkImageCreateInfo &image_create_info = image_state->createInfo;
        const VkFormat image_format = image_create_info.format;
        const VkImageAspectFlags aspect_mask = img_barrier.subresourceRange.aspectMask;
        // For a Depth/Stencil image both aspects MUST be set
        auto image_loc = loc.dot(Field::image);
        if (FormatIsDepthAndStencil(image_format)) {
            if (enabled_features.core12.separateDepthStencilLayouts) {
                if (!(aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
                    auto vuid = GetImageBarrierVUID(loc, ImageError::kNotDepthOrStencilAspect);
                    skip |= LogError(img_barrier.image, vuid,
                                     "%s references %s of format %s that must have either the depth or stencil "
                                     "aspects set, but its aspectMask is 0x%" PRIx32 ".",
                                     image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                     string_VkFormat(image_format), aspect_mask);
                }
            } else {
                auto const ds_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                if ((aspect_mask & ds_mask) != (ds_mask)) {
                    auto error = IsExtEnabled(device_extensions.vk_khr_separate_depth_stencil_layouts)
                                     ? ImageError::kNotSeparateDepthAndStencilAspect
                                     : ImageError::kNotDepthAndStencilAspect;
                    auto vuid = GetImageBarrierVUID(image_loc, error);
                    skip |= LogError(img_barrier.image, vuid,
                                     "%s references %s of format %s that must have the depth and stencil "
                                     "aspects set, but its aspectMask is 0x%" PRIx32 ".",
                                     image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                     string_VkFormat(image_format), aspect_mask);
                }
            }
        }

        if (img_barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            // TODO: Set memory invalid which is in mem_tracker currently
        } else if (!QueueFamilyIsExternal(img_barrier.srcQueueFamilyIndex)) {
            skip |= UpdateCommandBufferImageLayoutMap(cb_state, loc, img_barrier, current_map, layout_updates);
        }

        // checks color format and (single-plane or non-disjoint)
        // if ycbcr extension is not supported then single-plane and non-disjoint are always both true
        if ((FormatIsColor(image_format) == true) &&
            ((FormatIsMultiplane(image_format) == false) || (image_state->disjoint == false))) {
            if (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) {
                auto error = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion) ? ImageError::kNotColorAspect
                                                                                             : ImageError::kNotColorAspectYcbcr;
                const auto &vuid = GetImageBarrierVUID(loc, error);
                skip |= LogError(img_barrier.image, vuid,
                                 "%s references %s of format %s that must be only VK_IMAGE_ASPECT_COLOR_BIT, "
                                 "but its aspectMask is 0x%" PRIx32 ".",
                                 image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                                 string_VkFormat(image_format), aspect_mask);
            }
        }

        VkImageAspectFlags valid_disjoint_mask =
            VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        if ((FormatIsMultiplane(image_format) == true) && (image_state->disjoint == true) &&
            ((aspect_mask & valid_disjoint_mask) == 0)) {
            const auto &vuid = GetImageBarrierVUID(image_loc, ImageError::kBadMultiplanarAspect);
            skip |= LogError(img_barrier.image, vuid,
                             "%s references %s of format %s has aspectMask (0x%" PRIx32
                             ") but needs to include either an VK_IMAGE_ASPECT_PLANE_*_BIT or VK_IMAGE_ASPECT_COLOR_BIT.",
                             image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                             string_VkFormat(image_format), aspect_mask);
        }

        if ((FormatPlaneCount(image_format) == 2) && ((aspect_mask & VK_IMAGE_ASPECT_PLANE_2_BIT) != 0)) {
            const auto &vuid = GetImageBarrierVUID(image_loc, ImageError::kBadPlaneCount);
            skip |= LogError(img_barrier.image, vuid,
                             "%s references %s of format %s has only two planes but included "
                             "VK_IMAGE_ASPECT_PLANE_2_BIT in its aspectMask (0x%" PRIx32 ").",
                             image_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                             string_VkFormat(image_format), aspect_mask);
        }
    }
    return skip;
}

// Verify image barrier image state and that the image is consistent with FB image
template <typename ImgBarrier>
bool CoreChecks::ValidateImageBarrierAttachment(const Location &loc, CMD_BUFFER_STATE const *cb_state,
                                                const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                const safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                const ImgBarrier &img_barrier, const CMD_BUFFER_STATE *primary_cb_state) const {
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    bool skip = false;
    const auto *fb_state = framebuffer;
    assert(fb_state);
    const auto img_bar_image = img_barrier.image;
    bool image_match = false;
    bool sub_image_found = false;  // Do we find a corresponding subpass description
    VkImageLayout sub_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t attach_index = 0;
    // Verify that a framebuffer image matches barrier image
    const auto attachment_count = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
        auto view_state = primary_cb_state ? primary_cb_state->GetActiveAttachmentImageViewState(attachment)
                                           : cb_state->GetActiveAttachmentImageViewState(attachment);
        if (view_state && (img_bar_image == view_state->create_info.image)) {
            image_match = true;
            attach_index = attachment;
            break;
        }
    }
    if (image_match) {  // Make sure subpass is referring to matching attachment
        if (sub_desc.pDepthStencilAttachment && sub_desc.pDepthStencilAttachment->attachment == attach_index) {
            sub_image_layout = sub_desc.pDepthStencilAttachment->layout;
            sub_image_found = true;
        }
        if (!sub_image_found && IsExtEnabled(device_extensions.vk_khr_depth_stencil_resolve)) {
            const auto *resolve = LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(sub_desc.pNext);
            if (resolve && resolve->pDepthStencilResolveAttachment &&
                resolve->pDepthStencilResolveAttachment->attachment == attach_index) {
                sub_image_layout = resolve->pDepthStencilResolveAttachment->layout;
                sub_image_found = true;
            }
        }
        if (!sub_image_found) {
            for (uint32_t j = 0; j < sub_desc.colorAttachmentCount; ++j) {
                if (sub_desc.pColorAttachments && sub_desc.pColorAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pColorAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
                if (!sub_image_found && sub_desc.pResolveAttachments &&
                    sub_desc.pResolveAttachments[j].attachment == attach_index) {
                    sub_image_layout = sub_desc.pResolveAttachments[j].layout;
                    sub_image_found = true;
                    break;
                }
            }
        }
        if (!sub_image_found) {
            auto img_loc = loc.dot(Field::image);
            const auto &vuid = GetImageBarrierVUID(img_loc, ImageError::kRenderPassMismatch);
            skip |=
                LogError(rp_handle, vuid,
                         "%s Barrier for %s is not referenced by the VkSubpassDescription for active subpass (%d) of current %s.",
                         img_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                         report_data->FormatHandle(rp_handle).c_str());
        }
    } else {  // !image_match
        auto img_loc = loc.dot(Field::image);
        const auto &vuid = GetImageBarrierVUID(img_loc, ImageError::kRenderPassMismatch);
        skip |= LogError(fb_state->framebuffer(), vuid, "%s Barrier for %s does not match an image from the current %s.",
                         img_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(),
                         report_data->FormatHandle(fb_state->framebuffer()).c_str());
    }
    if (img_barrier.oldLayout != img_barrier.newLayout) {
        auto layout_loc = loc.dot(Field::oldLayout);
        const auto &vuid = GetImageBarrierVUID(layout_loc, ImageError::kRenderPassLayoutChange);
        skip |= LogError(cb_state->commandBuffer(), vuid,
                         "%s As the Image Barrier for %s is being executed within a render pass instance, oldLayout must "
                         "equal newLayout yet they are %s and %s.",
                         layout_loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(),
                         string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
    } else {
        if (sub_image_found && sub_image_layout != img_barrier.oldLayout) {
            const LogObjectList objlist(rp_handle, img_bar_image);
            auto layout_loc = loc.dot(Field::oldLayout);
            const auto &vuid = GetImageBarrierVUID(layout_loc, ImageError::kRenderPassLayoutChange);
            skip |= LogError(objlist, vuid,
                             "%s Barrier for %s is referenced by the VkSubpassDescription for active "
                             "subpass (%d) of current %s as having layout %s, but image barrier has layout %s.",
                             layout_loc.Message().c_str(), report_data->FormatHandle(img_bar_image).c_str(), active_subpass,
                             report_data->FormatHandle(rp_handle).c_str(), string_VkImageLayout(sub_image_layout),
                             string_VkImageLayout(img_barrier.oldLayout));
        }
    }
    return skip;
}

VulkanTypedHandle BarrierTypedHandle(const VkImageMemoryBarrier &barrier) {
    return VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage);
}

VulkanTypedHandle BarrierTypedHandle(const VkImageMemoryBarrier2KHR &barrier) {
    return VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage);
}

std::shared_ptr<const IMAGE_STATE> BarrierHandleState(const ValidationStateTracker &device_state,
                                                      const VkImageMemoryBarrier &barrier) {
    return device_state.Get<IMAGE_STATE>(barrier.image);
}

std::shared_ptr<const IMAGE_STATE> BarrierHandleState(const ValidationStateTracker &device_state,
                                                      const VkImageMemoryBarrier2KHR &barrier) {
    return device_state.Get<IMAGE_STATE>(barrier.image);
}

VulkanTypedHandle BarrierTypedHandle(const VkBufferMemoryBarrier &barrier) {
    return VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer);
}

VulkanTypedHandle BarrierTypedHandle(const VkBufferMemoryBarrier2KHR &barrier) {
    return VulkanTypedHandle(barrier.buffer, kVulkanObjectTypeBuffer);
}

const std::shared_ptr<const BUFFER_STATE> BarrierHandleState(const ValidationStateTracker &device_state,
                                                             const VkBufferMemoryBarrier &barrier) {
    return device_state.Get<BUFFER_STATE>(barrier.buffer);
}

std::shared_ptr<const BUFFER_STATE> BarrierHandleState(const ValidationStateTracker &device_state,
                                                       const VkBufferMemoryBarrier2KHR &barrier) {
    return device_state.Get<BUFFER_STATE>(barrier.buffer);
}

template <typename ImgBarrier>
void CoreChecks::EnqueueSubmitTimeValidateImageBarrierAttachment(const Location &loc, CMD_BUFFER_STATE *cb_state,
                                                                 const ImgBarrier &barrier) {
    // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
    if ((cb_state->activeRenderPass) && (VK_NULL_HANDLE == cb_state->activeFramebuffer) &&
        (VK_COMMAND_BUFFER_LEVEL_SECONDARY == cb_state->createInfo.level)) {
        const auto active_subpass = cb_state->activeSubpass;
        const auto rp_state = cb_state->activeRenderPass;
        const auto &sub_desc = rp_state->createInfo.pSubpasses[active_subpass];
        // Secondary CB case w/o FB specified delay validation
        auto *this_ptr = this;  // Required for older compilers with c++20 compatibility
        core_error::LocationCapture loc_capture(loc);
        const auto render_pass = rp_state->renderPass();
        cb_state->cmd_execute_commands_functions.emplace_back(
            [this_ptr, loc_capture, active_subpass, sub_desc, render_pass, barrier](
                const CMD_BUFFER_STATE &secondary_cb, const CMD_BUFFER_STATE *primary_cb, const FRAMEBUFFER_STATE *fb) {
                return this_ptr->ValidateImageBarrierAttachment(loc_capture.Get(), &secondary_cb, fb, active_subpass, sub_desc,
                                                                render_pass, barrier, primary_cb);
            });
    }
}

template <typename Barrier, typename TransferBarrier>
void CoreChecks::RecordBarrierValidationInfo(const Location &loc, CMD_BUFFER_STATE *cb_state, const Barrier &barrier,
                                             QFOTransferBarrierSets<TransferBarrier> &barrier_sets) {
    if (IsTransferOp(barrier)) {
        if (cb_state->IsReleaseOp(barrier) && !QueueFamilyIsExternal(barrier.dstQueueFamilyIndex)) {
            barrier_sets.release.emplace(barrier);
        } else if (cb_state->IsAcquireOp(barrier) && !QueueFamilyIsExternal(barrier.srcQueueFamilyIndex)) {
            barrier_sets.acquire.emplace(barrier);
        }
    }

    // 7.7.4: If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer is performed, and the
    // barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED.
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    const bool is_ownership_transfer = src_queue_family != dst_queue_family;

    if (is_ownership_transfer) {
        // Only enqueue submit time check if it is needed. If more submit time checks are added, change the criteria
        // TODO create a better named list, or rename the submit time lists to something that matches the broader usage...
        auto handle_state = BarrierHandleState(*this, barrier);
        const bool mode_concurrent = handle_state ? handle_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT : false;
        if (!mode_concurrent) {
            const auto typed_handle = BarrierTypedHandle(barrier);
            core_error::LocationCapture loc_capture(loc);
            cb_state->queue_submit_functions.emplace_back(
                [loc_capture, typed_handle, src_queue_family, dst_queue_family](
                    const ValidationStateTracker &device_data, const QUEUE_STATE &queue_state, const CMD_BUFFER_STATE &cb_state) {
                    return ValidateConcurrentBarrierAtSubmit(loc_capture.Get(), device_data, queue_state, cb_state, typed_handle,
                                                             src_queue_family, dst_queue_family);
                });
        }
    }
}

void CoreChecks::RecordBarriers(Func func_name, CMD_BUFFER_STATE *cb_state, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers) {
    for (uint32_t i = 0; i < bufferBarrierCount; i++) {
        Location loc(func_name, Struct::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        RecordBarrierValidationInfo(loc, cb_state, pBufferMemBarriers[i], cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; i++) {
        Location loc(func_name, Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const auto &img_barrier = pImageMemBarriers[i];
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

void CoreChecks::RecordBarriers(Func func_name, CMD_BUFFER_STATE *cb_state, const VkDependencyInfoKHR &dep_info) {
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        Location loc(func_name, Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        RecordBarrierValidationInfo(loc, cb_state, dep_info.pBufferMemoryBarriers[i], cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        Location loc(func_name, Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        const auto &img_barrier = dep_info.pImageMemoryBarriers[i];
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

template <typename TransferBarrier, typename Scoreboard>
bool CoreChecks::ValidateAndUpdateQFOScoreboard(const debug_report_data *report_data, const CMD_BUFFER_STATE &cb_state,
                                                const char *operation, const TransferBarrier &barrier,
                                                Scoreboard *scoreboard) const {
    // Record to the scoreboard or report that we have a duplication
    bool skip = false;
    auto inserted = scoreboard->emplace(barrier, &cb_state);
    if (!inserted.second && inserted.first->second != &cb_state) {
        // This is a duplication (but don't report duplicates from the same CB, as we do that at record time
        const LogObjectList objlist(cb_state.commandBuffer(), barrier.handle, inserted.first->second->commandBuffer());
        skip = LogWarning(objlist, TransferBarrier::ErrMsgDuplicateQFOInSubmit(),
                          "%s: %s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                          " to dstQueueFamilyIndex %" PRIu32 " duplicates existing barrier submitted in this batch from %s.",
                          "vkQueueSubmit()", TransferBarrier::BarrierName(), operation, TransferBarrier::HandleName(),
                          report_data->FormatHandle(barrier.handle).c_str(), barrier.srcQueueFamilyIndex,
                          barrier.dstQueueFamilyIndex, report_data->FormatHandle(inserted.first->second->commandBuffer()).c_str());
    }
    return skip;
}

template <typename TransferBarrier>
bool CoreChecks::ValidateQueuedQFOTransferBarriers(
    const CMD_BUFFER_STATE &cb_state, QFOTransferCBScoreboards<TransferBarrier> *scoreboards,
    const GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers) const {
    bool skip = false;
    const auto &cb_barriers = cb_state.GetQFOBarrierSets(TransferBarrier());
    const char *barrier_name = TransferBarrier::BarrierName();
    const char *handle_name = TransferBarrier::HandleName();
    // No release should have an extant duplicate (WARNING)
    for (const auto &release : cb_barriers.release) {
        // Check the global pending release barriers
        const auto set_it = global_release_barriers.find(release.handle);
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            const auto found = set_for_handle.find(release);
            if (found != set_for_handle.cend()) {
                skip |= LogWarning(cb_state.commandBuffer(), TransferBarrier::ErrMsgDuplicateQFOSubmitted(),
                                   "%s: %s releasing queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                                   " to dstQueueFamilyIndex %" PRIu32
                                   " duplicates existing barrier queued for execution, without intervening acquire operation.",
                                   "vkQueueSubmit()", barrier_name, handle_name, report_data->FormatHandle(found->handle).c_str(),
                                   found->srcQueueFamilyIndex, found->dstQueueFamilyIndex);
            }
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "releasing", release, &scoreboards->release);
    }
    // Each acquire must have a matching release (ERROR)
    for (const auto &acquire : cb_barriers.acquire) {
        const auto set_it = global_release_barriers.find(acquire.handle);
        bool matching_release_found = false;
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            matching_release_found = set_for_handle.find(acquire) != set_for_handle.cend();
        }
        if (!matching_release_found) {
            skip |= LogError(cb_state.commandBuffer(), TransferBarrier::ErrMsgMissingQFOReleaseInSubmit(),
                             "%s: in submitted command buffer %s acquiring ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                             " to dstQueueFamilyIndex %" PRIu32 " has no matching release barrier queued for execution.",
                             "vkQueueSubmit()", barrier_name, handle_name, report_data->FormatHandle(acquire.handle).c_str(),
                             acquire.srcQueueFamilyIndex, acquire.dstQueueFamilyIndex);
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "acquiring", acquire, &scoreboards->acquire);
    }
    return skip;
}

bool CoreChecks::ValidateQueuedQFOTransfers(const CMD_BUFFER_STATE &cb_state,
                                            QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
                                            QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards) const {
    bool skip = false;
    skip |=
        ValidateQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state, qfo_image_scoreboards, qfo_release_image_barrier_map);
    skip |= ValidateQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state, qfo_buffer_scoreboards,
                                                                        qfo_release_buffer_barrier_map);
    return skip;
}

template <typename TransferBarrier>
void RecordQueuedQFOTransferBarriers(QFOTransferBarrierSets<TransferBarrier> &cb_barriers,
                                     GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers) {
    // Add release barriers from this submit to the global map
    for (const auto &release : cb_barriers.release) {
        // the global barrier list is mapped by resource handle to allow cleanup on resource destruction
        // NOTE: vl_concurrent_ordered_map::find() makes a thread safe copy of the result, so we must
        // copy back after updating.
        auto iter = global_release_barriers.find(release.handle);
        iter->second.insert(release);
        global_release_barriers.insert_or_assign(release.handle, iter->second);
    }

    // Erase acquired barriers from this submit from the global map -- essentially marking releases as consumed
    for (const auto &acquire : cb_barriers.acquire) {
        // NOTE: We're not using [] because we don't want to create entries for missing releases
        auto set_it = global_release_barriers.find(acquire.handle);
        if (set_it != global_release_barriers.end()) {
            QFOTransferBarrierSet<TransferBarrier> &set_for_handle = set_it->second;
            set_for_handle.erase(acquire);
            if (set_for_handle.size() == 0) {  // Clean up empty sets
                global_release_barriers.erase(acquire.handle);
            } else {
                // NOTE: vl_concurrent_ordered_map::find() makes a thread safe copy of the result, so we must
                // copy back after updating.
                global_release_barriers.insert_or_assign(acquire.handle, set_for_handle);
            }
        }
    }
}

void CoreChecks::RecordQueuedQFOTransfers(CMD_BUFFER_STATE *cb_state) {
    RecordQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state->qfo_transfer_image_barriers, qfo_release_image_barrier_map);
    RecordQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state->qfo_transfer_buffer_barriers,
                                                              qfo_release_buffer_barrier_map);
}

template <typename Barrier, typename TransferBarrier>
bool CoreChecks::ValidateQFOTransferBarrierUniqueness(const Location &loc, const CMD_BUFFER_STATE *cb_state, const Barrier &barrier,
                                                      const QFOTransferBarrierSets<TransferBarrier> &barrier_sets) const {
    bool skip = false;
    const char *handle_name = TransferBarrier::HandleName();
    const char *transfer_type = nullptr;
    if (!IsTransferOp(barrier)) {
        return skip;
    }
    const TransferBarrier *barrier_record = nullptr;
    if (cb_state->IsReleaseOp(barrier) && !QueueFamilyIsExternal(barrier.dstQueueFamilyIndex)) {
        const auto found = barrier_sets.release.find(barrier);
        if (found != barrier_sets.release.cend()) {
            barrier_record = &(*found);
            transfer_type = "releasing";
        }
    } else if (cb_state->IsAcquireOp(barrier) && !QueueFamilyIsExternal(barrier.srcQueueFamilyIndex)) {
        const auto found = barrier_sets.acquire.find(barrier);
        if (found != barrier_sets.acquire.cend()) {
            barrier_record = &(*found);
            transfer_type = "acquiring";
        }
    }
    if (barrier_record != nullptr) {
        skip |=
            LogWarning(cb_state->commandBuffer(), TransferBarrier::ErrMsgDuplicateQFOInCB(),
                       "%s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                       " duplicates existing barrier recorded in this command buffer.",
                       loc.Message().c_str(), transfer_type, handle_name, report_data->FormatHandle(barrier_record->handle).c_str(),
                       barrier_record->srcQueueFamilyIndex, barrier_record->dstQueueFamilyIndex);
    }
    return skip;
}

namespace barrier_queue_families {
using sync_vuid_maps::GetBarrierQueueVUID;
using sync_vuid_maps::kQueueErrorSummary;
using sync_vuid_maps::QueueError;

class ValidatorState {
  public:
    ValidatorState(const ValidationStateTracker *device_data, LogObjectList &&obj, const core_error::Location &location,
                   const VulkanTypedHandle &barrier_handle, const VkSharingMode sharing_mode)
        : device_data_(device_data),
          objects_(std::move(obj)),
          loc_(location),
          barrier_handle_(barrier_handle),
          sharing_mode_(sharing_mode),
          limit_(static_cast<uint32_t>(device_data->physical_device_state->queue_family_properties.size())),
          mem_ext_(IsExtEnabled(device_data->device_extensions.vk_khr_external_memory)) {}

    // Log the messages using boilerplate from object state, and Vu specific information from the template arg
    // One and two family versions, in the single family version, Vu holds the name of the passed parameter
    bool LogMsg(QueueError vu_index, uint32_t family, const char *param_name) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *annotation = GetFamilyAnnotation(family);
        return device_data_->LogError(
            objects_, val_code, "%s Barrier using %s %s created with sharingMode %s, has %s %" PRIu32 "%s. %s",
            loc_.Message().c_str(), GetTypeString(), device_data_->report_data->FormatHandle(barrier_handle_).c_str(),
            GetModeString(), param_name, family, annotation, kQueueErrorSummary.at(vu_index).c_str());
    }

    bool LogMsg(QueueError vu_index, uint32_t src_family, uint32_t dst_family) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *src_annotation = GetFamilyAnnotation(src_family);
        const char *dst_annotation = GetFamilyAnnotation(dst_family);
        return device_data_->LogError(objects_, val_code,
                                      "%s Barrier using %s %s created with sharingMode %s, has srcQueueFamilyIndex %" PRIu32
                                      "%s and dstQueueFamilyIndex %" PRIu32 "%s. %s",
                                      loc_.Message().c_str(), GetTypeString(),
                                      device_data_->report_data->FormatHandle(barrier_handle_).c_str(), GetModeString(), src_family,
                                      src_annotation, dst_family, dst_annotation, kQueueErrorSummary.at(vu_index).c_str());
    }

    // This abstract Vu can only be tested at submit time, thus we need a callback from the closure containing the needed
    // data. Note that the mem_barrier is copied to the closure as the lambda lifespan exceed the guarantees of validity for
    // application input.
    static bool ValidateAtQueueSubmit(const QUEUE_STATE *queue_state, const ValidationStateTracker *device_data,
                                      uint32_t src_family, uint32_t dst_family, const ValidatorState &val) {
        auto error_code = QueueError::kSubmitQueueMustMatchSrcOrDst;
        uint32_t queue_family = queue_state->queueFamilyIndex;
        if ((src_family != queue_family) && (dst_family != queue_family)) {
            const std::string val_code = GetBarrierQueueVUID(val.loc_, error_code);
            const char *src_annotation = val.GetFamilyAnnotation(src_family);
            const char *dst_annotation = val.GetFamilyAnnotation(dst_family);
            return device_data->LogError(queue_state->Handle(), val_code,
                                         "%s Barrier submitted to queue with family index %" PRIu32
                                         ", using %s %s created with sharingMode %s, has "
                                         "srcQueueFamilyIndex %" PRIu32 "%s and dstQueueFamilyIndex %" PRIu32 "%s. %s",
                                         val.loc_.Message().c_str(), queue_family, val.GetTypeString(),
                                         device_data->report_data->FormatHandle(val.barrier_handle_).c_str(), val.GetModeString(),
                                         src_family, src_annotation, dst_family, dst_annotation,
                                         kQueueErrorSummary.at(error_code).c_str());
        }
        return false;
    }
    // Logical helpers for semantic clarity
    inline bool KhrExternalMem() const { return mem_ext_; }
    inline bool IsValid(uint32_t queue_family) const { return (queue_family < limit_); }
    inline bool IsValidOrSpecial(uint32_t queue_family) const {
        return IsValid(queue_family) || (mem_ext_ && QueueFamilyIsExternal(queue_family)) ||
               (queue_family == VK_QUEUE_FAMILY_IGNORED);
    }

    // Helpers for LogMsg
    const char *GetModeString() const { return string_VkSharingMode(sharing_mode_); }

    // Descriptive text for the various types of queue family index
    const char *GetFamilyAnnotation(uint32_t family) const {
        const char *external = " (VK_QUEUE_FAMILY_EXTERNAL)";
        const char *foreign = " (VK_QUEUE_FAMILY_FOREIGN_EXT)";
        const char *ignored = " (VK_QUEUE_FAMILY_IGNORED)";
        const char *valid = " (VALID)";
        const char *invalid = " (INVALID)";
        switch (family) {
            case VK_QUEUE_FAMILY_EXTERNAL:
                return external;
            case VK_QUEUE_FAMILY_FOREIGN_EXT:
                return foreign;
            case VK_QUEUE_FAMILY_IGNORED:
                return ignored;
            default:
                if (IsValid(family)) {
                    return valid;
                }
                return invalid;
        };
    }
    const char *GetTypeString() const { return object_string[barrier_handle_.type]; }
    VkSharingMode GetSharingMode() const { return sharing_mode_; }

  protected:
    const ValidationStateTracker *device_data_;
    const LogObjectList objects_;
    const core_error::Location loc_;
    const VulkanTypedHandle barrier_handle_;
    const VkSharingMode sharing_mode_;
    const uint32_t limit_;
    const bool mem_ext_;
};

bool Validate(const CoreChecks *device_data, const CMD_BUFFER_STATE *cb_state, const ValidatorState &val,
              const uint32_t src_queue_family, const uint32_t dst_queue_family) {
    bool skip = false;

    const bool mode_concurrent = val.GetSharingMode() == VK_SHARING_MODE_CONCURRENT;
    const bool src_ignored = QueueFamilyIsIgnored(src_queue_family);
    const bool dst_ignored = QueueFamilyIsIgnored(dst_queue_family);
    if (val.KhrExternalMem()) {
        if (mode_concurrent) {
            const bool sync2 = device_data->enabled_features.core13.synchronization2 != 0;
            // this requirement is removed by VK_KHR_synchronization2
            if (!(src_ignored || dst_ignored) && !sync2) {
                skip |= val.LogMsg(QueueError::kSrcOrDstMustBeIgnore, src_queue_family, dst_queue_family);
            }
            if ((src_ignored && !(dst_ignored || QueueFamilyIsExternal(dst_queue_family))) ||
                (dst_ignored && !(src_ignored || QueueFamilyIsExternal(src_queue_family)))) {
                skip |= val.LogMsg(QueueError::kSpecialOrIgnoreOnly, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if (src_queue_family != dst_queue_family) {
                if (!val.IsValidOrSpecial(dst_queue_family)) {
                    skip |= val.LogMsg(QueueError::kSrcAndDstValidOrSpecial, dst_queue_family, "dstQueueFamilyIndex");
                }
                if (!val.IsValidOrSpecial(src_queue_family)) {
                    skip |= val.LogMsg(QueueError::kSrcAndDstValidOrSpecial, src_queue_family, "srcQueueFamilyIndex");
                }
            }
        }
    } else {
        // No memory extension
        if (mode_concurrent) {
            const bool sync2 = device_data->enabled_features.core13.synchronization2 != 0;
            // this requirement is removed by VK_KHR_synchronization2
            if ((!src_ignored || !dst_ignored) && !sync2) {
                skip |= val.LogMsg(QueueError::kSrcAndDestMustBeIgnore, src_queue_family, dst_queue_family);
            }
        } else {
            // VK_SHARING_MODE_EXCLUSIVE
            if ((src_queue_family != dst_queue_family) && !(val.IsValid(src_queue_family) && val.IsValid(dst_queue_family))) {
                skip |= val.LogMsg(QueueError::kSrcAndDstBothValid, src_queue_family, dst_queue_family);
            }
        }
    }
    return skip;
}
}  // namespace barrier_queue_families

bool CoreChecks::ValidateConcurrentBarrierAtSubmit(const Location &loc, const ValidationStateTracker &state_data,
                                                   const QUEUE_STATE &queue_state, const CMD_BUFFER_STATE &cb_state,
                                                   const VulkanTypedHandle &typed_handle, uint32_t src_queue_family,
                                                   uint32_t dst_queue_family) {
    using barrier_queue_families::ValidatorState;
    ValidatorState val(&state_data, LogObjectList(cb_state.Handle()), loc, typed_handle, VK_SHARING_MODE_CONCURRENT);
    return ValidatorState::ValidateAtQueueSubmit(&queue_state, &state_data, src_queue_family, dst_queue_family, val);
}

// Type specific wrapper for image barriers
template <typename ImgBarrier>
bool CoreChecks::ValidateBarrierQueueFamilies(const Location &loc, const CMD_BUFFER_STATE *cb_state, const ImgBarrier &barrier,
                                              const IMAGE_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the image state
    barrier_queue_families::ValidatorState val(this, LogObjectList(cb_state->commandBuffer()), loc, state_data->Handle(),
                                               state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, cb_state, val, src_queue_family, dst_queue_family);
}

// Type specific wrapper for buffer barriers
template <typename BufBarrier>
bool CoreChecks::ValidateBarrierQueueFamilies(const Location &loc, const CMD_BUFFER_STATE *cb_state, const BufBarrier &barrier,
                                              const BUFFER_STATE *state_data) const {
    // State data is required
    if (!state_data) {
        return false;
    }

    // Create the validator state from the buffer state
    barrier_queue_families::ValidatorState val(this, LogObjectList(cb_state->commandBuffer()), loc, state_data->Handle(),
                                               state_data->createInfo.sharingMode);
    const uint32_t src_queue_family = barrier.srcQueueFamilyIndex;
    const uint32_t dst_queue_family = barrier.dstQueueFamilyIndex;
    return barrier_queue_families::Validate(this, cb_state, val, src_queue_family, dst_queue_family);
}

template <typename Barrier>
bool CoreChecks::ValidateBufferBarrier(const LogObjectList &objects, const Location &loc, const CMD_BUFFER_STATE *cb_state,
                                       const Barrier &mem_barrier) const {
    using sync_vuid_maps::BufferError;
    using sync_vuid_maps::GetBufferBarrierVUID;

    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(loc, cb_state, mem_barrier, cb_state->qfo_transfer_buffer_barriers);

    // Validate buffer barrier queue family indices
    auto buffer_state = Get<BUFFER_STATE>(mem_barrier.buffer);
    if (buffer_state) {
        auto buf_loc = loc.dot(Field::buffer);
        const auto &mem_vuid = GetBufferBarrierVUID(buf_loc, BufferError::kNoMemory);
        skip |= ValidateMemoryIsBoundToBuffer(cb_state->commandBuffer(), *buffer_state, loc.StringFunc().c_str(), mem_vuid.c_str());

        skip |= ValidateBarrierQueueFamilies(buf_loc, cb_state, mem_barrier, buffer_state.get());

        auto buffer_size = buffer_state->createInfo.size;
        if (mem_barrier.offset >= buffer_size) {
            auto offset_loc = loc.dot(Field::offset);
            const auto &vuid = GetBufferBarrierVUID(offset_loc, BufferError::kOffsetTooBig);

            skip |= LogError(objects, vuid, "%s %s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                             offset_loc.Message().c_str(), report_data->FormatHandle(mem_barrier.buffer).c_str(),
                             HandleToUint64(mem_barrier.offset), HandleToUint64(buffer_size));
        } else if (mem_barrier.size != VK_WHOLE_SIZE && (mem_barrier.offset + mem_barrier.size > buffer_size)) {
            auto size_loc = loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeOutOfRange);
            skip |= LogError(objects, vuid,
                             "%s %s has offset 0x%" PRIx64 " and size 0x%" PRIx64 " whose sum is greater than total size 0x%" PRIx64
                             ".",
                             size_loc.Message().c_str(), report_data->FormatHandle(mem_barrier.buffer).c_str(),
                             HandleToUint64(mem_barrier.offset), HandleToUint64(mem_barrier.size), HandleToUint64(buffer_size));
        }
        if (mem_barrier.size == 0) {
            auto size_loc = loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeZero);
            skip |= LogError(objects, vuid, "%s %s has a size of 0.", loc.Message().c_str(),
                             report_data->FormatHandle(mem_barrier.buffer).c_str());
        }
    }

    if (mem_barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL &&
        mem_barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL) {
        auto size_loc = loc.dot(Field::srcQueueFamilyIndex);
        const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kQueueFamilyExternal);
        skip |= LogError(objects, vuid, "Both srcQueueFamilyIndex and dstQueueFamilyIndex are VK_QUEUE_FAMILY_EXTERNAL.");
    }
    return skip;
}

template <typename Barrier>
bool CoreChecks::ValidateImageBarrier(const LogObjectList &objects, const Location &loc, const CMD_BUFFER_STATE *cb_state,
                                      const Barrier &mem_barrier) const {
    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(loc, cb_state, mem_barrier, cb_state->qfo_transfer_image_barriers);

    bool is_ilt = true;
    if (enabled_features.core13.synchronization2) {
        is_ilt = mem_barrier.oldLayout != mem_barrier.newLayout;
    }

    if (is_ilt) {
        if (mem_barrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED || mem_barrier.newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            auto layout_loc = loc.dot(Field::newLayout);
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(loc, sync_vuid_maps::ImageError::kBadLayout);
            skip |=
                LogError(cb_state->commandBuffer(), vuid, "%s Image Layout cannot be transitioned to UNDEFINED or PREINITIALIZED.",
                         layout_loc.Message().c_str());
        }
    }

    if (mem_barrier.newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
        if (!enabled_features.attachment_feedback_loop_layout_features.attachmentFeedbackLoopLayout) {
            auto layout_loc = loc.dot(Field::newLayout);
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(loc, sync_vuid_maps::ImageError::kBadAttFeedbackLoopLayout);
            skip |= LogError(cb_state->commandBuffer(), vuid,
                             "%s Image Layout cannot be transitioned to VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT if "
                             "the attachmentFeedbackLoopLayout feature is not enabled",
                             layout_loc.Message().c_str());
        }
    }

    auto image_data = Get<IMAGE_STATE>(mem_barrier.image);
    if (image_data) {
        auto image_loc = loc.dot(Field::image);

        skip |= ValidateMemoryIsBoundToImage(cb_state->commandBuffer(), *image_data, loc);

        skip |= ValidateBarrierQueueFamilies(image_loc, cb_state, mem_barrier, image_data.get());

        skip |= ValidateImageAspectMask(image_data->image(), image_data->createInfo.format, mem_barrier.subresourceRange.aspectMask,
                                        image_data->disjoint, loc.StringFunc().c_str());

        skip |=
            ValidateImageBarrierSubresourceRange(loc.dot(Field::subresourceRange), image_data.get(), mem_barrier.subresourceRange);
    }
    return skip;
}

bool CoreChecks::ValidateBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state, VkPipelineStageFlags src_stage_mask,
                                  VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount,
                                  const VkMemoryBarrier *pMemBarriers, uint32_t bufferBarrierCount,
                                  const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                  const VkImageMemoryBarrier *pImageMemBarriers) const {
    bool skip = false;
    LogObjectList objects(cb_state->commandBuffer());

    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        const auto &mem_barrier = pMemBarriers[i];
        auto loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier, src_stage_mask, dst_stage_mask);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
        const auto &mem_barrier = pImageMemBarriers[i];
        auto loc = outer_loc.dot(Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier, src_stage_mask, dst_stage_mask);
        skip |= ValidateImageBarrier(objects, loc, cb_state, mem_barrier);
    }
    {
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier);
        skip |= ValidateBarriersToImages(loc, cb_state, imageMemBarrierCount, pImageMemBarriers);
    }
    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        const auto &mem_barrier = pBufferMemBarriers[i];
        auto loc = outer_loc.dot(Struct::VkBufferMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier, src_stage_mask, dst_stage_mask);
        skip |= ValidateBufferBarrier(objects, loc, cb_state, mem_barrier);
    }
    return skip;
}

bool CoreChecks::ValidateDependencyInfo(const LogObjectList &objects, const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                        const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;

    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers(outer_loc, cb_state, dep_info);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    }
    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pMemoryBarriers[i];
        auto loc = outer_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier);
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pImageMemoryBarriers[i];
        auto loc = outer_loc.dot(Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier);
        skip |= ValidateImageBarrier(objects, loc, cb_state, mem_barrier);
    }
    {
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier2);
        skip |= ValidateBarriersToImages(loc, cb_state, dep_info->imageMemoryBarrierCount, dep_info->pImageMemoryBarriers);
    }

    for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pBufferMemoryBarriers[i];
        auto loc = outer_loc.dot(Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        skip |= ValidateMemoryBarrier(objects, loc, cb_state, mem_barrier);
        skip |= ValidateBufferBarrier(objects, loc, cb_state, mem_barrier);
    }

    return skip;
}

// template to check all original barrier structures
template <typename Barrier>
bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const Location &loc, const CMD_BUFFER_STATE *cb_state,
                                       const Barrier &barrier, VkPipelineStageFlags src_stage_mask,
                                       VkPipelineStageFlags dst_stage_mask) const {
    bool skip = false;
    assert(cb_state);
    auto queue_flags = cb_state->GetQueueFlags();

    if (!cb_state->IsAcquireOp(barrier)) {
        skip |= ValidateAccessMask(objects, loc.dot(Field::srcAccessMask), queue_flags, barrier.srcAccessMask, src_stage_mask);
    }
    if (!cb_state->IsReleaseOp(barrier)) {
        skip |= ValidateAccessMask(objects, loc.dot(Field::dstAccessMask), queue_flags, barrier.dstAccessMask, dst_stage_mask);
    }
    return skip;
}

// template to check all synchronization2 barrier structures
template <typename Barrier>
bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const Location &loc, const CMD_BUFFER_STATE *cb_state,
                                       const Barrier &barrier) const {
    bool skip = false;
    assert(cb_state);
    auto queue_flags = cb_state->GetQueueFlags();

    skip |= ValidatePipelineStage(objects, loc.dot(Field::srcStageMask), queue_flags, barrier.srcStageMask);
    if (!cb_state->IsAcquireOp(barrier)) {
        skip |=
            ValidateAccessMask(objects, loc.dot(Field::srcAccessMask), queue_flags, barrier.srcAccessMask, barrier.srcStageMask);
    }

    skip |= ValidatePipelineStage(objects, loc.dot(Field::dstStageMask), queue_flags, barrier.dstStageMask);
    if (!cb_state->IsReleaseOp(barrier)) {
        skip |=
            ValidateAccessMask(objects, loc.dot(Field::dstAccessMask), queue_flags, barrier.dstAccessMask, barrier.dstStageMask);
    }
    return skip;
}
