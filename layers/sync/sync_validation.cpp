/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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
#include <iostream>
#include <memory>
#include <vector>

#include "sync/sync_error_messages.h"
#include "sync/sync_validation.h"
#include "sync/sync_image.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "utils/convert_utils.h"
#include "utils/ray_tracing_utils.h"
#include "utils/text_utils.h"
#include "vk_layer_config.h"
#include "containers/tls_guard.h"

static bool GetShowStatsEnvVar() {
    // Set environment variable as non zero number to enable stats reporting
    const auto show_stats_str = GetEnvironment("VK_SYNCVAL_SHOW_STATS");
    return !show_stats_str.empty() && std::atoi(show_stats_str.c_str()) != 0;
}

SyncValidator::SyncValidator(vvl::dispatch::Device *dev, syncval::Instance *instance_vo)
    : BaseClass(dev, instance_vo, LayerObjectTypeSyncValidation), error_messages_(*this), report_stats_(GetShowStatsEnvVar()) {}

SyncValidator::~SyncValidator() {
    // Instance level SyncValidator does not have much to say
    const bool device_validation_object = (device != nullptr);

    if (device_validation_object && report_stats_) {
        stats.ReportOnDestruction();
    }
}

// Location to add per-queue submit debug info if built with -D DEBUG_CAPTURE_KEYBOARD=ON.
void SyncValidator::DebugCapture() {
    if (report_stats_) {
        const std::string report = stats.CreateReport();
        std::cout << report;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        OutputDebugString(report.c_str());
#endif
    }
}

bool SyncValidator::SyncError(SyncHazard hazard, const LogObjectList &objlist, const Location &loc,
                              const std::string &error_message) const {
    return LogError(string_SyncHazardVUID(hazard), objlist, loc, "%s", error_message.c_str());
}

ResourceUsageRange SyncValidator::ReserveGlobalTagRange(size_t tag_count) const {
    ResourceUsageRange reserve;
    reserve.begin = tag_limit_.fetch_add(tag_count);
    reserve.end = reserve.begin + tag_count;
    return reserve;
}

void SyncValidator::EnsureTimelineSignalsLimit(uint32_t signals_per_queue_limit, QueueId queue) {
    for (auto &[_, signals] : timeline_signals_) {
        const size_t initial_signal_count = signals.size();
        vvl::unordered_map<QueueId, uint32_t> signals_per_queue;
        for (const SignalInfo &signal : signals) {
            ++signals_per_queue[signal.first_scope.queue];
        }
        const bool filter_queue = queue != kQueueIdInvalid;
        for (auto it = signals.begin(); it != signals.end();) {
            if (filter_queue && it->first_scope.queue != queue) {
                ++it;
                continue;
            }
            auto &counter = signals_per_queue[it->first_scope.queue];
            if (counter > signals_per_queue_limit) {
                it = signals.erase(it);
                --counter;
            } else {
                ++it;
            }
        }
        stats.RemoveTimelineSignals(uint32_t(initial_signal_count - signals.size()));
    }
}

void SyncValidator::ApplySignalsUpdate(SignalsUpdate &update, const QueueBatchContext::Ptr &last_batch) {
    // NOTE: All conserved QueueBatchContexts need to have their access logs reset to use the global
    // logger and the only conserved QBCs are those referenced by unwaited signals and the last batch.

    for (auto &signal_entry : update.binary_signal_requests) {
        auto &signal_batch = signal_entry.second.batch;
        // Batches retained for signalled semaphore don't need to retain
        // event data, unless it's the last batch in the submit
        if (signal_batch != last_batch) {
            signal_batch->ResetEventsContext();
            // Make sure that retained batches are minimal, and trim
            // after the events contexts has been cleared.
            signal_batch->Trim();
        }
        const VkSemaphore semaphore = signal_entry.first;
        SignalInfo &signal_info = signal_entry.second;
        binary_signals_.insert_or_assign(semaphore, std::move(signal_info));
    }
    for (VkSemaphore semaphore : update.binary_unsignal_requests) {
        binary_signals_.erase(semaphore);
    }
    for (auto &[semaphore, new_signals] : update.timeline_signals) {
        std::vector<SignalInfo> &signals = timeline_signals_[semaphore];
        vvl::Append(signals, new_signals);
        stats.AddTimelineSignals((uint32_t)new_signals.size());

        // Update host sync points
        std::deque<TimelineHostSyncPoint> &host_sync_points = host_waitable_semaphores_[semaphore];
        for (SignalInfo &new_signal : new_signals) {
            if (new_signal.batch) {
                // The lifetimes of the semaphore host sync points are managed by vkWaitSemaphores.
                // kMaxTimelineHostSyncPoints limit is used when the program does not use vkWaitSemaphores.
                // We accumulate up to kMaxTimelineHostSyncPoints of the host sync points per semaphore.
                // Dropping old sync points cannot introduce false positives but may miss a sync hazard.
                // The limit is chosen to be large enough comparing to typical numbers of queue submissions
                // between host synchronization points.
                const uint32_t kMaxTimelineHostSyncPoints = 256;  // max ~6 Kb per semaphore
                if (host_sync_points.size() >= kMaxTimelineHostSyncPoints) {
                    host_sync_points.pop_front();
                }
                // Add a host sync point for this signal
                TimelineHostSyncPoint sync_point;
                assert(new_signal.first_scope.queue != kQueueIdInvalid);
                sync_point.queue_id = new_signal.first_scope.queue;
                sync_point.tag = new_signal.batch->GetTagRange().end - 1;
                sync_point.timeline_value = new_signal.timeline_value;
                host_sync_points.emplace_back(sync_point);
            }
        }
    }
    for (const auto &remove_signals_request : update.remove_timeline_signals_requests) {
        auto &signals = timeline_signals_[remove_signals_request.semaphore];
        for (auto it = signals.begin(); it != signals.end();) {
            const SignalInfo &signal = *it;
            if (signal.first_scope.queue == remove_signals_request.queue &&
                signal.timeline_value < remove_signals_request.signal_threshold_value) {
                it = signals.erase(it);
                stats.RemoveTimelineSignals(1);
                continue;
            }
            ++it;
        }
    }

    // Enforce max signals limit in case timeline is signaled multiple times and never/rarely is waited on.
    // This does not introduce errors/false-positives (check EnsureTimelineSignalsLimit documentation)
    const uint32_t kMaxTimelineSignalsPerQueue = 100;
    EnsureTimelineSignalsLimit(kMaxTimelineSignalsPerQueue);
}

void SyncValidator::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    auto tagged_wait_op = [queue_id, tag](const QueueBatchContext::Ptr &batch) {
        batch->ApplyTaggedWait(queue_id, tag);
        batch->Trim();

        // If there is a *pending* last batch then apply tagged wait for its accesses too.
        // A pending last batch might exist if this wait was initiated between QueueSubmit's
        // Validate and Record phases (from a different thread). The pending last batch might
        // contain *imported* accesses that are in the scope of this wait.
        auto batch_queue_state = batch->GetQueueSyncState();
        auto pending_batch = batch_queue_state ? batch_queue_state->PendingLastBatch() : nullptr;
        if (pending_batch) {
            pending_batch->ApplyTaggedWait(queue_id, tag);
            pending_batch->Trim();
        }
    };
    ForAllQueueBatchContexts(tagged_wait_op);
}

void SyncValidator::ApplyAcquireWait(const AcquiredImage &acquired) {
    auto acq_wait_op = [&acquired](const QueueBatchContext::Ptr &batch) {
        batch->ApplyAcquireWait(acquired);
        batch->Trim();
    };
    ForAllQueueBatchContexts(acq_wait_op);
}

template <typename BatchOp>
void SyncValidator::ForAllQueueBatchContexts(BatchOp &&op) {
    // Get last batch from each queue
    std::vector<QueueBatchContext::Ptr> batch_contexts = GetLastBatches([](auto) { return true; });

    // Get batches from binary signals
    for (auto &[_, signal] : binary_signals_) {
        if (!vvl::Contains(batch_contexts, signal.batch)) {
            batch_contexts.emplace_back(signal.batch);
        }
    }
    // Get batches from timeline signals
    for (auto &[_, signals] : timeline_signals_) {
        for (const auto &signal : signals) {
            if (signal.batch && !vvl::Contains(batch_contexts, signal.batch)) {
                batch_contexts.emplace_back(signal.batch);
            }
        }
    }
    // Get present batches
    device_state->ForEachShared<vvl::Swapchain>([&batch_contexts](const std::shared_ptr<vvl::Swapchain> &swapchain) {
        auto &sync_swapchain = syncval_state::SubState(*swapchain);
        sync_swapchain.GetPresentBatches(batch_contexts);
    });

    // Note: The const is to force the reference to const be on all platforms.
    //
    // It's not obivious (nor cross platform consitent), that the batch reference should be const
    // but since it's pointing to the actual *key* for the set it must be. This doesn't make the
    // object the shared pointer is referencing constant however.
    for (const auto &batch : batch_contexts) {
        op(batch);
    }
}

void SyncValidator::UpdateFenceHostSyncPoint(VkFence fence, FenceHostSyncPoint &&sync_point) {
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    if (!vvl::StateObject::Invalid(fence_state)) {
        waitable_fences_[fence_state->VkHandle()] = std::move(sync_point);
    }
}

void SyncValidator::WaitForFence(VkFence fence) {
    auto fence_it = waitable_fences_.find(fence);
    if (fence_it != waitable_fences_.end()) {
        // The fence may no longer be waitable for several valid reasons.
        FenceHostSyncPoint &wait_for = fence_it->second;
        if (wait_for.acquired.Invalid()) {
            // This is just a normal fence wait
            ApplyTaggedWait(wait_for.queue_id, wait_for.tag);
        } else {
            // This a fence wait for a present operation
            ApplyAcquireWait(wait_for.acquired);
        }
        waitable_fences_.erase(fence_it);
    }
}

void SyncValidator::WaitForSemaphore(VkSemaphore semaphore, uint64_t value) {
    std::deque<TimelineHostSyncPoint> *sync_points = vvl::Find(host_waitable_semaphores_, semaphore);
    if (!sync_points) {
        return;
    }
    auto matching_sync_point = [value](const TimelineHostSyncPoint &sync_point) { return sync_point.timeline_value >= value; };
    auto sync_point_it = std::find_if(sync_points->begin(), sync_points->end(), matching_sync_point);
    if (sync_point_it == sync_points->end()) {
        return;
    }

    const TimelineHostSyncPoint &sync_point = *sync_point_it;
    ApplyTaggedWait(sync_point.queue_id, sync_point.tag);

    // Remove signals before the resolving one (keep the resolving signal).
    std::vector<SignalInfo> &signals = timeline_signals_[semaphore];
    const size_t initial_signal_count = signals.size();
    vvl::erase_if(signals, [&sync_point](SignalInfo &signal) {
        return signal.first_scope.queue == sync_point.queue_id && signal.timeline_value < sync_point.timeline_value;
    });
    stats.RemoveTimelineSignals(uint32_t(initial_signal_count - signals.size()));

    // We can remove all sync points that are in the scope of current wait.
    // Subsequent attempts to synchronize on the host with already synchronized
    // timeline values will result in noop.
    sync_points->erase(sync_points->begin(), sync_point_it + 1 /* include resolving sync point too*/);
}

