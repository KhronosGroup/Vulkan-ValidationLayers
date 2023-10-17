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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_checks/core_validation.h"
#include "sync/sync_utils.h"
#include "generated/enum_flag_bits.h"

using sync_utils::BufferBarrier;
using sync_utils::ImageBarrier;
using sync_utils::MemoryBarrier;
using sync_utils::QueueFamilyBarrier;

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

bool CoreChecks::ValidateStageMaskHost(const Location &stage_mask_loc, VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;
    if ((stageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(stage_mask_loc, sync_vuid_maps::SubmitError::kHostStageMask);
        skip |= LogError(vuid, device, stage_mask_loc,
                         "must not include VK_PIPELINE_STAGE_HOST_BIT as the stage can't be invoked inside a command buffer.");
    }
    return skip;
}

bool CoreChecks::ValidateFenceForSubmit(const FENCE_STATE *fence_state, const char *inflight_vuid, const char *retired_vuid,
                                        const LogObjectList &objlist, const Location &loc) const {
    bool skip = false;

    if (fence_state && fence_state->Scope() == kSyncScopeInternal) {
        switch (fence_state->State()) {
            case FENCE_INFLIGHT:
                skip |= LogError(inflight_vuid, objlist, loc, "(%s) is already in use by another submission.",
                                 FormatHandle(fence_state->fence()).c_str());
                break;
            case FENCE_RETIRED:
                skip |= LogError(retired_vuid, objlist, loc,
                                 "(%s) submitted in SIGNALED state. Fences must be reset before being submitted",
                                 FormatHandle(fence_state->fence()).c_str());
                break;
            default:
                break;
        }
    }

    return skip;
}

bool SemaphoreSubmitState::ValidateBinaryWait(const Location &loc, VkQueue queue, const SEMAPHORE_STATE &semaphore_state) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    auto semaphore = semaphore_state.semaphore();
    if ((semaphore_state.Scope() == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
        VkQueue other_queue = AnotherQueueWaits(semaphore_state);
        if (other_queue) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kOtherQueueWaiting);
            const LogObjectList objlist(semaphore, queue, other_queue);
            skip |= core->LogError(vuid, objlist, loc, "queue (%s) is already waiting on semaphore (%s).",
                                   core->FormatHandle(other_queue).c_str(), core->FormatHandle(semaphore).c_str());
        } else if (CannotWait(semaphore_state)) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kBinaryCannotBeSignalled);
            const LogObjectList objlist(semaphore, queue);
            skip |= core->LogError(semaphore_state.Scope() == kSyncScopeInternal ? vuid : kVUID_Core_DrawState_QueueForwardProgress,
                                   objlist, loc, "queue (%s) is waiting on semaphore (%s) that has no way to be signaled.",
                                   core->FormatHandle(queue).c_str(), core->FormatHandle(semaphore).c_str());
        } else {
            signaled_semaphores.erase(semaphore);
            unsignaled_semaphores.insert(semaphore);
        }
    } else if (semaphore_state.Scope() == kSyncScopeExternalTemporary) {
        internal_semaphores.insert(semaphore);
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateWaitSemaphore(const Location &wait_semaphore_loc, VkSemaphore semaphore, uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;

    auto semaphore_state = core->Get<SEMAPHORE_STATE>(semaphore);
    if (!semaphore_state) {
        return skip;
    }
    switch (semaphore_state->type) {
        case VK_SEMAPHORE_TYPE_BINARY:
            skip = ValidateBinaryWait(wait_semaphore_loc, queue, *semaphore_state);
            break;
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(wait_semaphore_loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(vuid, semaphore, wait_semaphore_loc,
                                       "value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").", value,
                                       where.c_str(), core->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            timeline_waits[semaphore] = value;
        } break;
        default:
            break;
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateSignalSemaphore(const Location &signal_semaphore_loc, VkSemaphore semaphore, uint64_t value) {
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
                vvl::Func other_command = vvl::Func::Empty;
                if (CannotSignal(*semaphore_state, other_queue, other_command)) {
                    std::stringstream initiator;
                    if (other_command != vvl::Func::Empty) {
                        initiator << String(other_command);
                    }
                    if (other_queue != VK_NULL_HANDLE) {
                        if (other_command != vvl::Func::Empty) {
                            initiator << " on ";
                        }
                        initiator << core->FormatHandle(other_queue);
                        objlist.add(other_queue);
                    }
                    skip |= core->LogError(kVUID_Core_DrawState_QueueForwardProgress, objlist, signal_semaphore_loc,
                                           "is signaling %s (%s) that was previously "
                                           "signaled by %s but has not since been waited on by any queue.",
                                           core->FormatHandle(queue).c_str(), core->FormatHandle(semaphore).c_str(),
                                           initiator.str().c_str());
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
                const auto &vuid = GetQueueSubmitVUID(signal_semaphore_loc, SubmitError::kTimelineSemSmallValue);
                skip |= core->LogError(
                    vuid, objlist, signal_semaphore_loc,
                    "signal value (0x%" PRIx64 ") in %s must be greater than %s timeline semaphore %s value (0x%" PRIx64 ")", value,
                    core->FormatHandle(queue).c_str(), where.c_str(), core->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(signal_semaphore_loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(vuid, semaphore, signal_semaphore_loc,
                                       "value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").", value,
                                       where.c_str(), core->FormatHandle(semaphore).c_str(), bad_value);
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
                                             const Location &submit_loc) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (const auto d3d12_fence_submit_info = vku::FindStructInPNextChain<VkD3D12FenceSubmitInfoKHR>(submit.pNext)) {
        if (d3d12_fence_submit_info->waitSemaphoreValuesCount != submit.waitSemaphoreCount) {
            skip |= LogError("VUID-VkD3D12FenceSubmitInfoKHR-waitSemaphoreValuesCount-00079", state.queue, submit_loc,
                             "contains an instance of VkD3D12FenceSubmitInfoKHR, but its waitSemaphoreValuesCount (%" PRIu32
                             ") is different than %s (%" PRIu32 ").",
                             d3d12_fence_submit_info->waitSemaphoreValuesCount,
                             submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
        }
        if (d3d12_fence_submit_info->signalSemaphoreValuesCount != submit.signalSemaphoreCount) {
            skip |= LogError("VUID-VkD3D12FenceSubmitInfoKHR-signalSemaphoreValuesCount-00080", state.queue, submit_loc,
                             "contains an instance of VkD3D12FenceSubmitInfoKHR, but its signalSemaphoreValuesCount (%" PRIu32
                             ") is different than %s (%" PRIu32 ").",
                             d3d12_fence_submit_info->signalSemaphoreValuesCount,
                             submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
        }
    }
#endif
    auto *timeline_semaphore_submit_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        if (submit.pWaitDstStageMask) {
            const LogObjectList objlist(semaphore, state.queue);
            auto stage_mask_loc = submit_loc.dot(Field::pWaitDstStageMask, i);
            skip |= ValidatePipelineStage(objlist, stage_mask_loc, state.queue_flags, submit.pWaitDstStageMask[i]);
            skip |= ValidateStageMaskHost(stage_mask_loc, submit.pWaitDstStageMask[i]);
        }
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto wait_semaphore_loc = submit_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkSubmitInfo-pWaitSemaphores-03239", semaphore, wait_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo.",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkSubmitInfo-pNext-03240", semaphore, wait_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::waitSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->waitSemaphoreValueCount,
                    submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(wait_semaphore_loc, semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto signal_semaphore_loc = submit_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkSubmitInfo-pWaitSemaphores-03239", semaphore, signal_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkSubmitInfo-pNext-03241", semaphore, signal_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::signalSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->signalSemaphoreValueCount,
                    submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(signal_semaphore_loc, semaphore, value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo2KHR &submit,
                                             const Location &submit_loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < submit.waitSemaphoreInfoCount; ++i) {
        const auto &wait_info = submit.pWaitSemaphoreInfos[i];
        Location wait_info_loc = submit_loc.dot(Field::pWaitSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(wait_info.semaphore), wait_info_loc.dot(Field::stageMask), state.queue_flags,
                                      wait_info.stageMask);
        skip |= ValidateStageMaskHost(wait_info_loc.dot(Field::stageMask), wait_info.stageMask);
        skip |= state.ValidateWaitSemaphore(wait_info_loc.dot(Field::semaphore), wait_info.semaphore, wait_info.value);

        auto semaphore_state = Get<SEMAPHORE_STATE>(wait_info.semaphore);
        if (semaphore_state && semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            for (uint32_t sig_index = 0; sig_index < submit.signalSemaphoreInfoCount; sig_index++) {
                const auto &sig_info = submit.pSignalSemaphoreInfos[sig_index];
                if (wait_info.semaphore == sig_info.semaphore && wait_info.value >= sig_info.value) {
                    Location sig_loc = submit_loc.dot(Field::pSignalSemaphoreInfos, sig_index);
                    const LogObjectList objlist(wait_info.semaphore, state.queue);
                    skip |= LogError("VUID-VkSubmitInfo2-semaphore-03881", objlist, wait_info_loc.dot(Field::value),
                                     "(%" PRIu64 ") is less or equal to %s (%" PRIu64 ").", wait_info.value,
                                     sig_loc.dot(Field::value).Fields().c_str(), sig_info.value);
                }
            }
        }
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreInfoCount; ++i) {
        const auto &sem_info = submit.pSignalSemaphoreInfos[i];
        auto signal_info_loc = submit_loc.dot(Field::pSignalSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(sem_info.semaphore), signal_info_loc.dot(Field::stageMask), state.queue_flags,
                                      sem_info.stageMask);
        skip |= ValidateStageMaskHost(signal_info_loc.dot(Field::stageMask), sem_info.stageMask);
        skip |= state.ValidateSignalSemaphore(signal_info_loc.dot(Field::semaphore), sem_info.semaphore, sem_info.value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkBindSparseInfo &submit,
                                             const Location &submit_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        const LogObjectList objlist(semaphore, state.queue);
        // NOTE: there are no stage masks in bind sparse submissions
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto wait_semaphore_loc = submit_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkBindSparseInfo-pWaitSemaphores-03246", semaphore, wait_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkBindSparseInfo-pNext-03247", semaphore, wait_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::waitSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->waitSemaphoreValueCount,
                    submit_loc.dot(Field::waitSemaphoreCount).Fields().c_str(), submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(wait_semaphore_loc, semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto signal_semaphore_loc = submit_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError("VUID-VkBindSparseInfo-pWaitSemaphores-03246", semaphore, signal_semaphore_loc,
                                 "(%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(
                    "VUID-VkBindSparseInfo-pNext-03248", semaphore, signal_semaphore_loc,
                    "(%s) is a timeline semaphore, %s (%" PRIu32
                    ") is different than "
                    "%s (%" PRIu32 ").",
                    FormatHandle(semaphore).c_str(),
                    submit_loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::signalSemaphoreValueCount).Fields().c_str(),
                    timeline_semaphore_submit_info->signalSemaphoreValueCount,
                    submit_loc.dot(Field::signalSemaphoreCount).Fields().c_str(), submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(signal_semaphore_loc, semaphore, value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkFence *pFence,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    auto fence_export_info = vku::FindStructInPNextChain<VkExportFenceCreateInfo>(pCreateInfo->pNext);
    if (fence_export_info && fence_export_info->handleTypes != 0) {
        VkExternalFenceProperties external_properties = vku::InitStructHelper();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalFenceHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalFenceInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalFenceProperties(physical_device, &external_info, &external_properties);
            if ((external_properties.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError("VUID-VkExportFenceCreateInfo-handleTypes-01446", device,
                                 create_info_loc.pNext(Struct::VkExportFenceCreateInfo, Field::handleTypes),
                                 "(%s) does not support VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalFenceHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalFenceHandleTypeFlagBits>(fence_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (fence_export_info->handleTypes & external_properties.compatibleHandleTypes) != fence_export_info->handleTypes) {
            skip |= LogError("VUID-VkExportFenceCreateInfo-handleTypes-01446", device,
                             create_info_loc.pNext(Struct::VkExportFenceCreateInfo, Field::handleTypes),
                             "(%s) are not reported as compatible by vkGetPhysicalDeviceExternalFenceProperties (%s).",
                             string_VkExternalFenceHandleTypeFlags(fence_export_info->handleTypes).c_str(),
                             string_VkExternalFenceHandleTypeFlags(external_properties.compatibleHandleTypes).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    auto sem_type_create_info = vku::FindStructInPNextChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext);
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    if (sem_type_create_info) {
        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE && !enabled_features.timelineSemaphore) {
            skip |= LogError("VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252", device,
                             create_info_loc.dot(Field::semaphoreType),
                             "is VK_SEMAPHORE_TYPE_TIMELINE, but timelineSemaphore feature was not enabled.");
        }

        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_BINARY && sem_type_create_info->initialValue != 0) {
            skip |=
                LogError("VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279", device, create_info_loc.dot(Field::semaphoreType),
                         "is VK_SEMAPHORE_TYPE_BINARY, but initialValue is %" PRIu64 ".", sem_type_create_info->initialValue);
        }
    }

    auto sem_export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    if (sem_export_info && sem_export_info->handleTypes != 0) {
        VkExternalSemaphoreProperties external_properties = vku::InitStructHelper();
        bool export_supported = true;
        // Check export support
        auto check_export_support = [&](VkExternalSemaphoreHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalSemaphoreInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            DispatchGetPhysicalDeviceExternalSemaphoreProperties(physical_device, &external_info, &external_properties);
            if ((external_properties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) == 0) {
                export_supported = false;
                skip |= LogError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124", device,
                                 create_info_loc.pNext(Struct::VkExportSemaphoreCreateInfo, Field::handleTypes),
                                 "(%s) does not support VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT.",
                                 string_VkExternalSemaphoreHandleTypeFlagBits(flag));
            }
        };
        IterateFlags<VkExternalSemaphoreHandleTypeFlagBits>(sem_export_info->handleTypes, check_export_support);
        // Check handle types compatibility
        if (export_supported &&
            (sem_export_info->handleTypes & external_properties.compatibleHandleTypes) != sem_export_info->handleTypes) {
            skip |= LogError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124", device,
                             create_info_loc.pNext(Struct::VkExportSemaphoreCreateInfo, Field::handleTypes),
                             "(%s) are not reported as compatible by vkGetPhysicalDeviceExternalSemaphoreProperties (%s).",
                             string_VkExternalSemaphoreHandleTypeFlags(sem_export_info->handleTypes).c_str(),
                             string_VkExternalSemaphoreHandleTypeFlags(external_properties.compatibleHandleTypes).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateWaitSemaphores(device, pWaitInfo, timeout, error_obj);
}

bool CoreChecks::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                               const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pWaitInfo->pSemaphores[i]);
        if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError("VUID-VkSemaphoreWaitInfo-pSemaphores-03256", pWaitInfo->pSemaphores[i],
                             error_obj.location.dot(Field::pWaitInfo).dot(Field::pSemaphores, i), "%s was created with %s",
                             FormatHandle(pWaitInfo->pSemaphores[i]).c_str(), string_VkSemaphoreType(semaphore_state->type));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator,
                                             const ErrorObject &error_obj) const {
    auto fence_node = Get<FENCE_STATE>(fence);
    bool skip = false;
    if (fence_node) {
        if (fence_node->Scope() == kSyncScopeInternal && fence_node->State() == FENCE_INFLIGHT) {
            skip |= LogError("VUID-vkDestroyFence-fence-01120", fence, error_obj.location.dot(Field::fence), "(%s) is in use.",
                             FormatHandle(fence).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = Get<FENCE_STATE>(pFences[i]);
        if (fence_state && fence_state->Scope() == kSyncScopeInternal && fence_state->State() == FENCE_INFLIGHT) {
            skip |= LogError("VUID-vkResetFences-pFences-01123", pFences[i], error_obj.location.dot(Field::pFences, i),
                             "(%s) is in use.", FormatHandle(pFences[i]).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                                 const ErrorObject &error_obj) const {
    auto sema_node = Get<SEMAPHORE_STATE>(semaphore);
    bool skip = false;
    if (sema_node) {
        skip |= ValidateObjectNotInUse(sema_node.get(), error_obj.location.dot(Field::semaphore),
                                       "VUID-vkDestroySemaphore-semaphore-01137");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator,
                                             const ErrorObject &error_obj) const {
    auto event_state = Get<EVENT_STATE>(event);
    bool skip = false;
    if (event_state) {
        skip |= ValidateObjectNotInUse(event_state.get(), error_obj.location.dot(Field::event), "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator,
                                               const ErrorObject &error_obj) const {
    auto sampler_state = Get<SAMPLER_STATE>(sampler);
    bool skip = false;
    if (sampler_state) {
        skip |= ValidateObjectNotInUse(sampler_state.get(), error_obj.location.dot(Field::sampler),
                                       "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                            const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);
    const LogObjectList objlist(commandBuffer);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                             const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    const LogObjectList objlist(commandBuffer, event);

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.synchronization2,
                                         "VUID-vkCmdSetEvent2-synchronization2-03824", "synchronization2");
    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    if (pDependencyInfo->dependencyFlags != 0) {
        skip |= LogError("VUID-vkCmdSetEvent2-dependencyFlags-03825", objlist, dep_info_loc.dot(Field::dependencyFlags),
                         "(%s) must be 0.", string_VkDependencyFlags(pDependencyInfo->dependencyFlags).c_str());
    }
    skip |= ValidateDependencyInfo(objlist, dep_info_loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                               const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    const Location stage_mask_loc = error_obj.location.dot(Field::stageMask);

    bool skip = false;
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdResetEvent2-synchronization2-03829", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineStage(objlist, stage_mask_loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(stage_mask_loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

struct RenderPassDepState {
    using Field = vvl::Field;

    const CoreChecks *core;
    const std::string vuid;
    uint32_t active_subpass;
    const VkRenderPass rp_handle;
    const VkPipelineStageFlags2KHR disabled_features;
    const std::vector<uint32_t> &self_dependencies;
    const safe_VkSubpassDependency2 *dependencies;

    RenderPassDepState(const CoreChecks *c, const std::string &v, uint32_t subpass, const VkRenderPass handle,
                       const DeviceFeatures &features, const std::vector<uint32_t> &self_deps,
                       const safe_VkSubpassDependency2 *deps)
        : core(c),
          vuid(v),
          active_subpass(subpass),
          rp_handle(handle),
          disabled_features(sync_utils::DisabledPipelineStages(features)),
          self_dependencies(self_deps),
          dependencies(deps) {}

    VkMemoryBarrier2 GetSubPassDepBarrier(const safe_VkSubpassDependency2 &dep) const {
        // "If a VkMemoryBarrier2 is included in the pNext chain, srcStageMask, dstStageMask,
        // srcAccessMask, and dstAccessMask parameters are ignored. The synchronization and
        // access scopes instead are defined by the parameters of VkMemoryBarrier2."
        if (const auto override_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(dep.pNext)) {
            return *override_barrier;
        }

        VkMemoryBarrier2 barrier = vku::InitStructHelper();
        barrier.srcStageMask = dep.srcStageMask;
        barrier.dstStageMask = dep.dstStageMask;
        barrier.srcAccessMask = dep.srcAccessMask;
        barrier.dstAccessMask = dep.dstAccessMask;
        return barrier;
    }

    bool ValidateStage(const Location &barrier_loc, VkPipelineStageFlags2 src_stage_mask,
                       VkPipelineStageFlags2 dst_stage_mask) const {
        // Look for srcStageMask + dstStageMask superset in any self-dependency
        for (const auto self_dep_index : self_dependencies) {
            const auto subpass_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);

            const auto subpass_src_stages =
                sync_utils::ExpandPipelineStages(subpass_dep.srcStageMask, sync_utils::kAllQueueTypes, disabled_features);
            const auto barrier_src_stages =
                sync_utils::ExpandPipelineStages(src_stage_mask, sync_utils::kAllQueueTypes, disabled_features);

            const auto subpass_dst_stages =
                sync_utils::ExpandPipelineStages(subpass_dep.dstStageMask, sync_utils::kAllQueueTypes, disabled_features);
            const auto barrier_dst_stages =
                sync_utils::ExpandPipelineStages(dst_stage_mask, sync_utils::kAllQueueTypes, disabled_features);

            const bool is_subset = (barrier_src_stages == (subpass_src_stages & barrier_src_stages)) &&
                                   (barrier_dst_stages == (subpass_dst_stages & barrier_dst_stages));
            if (is_subset) return false;  // subset is found, return skip value (false)
        }
        return core->LogError(
            vuid, rp_handle, barrier_loc.dot(Field::srcStageMask),
            "(%s) and dstStageMask (%s) is not a subset of subpass dependency's srcStageMask and dstStageMask for "
            "any self-dependency of subpass %" PRIu32 " of %s.",
            string_VkPipelineStageFlags2(src_stage_mask).c_str(), string_VkPipelineStageFlags2(dst_stage_mask).c_str(),
            active_subpass, core->FormatHandle(rp_handle).c_str());
    }

    bool ValidateAccess(const Location &barrier_loc, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask) const {
        // Look for srcAccessMask + dstAccessMask superset in any self-dependency
        for (const auto self_dep_index : self_dependencies) {
            const auto subpass_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            const bool is_subset = (src_access_mask == (subpass_dep.srcAccessMask & src_access_mask)) &&
                                   (dst_access_mask == (subpass_dep.dstAccessMask & dst_access_mask));
            if (is_subset) return false;  // subset is found, return skip value (false)
        }
        return core->LogError(vuid, rp_handle, barrier_loc.dot(Field::srcAccessMask),
                              "(%s) and dstAccessMask (%s) is not a subset of subpass dependency's srcAccessMask and dstAccessMask "
                              "of subpass %" PRIu32 " of %s.",
                              string_VkAccessFlags2(src_access_mask).c_str(), string_VkAccessFlags2(dst_access_mask).c_str(),
                              active_subpass, core->FormatHandle(rp_handle).c_str());
    }

    bool ValidateDependencyFlag(const Location &dep_flags_loc, VkDependencyFlags dependency_flags) const {
        for (const auto self_dep_index : self_dependencies) {
            const auto &subpass_dep = dependencies[self_dep_index];
            const bool match = subpass_dep.dependencyFlags == dependency_flags;
            if (match) return false;  // match is found, return skip value (false)
        }
        return core->LogError(vuid, rp_handle, dep_flags_loc,
                              "(%s) does not equal VkSubpassDependency dependencyFlags value for any "
                              "self-dependency of subpass %" PRIu32 " of %s.",
                              string_VkDependencyFlags(dependency_flags).c_str(), active_subpass,
                              core->FormatHandle(rp_handle).c_str());
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
    RenderPassDepState state(this, "VUID-vkCmdPipelineBarrier-None-07889", cb_state->GetActiveSubpass(), rp_state->renderPass(),
                             enabled_features, rp_state->self_dependencies[cb_state->GetActiveSubpass()],
                             rp_state->createInfo.pDependencies);
    if (state.self_dependencies.size() == 0) {
        skip |= LogError("VUID-vkCmdPipelineBarrier-None-07889", state.rp_handle, outer_loc,
                         "Barriers cannot be set during subpass %" PRIu32 " of %s with no self-dependency specified.",
                         state.active_subpass, FormatHandle(state.rp_handle).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    skip |= state.ValidateStage(outer_loc, src_stage_mask, dst_stage_mask);

    if (0 != buffer_mem_barrier_count) {
        skip |= LogError("VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178", state.rp_handle,
                         outer_loc.dot(Field::bufferMemoryBarrierCount), "is non-zero (%" PRIu32 ") for subpass %" PRIu32 " of %s.",
                         buffer_mem_barrier_count, state.active_subpass, FormatHandle(rp_state->renderPass()).c_str());
    }
    for (uint32_t i = 0; i < mem_barrier_count; ++i) {
        const auto &mem_barrier = mem_barriers[i];
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= state.ValidateAccess(barrier_loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }

    for (uint32_t i = 0; i < image_mem_barrier_count; ++i) {
        const auto img_barrier = ImageBarrier(image_barriers[i], src_stage_mask, dst_stage_mask);
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        skip |= state.ValidateAccess(barrier_loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182", state.rp_handle,
                             barrier_loc.dot(Field::srcQueueFamilyIndex),
                             "is %" PRIu32 " and dstQueueFamilyIndex is %" PRIu32 " but both must be VK_QUEUE_FAMILY_IGNORED.",
                             img_barrier.srcQueueFamilyIndex, img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(barrier_loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass,
                                                   sub_desc, state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(outer_loc.dot(Field::dependencyFlags), dependency_flags);
    return skip;
}

bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                                    const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;
    const auto &rp_state = cb_state->activeRenderPass;
    if (rp_state->UsesDynamicRendering()) {
        return skip;
    }
    RenderPassDepState state(this, "VUID-vkCmdPipelineBarrier2-None-07889", cb_state->GetActiveSubpass(), rp_state->renderPass(),
                             enabled_features, rp_state->self_dependencies[cb_state->GetActiveSubpass()],
                             rp_state->createInfo.pDependencies);

    if (state.self_dependencies.size() == 0) {
        skip |= LogError(state.vuid, state.rp_handle, outer_loc,
                         "Barriers cannot be set during subpass %" PRIu32 " of %s with no self-dependency specified.",
                         state.active_subpass, FormatHandle(rp_state->renderPass()).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pMemoryBarriers[i];
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        skip |= state.ValidateStage(barrier_loc, mem_barrier.srcStageMask, mem_barrier.dstStageMask);
        skip |= state.ValidateAccess(barrier_loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }
    if (0 != dep_info->bufferMemoryBarrierCount) {
        skip |= LogError("VUID-vkCmdPipelineBarrier2-bufferMemoryBarrierCount-01178", state.rp_handle,
                         outer_loc.dot(Field::bufferMemoryBarrierCount), "is non-zero (%" PRIu32 ") for subpass %" PRIu32 " of %s.",
                         dep_info->bufferMemoryBarrierCount, state.active_subpass, FormatHandle(state.rp_handle).c_str());
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const auto img_barrier = ImageBarrier(dep_info->pImageMemoryBarriers[i]);
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);

        skip |= state.ValidateStage(barrier_loc, img_barrier.srcStageMask, img_barrier.dstStageMask);
        skip |= state.ValidateAccess(barrier_loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-01182", state.rp_handle,
                             barrier_loc.dot(Field::srcQueueFamilyIndex),
                             "is %" PRIu32 " and dstQueueFamilyIndex is %" PRIu32 " but both must be VK_QUEUE_FAMILY_IGNORED.",
                             img_barrier.srcQueueFamilyIndex, img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(barrier_loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass,
                                                   sub_desc, state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(outer_loc.dot(Field::dependencyFlags), dep_info->dependencyFlags);
    return skip;
}

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(const LogObjectList &objlist, const Location &stage_mask_loc,
                                                            VkQueueFlags queue_flags, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // these are always allowed by queues, calls that restrict them have dedicated VUs.
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
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(stage_mask_loc, entry.first);
            skip |= LogError(vuid, objlist, stage_mask_loc,
                             "(%s) is not compatible with the queue family properties (%s) of this command buffer.",
                             sync_utils::StringPipelineStageFlags(entry.first).c_str(), string_VkQueueFlags(queue_flags).c_str());
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
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(stage_mask_loc, bit);
            skip |= LogError(vuid, objlist, stage_mask_loc,
                             "(%s) is not compatible with the queue family properties (%s) of this command buffer.",
                             sync_utils::StringPipelineStageFlags(bit).c_str(), string_VkQueueFlags(queue_flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStageFeatureEnables(const LogObjectList &objlist, const Location &stage_mask_loc,
                                                     VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    if (!enabled_features.synchronization2 && stage_mask == 0) {
        const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(stage_mask_loc, 0, device_extensions);
        skip |= LogError(vuid, objlist, stage_mask_loc, "must not be 0 unless synchronization2 is enabled.");
    }

    auto disabled_stages = sync_utils::DisabledPipelineStages(enabled_features);
    auto bad_bits = stage_mask & disabled_stages;
    if (bad_bits == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_bits) * 8; i++) {
        VkPipelineStageFlags2KHR bit = 1ULL << i;
        if (bit & bad_bits) {
            const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(stage_mask_loc, bit, device_extensions);
            skip |= LogError(vuid, objlist, stage_mask_loc, "includes %s when the device does not have %s feature enabled.",
                             sync_utils::StringPipelineStageFlags(bit).c_str(), sync_vuid_maps::kFeatureNameMap.at(bit).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStage(const LogObjectList &objlist, const Location &stage_mask_loc, VkQueueFlags queue_flags,
                                       VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    skip |= ValidateStageMasksAgainstQueueCapabilities(objlist, stage_mask_loc, queue_flags, stage_mask);
    skip |= ValidatePipelineStageFeatureEnables(objlist, stage_mask_loc, stage_mask);
    return skip;
}

bool CoreChecks::ValidateAccessMask(const LogObjectList &objlist, const Location &access_mask_loc, const Location &stage_mask_loc,
                                    VkQueueFlags queue_flags, VkAccessFlags2KHR access_mask,
                                    VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;

    const auto expanded_pipeline_stages = sync_utils::ExpandPipelineStages(stage_mask, queue_flags);

    if (!enabled_features.rayQuery && (access_mask & VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR)) {
        const auto illegal_pipeline_stages = allVkPipelineShaderStageBits2 & ~VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        if (stage_mask & illegal_pipeline_stages) {
            // Select right vuid based on enabled extensions
            const auto &vuid = sync_vuid_maps::GetAccessMaskRayQueryVUIDSelector(access_mask_loc, device_extensions);
            skip |= LogError(vuid, objlist, stage_mask_loc, "contains pipeline stages %s.",
                             sync_utils::StringPipelineStageFlags(stage_mask).c_str());
        }
    }

    // Early out if all commands set
    if ((stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) != 0) return skip;

    // or if only generic memory accesses are specified (or we got a 0 mask)
    access_mask &= ~(VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR);
    if (access_mask == 0) return skip;

    const auto valid_accesses = sync_utils::CompatibleAccessMask(expanded_pipeline_stages);
    const auto bad_accesses = (access_mask & ~valid_accesses);
    if (bad_accesses == 0) {
        return skip;
    }

    for (size_t i = 0; i < sizeof(bad_accesses) * 8; i++) {
        VkAccessFlags2KHR bit = (1ULL << i);
        if (bad_accesses & bit) {
            const auto &vuid = sync_vuid_maps::GetBadAccessFlagsVUID(access_mask_loc, bit);
            skip |= LogError(vuid, objlist, access_mask_loc, "(%s) is not supported by stage mask (%s).",
                             sync_utils::StringAccessFlags(bit).c_str(), sync_utils::StringPipelineStageFlags(stage_mask).c_str());
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
            assert(global_event_data);  // caught with VUID-vkCmdWaitEvents-pEvents-parameter
            stage_mask |= global_event_data->stageMask;
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
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                              const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    auto queue_flags = cb_state->GetQueueFlags();
    const LogObjectList objlist(commandBuffer);

    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::dstStageMask), queue_flags, dstStageMask);

    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateBarriers(error_obj.location, cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i) {
        if (pBufferMemoryBarriers[i].srcQueueFamilyIndex != pBufferMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803", commandBuffer,
                             error_obj.location.dot(Field::pBufferMemoryBarriers, i),
                             "has different srcQueueFamilyIndex (%" PRIu32 ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             pBufferMemoryBarriers[i].srcQueueFamilyIndex, pBufferMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        if (pImageMemoryBarriers[i].srcQueueFamilyIndex != pImageMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803", commandBuffer,
                             error_obj.location.dot(Field::pImageMemoryBarriers, i),
                             "has different srcQueueFamilyIndex (%" PRIu32 ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             pImageMemoryBarriers[i].srcQueueFamilyIndex, pImageMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }

    if (cb_state->activeRenderPass && ((srcStageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0)) {
        skip |= LogError("VUID-vkCmdWaitEvents-srcStageMask-07308", commandBuffer, error_obj.location.dot(Field::srcStageMask),
                         "is %s.", sync_utils::StringPipelineStageFlags(srcStageMask).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdWaitEvents2-synchronization2-03836", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    for (uint32_t i = 0; (i < eventCount) && !skip; i++) {
        const LogObjectList objlist(commandBuffer, pEvents[i]);
        const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfos, i);
        if (pDependencyInfos[i].dependencyFlags != 0) {
            skip |= LogError("VUID-vkCmdWaitEvents2-dependencyFlags-03844", objlist, dep_info_loc.dot(Field::dependencyFlags),
                             "(%s) must be 0.", string_VkDependencyFlags(pDependencyInfos[i].dependencyFlags).c_str());
        }
        skip |= ValidateDependencyInfo(objlist, dep_info_loc, cb_state.get(), &pDependencyInfos[i]);
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

void CORE_CMD_BUFFER_STATE::RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags2KHR srcStageMask) {
    // CMD_BUFFER_STATE will add to the events vector.
    auto first_event_index = events.size();
    CMD_BUFFER_STATE::RecordWaitEvents(command, eventCount, pEvents, srcStageMask);
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
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers, sourceStageMask, dstStageMask);
}

void CoreChecks::RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, Func command) {
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
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, Func::vkCmdWaitEvents2KHR);
}

void CoreChecks::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             const VkDependencyInfo *pDependencyInfos) {
    StateTracker::PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, Func::vkCmdWaitEvents2);
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(record_obj.location.function, cb_state.get(), sourceStageMask, dstStageMask, bufferMemoryBarrierCount,
                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(record_obj.location.function, cb_state.get(), dep_info);
    }
}

void CoreChecks::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(record_obj.location.function, cb_state.get(), dep_info);
    }
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    auto queue_flags = cb_state->GetQueueFlags();

    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, error_obj.location.dot(Field::dstStageMask), queue_flags, dstStageMask);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (cb_state->activeRenderPass && !cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= ValidateRenderPassPipelineBarriers(error_obj.location, cb_state.get(), srcStageMask, dstStageMask, dependencyFlags,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip =
                LogError("VUID-vkCmdPipelineBarrier-dependencyFlags-01186", objlist, error_obj.location.dot(Field::dependencyFlags),
                         "VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance.");
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        // In dynamic rendering, vkCmdPipelineBarrier is only allowed for VK_EXT_shader_tile_image
        skip |= ValidateShaderTileImageBarriers(objlist, error_obj.location, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                                                bufferMemoryBarrierCount, imageMemoryBarrierCount, srcStageMask, dstStageMask);
    }
    skip |= ValidateBarriers(error_obj.location, cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);

    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdPipelineBarrier2-synchronization2-03848", commandBuffer, error_obj.location,
                         "the synchronization2 feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers(dep_info_loc, cb_state.get(), pDependencyInfo);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (pDependencyInfo->dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip = LogError("VUID-vkCmdPipelineBarrier2-dependencyFlags-01186", objlist, dep_info_loc.dot(Field::dependencyFlags),
                            "VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance.");
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        // In dynamic rendering, vkCmdPipelineBarrier2 is only allowed for  VK_EXT_shader_tile_image
        skip |= ValidateShaderTileImageBarriers(objlist, dep_info_loc, *pDependencyInfo);
    }
    skip |= ValidateDependencyInfo(objlist, dep_info_loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
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

    RecordBarriers(Func::vkCmdPipelineBarrier, cb_state.get(), srcStageMask, dstStageMask, bufferMemoryBarrierCount,
                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers, srcStageMask, dstStageMask);
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

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->write_in_use) {
            skip |= LogError(kVUID_Core_DrawState_QueueForwardProgress, event, error_obj.location.dot(Field::event),
                             "(%s) that is already in use by a command buffer.", FormatHandle(event).c_str());
        }
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError("VUID-vkSetEvent-event-03941", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.", FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError("VUID-vkResetEvent-event-03823", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.", FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject &error_obj) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError("VUID-vkGetEventStatus-event-03940", event, error_obj.location.dot(Field::event),
                             "(%s) was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.", FormatHandle(event).c_str());
        }
    }
    return skip;
}
bool CoreChecks::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const Location signal_loc = error_obj.location.dot(Field::pSignalInfo);
    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (!semaphore_state) {
        return skip;
    }
    if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-semaphore-03257", pSignalInfo->semaphore, signal_loc.dot(Field::semaphore),
                         "%s was created with %s.", FormatHandle(pSignalInfo->semaphore).c_str(),
                         string_VkSemaphoreType(semaphore_state->type));
        return skip;
    }

    const auto completed = semaphore_state->Completed();
    if (completed.payload >= pSignalInfo->value) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-value-03258", pSignalInfo->semaphore, signal_loc.dot(Field::value),
                         "(%" PRIu64 ") must be greater than current semaphore %s value (%" PRIu64 ").", pSignalInfo->value,
                         FormatHandle(pSignalInfo->semaphore).c_str(), completed.payload);
        return skip;
    }
    auto exceeds_pending = [pSignalInfo](const SEMAPHORE_STATE::SemOp &op, bool is_pending) {
        return is_pending && op.IsSignal() && pSignalInfo->value >= op.payload;
    };
    auto last_op = semaphore_state->LastOp(exceeds_pending);
    if (last_op) {
        skip |= LogError("VUID-VkSemaphoreSignalInfo-value-03259", pSignalInfo->semaphore, signal_loc.dot(Field::value),
                         "(%" PRIu64 ") must be less than value of any pending signal operation (%" PRIu64 ") for semaphore %s.",
                         pSignalInfo->value, last_op->payload, FormatHandle(pSignalInfo->semaphore).c_str());
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
        const Location loc = error_obj.location.dot(Struct::VkSemaphoreSignalInfo, Field::value);
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kTimelineSemMaxDiff);
        skip |= LogError(vuid, semaphore_state->Handle(), loc,
                         "(%" PRIu64 ") exceeds limit regarding %s semaphore %s payload (%" PRIu64 ").", pSignalInfo->value,
                         FormatHandle(*semaphore_state).c_str(), where, bad_value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                   const ErrorObject &error_obj) const {
    return PreCallValidateSignalSemaphore(device, pSignalInfo, error_obj);
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError("VUID-vkGetSemaphoreCounterValue-semaphore-03255", semaphore, error_obj.location.dot(Field::semaphore),
                         "%s was created with %s.", FormatHandle(semaphore).c_str(), string_VkSemaphoreType(semaphore_state->type));
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue, error_obj);
}

// VkSubpassDependency validation happens when vkCreateRenderPass() is called.
// Dependencies between subpasses can only use pipeline stages compatible with VK_QUEUE_GRAPHICS_BIT,
// for external subpasses we don't have a yet command buffer so we have to assume all of them are valid.
static inline VkQueueFlags SubpassToQueueFlags(uint32_t subpass) {
    return subpass == VK_SUBPASS_EXTERNAL ? sync_utils::kAllQueueTypes : static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT);
}

bool CoreChecks::ValidateSubpassDependency(const ErrorObject &error_obj, const Location &in_loc,
                                           const VkSubpassDependency2 &dependency) const {
    bool skip = false;
    VkMemoryBarrier2KHR converted_barrier;
    const auto *mem_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(dependency.pNext);
    const Location loc = mem_barrier ? in_loc.dot(Field::pNext) : in_loc;

    if (mem_barrier) {
        converted_barrier = *mem_barrier;
    } else {
        // use the subpass dependency flags, upconverted into wider synchronization2 fields.
        converted_barrier.srcStageMask = dependency.srcStageMask;
        converted_barrier.dstStageMask = dependency.dstStageMask;
        converted_barrier.srcAccessMask = dependency.srcAccessMask;
        converted_barrier.dstAccessMask = dependency.dstAccessMask;
    }
    auto src_queue_flags = SubpassToQueueFlags(dependency.srcSubpass);
    skip |= ValidatePipelineStage(error_obj.objlist, loc.dot(Field::srcStageMask), src_queue_flags, converted_barrier.srcStageMask);
    skip |= ValidateAccessMask(error_obj.objlist, loc.dot(Field::srcAccessMask), loc.dot(Field::srcStageMask), src_queue_flags,
                               converted_barrier.srcAccessMask, converted_barrier.srcStageMask);

    auto dst_queue_flags = SubpassToQueueFlags(dependency.dstSubpass);
    skip |= ValidatePipelineStage(error_obj.objlist, loc.dot(Field::dstStageMask), dst_queue_flags, converted_barrier.dstStageMask);
    skip |= ValidateAccessMask(error_obj.objlist, loc.dot(Field::dstAccessMask), loc.dot(Field::dstStageMask), dst_queue_flags,
                               converted_barrier.dstAccessMask, converted_barrier.dstStageMask);
    return skip;
}

// Verify an ImageMemoryBarrier's old/new ImageLayouts are compatible with the Image's ImageUsageFlags.
bool CoreChecks::ValidateBarrierLayoutToImageUsage(const Location &layout_loc, VkImage image, VkImageLayout layout,
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
        const auto &vuid = sync_vuid_maps::GetBadImageLayoutVUID(layout_loc, layout);
        skip |= LogError(vuid, image, layout_loc, "(%s) is not compatible with %s usage flags 0x%" PRIx32 ".",
                         string_VkImageLayout(layout), FormatHandle(image).c_str(), usage_flags);
    }
    return skip;
}

// Verify image barriers are compatible with the images they reference.
bool CoreChecks::ValidateBarriersToImages(const Location &barrier_loc, const CMD_BUFFER_STATE *cb_state,
                                          const ImageBarrier &img_barrier,
                                          CommandBufferImageLayoutMap &layout_updates_state) const {
    bool skip = false;
    using sync_vuid_maps::GetImageBarrierVUID;
    using sync_vuid_maps::ImageError;

    const CommandBufferImageLayoutMap &current_map = cb_state->GetImageSubresourceLayoutMap();

    {
        auto image_state = Get<IMAGE_STATE>(img_barrier.image);
        if (!image_state) {
            return skip;
        }
        auto image_loc = barrier_loc.dot(Field::image);

        if ((img_barrier.srcQueueFamilyIndex != img_barrier.dstQueueFamilyIndex) ||
            (img_barrier.oldLayout != img_barrier.newLayout)) {
            VkImageUsageFlags usage_flags = image_state->createInfo.usage;
            skip |= ValidateBarrierLayoutToImageUsage(barrier_loc.dot(Field::oldLayout), img_barrier.image, img_barrier.oldLayout,
                                                      usage_flags);
            skip |= ValidateBarrierLayoutToImageUsage(barrier_loc.dot(Field::newLayout), img_barrier.image, img_barrier.newLayout,
                                                      usage_flags);
        }

        // Make sure layout is able to be transitioned, currently only presented shared presentable images are locked
        if (image_state->layout_locked) {
            // TODO: waiting for VUID https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5078
            skip |= LogError("VUID-Undefined", img_barrier.image, image_loc,
                             "(%s) is a shared presentable and attempting to transition from layout %s to layout %s, but image has "
                             "already been presented and cannot have its layout transitioned.",
                             FormatHandle(img_barrier.image).c_str(), string_VkImageLayout(img_barrier.oldLayout),
                             string_VkImageLayout(img_barrier.newLayout));
        }

        const VkImageCreateInfo &image_create_info = image_state->createInfo;
        const VkFormat image_format = image_create_info.format;
        const VkImageAspectFlags aspect_mask = img_barrier.subresourceRange.aspectMask;
        const bool has_depth_mask = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
        const bool has_stencil_mask = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;

        if (vkuFormatIsDepthAndStencil(image_format)) {
            if (enabled_features.separateDepthStencilLayouts) {
                if (!has_depth_mask && !has_stencil_mask) {
                    auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotDepthOrStencilAspect);
                    skip |=
                        LogError(vuid, img_barrier.image, image_loc, "(%s) has depth/stencil format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
                }
            } else {
                if (!has_depth_mask || !has_stencil_mask) {
                    auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotDepthAndStencilAspect);
                    skip |=
                        LogError(vuid, img_barrier.image, image_loc, "(%s) has depth/stencil format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
                }
            }
        }

        if (has_depth_mask) {
            if (IsImageLayoutStencilOnly(img_barrier.oldLayout) || IsImageLayoutStencilOnly(img_barrier.newLayout)) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kSeparateDepthWithStencilLayout);
                skip |= LogError(
                    vuid, img_barrier.image, image_loc,
                    "(%s) has stencil format %s has depth aspect with stencil only layouts, oldLayout = %s and newLayout = %s.",
                    FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                    string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
            }
        }
        if (has_stencil_mask) {
            if (IsImageLayoutDepthOnly(img_barrier.oldLayout) || IsImageLayoutDepthOnly(img_barrier.newLayout)) {
                auto vuid = GetImageBarrierVUID(barrier_loc, ImageError::kSeparateStencilhWithDepthLayout);
                skip |= LogError(
                    vuid, img_barrier.image, image_loc,
                    "(%s) has depth format %s has stencil aspect with depth only layouts, oldLayout = %s and newLayout = %s.",
                    FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                    string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout));
            }
        }

        if (img_barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            // TODO: Set memory invalid which is in mem_tracker currently
        } else if (!IsQueueFamilyExternal(img_barrier.srcQueueFamilyIndex)) {
            skip |= UpdateCommandBufferImageLayoutMap(cb_state, image_loc, img_barrier, current_map, layout_updates_state);
        }

        // checks color format and (single-plane or non-disjoint)
        // if ycbcr extension is not supported then single-plane and non-disjoint are always both true

        if (vkuFormatIsColor(image_format) && (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT)) {
            if (!vkuFormatIsMultiplane(image_format)) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotColorAspectSinglePlane);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has color format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            } else if (!image_state->disjoint) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kNotColorAspectNonDisjoint);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has color format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }

        if ((vkuFormatIsMultiplane(image_format)) && (image_state->disjoint == true)) {
            if (!IsValidPlaneAspect(image_format, aspect_mask) && ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) == 0)) {
                const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kBadMultiplanarAspect);
                skip |= LogError(vuid, img_barrier.image, image_loc, "(%s) has Multiplane format %s, but its aspectMask is %s.",
                                 FormatHandle(img_barrier.image).c_str(), string_VkFormat(image_format),
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            }
        }
    }
    return skip;
}

// Verify image barrier image state and that the image is consistent with FB image
bool CoreChecks::ValidateImageBarrierAttachment(const Location &barrier_loc, CMD_BUFFER_STATE const *cb_state,
                                                const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                const safe_VkSubpassDescription2 &sub_desc, const VkRenderPass rp_handle,
                                                const ImageBarrier &img_barrier, const CMD_BUFFER_STATE *primary_cb_state) const {
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
    uint64_t image_ahb_format = 0;
    const Location image_loc = barrier_loc.dot(Field::image);
    // Verify that a framebuffer image matches barrier image
    const auto attachment_count = fb_state->createInfo.attachmentCount;
    for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
        auto view_state = primary_cb_state ? primary_cb_state->GetActiveAttachmentImageViewState(attachment)
                                           : cb_state->GetActiveAttachmentImageViewState(attachment);
        if (view_state && (img_bar_image == view_state->create_info.image)) {
            image_match = true;
            attach_index = attachment;
            image_ahb_format = view_state->image_state->ahb_format;
            break;
        }
    }
    if (image_match) {  // Make sure subpass is referring to matching attachment
        if (sub_desc.pDepthStencilAttachment && sub_desc.pDepthStencilAttachment->attachment == attach_index) {
            sub_image_layout = sub_desc.pDepthStencilAttachment->layout;
            sub_image_found = true;
        }
        if (!sub_image_found) {
            const auto *resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(sub_desc.pNext);
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
                    if (image_ahb_format == 0) {
                        skip |= LogError("VUID-vkCmdPipelineBarrier2-image-09374", rp_handle, image_loc,
                                         "(%s) for subpass %" PRIu32 " was not created with an externalFormat.",
                                         FormatHandle(img_bar_image).c_str(), active_subpass);
                    }
                    break;
                }
            }
        }
        if (!sub_image_found) {
            const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatch);
            skip |= LogError(vuid, rp_handle, image_loc,
                             "(%s) is not referenced by the VkSubpassDescription for active subpass (%" PRIu32 ") of current %s.",
                             FormatHandle(img_bar_image).c_str(), active_subpass, FormatHandle(rp_handle).c_str());
        }

    } else {  // !image_match
        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassMismatch);
        skip |= LogError(vuid, fb_state->framebuffer(), image_loc, "(%s) does not match an image from the current %s.",
                         FormatHandle(img_bar_image).c_str(), FormatHandle(fb_state->framebuffer()).c_str());
    }
    if (img_barrier.oldLayout != img_barrier.newLayout) {
        const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassLayoutChange);
        skip |= LogError(vuid, cb_state->commandBuffer(), barrier_loc.dot(Field::oldLayout),
                         "is %s and newLayout is %s, but %s is being executed within a render pass instance.",
                         string_VkImageLayout(img_barrier.oldLayout), string_VkImageLayout(img_barrier.newLayout),
                         FormatHandle(img_barrier.image).c_str());
    } else {
        if (sub_image_found && sub_image_layout != img_barrier.oldLayout) {
            const LogObjectList objlist(rp_handle, img_bar_image);
            const auto &vuid = GetImageBarrierVUID(barrier_loc, ImageError::kRenderPassLayoutChange);
            skip |= LogError(vuid, objlist, image_loc,
                             "(%s) is referenced by the VkSubpassDescription for active "
                             "subpass (%" PRIu32 ") of current %s as having layout %s, but image barrier has layout %s.",
                             FormatHandle(img_bar_image).c_str(), active_subpass, FormatHandle(rp_handle).c_str(),
                             string_VkImageLayout(sub_image_layout), string_VkImageLayout(img_barrier.oldLayout));
        }
    }
    return skip;
}

void CoreChecks::EnqueueSubmitTimeValidateImageBarrierAttachment(const Location &loc, CMD_BUFFER_STATE *cb_state,
                                                                 const ImageBarrier &barrier) {
    // Secondary CBs can have null framebuffer so queue up validation in that case 'til FB is known
    if ((cb_state->activeRenderPass) && (VK_NULL_HANDLE == cb_state->activeFramebuffer) &&
        (VK_COMMAND_BUFFER_LEVEL_SECONDARY == cb_state->createInfo.level)) {
        const auto active_subpass = cb_state->GetActiveSubpass();
        const auto rp_state = cb_state->activeRenderPass;
        const auto &sub_desc = rp_state->createInfo.pSubpasses[active_subpass];
        // Secondary CB case w/o FB specified delay validation
        auto *this_ptr = this;  // Required for older compilers with c++20 compatibility
        vvl::LocationCapture loc_capture(loc);
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
        if (cb_state->IsReleaseOp(barrier) && !IsQueueFamilyExternal(barrier.dstQueueFamilyIndex)) {
            barrier_sets.release.emplace(barrier);
        } else if (cb_state->IsAcquireOp(barrier) && !IsQueueFamilyExternal(barrier.srcQueueFamilyIndex)) {
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
        auto handle_state = barrier.GetResourceState(*this);
        const bool mode_concurrent = handle_state && handle_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT;
        if (!mode_concurrent) {
            const auto typed_handle = barrier.GetTypedHandle();
            vvl::LocationCapture loc_capture(loc);
            cb_state->queue_submit_functions.emplace_back(
                [loc_capture, typed_handle, src_queue_family, dst_queue_family](
                    const ValidationStateTracker &device_data, const QUEUE_STATE &queue_state, const CMD_BUFFER_STATE &cb_state) {
                    return ValidateConcurrentBarrierAtSubmit(loc_capture.Get(), device_data, queue_state, cb_state, typed_handle,
                                                             src_queue_family, dst_queue_family);
                });
        }
    }
}

void CoreChecks::RecordBarriers(Func func_name, CMD_BUFFER_STATE *cb_state, VkPipelineStageFlags src_stage_mask,
                                VkPipelineStageFlags dst_stage_mask, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers) {
    for (uint32_t i = 0; i < bufferBarrierCount; i++) {
        Location loc(func_name, Struct::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(pBufferMemBarriers[i], src_stage_mask, dst_stage_mask);
        RecordBarrierValidationInfo(loc, cb_state, barrier, cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; i++) {
        Location loc(func_name, Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(pImageMemBarriers[i], src_stage_mask, dst_stage_mask);
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

void CoreChecks::RecordBarriers(Func func_name, CMD_BUFFER_STATE *cb_state, const VkDependencyInfoKHR &dep_info) {
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        Location loc(func_name, Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(dep_info.pBufferMemoryBarriers[i]);
        RecordBarrierValidationInfo(loc, cb_state, barrier, cb_state->qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        Location loc(func_name, Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(dep_info.pImageMemoryBarriers[i]);
        RecordBarrierValidationInfo(loc, cb_state, img_barrier, cb_state->qfo_transfer_image_barriers);
        EnqueueSubmitTimeValidateImageBarrierAttachment(loc, cb_state, img_barrier);
    }
}

template <typename TransferBarrier, typename Scoreboard>
bool CoreChecks::ValidateAndUpdateQFOScoreboard(const debug_report_data *report_data, const CMD_BUFFER_STATE &cb_state,
                                                const char *operation, const TransferBarrier &barrier, Scoreboard *scoreboard,
                                                const Location &loc) const {
    // Record to the scoreboard or report that we have a duplication
    bool skip = false;
    auto inserted = scoreboard->emplace(barrier, &cb_state);
    if (!inserted.second && inserted.first->second != &cb_state) {
        // This is a duplication (but don't report duplicates from the same CB, as we do that at record time
        const LogObjectList objlist(cb_state.commandBuffer(), barrier.handle, inserted.first->second->commandBuffer());
        skip = LogWarning(TransferBarrier::ErrMsgDuplicateQFOInSubmit(), objlist, loc,
                          "%s %s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                          " duplicates existing barrier submitted in this batch from %s.",
                          TransferBarrier::BarrierName(), operation, TransferBarrier::HandleName(),
                          FormatHandle(barrier.handle).c_str(), barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex,
                          FormatHandle(inserted.first->second->commandBuffer()).c_str());
    }
    return skip;
}

template <typename TransferBarrier>
bool CoreChecks::ValidateQueuedQFOTransferBarriers(const CMD_BUFFER_STATE &cb_state,
                                                   QFOTransferCBScoreboards<TransferBarrier> *scoreboards,
                                                   const GlobalQFOTransferBarrierMap<TransferBarrier> &global_release_barriers,
                                                   const Location &loc) const {
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
                skip |= LogWarning(TransferBarrier::ErrMsgDuplicateQFOSubmitted(), cb_state.commandBuffer(), loc,
                                   "%s releasing queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                                   " to dstQueueFamilyIndex %" PRIu32
                                   " duplicates existing barrier queued for execution, without intervening acquire operation.",
                                   barrier_name, handle_name, FormatHandle(found->handle).c_str(), found->srcQueueFamilyIndex,
                                   found->dstQueueFamilyIndex);
            }
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "releasing", release, &scoreboards->release, loc);
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
            skip |= LogError(TransferBarrier::ErrMsgMissingQFOReleaseInSubmit(), cb_state.commandBuffer(), loc,
                             "in submitted command buffer %s acquiring ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32
                             " to dstQueueFamilyIndex %" PRIu32 " has no matching release barrier queued for execution.",
                             barrier_name, handle_name, FormatHandle(acquire.handle).c_str(), acquire.srcQueueFamilyIndex,
                             acquire.dstQueueFamilyIndex);
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "acquiring", acquire, &scoreboards->acquire, loc);
    }
    return skip;
}

bool CoreChecks::ValidateQueuedQFOTransfers(const CMD_BUFFER_STATE &cb_state,
                                            QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
                                            QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards,
                                            const Location &loc) const {
    bool skip = false;
    skip |= ValidateQueuedQFOTransferBarriers<QFOImageTransferBarrier>(cb_state, qfo_image_scoreboards,
                                                                       qfo_release_image_barrier_map, loc);
    skip |= ValidateQueuedQFOTransferBarriers<QFOBufferTransferBarrier>(cb_state, qfo_buffer_scoreboards,
                                                                        qfo_release_buffer_barrier_map, loc);
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
bool CoreChecks::ValidateQFOTransferBarrierUniqueness(const Location &barrier_loc, const CMD_BUFFER_STATE *cb_state,
                                                      const Barrier &barrier,
                                                      const QFOTransferBarrierSets<TransferBarrier> &barrier_sets) const {
    bool skip = false;
    const char *handle_name = TransferBarrier::HandleName();
    const char *transfer_type = nullptr;
    if (!IsTransferOp(barrier)) {
        return skip;
    }
    const TransferBarrier *barrier_record = nullptr;
    if (cb_state->IsReleaseOp(barrier) && !IsQueueFamilyExternal(barrier.dstQueueFamilyIndex)) {
        const auto found = barrier_sets.release.find(barrier);
        if (found != barrier_sets.release.cend()) {
            barrier_record = &(*found);
            transfer_type = "releasing";
        }
    } else if (cb_state->IsAcquireOp(barrier) && !IsQueueFamilyExternal(barrier.srcQueueFamilyIndex)) {
        const auto found = barrier_sets.acquire.find(barrier);
        if (found != barrier_sets.acquire.cend()) {
            barrier_record = &(*found);
            transfer_type = "acquiring";
        }
    }
    if (barrier_record != nullptr) {
        skip |= LogWarning(TransferBarrier::ErrMsgDuplicateQFOInCB(), cb_state->commandBuffer(), barrier_loc,
                           "%s queue ownership of %s (%s), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                           " duplicates existing barrier recorded in this command buffer.",
                           transfer_type, handle_name, FormatHandle(barrier_record->handle).c_str(),
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
    ValidatorState(const ValidationStateTracker *device_data, const LogObjectList &obj, const Location &location,
                   const VulkanTypedHandle &barrier_handle, const VkSharingMode sharing_mode)
        : device_data_(device_data),
          objects_(std::move(obj)),
          loc_(location),
          barrier_handle_(barrier_handle),
          sharing_mode_(sharing_mode),
          limit_(static_cast<uint32_t>(device_data->physical_device_state->queue_family_properties.size())) {}

    // Log the messages using boilerplate from object state, and Vu specific information from the template arg
    // One and two family versions, in the single family version, Vu holds the name of the passed parameter
    bool LogMsg(QueueError vu_index, uint32_t family, const char *param_name) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *annotation = GetFamilyAnnotation(family);
        return device_data_->LogError(val_code, objects_, loc_,
                                      "barrier using %s %s created with sharingMode %s, has %s %" PRIu32 "%s. %s", GetTypeString(),
                                      device_data_->FormatHandle(barrier_handle_).c_str(), GetModeString(), param_name, family,
                                      annotation, kQueueErrorSummary.at(vu_index).c_str());
    }

    bool LogMsg(QueueError vu_index, uint32_t src_family, uint32_t dst_family) const {
        const std::string val_code = GetBarrierQueueVUID(loc_, vu_index);
        const char *src_annotation = GetFamilyAnnotation(src_family);
        const char *dst_annotation = GetFamilyAnnotation(dst_family);
        return device_data_->LogError(val_code, objects_, loc_,
                                      "barrier using %s %s created with sharingMode %s, has srcQueueFamilyIndex %" PRIu32
                                      "%s and dstQueueFamilyIndex %" PRIu32 "%s. %s",
                                      GetTypeString(), device_data_->FormatHandle(barrier_handle_).c_str(), GetModeString(),
                                      src_family, src_annotation, dst_family, dst_annotation,
                                      kQueueErrorSummary.at(vu_index).c_str());
    }

    // This abstract Vu can only be tested at submit time, thus we need a callback from the closure containing the needed
    // data. Note that the mem_barrier is copied to the closure as the lambda lifespan exceed the guarantees of validity for
    // application input.
    static bool ValidateAtQueueSubmit(const QUEUE_STATE *queue_state, const ValidationStateTracker *device_data,
                                      uint32_t src_family, uint32_t dst_family, const ValidatorState &val) {
        uint32_t queue_family = queue_state->queueFamilyIndex;
        if ((src_family != queue_family) && (dst_family != queue_family)) {
            const char *src_annotation = val.GetFamilyAnnotation(src_family);
            const char *dst_annotation = val.GetFamilyAnnotation(dst_family);
            return device_data->LogError("VUID-vkQueueSubmit-pSubmits-04626", queue_state->Handle(), val.loc_,
                                         "barrier submitted to queue with family index %" PRIu32
                                         ", using %s %s created with sharingMode %s, has "
                                         "srcQueueFamilyIndex %" PRIu32 "%s and dstQueueFamilyIndex %" PRIu32
                                         "%s. Source or destination queue family must match submit queue family, if not ignored.",
                                         queue_family, val.GetTypeString(), device_data->FormatHandle(val.barrier_handle_).c_str(),
                                         val.GetModeString(), src_family, src_annotation, dst_family, dst_annotation);
        }
        return false;
    }
    // Logical helpers for semantic clarity
    inline bool IsValid(uint32_t queue_family) const { return (queue_family < limit_); }
    inline bool IsValidOrSpecial(uint32_t queue_family) const {
        return IsValid(queue_family) || IsQueueFamilyExternal(queue_family) || (queue_family == VK_QUEUE_FAMILY_IGNORED);
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
        }
    }
    const char *GetTypeString() const { return object_string[barrier_handle_.type]; }
    VkSharingMode GetSharingMode() const { return sharing_mode_; }

  protected:
    const ValidationStateTracker *device_data_;
    const LogObjectList objects_;
    const Location loc_;
    const VulkanTypedHandle barrier_handle_;
    const VkSharingMode sharing_mode_;
    const uint32_t limit_;
};

static bool Validate(const CoreChecks *device_data, const ValidatorState &val, const uint32_t src_queue_family,
                     const uint32_t dst_queue_family) {
    bool skip = false;

    if (!IsExtEnabled(device_data->device_extensions.vk_khr_external_memory)) {
        if (src_queue_family == VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= val.LogMsg(QueueError::kSrcNoExternalExt, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family == VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= val.LogMsg(QueueError::kDstNoExternalExt, dst_queue_family, "dstQueueFamilyIndex");
        }

        if (val.GetSharingMode() == VK_SHARING_MODE_EXCLUSIVE && src_queue_family != dst_queue_family) {
            if (!val.IsValid(src_queue_family)) {
                skip |= val.LogMsg(QueueError::kExclusiveSrc, src_queue_family, "srcQueueFamilyIndex");
            }
            if (!val.IsValid(dst_queue_family)) {
                skip |= val.LogMsg(QueueError::kExclusiveDst, dst_queue_family, "dstQueueFamilyIndex");
            }
        }
    } else {
        if (val.GetSharingMode() == VK_SHARING_MODE_EXCLUSIVE && src_queue_family != dst_queue_family) {
            if (!val.IsValidOrSpecial(src_queue_family)) {
                skip |= val.LogMsg(QueueError::kExclusiveSrc, src_queue_family, "srcQueueFamilyIndex");
            }
            if (!val.IsValidOrSpecial(dst_queue_family)) {
                skip |= val.LogMsg(QueueError::kExclusiveDst, dst_queue_family, "dstQueueFamilyIndex");
            }
        }
    }

    if (!IsExtEnabled(device_data->device_extensions.vk_ext_queue_family_foreign)) {
        if (src_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            skip |= val.LogMsg(QueueError::kSrcNoForeignExt, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            skip |= val.LogMsg(QueueError::kDstNoForeignExt, dst_queue_family, "dstQueueFamilyIndex");
        }
    }

    if (!device_data->enabled_features.synchronization2 && val.GetSharingMode() == VK_SHARING_MODE_CONCURRENT) {
        if (src_queue_family != VK_QUEUE_FAMILY_IGNORED && src_queue_family != VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= val.LogMsg(QueueError::kSync1ConcurrentSrc, src_queue_family, "srcQueueFamilyIndex");
        } else if (dst_queue_family != VK_QUEUE_FAMILY_IGNORED && dst_queue_family != VK_QUEUE_FAMILY_EXTERNAL) {
            skip |= val.LogMsg(QueueError::kSync1ConcurrentDst, dst_queue_family, "dstQueueFamilyIndex");
        } else if (src_queue_family != VK_QUEUE_FAMILY_IGNORED && dst_queue_family != VK_QUEUE_FAMILY_IGNORED) {
            skip |= val.LogMsg(QueueError::kSync1ConcurrentNoIgnored, src_queue_family, dst_queue_family);
        }
    }

    return skip;
}

static bool ValidateHostStage(const ValidationObject *validation_obj, const LogObjectList &objects, const Location &barrier_loc,
                              const QueueFamilyBarrier &barrier) {
    // QueueError::kHostStage vuids are applicable only to sync2
    if (barrier_loc.structure != vvl::Struct::VkBufferMemoryBarrier2 &&
        barrier_loc.structure != vvl::Struct::VkImageMemoryBarrier2) {
        return false;
    }
    auto log_msg = [validation_obj, &objects, &barrier](const Location &stage_loc) {
        const auto &vuid = GetBarrierQueueVUID(stage_loc, QueueError::kHostStage);
        return validation_obj->LogError(vuid, objects, stage_loc,
                                        "is VK_PIPELINE_STAGE_2_HOST_BIT but srcQueueFamilyIndex (%" PRIu32
                                        ") != dstQueueFamilyIndex (%" PRIu32 ").",
                                        barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex);
    };
    bool skip = false;
    if (barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex) {
        if (barrier.srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= log_msg(barrier_loc.dot(vvl::Field::srcStageMask));
        } else if (barrier.dstStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= log_msg(barrier_loc.dot(vvl::Field::dstStageMask));
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

bool CoreChecks::ValidateBarrierQueueFamilies(const LogObjectList &objects, const Location &barrier_loc, const Location &field_loc,
                                              const QueueFamilyBarrier &barrier, const VulkanTypedHandle &handle,
                                              VkSharingMode sharing_mode) const {
    bool skip = false;
    barrier_queue_families::ValidatorState val(this, objects, field_loc, handle, sharing_mode);
    skip |= barrier_queue_families::Validate(this, val, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex);
    skip |= barrier_queue_families::ValidateHostStage(this, objects, barrier_loc, barrier);
    return skip;
}

bool CoreChecks::ValidateBufferBarrier(const LogObjectList &objects, const Location &barrier_loc, const CMD_BUFFER_STATE *cb_state,
                                       const BufferBarrier &mem_barrier) const {
    using sync_vuid_maps::BufferError;
    using sync_vuid_maps::GetBufferBarrierVUID;

    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(barrier_loc, cb_state, mem_barrier, cb_state->qfo_transfer_buffer_barriers);

    // Validate buffer barrier queue family indices
    auto buffer_state = Get<BUFFER_STATE>(mem_barrier.buffer);
    if (buffer_state) {
        auto buf_loc = barrier_loc.dot(Field::buffer);
        const auto &mem_vuid = GetBufferBarrierVUID(buf_loc, BufferError::kNoMemory);
        skip |= ValidateMemoryIsBoundToBuffer(cb_state->commandBuffer(), *buffer_state, buf_loc, mem_vuid.c_str());

        skip |= ValidateBarrierQueueFamilies(objects, barrier_loc, buf_loc, mem_barrier, buffer_state->Handle(),
                                             buffer_state->createInfo.sharingMode);

        auto buffer_size = buffer_state->createInfo.size;
        if (mem_barrier.offset >= buffer_size) {
            auto offset_loc = barrier_loc.dot(Field::offset);
            const auto &vuid = GetBufferBarrierVUID(offset_loc, BufferError::kOffsetTooBig);
            skip |=
                LogError(vuid, objects, offset_loc, "%s has offset 0x%" PRIx64 " which is not less than total size 0x%" PRIx64 ".",
                         FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset), HandleToUint64(buffer_size));
        } else if (mem_barrier.size != VK_WHOLE_SIZE && (mem_barrier.offset + mem_barrier.size > buffer_size)) {
            auto size_loc = barrier_loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeOutOfRange);
            skip |=
                LogError(vuid, objects, size_loc,
                         "%s has offset 0x%" PRIx64 " and size 0x%" PRIx64 " whose sum is greater than total size 0x%" PRIx64 ".",
                         FormatHandle(mem_barrier.buffer).c_str(), HandleToUint64(mem_barrier.offset),
                         HandleToUint64(mem_barrier.size), HandleToUint64(buffer_size));
        }
        if (mem_barrier.size == 0) {
            auto size_loc = barrier_loc.dot(Field::size);
            const auto &vuid = GetBufferBarrierVUID(size_loc, BufferError::kSizeZero);
            skip |= LogError(vuid, objects, barrier_loc, "%s has a size of 0.", FormatHandle(mem_barrier.buffer).c_str());
        }
    }

    if ((mem_barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
         mem_barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT) &&
        (mem_barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
         mem_barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT)) {
        auto loc = barrier_loc.dot(Field::srcQueueFamilyIndex);
        const auto &vuid = GetBufferBarrierVUID(loc, BufferError::kQueueFamilyExternal);
        skip |=
            LogError(vuid, objects, loc,
                     "both srcQueueFamilyIndex and dstQueueFamilyIndex are VK_QUEUE_FAMILY_EXTERNAL/VK_QUEUE_FAMILY_FOREIGN_EXT.");
    }
    return skip;
}

bool CoreChecks::ValidateImageBarrier(const LogObjectList &objects, const Location &barrier_loc, const CMD_BUFFER_STATE *cb_state,
                                      const ImageBarrier &mem_barrier) const {
    bool skip = false;

    skip |= ValidateQFOTransferBarrierUniqueness(barrier_loc, cb_state, mem_barrier, cb_state->qfo_transfer_image_barriers);
    const VkImageLayout old_layout = mem_barrier.oldLayout;
    const VkImageLayout new_layout = mem_barrier.newLayout;

    bool is_ilt = true;
    if (enabled_features.synchronization2) {
        is_ilt = old_layout != new_layout;
    } else {
        if (old_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || old_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadSync2OldLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::oldLayout),
                             "is %s, but the synchronization2 feature was not enabled.", string_VkImageLayout(old_layout));
        }
        if (new_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || new_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadSync2NewLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout),
                             "is %s, but the synchronization2 feature was not enabled.", string_VkImageLayout(new_layout));
        }
    }

    if (is_ilt) {
        if (new_layout == VK_IMAGE_LAYOUT_UNDEFINED || new_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout), "is %s.", string_VkImageLayout(new_layout));
        }
    }

    if (new_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
        if (!enabled_features.attachmentFeedbackLoopLayout) {
            const auto &vuid =
                sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kBadAttFeedbackLoopLayout);
            skip |= LogError(vuid, objects, barrier_loc.dot(Field::newLayout),
                             "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, but the attachmentFeedbackLoopLayout "
                             "feature was not enabled.");
        }
    }

    auto image_data = Get<IMAGE_STATE>(mem_barrier.image);
    if (image_data) {
        auto image_loc = barrier_loc.dot(Field::image);
        // TODO - use LocationVuidAdapter
        const auto &vuid = sync_vuid_maps::GetImageBarrierVUID(barrier_loc, sync_vuid_maps::ImageError::kNoMemory);
        skip |= ValidateMemoryIsBoundToImage(objects, *image_data, image_loc, vuid.c_str());

        skip |= ValidateBarrierQueueFamilies(objects, barrier_loc, image_loc, mem_barrier, image_data->Handle(),
                                             image_data->createInfo.sharingMode);

        skip |= ValidateImageAspectMask(image_data->image(), image_data->createInfo.format, mem_barrier.subresourceRange.aspectMask,
                                        image_data->disjoint, image_loc);

        skip |= ValidateImageBarrierSubresourceRange(barrier_loc.dot(Field::subresourceRange), *image_data,
                                                     mem_barrier.subresourceRange);
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

    // Tracks duplicate layout transition for image barriers.
    // Keeps state between ValidateBarriersToImages calls.
    CommandBufferImageLayoutMap layout_updates_state;

    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        const MemoryBarrier barrier(pMemBarriers[i], src_stage_mask, dst_stage_mask);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier);
    }
    for (uint32_t i = 0; i < imageMemBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        const ImageBarrier barrier(pImageMemBarriers[i], src_stage_mask, dst_stage_mask);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier);
        skip |= ValidateImageBarrier(objects, barrier_loc, cb_state, barrier);
        skip |= ValidateBarriersToImages(barrier_loc, cb_state, barrier, layout_updates_state);
    }
    for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
        const Location barrier_loc = outer_loc.dot(Struct::VkBufferMemoryBarrier, Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(pBufferMemBarriers[i], src_stage_mask, dst_stage_mask);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, barrier);
        skip |= ValidateBufferBarrier(objects, barrier_loc, cb_state, barrier);
    }
    return skip;
}

bool CoreChecks::ValidateDependencyInfo(const LogObjectList &objects, const Location &dep_info_loc,
                                        const CMD_BUFFER_STATE *cb_state, const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;

    // Tracks duplicate layout transition for image barriers.
    // Keeps state between ValidateBarriersToImages calls.
    CommandBufferImageLayoutMap layout_updates_state;

    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        const MemoryBarrier mem_barrier(dep_info->pMemoryBarriers[i]);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, mem_barrier);
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);
        const ImageBarrier mem_barrier(dep_info->pImageMemoryBarriers[i]);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, mem_barrier);
        skip |= ValidateImageBarrier(objects, barrier_loc, cb_state, mem_barrier);
        skip |= ValidateBarriersToImages(barrier_loc, cb_state, mem_barrier, layout_updates_state);
    }
    for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; ++i) {
        const Location barrier_loc = dep_info_loc.dot(Struct::VkBufferMemoryBarrier2, Field::pBufferMemoryBarriers, i);
        const BufferBarrier mem_barrier(dep_info->pBufferMemoryBarriers[i]);
        skip |= ValidateMemoryBarrier(objects, barrier_loc, cb_state, mem_barrier);
        skip |= ValidateBufferBarrier(objects, barrier_loc, cb_state, mem_barrier);
    }

    return skip;
}

bool CoreChecks::ValidatePipelineStageForShaderTileImage(const LogObjectList &objlist, const Location &loc,
                                                         VkPipelineStageFlags2KHR stage_mask, const std::string &vuid) const {
    bool skip = false;
    if (HasNonFramebufferStagePipelineStageFlags(stage_mask)) {
        skip |= LogError(vuid, objlist, loc, "(%s) is restricted to framebuffer space stages (%s).",
                         sync_utils::StringPipelineStageFlags(stage_mask).c_str(),
                         sync_utils::StringPipelineStageFlags(kFramebufferStagePipelineStageFlags).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateAccessMaskForShaderTileImage(const LogObjectList &objlist, const Location &loc,
                                                      VkAccessFlags2KHR access_mask, const std::string &vuid) const {
    bool skip = false;
    if (HasNonShaderTileImageAccessFlags(access_mask)) {
        skip |= LogError(vuid, objlist, loc, "(%s) is not from allowed access mask (%s).",
                         sync_utils::StringAccessFlags(access_mask).c_str(),
                         sync_utils::StringAccessFlags(kShaderTileImageAllowedAccessFlags).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateShaderTileImageBarriers(const LogObjectList &objlist, const Location &outer_loc,
                                                 const VkDependencyInfo &dep_info) const {
    bool skip = false;
    const auto &vuid =
        sync_vuid_maps::GetShaderTileImageVUID(outer_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageBarrierError);

    skip |= ValidateShaderTimeImageCommon(objlist, outer_loc, vuid, dep_info.dependencyFlags, dep_info.bufferMemoryBarrierCount,
                                          dep_info.imageMemoryBarrierCount);

    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; ++i) {
        const Location loc = outer_loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        const auto &mem_barrier = dep_info.pMemoryBarriers[i];
        skip |= ValidatePipelineStageForShaderTileImage(objlist, loc.dot(Field::srcStageMask), mem_barrier.srcStageMask, vuid);
        skip |= ValidatePipelineStageForShaderTileImage(objlist, loc.dot(Field::dstStageMask), mem_barrier.dstStageMask, vuid);
        skip |= ValidateAccessMaskForShaderTileImage(objlist, loc.dot(Field::srcAccessMask), mem_barrier.srcAccessMask, vuid);
        skip |= ValidateAccessMaskForShaderTileImage(objlist, loc.dot(Field::dstAccessMask), mem_barrier.dstAccessMask, vuid);
    }
    return skip;
}

bool CoreChecks::ValidateShaderTileImageBarriers(const LogObjectList &objlist, const Location &outer_loc,
                                                 VkDependencyFlags dependency_flags, uint32_t memory_barrier_count,
                                                 const VkMemoryBarrier *memory_barriers, uint32_t buffer_barrier_count,
                                                 uint32_t image_barrier_count, VkPipelineStageFlags src_stage_mask,
                                                 VkPipelineStageFlags dst_stage_mask) const {
    bool skip = false;
    const auto &vuid =
        sync_vuid_maps::GetShaderTileImageVUID(outer_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageBarrierError);

    skip |= ValidateShaderTimeImageCommon(objlist, outer_loc, vuid, dependency_flags, buffer_barrier_count, image_barrier_count);
    skip |= ValidatePipelineStageForShaderTileImage(objlist, outer_loc.dot(Field::srcStageMask), src_stage_mask, vuid);
    skip |= ValidatePipelineStageForShaderTileImage(objlist, outer_loc.dot(Field::dstStageMask), dst_stage_mask, vuid);

    for (uint32_t i = 0; i < memory_barrier_count; ++i) {
        const Location loc = outer_loc.dot(Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        const auto &mem_barrier = memory_barriers[i];
        skip |= ValidateAccessMaskForShaderTileImage(objlist, loc.dot(Field::srcAccessMask), mem_barrier.srcAccessMask, vuid);
        skip |= ValidateAccessMaskForShaderTileImage(objlist, loc.dot(Field::dstAccessMask), mem_barrier.dstAccessMask, vuid);
    }
    return skip;
}

bool CoreChecks::ValidateShaderTimeImageCommon(const LogObjectList &objlist, const Location &outer_loc,
                                               const std::string &barrier_error_vuid, VkDependencyFlags dependency_flags,
                                               uint32_t buffer_barrier_count, uint32_t image_barrier_count) const {
    bool skip = false;

    // Check shader tile image features
    const bool features_enabled = enabled_features.shaderTileImageColorReadAccess ||
                                  enabled_features.shaderTileImageDepthReadAccess ||
                                  enabled_features.shaderTileImageStencilReadAccess;
    if (!features_enabled) {
        const auto &feature_error_vuid =
            sync_vuid_maps::GetShaderTileImageVUID(outer_loc, sync_vuid_maps::ShaderTileImageError::kShaderTileImageFeatureError);
        skip |= LogError(feature_error_vuid, objlist, outer_loc,
                         "can not be called inside a dynamic rendering instance. This can be fixed by enabling the "
                         "VK_EXT_shader_tile_image features.");
    }

    // Check basic parameter requirements for shader tile image barriers
    if ((dependency_flags & VK_DEPENDENCY_BY_REGION_BIT) != VK_DEPENDENCY_BY_REGION_BIT) {
        skip |= LogError(barrier_error_vuid, objlist, outer_loc.dot(Field::dependencyFlags),
                         "should contain VK_DEPENDENCY_BY_REGION_BIT.");
    }
    if (buffer_barrier_count != 0 || image_barrier_count != 0) {
        skip |= LogError(barrier_error_vuid, objlist, outer_loc, "can only include memory barriers.");
    }
    return skip;
}

bool CoreChecks::ValidateMemoryBarrier(const LogObjectList &objects, const Location &barrier_loc, const CMD_BUFFER_STATE *cb_state,
                                       const MemoryBarrier &barrier) const {
    bool skip = false;
    assert(cb_state);
    auto queue_flags = cb_state->GetQueueFlags();

    const bool is_sync2 =
        IsValueIn(barrier_loc.structure, {Struct::VkMemoryBarrier2, Struct::VkBufferMemoryBarrier2, Struct::VkImageMemoryBarrier2});

    // Validate only Sync2 stages because they are defined for each Sync2 barrier structure.
    // Sync1 stages are the same for all barrier structures and are validated in other place once per top-level API call.
    if (is_sync2) {
        skip |= ValidatePipelineStage(objects, barrier_loc.dot(Field::srcStageMask), queue_flags, barrier.srcStageMask);
        skip |= ValidatePipelineStage(objects, barrier_loc.dot(Field::dstStageMask), queue_flags, barrier.dstStageMask);
    }
    if (!cb_state->IsAcquireOp(barrier)) {
        skip |= ValidateAccessMask(objects, barrier_loc.dot(Field::srcAccessMask), barrier_loc.dot(Field::srcStageMask),
                                   queue_flags, barrier.srcAccessMask, barrier.srcStageMask);
    }
    if (!cb_state->IsReleaseOp(barrier)) {
        skip |= ValidateAccessMask(objects, barrier_loc.dot(Field::dstAccessMask), barrier_loc.dot(Field::dstStageMask),
                                   queue_flags, barrier.dstAccessMask, barrier.dstStageMask);
    }

    if (barrier_loc.function == Func::vkCmdSetEvent2) {
        if (barrier.srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= LogError("VUID-vkCmdSetEvent2-srcStageMask-09391", objects, barrier_loc.dot(Field::srcStageMask),
                             "is VK_PIPELINE_STAGE_2_HOST_BIT.");
        }
        if (barrier.dstStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
            skip |= LogError("VUID-vkCmdSetEvent2-dstStageMask-09392", objects, barrier_loc.dot(Field::dstStageMask),
                             "is VK_PIPELINE_STAGE_2_HOST_BIT.");
        }
    }
    return skip;
}