void SyncValidator::UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos) {
    for (const auto &info : vvl::make_span(infos, count)) {
        if (VK_NULL_HANDLE == info.image) continue;
        auto image_state = Get<vvl::Image>(info.image);

        // Need to protect if some VkBindMemoryStatus are not VK_SUCCESS
        if (!image_state->HasBeenBound()) continue;

        auto &sub_state = syncval_state::SubState(*image_state);
        if (sub_state.IsTiled()) {
            sub_state.SetOpaqueBaseAddress(*device_state);
        }
    }
}

std::shared_ptr<const QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) const {
    for (const auto &queue_sync_state : queue_sync_states_) {
        if (queue_sync_state->GetQueueState()->VkHandle() == queue) {
            return queue_sync_state;
        }
    }
    return {};
}

void SyncValidator::Created(vvl::CommandBuffer &cb_state) {
    cb_state.SetSubState(container_type, std::make_unique<syncval_state::CommandBufferSubState>(*this, cb_state));
}

void SyncValidator::Created(vvl::Swapchain &swapchain_state) {
    swapchain_state.SetSubState(container_type, std::make_unique<syncval_state::SwapchainSubState>(swapchain_state));
}

void SyncValidator::Created(vvl::Image &image_state) {
    image_state.SetSubState(container_type, std::make_unique<syncval_state::ImageSubState>(image_state));
}

void SyncValidator::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator,
                                               const RecordObject &record_obj) {
    if (const auto buffer_state = Get<vvl::Buffer>(buffer)) {
        const VkDeviceSize base_address = ResourceBaseAddress(*buffer_state);
        const ResourceAccessRange buffer_range(base_address, base_address + buffer_state->create_info.size);
        auto batch_op = [&buffer_range](const QueueBatchContext::Ptr &batch) {
            batch->OnResourceDestroyed(buffer_range);
            batch->Trim();
        };
        ForAllQueueBatchContexts(batch_op);
    }
}

void SyncValidator::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    if (const auto image_state = Get<vvl::Image>(image)) {
        auto batch_op = [&image_state](const QueueBatchContext::Ptr &batch) {
            const auto &sub_state = syncval_state::SubState(*image_state);
            ImageRangeGen range_gen = sub_state.MakeImageRangeGen(image_state->full_range, false);
            for (; range_gen->non_empty(); ++range_gen) {
                const ResourceAccessRange subresource_range = *range_gen;
                batch->OnResourceDestroyed(subresource_range);
            }
            batch->Trim();
        };
        ForAllQueueBatchContexts(batch_op);
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) {
        return false;
    }

    const auto *cb_context = syncval_state::AccessContext(*cb_state);
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcBuffer);
                const std::string error = error_messages_.BufferCopyError(hazard, *cb_context, error_obj.location.function,
                                                                          FormatHandle(srcBuffer), region_index, src_range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstBuffer);
                const std::string error = error_messages_.BufferCopyError(hazard, *cb_context, error_obj.location.function,
                                                                          FormatHandle(dstBuffer), region_index, dst_range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(pCopyBufferInfo->dstBuffer);

    for (const auto [region_index, copy_region] : vvl::enumerate(pCopyBufferInfo->pRegions, pCopyBufferInfo->regionCount)) {
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                // TODO -- add tag information to log msg when useful.
                // TODO: there are no tests for this error
                const LogObjectList objlist(commandBuffer, pCopyBufferInfo->srcBuffer);
                const std::string error =
                    error_messages_.BufferCopyError(hazard, *cb_context, error_obj.location.function,
                                                    FormatHandle(pCopyBufferInfo->srcBuffer), region_index, src_range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                // TODO: there are no tests for this error
                const LogObjectList objlist(commandBuffer, pCopyBufferInfo->dstBuffer);
                const std::string error =
                    error_messages_.BufferCopyError(hazard, *cb_context, error_obj.location.function,
                                                    FormatHandle(pCopyBufferInfo->dstBuffer), region_index, dst_range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<vvl::Image>(srcImage);
    auto dst_image = Get<vvl::Image>(dstImage);
    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(srcImage), region_index,
                    copy_region.srcOffset, copy_region.extent, copy_region.srcSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(dstImage), region_index,
                    copy_region.dstOffset, copy_region.extent, copy_region.dstSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<vvl::Image>(pCopyImageInfo->srcImage);
    auto dst_image = Get<vvl::Image>(pCopyImageInfo->dstImage);

    for (const auto [region_index, copy_region] : vvl::enumerate(pCopyImageInfo->pRegions, pCopyImageInfo->regionCount)) {
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pCopyImageInfo->srcImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(pCopyImageInfo->srcImage), region_index,
                    copy_region.srcOffset, copy_region.extent, copy_region.srcSubresource);
                // TODO: this error not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pCopyImageInfo->dstImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(pCopyImageInfo->dstImage), region_index,
                    copy_region.dstOffset, copy_region.extent, copy_region.dstSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask,
                                           dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                           pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    skip |= pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(
        record_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask, dstStageMask, memoryBarrierCount,
        pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                           *pDependencyInfo);
    skip |= pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                         const RecordObject &record_obj) {
    PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                      const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::FinishDeviceSetup(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // The state tracker sets up the device state
    BaseClass::FinishDeviceSetup(pCreateInfo, loc);

    // Returns queues in the same order as advertised by the driver.
    // This allows to have deterministic QueueId between runs that simplifies debugging.
    auto get_sorted_queues = [this]() {
        std::vector<std::shared_ptr<vvl::Queue>> queues;
        device_state->ForEachShared<vvl::Queue>(
            [&queues](const std::shared_ptr<vvl::Queue> &queue) { queues.emplace_back(queue); });
        std::sort(queues.begin(), queues.end(), [](const auto &q1, const auto &q2) {
            return (q1->queue_family_index < q2->queue_family_index) ||
                   (q1->queue_family_index == q2->queue_family_index && q1->queue_index < q2->queue_index);
        });
        return queues;
    };
    queue_sync_states_.reserve(device_state->Count<vvl::Queue>());
    for (const auto &queue : get_sorted_queues()) {
        queue_sync_states_.emplace_back(std::make_shared<QueueSyncState>(queue, queue_id_limit_++));
    }

    const auto env_debug_command_number = GetEnvironment("VK_SYNCVAL_DEBUG_COMMAND_NUMBER");
    if (!env_debug_command_number.empty()) {
        debug_command_number = static_cast<uint32_t>(std::stoul(env_debug_command_number));
    }
    const auto env_debug_reset_count = GetEnvironment("VK_SYNCVAL_DEBUG_RESET_COUNT");
    if (!env_debug_reset_count.empty()) {
        debug_reset_count = static_cast<uint32_t>(std::stoul(env_debug_reset_count));
    }
    debug_cmdbuf_pattern = GetEnvironment("VK_SYNCVAL_DEBUG_CMDBUF_PATTERN");
    text::ToLower(debug_cmdbuf_pattern);
}

void SyncValidator::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                               const RecordObject &record_obj) {
    queue_sync_states_.clear();
    binary_signals_.clear();
    timeline_signals_.clear();
    waitable_fences_.clear();
    host_waitable_semaphores_.clear();
}

void SyncValidator::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                  const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    assert(!vvl::Contains(timeline_signals_, *pSemaphore));
}

void SyncValidator::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                                  const RecordObject &record_obj) {
    if (auto sem_state = Get<vvl::Semaphore>(semaphore); sem_state && (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE)) {
        if (auto it = timeline_signals_.find(semaphore); it != timeline_signals_.end()) {
            stats.RemoveTimelineSignals((uint32_t)it->second.size());
            timeline_signals_.erase(it);
        }
    }
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    if (cb_state) {
        SyncOpBeginRenderPass sync_op(error_obj.location.function, *this, pRenderPassBegin, pSubpassBeginInfo);
        skip |= sync_op.Validate(*syncval_state::AccessContext(*cb_state));
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject &error_obj) const {
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    return ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                       const ErrorObject &error_obj) const {
    return ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);
    SyncOpNextSubpass sync_op(error_obj.location.function, *this, pSubpassBeginInfo, pSubpassEndInfo);
    return sync_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                  const ErrorObject &error_obj) const {
    // Convert to a NextSubpass2
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper();
    return ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, &subpass_end_info, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                      const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    SyncOpEndRenderPass sync_op(error_obj.location.function, *this, pSubpassEndInfo);
    skip |= sync_op.Validate(*cb_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return ValidateCmdEndRenderPass(commandBuffer, nullptr, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                     const ErrorObject &error_obj) const {
    return ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

// Simple heuristic rule to detect WAW operations representing algorithmically safe or increment
// updates to a resource which do not conflict at the byte level.
// TODO: Revisit this rule to see if it needs to be tighter or looser
// TODO: Add programatic control over suppression heuristics
bool SyncValidator::SuppressedBoundDescriptorWAW(const HazardResult &hazard) const {
    assert(hazard.IsHazard());
    return hazard.IsWAWHazard();
}

bool SyncValidator::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state || !pRenderingInfo) return skip;

    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state(&skip, std::move(cb_state));
    cmd_state->AddRenderingInfo(*this, *pRenderingInfo);

    // We need to set skip, because the TlsGuard destructor is looking at the skip value for RAII cleanup.
    skip |= syncval_state::AccessContext(*cmd_state->cb_state)->ValidateBeginRendering(error_obj, *cmd_state);
    return skip;
}

void SyncValidator::PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                       const RecordObject &record_obj) {
    PostCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                    const RecordObject &record_obj) {
    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state;

    assert(cmd_state && cmd_state->cb_state && (cmd_state->cb_state->VkHandle() == commandBuffer));
    // Note: for fine grain locking need to to something other than cast.
    auto cb_state = std::const_pointer_cast<vvl::CommandBuffer>(cmd_state->cb_state);
    syncval_state::AccessContext(*cb_state)->RecordBeginRendering(*cmd_state, record_obj.location);
}

bool SyncValidator::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip |= syncval_state::AccessContext(*cb_state)->ValidateEndRendering(error_obj);
    return skip;
}

void SyncValidator::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void SyncValidator::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;

    syncval_state::AccessContext(*cb_state)->RecordEndRendering(record_obj);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_image = Get<vvl::Image>(dstImage);

    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        HazardResult hazard;
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range =
                    MakeRange(copy_region.bufferOffset, dst_image->GetBufferSizeFromCopyImage(copy_region));
                hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
                if (hazard.IsHazard()) {
                    // PHASE1 TODO -- add tag information to log msg when useful.
                    const LogObjectList objlist(commandBuffer, srcBuffer);
                    const std::string error = error_messages_.BufferCopyError(hazard, *cb_access_context, loc.function,
                                                                              FormatHandle(srcBuffer), region_index, src_range);
                    skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                }
            }

            hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                           copy_region.imageExtent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, loc.function, FormatHandle(dstImage), region_index, copy_region.imageOffset,
                    copy_region.imageExtent, copy_region.imageSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (skip) break;
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, error_obj.location.dot(Field::pCopyBufferToImageInfo));
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<vvl::Image>(srcImage);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    const VkDeviceMemory dst_memory = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemoryState()->VkHandle() : VK_NULL_HANDLE;
    for (const auto [region_index, copy_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                                copy_region.imageExtent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, loc.function, FormatHandle(srcImage), region_index, copy_region.imageOffset,
                    copy_region.imageExtent, copy_region.imageSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (dst_memory != VK_NULL_HANDLE) {
                ResourceAccessRange dst_range =
                    MakeRange(copy_region.bufferOffset, src_image->GetBufferSizeFromCopyImage(copy_region));
                hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
                if (hazard.IsHazard()) {
                    const LogObjectList objlist(commandBuffer, dstBuffer);
                    const std::string error = error_messages_.BufferCopyError(hazard, *cb_access_context, loc.function,
                                                                              FormatHandle(dstBuffer), region_index, dst_range);
                    skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                }
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, error_obj.location.dot(Field::pCopyImageToBufferInfo));
}

template <typename RegionType>
bool SyncValidator::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<vvl::Image>(srcImage);
    auto dst_image = Get<vvl::Image>(dstImage);

    for (const auto [region_index, blit_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(blit_region.srcSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const std::string error =
                    error_messages_.ImageCopyResolveBlitError(hazard, *cb_access_context, loc.function, FormatHandle(srcImage),
                                                              region_index, offset, extent, blit_region.srcSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }

        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(blit_region.dstSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const std::string error =
                    error_messages_.ImageCopyResolveBlitError(hazard, *cb_access_context, loc.function, FormatHandle(dstImage),
                                                              region_index, offset, extent, blit_region.dstSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                error_obj.location);
}

bool SyncValidator::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, error_obj.location.dot(Field::pBlitImageInfo));
}

bool SyncValidator::ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                           const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                           const uint32_t drawCount, const uint32_t stride, const Location &loc) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    auto buf_state = Get<vvl::Buffer>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_context.GetCBState().Handle(), buf_state->Handle());
            const std::string resource_description = "indirect " + FormatHandle(buffer);
            const auto error = error_messages_.BufferError(hazard, cb_context, loc.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), objlist, loc, error);
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(cb_context.GetCBState().Handle(), buf_state->Handle());
                const std::string resource_description = "indirect " + FormatHandle(buffer);
                const auto error = error_messages_.BufferError(hazard, cb_context, loc.function, resource_description, range);
                skip |= SyncError(hazard.Hazard(), objlist, loc, error);
                break;
            }
        }
    }
    return skip;
}

void SyncValidator::RecordIndirectBuffer(CommandBufferAccessContext &cb_context, const ResourceUsageTag tag,
                                         const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                         const uint32_t drawCount, uint32_t stride) {
    auto buf_state = Get<vvl::Buffer>(buffer);
    auto tag_ex = buf_state ? cb_context.AddCommandHandle(tag, buf_state->Handle()) : ResourceUsageTagEx{tag};

    VkDeviceSize size = struct_size;
    AccessContext &context = *cb_context.GetCurrentAccessContext();
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                  tag_ex);
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                      tag_ex);
        }
    }
}

bool SyncValidator::ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context, VkBuffer buffer,
                                        VkDeviceSize offset, const Location &loc) const {
    bool skip = false;

    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.IsHazard()) {
        const LogObjectList objlist(cb_context.GetCBState().Handle(), count_buf_state->Handle());
        const std::string resource_description = "draw count " + FormatHandle(buffer);
        const auto error = error_messages_.BufferError(hazard, cb_context, loc.function, resource_description, range);
        skip |= SyncError(hazard.Hazard(), objlist, loc, error);
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(CommandBufferAccessContext &cb_context, const ResourceUsageTag tag, VkBuffer buffer,
                                      VkDeviceSize offset) {
    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, count_buf_state->Handle());
    AccessContext &context = *cb_context.GetCurrentAccessContext();
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                              tag_ex);
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    skip |= syncval_state::AccessContext(*cb_state)->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE,
                                                                                       error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                              const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_context, *context, sizeof(VkDispatchIndirectCommand), buffer, offset, 1,
                                   sizeof(VkDispatchIndirectCommand), error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDispatchIndirectCommand), buffer, offset, 1,
                         sizeof(VkDispatchIndirectCommand));
}

bool SyncValidator::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    skip |= syncval_state::AccessContext(*cb_state)->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE,
                                                                                       error_obj.location);
    return skip;
}

bool SyncValidator::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                      uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                          error_obj);
}

void SyncValidator::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                  uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto cb_access_context = syncval_state::AccessContext(*cb_state);
    const ResourceUsageTag tag = cb_access_context->NextCommandTag(record_obj.location.function);
    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

void SyncValidator::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                     uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                     uint32_t groupCountZ, const RecordObject &record_obj) {
    PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                  record_obj);
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertex(vertexCount, firstVertex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                          uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertexIndex(indexCount, firstIndex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                 uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                 const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride,
                                   error_obj.location);
    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    if (drawCount == 0) return;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertex(?, ?, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount,
                                   stride, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertexIndex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertexIndex(?, ?, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndirectCommand), buffer, offset, maxDrawCount,
                                   stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, countBuffer, countBufferOffset, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, Func command) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(command);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*cb_access_context, tag, countBuffer, countBufferOffset);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertex(?, ?, tag);
}

void SyncValidator::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject &record_obj) {
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               record_obj.location.function);
}
bool SyncValidator::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, maxDrawCount,
                                   stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, countBuffer, countBufferOffset, error_obj.location);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // skip |= cb_access_context->ValidateDrawVertexIndex(?, ?, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, Func command) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(command);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*cb_access_context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*cb_access_context, tag, countBuffer, countBufferOffset);

    // TODO: Shader instrumentation support is needed to read indirect buffer content (new syncval mode)
    // cb_access_context->RecordDrawVertexIndex(?, ?, tag);
}

void SyncValidator::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride,
                                                              const RecordObject &record_obj) {
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject &record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    if (auto image_state = Get<vvl::Image>(image)) {
        for (const auto [range_index, range] : vvl::enumerate(pRanges, rangeCount)) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, image);
                const auto error = error_messages_.ImageClearError(hazard, *cb_access_context, error_obj.location.function,
                                                                   FormatHandle(image), range_index, range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    if (auto image_state = Get<vvl::Image>(image)) {
        for (const auto [range_index, range] : vvl::enumerate(pRanges, rangeCount)) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, image);
                const auto error = error_messages_.ImageClearError(hazard, *cb_access_context, error_obj.location.function,
                                                                   FormatHandle(image), range_index, range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                       const VkClearRect *pRects, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);

    for (const VkClearAttachment &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto [rect_index, rect] : vvl::enumerate(pRects, rectCount)) {
            skip |=
                syncval_state::AccessContext(*cb_state)->ValidateClearAttachment(error_obj.location, attachment, rect_index, rect);
        }
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer && queryCount > 0) {
        const uint32_t query_size = (flags & VK_QUERY_RESULT_64_BIT) ? 8 : 4;
        const VkDeviceSize range_size = (queryCount - 1) * stride + query_size;
        const ResourceAccessRange range = MakeRange(dstOffset, range_size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, queryPool, dstBuffer);
            const std::string resource_description = "dstBuffer " + FormatHandle(dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }

    // TODO:Track VkQueryPool
    return skip;
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            const std::string resource_description = "dstBuffer " + FormatHandle(dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<vvl::Image>(srcImage);
    auto dst_image = Get<vvl::Image>(dstImage);

    for (const auto [region_index, resolve_region] : vvl::enumerate(pRegions, regionCount)) {
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, srcImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(srcImage), region_index,
                    resolve_region.srcOffset, resolve_region.extent, resolve_region.srcSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dstImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(dstImage), region_index,
                    resolve_region.dstOffset, resolve_region.extent, resolve_region.dstSubresource);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const Location image_info_loc = error_obj.location.dot(Field::pResolveImageInfo);
    auto src_image = Get<vvl::Image>(pResolveImageInfo->srcImage);
    auto dst_image = Get<vvl::Image>(pResolveImageInfo->dstImage);

    for (const auto [region_index, resolve_region] : vvl::enumerate(pResolveImageInfo->pRegions, pResolveImageInfo->regionCount)) {
        const Location region_loc = image_info_loc.dot(Field::pRegions, region_index);
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pResolveImageInfo->srcImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(pResolveImageInfo->srcImage),
                    region_index, resolve_region.srcOffset, resolve_region.extent, resolve_region.srcSubresource);
                // TODO: this error is not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, region_loc, error);
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, pResolveImageInfo->dstImage);
                const std::string error = error_messages_.ImageCopyResolveBlitError(
                    hazard, *cb_access_context, error_obj.location.function, FormatHandle(pResolveImageInfo->dstImage),
                    region_index, resolve_region.dstOffset, resolve_region.extent, resolve_region.dstSubresource);
                // TODO: this error is not covered by the test
                skip |= SyncError(hazard.Hazard(), objlist, region_loc, error);
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_CLEAR_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            const std::string resource_description = "dstBuffer " + FormatHandle(dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const std::string resource_description = "dstBuffer " + FormatHandle(dstBuffer);
            const auto error =
                error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), dstBuffer, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    auto src_buffer = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange);
        auto hazard = context->DetectHazard(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, src_range);
        if (hazard.IsHazard()) {
            const std::string resource_description = "bitstream buffer " + FormatHandle(pDecodeInfo->srcBuffer);
            // TODO: there are no tests for this error
            const auto error = error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function,
                                                           resource_description, src_range);
            skip |= SyncError(hazard.Hazard(), src_buffer->Handle(), error_obj.location, error);
        }
    }

    auto dst_resource = vvl::VideoPictureResource(*device_state, pDecodeInfo->dstPictureResource);
    if (dst_resource) {
        auto hazard = context->DetectHazard(*vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
        if (hazard.IsHazard()) {
            std::stringstream ss;
            ss << "decode output picture ";
            ss << Location(Func::Empty, Field::pDecodeInfo).dot(Field::dstPictureResource).Fields();
            ss << " ";
            FormatVideoPictureResouce(*this, pDecodeInfo->dstPictureResource, ss);
            const std::string resouce_description = ss.str();
            const std::string error =
                error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resouce_description);
            skip |= SyncError(hazard.Hazard(), dst_resource.image_view_state->Handle(), error_obj.location, error);
        }
    }

    if (pDecodeInfo->pSetupReferenceSlot != nullptr && pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        const VkVideoPictureResourceInfoKHR &video_picture = *pDecodeInfo->pSetupReferenceSlot->pPictureResource;
        auto setup_resource = vvl::VideoPictureResource(*device_state, video_picture);
        if (setup_resource && (setup_resource != dst_resource)) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << "reconstructed picture ";
                ss << Location(Func::Empty, Field::pDecodeInfo)
                          .dot(Field::pSetupReferenceSlot)
                          .dot(Field::pPictureResource)
                          .Fields();
                ss << " ";
                FormatVideoPictureResouce(*this, video_picture, ss);
                const std::string resouce_description = ss.str();
                const std::string error =
                    error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resouce_description);
                skip |= SyncError(hazard.Hazard(), setup_resource.image_view_state->Handle(), error_obj.location, error);
            }
        }
    }

    for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
        if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            const VkVideoPictureResourceInfoKHR &video_picture = *pDecodeInfo->pReferenceSlots[i].pPictureResource;
            auto reference_resource = vvl::VideoPictureResource(*device_state, video_picture);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ);
                if (hazard.IsHazard()) {
                    std::stringstream ss;
                    ss << "reference picture " << i << " ";
                    ss << Location(Func::Empty, Field::pDecodeInfo)
                              .dot(Field::pReferenceSlots, i)
                              .dot(Field::pPictureResource)
                              .Fields();
                    ss << " ";
                    FormatVideoPictureResouce(*this, video_picture, ss);
                    const std::string resouce_description = ss.str();
                    const std::string error =
                        error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resouce_description);
                    skip |= SyncError(hazard.Hazard(), reference_resource.image_view_state->Handle(), error_obj.location, error);
                }
            }
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    auto dst_buffer = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (dst_buffer) {
        const ResourceAccessRange dst_range = MakeRange(*dst_buffer, pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, dst_range);
        if (hazard.IsHazard()) {
            const std::string resource_description = "bitstream buffer " + FormatHandle(pEncodeInfo->dstBuffer);
            const auto error = error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function,
                                                           resource_description, dst_range);
            skip |= SyncError(hazard.Hazard(), dst_buffer->Handle(), error_obj.location, error);
        }
    }

    if (auto src_resource = vvl::VideoPictureResource(*device_state, pEncodeInfo->srcPictureResource)) {
        auto hazard = context->DetectHazard(*vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
        if (hazard.IsHazard()) {
            std::stringstream ss;
            ss << "encode input picture ";
            ss << Location(Func::Empty, Field::pEncodeInfo).dot(Field::srcPictureResource).Fields();
            ss << " ";
            FormatVideoPictureResouce(*this, pEncodeInfo->srcPictureResource, ss);
            const std::string resouce_description = ss.str();
            // TODO: there are no tests for this error
            const std::string error =
                error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resouce_description);
            skip |= SyncError(hazard.Hazard(), src_resource.image_view_state->Handle(), error_obj.location, error);
        }
    }

    if (pEncodeInfo->pSetupReferenceSlot != nullptr && pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        const VkVideoPictureResourceInfoKHR &video_picture = *pEncodeInfo->pSetupReferenceSlot->pPictureResource;
        auto setup_resource = vvl::VideoPictureResource(*device_state, video_picture);
        if (setup_resource) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE);
            if (hazard.IsHazard()) {
                std::stringstream ss;
                ss << "reconstructed picture ";
                ss << Location(Func::Empty, Field::pEncodeInfo)
                          .dot(Field::pSetupReferenceSlot)
                          .dot(Field::pPictureResource)
                          .Fields();
                ss << " ";
                FormatVideoPictureResouce(*this, video_picture, ss);
                const std::string resouce_description = ss.str();
                const std::string error =
                    error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resouce_description);
                skip |= SyncError(hazard.Hazard(), setup_resource.image_view_state->Handle(), error_obj.location, error);
            }
        }
    }

    for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
        if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            const VkVideoPictureResourceInfoKHR &video_picture = *pEncodeInfo->pReferenceSlots[i].pPictureResource;
            auto reference_resource = vvl::VideoPictureResource(*device_state, video_picture);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
                if (hazard.IsHazard()) {
                    std::stringstream ss;
                    ss << "reference picture " << i << " ";
                    ss << Location(Func::Empty, Field::pEncodeInfo)
                              .dot(Field::pReferenceSlots, i)
                              .dot(Field::pPictureResource)
                              .Fields();
                    ss << " ";
                    FormatVideoPictureResouce(*this, video_picture, ss);
                    const std::string resource_description = ss.str();
                    const std::string error =
                        error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resource_description);
                    skip |= SyncError(hazard.Hazard(), reference_resource.image_view_state->Handle(), error_obj.location, error);
                }
            }
        }
    }

    if (pEncodeInfo->flags & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
        auto quantization_map_info = vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapInfoKHR>(pEncodeInfo->pNext);
        if (quantization_map_info) {
            auto image_view_state = Get<vvl::ImageView>(quantization_map_info->quantizationMap);
            if (image_view_state) {
                VkOffset3D offset = {0, 0, 0};
                VkExtent3D extent = {quantization_map_info->quantizationMapExtent.width,
                                     quantization_map_info->quantizationMapExtent.height, 1};
                auto hazard = context->DetectHazard(*image_view_state, offset, extent, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ,
                                                    SyncOrdering::kOrderingNone);
                if (hazard.IsHazard()) {
                    std::stringstream ss;
                    ss << "quantization map ";
                    ss << Location(Func::Empty, Field::pEncodeInfo).dot(Field::quantizationMap).Fields();
                    ss << " ";
                    FormatVideoQuantizationMap(*this, *quantization_map_info, ss);
                    const std::string resource_description = ss.str();
                    const std::string error =
                        error_messages_.VideoError(hazard, *cb_access_context, error_obj.location.function, resource_description);
                    skip |= SyncError(hazard.Hazard(), image_view_state->Handle(), error_obj.location, error);
                }
            }
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);
    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask,
                                             cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo *pDependencyInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);
    if (!pDependencyInfo) return skip;

    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  const VkDependencyInfoKHR *pDependencyInfo, const RecordObject &record_obj) {
    PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               const VkDependencyInfo *pDependencyInfo, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);
    if (!pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event,
                                             *pDependencyInfo, cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

bool SyncValidator::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask, const ErrorObject &error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

void SyncValidator::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                    VkPipelineStageFlags2KHR stageMask, const RecordObject &record_obj) {
    PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void SyncValidator::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                 const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                    pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return wait_events_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                uint32_t bufferMemoryBarrierCount,
                                                const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                                               bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                               pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                     const VkDependencyInfoKHR *pDependencyInfos,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

void SyncValidator::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                    const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = syncval_state::AccessContext(*cb_state);

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, pDependencyInfos);
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = syncval_state::AccessContext(*cb_state);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            const std::string resource_description = "dstBuffer " + FormatHandle(dstBuffer);
            // TODO: there are no tests for this error
            const auto error =
                error_messages_.BufferError(hazard, *cb_access_context, error_obj.location.function, resource_description, range);
            skip |= SyncError(hazard.Hazard(), dstBuffer, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                           const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = syncval_state::AccessContext(*cb_state);
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        const ResourceUsageTagEx tag_ex = cb_access_context->AddCommandHandle(tag, dst_buffer->Handle());
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = syncval_state::AccessContext(*cb_state);

    // Heavyweight, but we need a proxy copy of the active command buffer access context
    CommandBufferAccessContext proxy_cb_context(*cb_context, CommandBufferAccessContext::AsProxyContext());

    auto &proxy_label_commands = proxy_cb_context.GetProxyLabelCommands();
    proxy_label_commands = cb_state->GetLabelCommands();

    // Make working copies of the access and events contexts
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        if (cb_index == 0) {
            proxy_cb_context.NextCommandTag(error_obj.location.function, ResourceUsageRecord::SubcommandType::kIndex);
        } else {
            proxy_cb_context.NextSubcommandTag(error_obj.location.function, ResourceUsageRecord::SubcommandType::kIndex);
        }

        const auto recorded_cb = Get<vvl::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        const auto *recorded_cb_context = syncval_state::AccessContext(*recorded_cb);
        assert(recorded_cb_context);

        const ResourceUsageTag base_tag = proxy_cb_context.GetTagCount();
        skip |= ReplayState(proxy_cb_context, *recorded_cb_context, error_obj, cb_index, base_tag).ValidateFirstUse();

        // Update proxy label commands so they can be used by ImportRecordedAccessLog
        const auto &recorded_label_commands = recorded_cb->GetLabelCommands();
        proxy_label_commands.insert(proxy_label_commands.end(), recorded_label_commands.begin(), recorded_label_commands.end());

        // The barriers have already been applied in ValidatFirstUse
        proxy_cb_context.ImportRecordedAccessLog(*recorded_cb_context);
        proxy_cb_context.ResolveExecutedCommandBuffer(*recorded_cb_context->GetCurrentAccessContext(), base_tag);
    }
    proxy_label_commands.clear();

    return skip;
}

void SyncValidator::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                  const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image;
    bind_info.memory = memory;
    bind_info.memoryOffset = memoryOffset;
    UpdateSyncImageMemoryBindState(1, &bind_info);
}

void SyncValidator::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   const RecordObject &record_obj) {
    // Don't check |record_obj.result| as some binds might still be valid
    UpdateSyncImageMemoryBindState(bindInfoCount, pBindInfos);
}

void SyncValidator::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo *pBindInfos, const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void SyncValidator::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS || !syncval_settings.submit_time_validation || queue == VK_NULL_HANDLE) {
        return;
    }
    const auto queue_state = GetQueueSyncStateShared(queue);
    if (!queue_state) return;  // Invalid queue
    QueueId waited_queue = queue_state->GetQueueId();
    ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);

    // For each timeline, remove all signals signaled on the waited queue, except the last one.
    // The last signal is needed to represent the current timeline state.
    EnsureTimelineSignalsLimit(1, waited_queue);

    // Eliminate host waitable objects from the current queue.
    vvl::EraseIf(waitable_fences_, [waited_queue](const auto &sf) { return sf.second.queue_id == waited_queue; });
    for (auto &[semaphore, sync_points] : host_waitable_semaphores_) {
        vvl::EraseIf(sync_points, [waited_queue](const auto &sync_point) { return sync_point.queue_id == waited_queue; });
    }
}

void SyncValidator::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    // We need to treat this a fence waits for all queues... noting that present engine ops will be preserved.
    ForAllQueueBatchContexts(
        [](const QueueBatchContext::Ptr &batch) { batch->ApplyTaggedWait(kQueueAny, ResourceUsageRecord::kMaxIndex); });

    // For each timeline keep only the last signal per queue.
    // The last signal is needed to represent the current timeline state.
    EnsureTimelineSignalsLimit(1);

    // As we we've waited for everything on device, any waits are mooted. (except for acquires)
    vvl::EraseIf(waitable_fences_, [](const auto &waitable) { return waitable.second.acquired.Invalid(); });
    host_waitable_semaphores_.clear();
}

struct QueuePresentCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    SignalsUpdate signals_update;
    PresentedImages presented_images;
    QueuePresentCmdState(const SyncValidator &sync_validator) : signals_update(sync_validator) {}
};

bool SyncValidator::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!syncval_settings.submit_time_validation) return skip;

    ClearPending();

    vvl::TlsGuard<QueuePresentCmdState> cmd_state(&skip, *this);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    QueueBatchContext::ConstPtr last_batch = cmd_state->queue->LastBatch();
    QueueBatchContext::Ptr batch(std::make_shared<QueueBatchContext>(*this, *cmd_state->queue));

    uint32_t present_tag_count = SetupPresentInfo(*pPresentInfo, batch, cmd_state->presented_images);

    const auto wait_semaphores = vvl::make_span(pPresentInfo->pWaitSemaphores, pPresentInfo->waitSemaphoreCount);

    auto resolved_batches = batch->ResolvePresentWaits(wait_semaphores, cmd_state->presented_images, cmd_state->signals_update);

    // Import the previous batch information
    if (last_batch && !vvl::Contains(resolved_batches, last_batch)) {
        batch->ResolveLastBatch(last_batch);
        resolved_batches.emplace_back(std::move(last_batch));
    }

    // The purpose of keeping return value is to ensure async batches are alive during validation.
    // Validation accesses raw pointer to async contexts stored in AsyncReference.
    const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

    const ResourceUsageTag global_range_start = batch->SetupBatchTags(present_tag_count);
    // Update the present tags (convert to global range)
    for (auto &presented : cmd_state->presented_images) {
        presented.tag += global_range_start;
    }

    skip |= batch->DoQueuePresentValidate(error_obj.location, cmd_state->presented_images);
    batch->DoPresentOperations(cmd_state->presented_images);
    batch->LogPresentOperations(cmd_state->presented_images, submit_id);

    if (!skip) {
        cmd_state->queue->SetPendingLastBatch(std::move(batch));
    }
    return skip;
}

uint32_t SyncValidator::SetupPresentInfo(const VkPresentInfoKHR &present_info, QueueBatchContext::Ptr &batch,
                                         PresentedImages &presented_images) const {
    const VkSwapchainKHR *const swapchains = present_info.pSwapchains;
    const uint32_t *const image_indices = present_info.pImageIndices;
    const uint32_t swapchain_count = present_info.swapchainCount;

    // Create the working list of presented images
    presented_images.reserve(swapchain_count);
    for (uint32_t present_index = 0; present_index < swapchain_count; present_index++) {
        // Note: Given the "EraseIf" implementation for acquire fence waits, each presentation needs a unique tag.
        const ResourceUsageTag tag = presented_images.size();
        presented_images.emplace_back(const_cast<SyncValidator &>(*this), batch, swapchains[present_index],
                                      image_indices[present_index], present_index, tag);
        if (presented_images.back().Invalid()) {
            presented_images.pop_back();
        }
    }
    // Present is tagged for each swapchain.
    return static_cast<uint32_t>(presented_images.size());
}

void SyncValidator::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                  const RecordObject &record_obj) {
    stats.UpdateMemoryStats();

    if (!syncval_settings.submit_time_validation) {
        return;
    }

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueuePresentCmdState> cmd_state;

    // See vvl::Device::PostCallRecordQueuePresentKHR for spec excerpt supporting
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }

    // Update the state with the data from the validate phase
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    if (!queue_state) return;  // Invalid Queue
    ApplySignalsUpdate(cmd_state->signals_update, queue_state->PendingLastBatch());
    for (auto &presented : cmd_state->presented_images) {
        presented.ExportToSwapchain(*this);
    }
    queue_state->ApplyPendingLastBatch();
}

void SyncValidator::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                      const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
}

void SyncValidator::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                       uint32_t *pImageIndex, const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj);
}

void SyncValidator::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;

    // Get the image out of the presented list and create apppropriate fences/semaphores.
    auto swapchain_base = Get<vvl::Swapchain>(swapchain);
    if (vvl::StateObject::Invalid(swapchain_base)) return;  // Invalid acquire calls to be caught in CoreCheck/Parameter validation

    auto &swapchain_state = syncval_state::SubState(*swapchain_base);

    PresentedImage presented = swapchain_state.MovePresentedImage(*pImageIndex);
    if (presented.Invalid()) return;

    // No way to make access safe, so nothing to record
    if ((semaphore == VK_NULL_HANDLE) && (fence == VK_NULL_HANDLE)) return;

    // We create a queue-less QBC for the Semaphore and fences to wait on

    // Note: this is a heavyweight way to deal with the fact that all operation logs live in the QueueBatchContext... and
    // acquire doesn't happen on a queue, but we need a place to put the acquire operation access record.
    auto batch = std::make_shared<QueueBatchContext>(*this);
    batch->SetupAccessContext(presented);
    const ResourceUsageTag acquire_tag = batch->SetupBatchTags(1);
    batch->DoAcquireOperation(presented);
    batch->LogAcquireOperation(presented, record_obj.location.function);

    // Now swap out the present queue batch with the acquired one.
    // Note that fence and signal will read the acquire batch from presented, so this needs to be done before
    // setting up the synchronization
    presented.batch = std::move(batch);

    if (semaphore != VK_NULL_HANDLE) {
        std::shared_ptr<const vvl::Semaphore> sem_state = Get<vvl::Semaphore>(semaphore);
        if (sem_state) {
            // This will ignore any duplicated signal (emplace does not update existing entry),
            // and the core validation reports and error in this case.
            binary_signals_.emplace(sem_state->VkHandle(), SignalInfo(sem_state, presented, acquire_tag));
        }
    }
    if (fence != VK_NULL_HANDLE) {
        FenceHostSyncPoint sync_point;
        sync_point.tag = acquire_tag;
        sync_point.acquired = AcquiredImage(presented, acquire_tag);
        UpdateFenceHostSyncPoint(fence, std::move(sync_point));
    }
}

bool SyncValidator::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                               const ErrorObject &error_obj) const {
    SubmitInfoConverter submit_info(pSubmits, submitCount);
    return ValidateQueueSubmit(queue, submitCount, submit_info.submit_infos2.data(), fence, error_obj);
}

static std::vector<CommandBufferConstPtr> GetCommandBuffers(const vvl::DeviceState &device_state,
                                                            const VkSubmitInfo2 &submit_info) {
    // Collected command buffers have the same indexing as in the input VkSubmitInfo2 for reporting purposes.
    // If Get query returns null, it is stored in the result array to keep original indexing.
    std::vector<CommandBufferConstPtr> command_buffers;
    command_buffers.reserve(submit_info.commandBufferInfoCount);
    for (const auto &cb_info : vvl::make_span(submit_info.pCommandBufferInfos, submit_info.commandBufferInfoCount)) {
        command_buffers.emplace_back(device_state.Get<vvl::CommandBuffer>(cb_info.commandBuffer));
    }
    return command_buffers;
}

bool SyncValidator::ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!syncval_settings.submit_time_validation) return skip;

    std::lock_guard lock_guard(queue_submit_mutex_);

    ClearPending();

    QueueSubmitCmdState cmd_state_obj(*this);
    QueueSubmitCmdState* cmd_state = &cmd_state_obj;

    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    auto &queue_sync_state = cmd_state->queue;
    SignalsUpdate &signals_update = cmd_state->signals_update;

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = queue_sync_state->ReserveSubmitId();

    // Update label stack as we progress through batches and command buffers
    auto current_label_stack = queue_sync_state->GetQueueState()->cmdbuf_label_stack;

    BatchContextConstPtr last_batch = queue_sync_state->LastBatch();
    bool has_unresolved_batches = !queue_sync_state->UnresolvedBatches().empty();

    BatchContextPtr new_last_batch;
    std::vector<UnresolvedBatch> new_unresolved_batches;
    bool new_timeline_signals = false;

    for (uint32_t batch_idx = 0; batch_idx < submitCount; batch_idx++) {
        const VkSubmitInfo2 &submit = pSubmits[batch_idx];
        auto batch = std::make_shared<QueueBatchContext>(*this, *queue_sync_state);

        const auto wait_semaphores = vvl::make_span(submit.pWaitSemaphoreInfos, submit.waitSemaphoreInfoCount);
        std::vector<VkSemaphoreSubmitInfo> unresolved_waits;
        auto resolved_batches = batch->ResolveSubmitWaits(wait_semaphores, unresolved_waits, signals_update);

        // Add unresolved batch
        if (has_unresolved_batches || !unresolved_waits.empty()) {
            UnresolvedBatch unresolved_batch;
            unresolved_batch.batch = std::move(batch);
            unresolved_batch.submit_index = submit_id;
            unresolved_batch.batch_index = batch_idx;
            unresolved_batch.command_buffers = GetCommandBuffers(*device_state, submit);
            unresolved_batch.unresolved_waits = std::move(unresolved_waits);
            unresolved_batch.resolved_dependencies = std::move(resolved_batches);
            if (submit.pSignalSemaphoreInfos && submit.signalSemaphoreInfoCount) {
                const auto last_info = submit.pSignalSemaphoreInfos + submit.signalSemaphoreInfoCount;
                unresolved_batch.signals.assign(submit.pSignalSemaphoreInfos, last_info);
            }
            unresolved_batch.label_stack = current_label_stack;
            new_unresolved_batches.emplace_back(std::move(unresolved_batch));
            has_unresolved_batches = true;
            stats.AddUnresolvedBatch();
            continue;
        }
        new_last_batch = batch;

        // Import the previous batch information
        if (last_batch && !vvl::Contains(resolved_batches, last_batch)) {
            batch->ResolveLastBatch(last_batch);
            resolved_batches.emplace_back(std::move(last_batch));
        }

        // The purpose of keeping return value is to ensure async batches are alive during validation.
        // Validation accesses raw pointer to async contexts stored in AsyncReference.
        // TODO: All syncval tests pass when the return value is ignored. Write a regression test that fails/crashes in this case.
        const auto async_batches = batch->RegisterAsyncContexts(resolved_batches);

        const auto command_buffers = GetCommandBuffers(*device_state, submit);
        skip |= batch->ValidateSubmit(command_buffers, submit_id, batch_idx, current_label_stack, error_obj);

        const auto submit_signals = vvl::make_span(submit.pSignalSemaphoreInfos, submit.signalSemaphoreInfoCount);
        new_timeline_signals |= signals_update.RegisterSignals(batch, submit_signals);

        // Unless the previous batch was referenced by a signal it will self destruct
        // in the record phase when the last batch is updated.
        last_batch = batch;
    }

    // Schedule state update
    if (new_last_batch) {
        queue_sync_state->SetPendingLastBatch(std::move(new_last_batch));
    }
    if (!new_unresolved_batches.empty()) {
        auto unresolved_batches = queue_sync_state->UnresolvedBatches();
        vvl::Append(unresolved_batches, new_unresolved_batches);
        queue_sync_state->SetPendingUnresolvedBatches(std::move(unresolved_batches));
    }

    // Check if timeline signals resolve existing wait-before-signal dependencies
    if (new_timeline_signals) {
        skip |= PropagateTimelineSignals(signals_update, error_obj);
    }

    if (!skip) {
        const_cast<SyncValidator *>(this)->RecordQueueSubmit(queue, fence, cmd_state);
    }

    // Note that if we skip, guard cleans up for us, but cannot release the reserved tag range
    return skip;
}

bool SyncValidator::PropagateTimelineSignals(SignalsUpdate &signals_update, const ErrorObject &error_obj) const {
    bool skip = false;
    // Initialize per-queue unresolved batches state.
    std::vector<UnresolvedQueue> queues;
    for (const auto &queue_state : queue_sync_states_) {
        if (!queue_state->PendingUnresolvedBatches().empty()) {
            // Pending request defines the final unresolved list (current + new unresolved batches)
            queues.emplace_back(UnresolvedQueue{queue_state, queue_state->PendingUnresolvedBatches()});
        } else if (!queue_state->UnresolvedBatches().empty()) {
            queues.emplace_back(UnresolvedQueue{queue_state, queue_state->UnresolvedBatches()});
        }
    }

    // Each iteration uses registered timeline signals to resolve existing unresolved batches.
    // Each resolved batch can generate new timeline signals which can resolve more unresolved batches on the next iteration.
    // This finishes when all unresolved batches are resolved or when iteration does not generate new timeline signals.
    while (PropagateTimelineSignalsIteration(queues, signals_update, skip, error_obj)) {
        ;
    }

    // Schedule unresolved state update
    for (UnresolvedQueue &queue : queues) {
        if (queue.update_unresolved) {
            queue.queue_state->SetPendingUnresolvedBatches(std::move(queue.unresolved_batches));
        }
    }
    return skip;
}

bool SyncValidator::PropagateTimelineSignalsIteration(std::vector<UnresolvedQueue> &queues, SignalsUpdate &signals_update,
                                                      bool &skip, const ErrorObject &error_obj) const {
    bool has_new_timeline_signals = false;
    for (auto &queue : queues) {
        if (queue.unresolved_batches.empty()) {
            continue;  // all batches for this queue were resolved by previous iterations
        }

        BatchContextPtr last_batch =
            queue.queue_state->PendingLastBatch() ? queue.queue_state->PendingLastBatch() : queue.queue_state->LastBatch();
        const BatchContextPtr initial_last_batch = last_batch;

        while (!queue.unresolved_batches.empty()) {
            auto &unresolved_batch = queue.unresolved_batches.front();

            has_new_timeline_signals |= ProcessUnresolvedBatch(unresolved_batch, signals_update, last_batch, skip, error_obj);

            // Remove processed batch from the (local) unresolved list
            queue.unresolved_batches.erase(queue.unresolved_batches.begin());

            // Propagate change into the queue's (global) unresolved state
            queue.update_unresolved = true;

            stats.RemoveUnresolvedBatch();
        }
        if (last_batch != initial_last_batch) {
            queue.queue_state->SetPendingLastBatch(std::move(last_batch));
        }
    }
    return has_new_timeline_signals;
}

bool SyncValidator::ProcessUnresolvedBatch(UnresolvedBatch &unresolved_batch, SignalsUpdate &signals_update,
                                           BatchContextPtr &last_batch, bool &skip, const ErrorObject &error_obj) const {
    // Resolve waits that have matching signal
    auto it = unresolved_batch.unresolved_waits.begin();
    while (it != unresolved_batch.unresolved_waits.end()) {
        const VkSemaphoreSubmitInfo &wait_info = *it;
        auto resolving_signal = signals_update.OnTimelineWait(wait_info.semaphore, wait_info.value);
        if (!resolving_signal.has_value()) {
            ++it;
            continue;  // resolving signal not found, the wait stays unresolved
        }
        if (resolving_signal->batch) {  // null for host signals
            unresolved_batch.batch->ResolveSubmitSemaphoreWait(*resolving_signal, wait_info.stageMask);
            unresolved_batch.batch->ImportTags(*resolving_signal->batch);
            unresolved_batch.resolved_dependencies.emplace_back(resolving_signal->batch);
        }
        it = unresolved_batch.unresolved_waits.erase(it);
    }

    // This batch still has unresolved waits
    if (!unresolved_batch.unresolved_waits.empty()) {
        return false;  // no new timeline signals were registered
    }

    // Process fully resolved batch
    UnresolvedBatch &ready_batch = unresolved_batch;
    if (last_batch && !vvl::Contains(ready_batch.resolved_dependencies, last_batch)) {
        ready_batch.batch->ResolveLastBatch(last_batch);
        ready_batch.resolved_dependencies.emplace_back(std::move(last_batch));
    }
    last_batch = ready_batch.batch;

    const auto async_batches = ready_batch.batch->RegisterAsyncContexts(ready_batch.resolved_dependencies);

    skip |= ready_batch.batch->ValidateSubmit(ready_batch.command_buffers, ready_batch.submit_index, ready_batch.batch_index,
                                              ready_batch.label_stack, error_obj);

    const auto submit_signals = vvl::make_span(ready_batch.signals.data(), ready_batch.signals.size());
    return signals_update.RegisterSignals(ready_batch.batch, submit_signals);
}

void SyncValidator::RecordQueueSubmit(VkQueue queue, VkFence fence, QueueSubmitCmdState *cmd_state) {
    stats.UpdateMemoryStats();

    // If this return is above the TlsGuard, then the Validate phase return must also be.
    if (!syncval_settings.submit_time_validation) {
        return;
    }

    if (!cmd_state->queue) {
        return;
    }

    // Don't need to look up the queue state again, but we need a non-const version
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    ApplySignalsUpdate(cmd_state->signals_update, queue_state->PendingLastBatch());

    // Apply the pending state from the validation phase. Check all queues because timeline signals
    // on the current queue can resolve wait-before-signal batches on other queues.
    for (const auto &qs : queue_sync_states_) {
        qs->ApplyPendingLastBatch();
        qs->ApplyPendingUnresolvedBatches();
    }

    FenceHostSyncPoint sync_point;
    sync_point.queue_id = queue_state->GetQueueId();
    sync_point.tag = ReserveGlobalTagRange(1).begin;
    UpdateFenceHostSyncPoint(fence, std::move(sync_point));
}

bool SyncValidator::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                   VkFence fence, const ErrorObject &error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool SyncValidator::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                                const ErrorObject &error_obj) const {
    return ValidateQueueSubmit(queue, submitCount, pSubmits, fence, error_obj);
}

void SyncValidator::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) return;
    if (record_obj.result == VK_SUCCESS) {
        // fence is signalled, mark it as waited for
        WaitForFence(fence);
    }
}

void SyncValidator::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) return;
    if ((record_obj.result == VK_SUCCESS) && ((VK_TRUE == waitAll) || (1 == fenceCount))) {
        // We can only know the pFences have signal if we waited for all of them, or there was only one of them
        for (uint32_t i = 0; i < fenceCount; i++) {
            WaitForFence(pFences[i]);
        }
    }
}

bool SyncValidator::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    if (!syncval_settings.submit_time_validation) {
        return skip;
    }
    ClearPending();
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state(&skip, *this);
    SignalsUpdate &signals_update = cmd_state->signals_update;

    auto semaphore_state = Get<vvl::Semaphore>(pSignalInfo->semaphore);
    if (!semaphore_state) {
        return skip;
    }

    std::vector<SignalInfo> &signals = signals_update.timeline_signals[pSignalInfo->semaphore];

    // Reject invalid signal
    if (!signals.empty() && pSignalInfo->value <= signals.back().timeline_value) {
        return skip;  // [core validation check]: strictly increasing signal values
    }

    signals.emplace_back(SignalInfo(semaphore_state, pSignalInfo->value));
    skip |= PropagateTimelineSignals(signals_update, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                      const ErrorObject &error_obj) const {
    return PreCallValidateSignalSemaphore(device, pSignalInfo, error_obj);
}

void SyncValidator::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                  const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) {
        return;
    }

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state;

    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    ApplySignalsUpdate(cmd_state->signals_update, nullptr);
    for (const auto &qs : queue_sync_states_) {
        qs->ApplyPendingLastBatch();
        qs->ApplyPendingUnresolvedBatches();
    }
}

void SyncValidator::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                     const RecordObject &record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void SyncValidator::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                 const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) {
        return;
    }
    const bool wait_all = pWaitInfo->semaphoreCount == 1 || (pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT) == 0;
    if (record_obj.result == VK_SUCCESS && wait_all) {
        for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
            WaitForSemaphore(pWaitInfo->pSemaphores[i], pWaitInfo->pValues[i]);
        }
    }
}

void SyncValidator::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                    const RecordObject &record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void SyncValidator::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                           const RecordObject &record_obj) {
    if (!syncval_settings.submit_time_validation) {
        return;
    }
    if (record_obj.result == VK_SUCCESS) {
        WaitForSemaphore(semaphore, *pValue);
    }
}

void SyncValidator::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                              const RecordObject &record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void SyncValidator::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                        VkImage *pSwapchainImages, const RecordObject &record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    auto swapchain_state = Get<vvl::Swapchain>(swapchain);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            vvl::SwapchainImage &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) {
                auto *sync_image = swapchain_image.image_state;
                auto &sub_state = syncval_state::SubState(*sync_image);
                assert(sub_state.IsTiled());  // This is the assumption from the spec, and the implementation relies on it
                sub_state.SetOpaqueBaseAddress(*device_state);
            }
        }
    }
}

// Returns null when device address is asssociated with no buffers or more than one buffer.
// Otherwise returns a valid buffer (device address is associated with a single buffer).
// When syncval adds memory aliasing support the need of this function can be revisited.
static const vvl::Buffer *GetSingleBufferFromDeviceAddress(const vvl::DeviceState &device, VkDeviceAddress device_address) {
    vvl::span<vvl::Buffer *const> buffers = device.GetBuffersByAddress(device_address);
    if (buffers.empty()) {
        return nullptr;
    }
    if (buffers.size() > 1) {  // memory aliasing use case
        return nullptr;
    }
    return buffers[0];
}

struct AccelerationStructureGeometryInfo {
    const vvl::Buffer *vertex_data = nullptr;
    ResourceAccessRange vertex_range;
    const vvl::Buffer *index_data = nullptr;
    ResourceAccessRange index_range;
    const vvl::Buffer *transform_data = nullptr;
    ResourceAccessRange transform_range;
    const vvl::Buffer *aabb_data = nullptr;
    ResourceAccessRange aabb_range;
    const vvl::Buffer *instance_data = nullptr;
    ResourceAccessRange instance_range;
};

static std::optional<AccelerationStructureGeometryInfo> GetValidGeometryInfo(
    const vvl::DeviceState &device, const VkAccelerationStructureGeometryKHR &geometry,
    const VkAccelerationStructureBuildRangeInfoKHR &range_info) {
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        const VkAccelerationStructureGeometryTrianglesDataKHR &triangles = geometry.geometry.triangles;
        AccelerationStructureGeometryInfo geometry_info;

        // Assume that synchronization ranges cover the entire vertex struct,
        // even if positional data is strided (i.e., interleaved with other attributes).
        // That is, the application does not attempt to synchronize each position variable
        // with a separate buffer barrier range.
        const vvl::Buffer *p_vertex_data = GetSingleBufferFromDeviceAddress(device, triangles.vertexData.deviceAddress);

        if (triangles.indexType == VK_INDEX_TYPE_NONE_KHR) {
            // Vertex data
            if (p_vertex_data) {
                geometry_info.vertex_data = p_vertex_data;
                const VkDeviceSize base_vertex_offset = triangles.vertexData.deviceAddress - p_vertex_data->deviceAddress;
                const VkDeviceSize local_offset = range_info.primitiveOffset + range_info.firstVertex * triangles.vertexStride;
                const VkDeviceSize offset = base_vertex_offset + local_offset;
                const VkDeviceSize vertex_data_size = 3 * range_info.primitiveCount * triangles.vertexStride;
                geometry_info.vertex_range = MakeRange(*p_vertex_data, offset, vertex_data_size);
            }
        } else {
            // Vertex data
            if (p_vertex_data) {
                geometry_info.vertex_data = p_vertex_data;
                const VkDeviceSize base_vertex_offset = triangles.vertexData.deviceAddress - p_vertex_data->deviceAddress;
                const VkDeviceSize local_offset = range_info.firstVertex * triangles.vertexStride;
                const VkDeviceSize offset = base_vertex_offset + local_offset;
                const VkDeviceSize all_vertex_data_size = (triangles.maxVertex + 1) * triangles.vertexStride;
                const VkDeviceSize potentially_accessed_vertex_data_size = all_vertex_data_size - local_offset;
                geometry_info.vertex_range = MakeRange(*p_vertex_data, offset, potentially_accessed_vertex_data_size);
            }
            // Index data
            const auto p_index_data = GetSingleBufferFromDeviceAddress(device, triangles.indexData.deviceAddress);
            if (p_index_data) {
                geometry_info.index_data = p_index_data;
                const VkDeviceSize base_index_offset = triangles.indexData.deviceAddress - p_index_data->deviceAddress;
                const uint32_t index_size = GetIndexBitsSize(triangles.indexType) / 8;
                const VkDeviceSize offset = base_index_offset + range_info.primitiveOffset;
                const uint32_t index_data_size = 3 * range_info.primitiveCount * index_size;
                geometry_info.index_range = MakeRange(*p_index_data, offset, index_data_size);
            }
        }
        // Transform data
        if (const vvl::Buffer *p_transform_data = GetSingleBufferFromDeviceAddress(device, triangles.transformData.deviceAddress)) {
            const VkDeviceSize base_offset = triangles.transformData.deviceAddress - p_transform_data->deviceAddress;
            const VkDeviceSize offset = base_offset + range_info.transformOffset;
            geometry_info.transform_data = p_transform_data;
            geometry_info.transform_range = MakeRange(*p_transform_data, offset, sizeof(VkTransformMatrixKHR));
        }
        return geometry_info;
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        // Make a similar assumption for strided aabb data as for vertex data - synchronization ranges themselves are not strided.
        const VkAccelerationStructureGeometryAabbsDataKHR &aabbs = geometry.geometry.aabbs;
        if (const vvl::Buffer *p_aabbs = GetSingleBufferFromDeviceAddress(device, aabbs.data.deviceAddress)) {
            AccelerationStructureGeometryInfo geometry_info;
            geometry_info.aabb_data = p_aabbs;
            const VkDeviceSize base_offset = aabbs.data.deviceAddress - p_aabbs->deviceAddress;
            const VkDeviceSize offset = base_offset + range_info.primitiveOffset;
            const VkDeviceSize aabb_data_size = range_info.primitiveCount * sizeof(VkAabbPositionsKHR);
            geometry_info.aabb_range = MakeRange(*p_aabbs, offset, aabb_data_size);
            return geometry_info;
        }
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        const VkAccelerationStructureGeometryInstancesDataKHR &instances = geometry.geometry.instances;
        if (const vvl::Buffer *p_instances = GetSingleBufferFromDeviceAddress(device, instances.data.deviceAddress)) {
            AccelerationStructureGeometryInfo geometry_info;
            geometry_info.instance_data = p_instances;
            const VkDeviceSize base_offset = instances.data.deviceAddress - p_instances->deviceAddress;
            const VkDeviceSize offset = base_offset + range_info.primitiveOffset;
            const VkDeviceSize instance_data_size =
                range_info.primitiveCount *
                (instances.arrayOfPointers ? sizeof(VkDeviceAddress) : sizeof(VkAccelerationStructureInstanceKHR));
            geometry_info.instance_range = MakeRange(*p_instances, offset, instance_data_size);
            return geometry_info;
        }
    }
    return {};
}

bool SyncValidator::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    for (const auto [i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, i);
        // Validate scratch buffer
        if (const vvl::Buffer *p_scratch_buffer = GetSingleBufferFromDeviceAddress(*device_state, info.scratchData.deviceAddress)) {
            const vvl::Buffer &scratch_buffer = *p_scratch_buffer;
            const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Device, device, info, ppBuildRangeInfos[i]);
            const VkDeviceSize offset = info.scratchData.deviceAddress - scratch_buffer.deviceAddress;
            const ResourceAccessRange range = MakeRange(scratch_buffer, offset, scratch_size);
            auto hazard =
                context.DetectHazard(scratch_buffer, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE, range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, scratch_buffer.Handle());
                const std::string resource_description = "scratch buffer " + FormatHandle(scratch_buffer.VkHandle());
                const auto error =
                    error_messages_.BufferError(hazard, cb_context, error_obj.location.function, resource_description, range);
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        // Validate access to source acceleration structure
        if (const auto src_accel = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure)) {
            const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
            auto hazard = context.DetectHazard(*src_accel->buffer_state,
                                               SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_READ, range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, src_accel->buffer_state->Handle(), src_accel->Handle());
                const std::string resource_description = FormatHandle(src_accel->buffer_state->VkHandle());
                const std::string error = error_messages_.AccelerationStructureError(
                    hazard, cb_context, error_obj.location.function, resource_description, range, info.srcAccelerationStructure,
                    info_loc.dot(Field::srcAccelerationStructure));
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        // Validate access to the acceleration structure being built
        if (const auto dst_accel = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            const ResourceAccessRange dst_range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
            auto hazard = context.DetectHazard(*dst_accel->buffer_state,
                                               SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE, dst_range);
            if (hazard.IsHazard()) {
                const LogObjectList objlist(commandBuffer, dst_accel->buffer_state->Handle(), dst_accel->Handle());
                const std::string resource_description = FormatHandle(dst_accel->buffer_state->VkHandle());
                const std::string error = error_messages_.AccelerationStructureError(
                    hazard, cb_context, error_obj.location.function, resource_description, dst_range, info.dstAccelerationStructure,
                    info_loc.dot(Field::dstAccelerationStructure));
                skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
            }
        }
        // Validate geometry buffers
        const VkAccelerationStructureBuildRangeInfoKHR *p_range_infos = ppBuildRangeInfos[i];
        if (!p_range_infos) {
            continue;  // [core validation check]: range pointers should be valid
        }
        for (uint32_t k = 0; k < info.geometryCount; k++) {
            const auto *p_geometry = info.pGeometries ? &info.pGeometries[k] : info.ppGeometries[k];
            if (!p_geometry) {
                continue;  // [core validation check]: null pointer in ppGeometries
            }
            const auto geometry_info = GetValidGeometryInfo(*device_state, *p_geometry, p_range_infos[k]);
            if (!geometry_info.has_value()) {
                continue;
            }
            auto validate_accel_input_geometry = [this, &context, &cb_context, &commandBuffer, &error_obj](
                                                     const vvl::Buffer &geometry_data, const ResourceAccessRange &geometry_range,
                                                     const char *data_description) {
                auto hazard = context.DetectHazard(geometry_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ, geometry_range);
                if (hazard.IsHazard()) {
                    const LogObjectList objlist(commandBuffer, geometry_data.Handle());
                    std::stringstream ss;
                    ss << data_description << " ";
                    ss << FormatHandle(geometry_data.Handle());
                    const std::string resource_description = ss.str();
                    const std::string error = error_messages_.BufferError(hazard, cb_context, error_obj.location.function,
                                                                          resource_description, geometry_range);
                    return SyncError(hazard.Hazard(), objlist, error_obj.location, error);
                }
                return false;
            };
            if (geometry_info->vertex_data) {
                skip |= validate_accel_input_geometry(*geometry_info->vertex_data, geometry_info->vertex_range, "vertex data");
            }
            if (geometry_info->index_data) {
                skip |= validate_accel_input_geometry(*geometry_info->index_data, geometry_info->index_range, "index data");
            }
            if (geometry_info->transform_data) {
                skip |=
                    validate_accel_input_geometry(*geometry_info->transform_data, geometry_info->transform_range, "transform data");
            }
            if (geometry_info->aabb_data) {
                skip |= validate_accel_input_geometry(*geometry_info->aabb_data, geometry_info->aabb_range, "aabb data");
            }
            if (geometry_info->instance_data) {
                skip |=
                    validate_accel_input_geometry(*geometry_info->instance_data, geometry_info->instance_range, "instance data");
            }
        }
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);

    for (const auto [i, info] : vvl::enumerate(pInfos, infoCount)) {
        // Record scratch buffer access
        if (const vvl::Buffer *p_scratch_buffer = GetSingleBufferFromDeviceAddress(*device_state, info.scratchData.deviceAddress)) {
            const vvl::Buffer &scratch_buffer = *p_scratch_buffer;
            const VkDeviceSize scratch_size = rt::ComputeScratchSize(rt::BuildType::Device, device, info, ppBuildRangeInfos[i]);
            const VkDeviceSize offset = info.scratchData.deviceAddress - scratch_buffer.deviceAddress;
            const ResourceAccessRange scratch_range = MakeRange(scratch_buffer, offset, scratch_size);
            const ResourceUsageTagEx scratch_tag_ex = cb_context.AddCommandHandle(tag, scratch_buffer.Handle());
            context.UpdateAccessState(scratch_buffer, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE,
                                      SyncOrdering::kNonAttachment, scratch_range, scratch_tag_ex);
        }

        const auto src_accel = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
        const auto dst_accel = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

        // Record source acceleration structure access (READ).
        // If the source is the same as the destination then no need to record READ
        // (destination update will replace access with WRITE anyway).
        if (src_accel && src_accel != dst_accel) {
            const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
            const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, src_accel->buffer_state->Handle());
            context.UpdateAccessState(*src_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_READ,
                                      SyncOrdering::kNonAttachment, range, tag_ex);
        }
        // Record destination acceleration structure access (WRITE)
        if (dst_accel) {
            const ResourceAccessRange dst_range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
            const ResourceUsageTagEx dst_tag_ex = cb_context.AddCommandHandle(tag, dst_accel->buffer_state->Handle());
            context.UpdateAccessState(*dst_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE,
                                      SyncOrdering::kNonAttachment, dst_range, dst_tag_ex);
        }
        // Record geometry buffer acceses (READ)
        const VkAccelerationStructureBuildRangeInfoKHR *p_range_infos = ppBuildRangeInfos[i];
        if (!p_range_infos) {
            continue;  // [core validation check]: range pointers should be valid
        }
        for (uint32_t k = 0; k < info.geometryCount; k++) {
            const auto *p_geometry = info.pGeometries ? &info.pGeometries[k] : info.ppGeometries[k];
            if (!p_geometry) {
                continue;  // [core validation check]: null pointer in ppGeometries
            }
            const auto geometry_info = GetValidGeometryInfo(*device_state, *p_geometry, p_range_infos[k]);
            if (!geometry_info.has_value()) {
                continue;
            }
            if (geometry_info->vertex_data) {
                const ResourceUsageTagEx vertex_tag_ex = cb_context.AddCommandHandle(tag, geometry_info->vertex_data->Handle());
                context.UpdateAccessState(*geometry_info->vertex_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ,
                                          SyncOrdering::kNonAttachment, geometry_info->vertex_range, vertex_tag_ex);
            }
            if (geometry_info->index_data) {
                const ResourceUsageTagEx index_tag_ex = cb_context.AddCommandHandle(tag, geometry_info->index_data->Handle());
                context.UpdateAccessState(*geometry_info->index_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ,
                                          SyncOrdering::kNonAttachment, geometry_info->index_range, index_tag_ex);
            }
            if (geometry_info->transform_data) {
                const ResourceUsageTagEx transform_tag_ex =
                    cb_context.AddCommandHandle(tag, geometry_info->transform_data->Handle());
                context.UpdateAccessState(*geometry_info->transform_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ,
                                          SyncOrdering::kNonAttachment, geometry_info->transform_range, transform_tag_ex);
            }
            if (geometry_info->aabb_data) {
                const ResourceUsageTagEx aabb_tag_ex = cb_context.AddCommandHandle(tag, geometry_info->aabb_data->Handle());
                context.UpdateAccessState(*geometry_info->aabb_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ,
                                          SyncOrdering::kNonAttachment, geometry_info->aabb_range, aabb_tag_ex);
            }
            if (geometry_info->instance_data) {
                const ResourceUsageTagEx instance_tag_ex = cb_context.AddCommandHandle(tag, geometry_info->instance_data->Handle());
                context.UpdateAccessState(*geometry_info->instance_data, SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ,
                                          SyncOrdering::kNonAttachment, geometry_info->instance_range, instance_tag_ex);
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const Location info_loc = error_obj.location.dot(Field::pInfo);

    if (const auto src_accel = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
        auto hazard =
            context.DetectHazard(*src_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state->Handle(), src_accel->buffer_state->Handle(), src_accel->Handle());
            const std::string resource_description = FormatHandle(src_accel->buffer_state->VkHandle());
            const std::string error = error_messages_.AccelerationStructureError(
                hazard, cb_context, error_obj.location.function, resource_description, range, pInfo->src, info_loc.dot(Field::src));
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    if (const auto dst_accel = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const ResourceAccessRange range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
        auto hazard =
            context.DetectHazard(*dst_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state->Handle(), dst_accel->buffer_state->Handle(), dst_accel->Handle());
            const std::string resource_description = FormatHandle(dst_accel->buffer_state->VkHandle());
            const std::string error = error_messages_.AccelerationStructureError(
                hazard, cb_context, error_obj.location.function, resource_description, range, pInfo->dst, info_loc.dot(Field::dst));
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                  const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);

    if (const auto src_accel = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
        const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, src_accel->buffer_state->Handle());
        context.UpdateAccessState(*src_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ,
                                  SyncOrdering::kNonAttachment, range, tag_ex);
    }
    if (const auto dst_accel = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const ResourceAccessRange range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
        const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, dst_accel->buffer_state->Handle());
        context.UpdateAccessState(*dst_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE,
                                  SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const Location info_loc = error_obj.location.dot(Field::pInfo);

    if (const auto src_accel = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
        auto hazard =
            context.DetectHazard(*src_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state->Handle(), src_accel->buffer_state->Handle(), src_accel->Handle());
            const std::string resource_description = FormatHandle(src_accel->buffer_state->VkHandle());
            const std::string error = error_messages_.AccelerationStructureError(
                hazard, cb_context, error_obj.location.function, resource_description, range, pInfo->src, info_loc.dot(Field::src));
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }

    // NOTE: do not validate src_buffer. This requires recording query and then waiting for it after submit.
    // Currently syncval does not support this but even if support is available this affects application:
    // it flushes entire GPU frame and it also affects app scheduling behavior (CPU and GPU frames do not overlap
    // anymore, and this can hide resource scheduling issues). Such submit-wait-validation can be an optional feature.

    return skip;
}

void SyncValidator::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                          const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                          const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);

    if (const auto src_accel = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const ResourceAccessRange range = MakeRange(src_accel->create_info.offset, src_accel->create_info.size);
        const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, src_accel->buffer_state->Handle());
        context.UpdateAccessState(*src_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ,
                                  SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const Location info_loc = error_obj.location.dot(Field::pInfo);

    if (const auto dst_accel = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const ResourceAccessRange range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
        auto hazard =
            context.DetectHazard(*dst_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE, range);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state->Handle(), dst_accel->buffer_state->Handle(), dst_accel->Handle());
            const std::string resource_description = FormatHandle(dst_accel->buffer_state->VkHandle());
            const std::string error = error_messages_.AccelerationStructureError(
                hazard, cb_context, error_obj.location.function, resource_description, range, pInfo->dst, info_loc.dot(Field::dst));
            skip |= SyncError(hazard.Hazard(), objlist, error_obj.location, error);
        }
    }

    // NOTE: do not validate src_buffer. This requires recording query and then waiting for it after submit.
    // Currently syncval does not support this but even if support is available this affects application:
    // it flushes entire GPU frame and it also affects app scheduling behavior (CPU and GPU frames do not overlap
    // anymore, and this can hide resource scheduling issues). Such submit-wait-validation can be an optional feature.

    return skip;
}

void SyncValidator::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                          const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                          const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &context = *cb_context.GetCurrentAccessContext();

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);

    if (const auto dst_accel = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        const ResourceAccessRange range = MakeRange(dst_accel->create_info.offset, dst_accel->create_info.size);
        const ResourceUsageTagEx tag_ex = cb_context.AddCommandHandle(tag, dst_accel->buffer_state->Handle());
        context.UpdateAccessState(*dst_accel->buffer_state, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE,
                                  SyncOrdering::kNonAttachment, range, tag_ex);
    }
}

bool SyncValidator::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                   uint32_t width, uint32_t height, uint32_t depth,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);

    skip |= cb_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    return skip;
}

void SyncValidator::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                  const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                  uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);
    cb_context.RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, tag);
}

bool SyncValidator::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                           VkDeviceAddress indirectDeviceAddress,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &access_context = *cb_context.GetCurrentAccessContext();

    skip |= cb_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);

    if (const vvl::Buffer *indirect_buffer = GetSingleBufferFromDeviceAddress(*device_state, indirectDeviceAddress)) {
        skip |= ValidateIndirectBuffer(cb_context, access_context, sizeof(VkTraceRaysIndirectCommandKHR),
                                       indirect_buffer->VkHandle(), 0, 1, 0, error_obj.location);
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                          const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                          const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                          const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                          const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                          VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);
    cb_context.RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, tag);

    if (const vvl::Buffer *indirect_buffer = GetSingleBufferFromDeviceAddress(*device_state, indirectDeviceAddress)) {
        RecordIndirectBuffer(cb_context, tag, sizeof(VkTraceRaysIndirectCommandKHR), indirect_buffer->VkHandle(), 0, 1, 0);
    }
}

bool SyncValidator::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);
    auto &access_context = *cb_context.GetCurrentAccessContext();

    skip |= cb_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);

    if (const vvl::Buffer *indirect_buffer = GetSingleBufferFromDeviceAddress(*device_state, indirectDeviceAddress)) {
        skip |= ValidateIndirectBuffer(cb_context, access_context, sizeof(VkTraceRaysIndirectCommand2KHR),
                                       indirect_buffer->VkHandle(), 0, 1, 0, error_obj.location);
    }
    return skip;
}

void SyncValidator::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                           const RecordObject &record_obj) {
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    auto &cb_context = *syncval_state::AccessContext(*cb_state);

    const ResourceUsageTag tag = cb_context.NextCommandTag(record_obj.location.function);
    cb_context.RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, tag);

    if (const vvl::Buffer *indirect_buffer = GetSingleBufferFromDeviceAddress(*device_state, indirectDeviceAddress)) {
        RecordIndirectBuffer(cb_context, tag, sizeof(VkTraceRaysIndirectCommand2KHR), indirect_buffer->VkHandle(), 0, 1, 0);
    }
}
