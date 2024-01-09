/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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
#include <limits>
#include <memory>
#include <vector>

#include "sync/sync_validation.h"

ResourceUsageRange SyncValidator::ReserveGlobalTagRange(size_t tag_count) const {
    ResourceUsageRange reserve;
    reserve.begin = tag_limit_.fetch_add(tag_count);
    reserve.end = reserve.begin + tag_count;
    return reserve;
}

void SyncValidator::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    auto tagged_wait_op = [queue_id, tag](const std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyTaggedWait(queue_id, tag);
        batch->Trim();
    };
    ForAllQueueBatchContexts(tagged_wait_op);
}

void SyncValidator::ApplyAcquireWait(const AcquiredImage &acquired) {
    auto acq_wait_op = [&acquired](const std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyAcquireWait(acquired);
        batch->Trim();
    };
    ForAllQueueBatchContexts(acq_wait_op);
}

template <typename BatchOp>
void SyncValidator::ForAllQueueBatchContexts(BatchOp &&op) {
    // Often we need to go through every queue batch context and apply synchronization operations
    // As usual -- two groups, the "last batch" and the signaled semaphores
    QueueBatchContext::BatchSet queue_batch_contexts = GetQueueBatchSnapshot();

    // Note: The const is to force the reference to const be on all platforms.
    //
    // It's not obivious (nor cross platform consitent), that the batch reference should be const
    // but since it's pointing to the actual *key* for the set it must be. This doesn't make the
    // object the shared pointer is referencing constant however.
    for (const auto &batch : queue_batch_contexts) {
        op(batch);
    }
}

void SyncValidator::UpdateFenceWaitInfo(VkFence fence, QueueId queue_id, ResourceUsageTag tag) {
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, queue_id, tag));
}
void SyncValidator::UpdateFenceWaitInfo(VkFence fence, const PresentedImage &image, ResourceUsageTag tag) {
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, image, tag));
}

void SyncValidator::UpdateFenceWaitInfo(std::shared_ptr<const vvl::Fence> &fence_state, FenceSyncState &&wait_info) {
    if (vvl::StateObject::Invalid(fence_state)) return;
    waitable_fences_[fence_state->VkHandle()] = std::move(wait_info);
}

void SyncValidator::WaitForFence(VkFence fence) {
    auto fence_it = waitable_fences_.find(fence);
    if (fence_it != waitable_fences_.end()) {
        // The fence may no longer be waitable for several valid reasons.
        FenceSyncState &wait_for = fence_it->second;
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

void SyncValidator::UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos) {
    for (const auto &info : vvl::make_span(infos, count)) {
        if (VK_NULL_HANDLE == info.image) continue;
        auto image_state = Get<ImageState>(info.image);
        if (image_state->IsTiled()) {
            image_state->SetOpaqueBaseAddress(*this);
        }
    }
}

const QueueSyncState *SyncValidator::GetQueueSyncState(VkQueue queue) const {
    return syncval_state::GetMappedPlainFromShared(queue_sync_states_, queue);
}

QueueSyncState *SyncValidator::GetQueueSyncState(VkQueue queue) {
    return syncval_state::GetMappedPlainFromShared(queue_sync_states_, queue);
}

std::shared_ptr<const QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) const {
    return syncval_state::GetMapped(queue_sync_states_, queue, []() { return std::shared_ptr<QueueSyncState>(); });
}

std::shared_ptr<QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) {
    return syncval_state::GetMapped(queue_sync_states_, queue, []() { return std::shared_ptr<QueueSyncState>(); });
}

std::shared_ptr<vvl::CommandBuffer> SyncValidator::CreateCmdBufferState(VkCommandBuffer cb,
                                                                        const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                        const vvl::CommandPool *cmd_pool) {
    auto cb_state = std::make_shared<syncval_state::CommandBuffer>(this, cb, pCreateInfo, cmd_pool);
    if (cb_state) {
        cb_state->access_context.SetSelfReference();
    }
    return std::static_pointer_cast<vvl::CommandBuffer>(cb_state);
}

std::shared_ptr<vvl::Swapchain> SyncValidator::CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                                    VkSwapchainKHR swapchain) {
    return std::static_pointer_cast<vvl::Swapchain>(std::make_shared<syncval_state::Swapchain>(this, create_info, swapchain));
}

std::shared_ptr<vvl::Image> SyncValidator::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                            VkFormatFeatureFlags2KHR features) {
    return std::make_shared<ImageState>(this, img, pCreateInfo, features);
}

std::shared_ptr<vvl::Image> SyncValidator::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                            VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                            VkFormatFeatureFlags2KHR features) {
    return std::make_shared<ImageState>(this, img, pCreateInfo, swapchain, swapchain_index, features);
}
std::shared_ptr<vvl::ImageView> SyncValidator::CreateImageViewState(
    const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
    const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageViewState>(image_state, iv, ci, ff, cubic_props);
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcBuffer, error_obj.location,
                             "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<vvl::Buffer>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(pCopyBufferInfo->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfo->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfo->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                // TODO -- add tag information to log msg when useful.
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyBufferInfo->srcBuffer, error_obj.location,
                             "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pCopyBufferInfo->srcBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyBufferInfo->dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pCopyBufferInfo->dstBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
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

void SyncValidator::RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(command);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<vvl::Buffer>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<vvl::Buffer>(pCopyBufferInfo->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfo->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfo->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                   const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, error_obj.location,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, error_obj.location,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyImageInfo->srcImage, error_obj.location,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pCopyImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyImageInfo->dstImage, error_obj.location,
                                 "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pCopyImageInfo->dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
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

void SyncValidator::RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                  const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                               const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask,
                                           dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                                           bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                           pImageMemoryBarriers);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                           pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                           imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                           *pDependencyInfo);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                        const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    ForEachShared<vvl::Queue>([this](const std::shared_ptr<vvl::Queue> &queue_state) {
        auto queue_flags = physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags;
        std::shared_ptr<QueueSyncState> queue_sync_state =
            std::make_shared<QueueSyncState>(queue_state, queue_flags, queue_id_limit_++);
        queue_sync_states_.emplace(std::make_pair(queue_state->VkHandle(), std::move(queue_sync_state)));
    });

    const auto env_debug_command_number = GetEnvironment("VK_SYNCVAL_DEBUG_COMMAND_NUMBER");
    if (!env_debug_command_number.empty()) {
        debug_command_number = static_cast<uint32_t>(std::stoul(env_debug_command_number));
    }
    const auto env_debug_reset_count = GetEnvironment("VK_SYNCVAL_DEBUG_RESET_COUNT");
    if (!env_debug_reset_count.empty()) {
        debug_reset_count = static_cast<uint32_t>(std::stoul(env_debug_reset_count));
    }
    debug_cmdbuf_pattern = GetEnvironment("VK_SYNCVAL_DEBUG_CMDBUF_PATTERN");
    vvl::ToLower(debug_cmdbuf_pattern);
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        SyncOpBeginRenderPass sync_op(error_obj.location.function, *this, pRenderPassBegin, pSubpassBeginInfo);
        skip = sync_op.Validate(cb_state->access_context);
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, error_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     const RecordObject &record_obj) {
    // The state tracker sets up the command buffer state
    StateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    cb_state->access_context.Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        cb_state->access_context.RecordSyncOp<SyncOpBeginRenderPass>(command, *this, pRenderPassBegin, pSubpassBeginInfo);
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                         const RecordObject &record_obj) {
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    SyncOpNextSubpass sync_op(error_obj.location.function, *this, pSubpassBeginInfo, pSubpassEndInfo);
    return sync_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                  const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents, error_obj);
    // Convert to a NextSubpass2
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper();
    skip |= ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, &subpass_end_info, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                      const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
    return skip;
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpNextSubpass>(command, *this, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                 const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    auto *cb_context = &cb_state->access_context;

    SyncOpEndRenderPass sync_op(error_obj.location.function, *this, pSubpassEndInfo);
    skip |= sync_op.Validate(*cb_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, nullptr, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpEndRenderPass>(command, *this, pSubpassEndInfo);
}

// Simple heuristic rule to detect WAW operations representing algorithmically safe or increment
// updates to a resource which do not conflict at the byte level.
// TODO: Revisit this rule to see if it needs to be tighter or looser
// TODO: Add programatic control over suppression heuristics
bool SyncValidator::SupressedBoundDescriptorWAW(const HazardResult &hazard) const {
    assert(hazard.IsHazard());
    return hazard.IsWAWHazard();
}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, nullptr, record_obj.location.function);
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                    const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, record_obj.location.function);
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                       const RecordObject &record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state || !pRenderingInfo) return skip;

    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state(&skip, std::move(cb_state));
    cmd_state->AddRenderingInfo(*this, *pRenderingInfo);

    // We need to set skip, because the TlsGuard destructor is looking at the skip value for RAII cleanup.
    skip = cmd_state->cb_state->access_context.ValidateBeginRendering(error_obj, *cmd_state);
    return skip;
}

void SyncValidator::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                   const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state;

    assert(cmd_state && cmd_state->cb_state && (cmd_state->cb_state->commandBuffer() == commandBuffer));
    // Note: for fine grain locking need to to something other than cast.
    auto cb_state = std::const_pointer_cast<syncval_state::CommandBuffer>(cmd_state->cb_state);
    cb_state->access_context.RecordBeginRendering(*cmd_state, record_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip = cb_state->access_context.ValidateEndRendering(error_obj);
    return skip;
}

void SyncValidator::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void SyncValidator::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdEndRendering(commandBuffer, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;

    cb_state->access_context.RecordEndRendering(record_obj);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        HazardResult hazard;
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format, dst_image->createInfo.arrayLayers));
                hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
                if (hazard.IsHazard()) {
                    // PHASE1 TODO -- add tag information to log msg when useful.
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), srcBuffer, loc,
                                     "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                     string_SyncHazard(hazard.Hazard()), FormatHandle(srcBuffer).c_str(), region,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }

            hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                           copy_region.imageExtent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, loc,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
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
void SyncValidator::RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                               Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_buffer = Get<vvl::Buffer>(srcBuffer);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format, dst_image->createInfo.arrayLayers));
                context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
            }
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                       const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, record_obj.location.function);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->deviceMemory() : VK_NULL_HANDLE;
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                                copy_region.imageExtent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, loc,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (dst_mem) {
                ResourceAccessRange dst_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format, src_image->createInfo.arrayLayers));
                hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
                if (hazard.IsHazard()) {
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, loc,
                                     "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                     string_SyncHazard(hazard.Hazard()), FormatHandle(dstBuffer).c_str(), region,
                                     cb_access_context->FormatHazard(hazard).c_str());
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
void SyncValidator::RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->deviceMemory() : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, tag);
            if (dst_buffer) {
                ResourceAccessRange dst_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format, src_image->createInfo.arrayLayers));
                context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
            }
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                      const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                       const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, record_obj.location.function);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
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
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, loc,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
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
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, loc,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
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

template <typename RegionType>
void SyncValidator::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const RegionType *pRegions, VkFilter filter, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    const auto tag = cb_state->access_context.NextCommandTag(command);
    auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            context->UpdateAccessState(*src_image, SYNC_BLIT_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(blit_region.srcSubresource), offset, extent, tag);
        }
        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            context->UpdateAccessState(*dst_image, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(blit_region.dstSubresource), offset, extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter, record_obj);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                       record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                  const RecordObject &record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                               const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, record_obj.location.function);
}

bool SyncValidator::ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                           VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                           const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                           const Location &loc) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    auto buf_state = Get<vvl::Buffer>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                             "Hazard %s for indirect %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(),
                             cb_context.FormatHazard(hazard).c_str());
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                 "Hazard %s for indirect %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(),
                                 cb_context.FormatHazard(hazard).c_str());
                break;
            }
        }
    }
    return skip;
}

void SyncValidator::RecordIndirectBuffer(AccessContext &context, const ResourceUsageTag tag, const VkDeviceSize struct_size,
                                         const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount,
                                         uint32_t stride) {
    auto buf_state = Get<vvl::Buffer>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range, tag);
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                      tag);
        }
    }
}

bool SyncValidator::ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                        VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        const Location &loc) const {
    bool skip = false;

    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.IsHazard()) {
        skip |=
            LogError(string_SyncHazardVUID(hazard.Hazard()), count_buf_state->buffer(), loc,
                     "Hazard %s for countBuffer %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                     FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(), cb_context.FormatHazard(hazard).c_str());
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(AccessContext &context, const ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset) {
    auto count_buf_state = Get<vvl::Buffer>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range, tag);
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                             const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    const auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    skip |= ValidateIndirectBuffer(cb_state->access_context, *context, commandBuffer, sizeof(VkDispatchIndirectCommand), buffer,
                                   offset, 1, sizeof(VkDispatchIndirectCommand), error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDispatchIndirectCommand), buffer, offset, 1, sizeof(VkDispatchIndirectCommand));
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertex(vertexCount, firstVertex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertexIndex(indexCount, firstIndex, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                              record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    if (drawCount == 0) return;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(std::optional<uint32_t>(), 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   maxDrawCount, stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, error_obj.location);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride, record_obj);
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

void SyncValidator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, maxDrawCount, stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, error_obj.location);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will update the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride, record_obj);
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

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), image, error_obj.location,
                                 "Hazard %s for %s, range index %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(image).c_str(), index, cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), image, error_obj.location,
                                 "Hazard %s for %s, range index %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(image).c_str(), index, cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges,
                                                         record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                       const VkClearRect *pRects, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    for (const auto [attachment_index, attachment] : vvl::enumerate(pAttachments, attachmentCount)) {
        Location attachment_loc = error_obj.location.dot(Field::pAttachments, attachment_index);
        for (const auto [rect_index, rect] : vvl::enumerate(pRects, rectCount)) {
            Location rect_loc = attachment_loc.dot(Field::pRects, rect_index);
            skip |= cb_state->access_context.ValidateClearAttachment(rect_loc, *attachment, *rect);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                     const VkClearRect *pRects, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    auto cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    for (const auto &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto &rect : vvl::make_span(pRects, rectCount)) {
            cb_access_context->RecordClearAttachment(tag, attachment, rect);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    // TODO:Track VkQueryPool
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags,
                                                         const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset,
                                                       stride, flags, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }

    // TODO:Track VkQueryPool
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, error_obj.location,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, error_obj.location,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve *pRegions, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                               pRegions, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const Location image_info_loc = error_obj.location.dot(Field::pResolveImageInfo);
    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const Location region_loc = image_info_loc.dot(Field::pRegions, region);
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pResolveImageInfo->srcImage, region_loc,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pResolveImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pResolveImageInfo->dstImage, region_loc,
                                 "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pResolveImageInfo->dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
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

void SyncValidator::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                  const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                     const RecordObject &record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    const Location decode_info_loc = error_obj.location.dot(Field::pDecodeInfo);

    auto src_buffer = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange);
        auto hazard = context->DetectHazard(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, src_range);
        if (hazard.IsHazard()) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), src_buffer->buffer(), decode_info_loc.dot(Field::srcBuffer),
                             "Hazard %s for bitstream buffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pDecodeInfo->srcBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    auto dst_resource = vvl::VideoPictureResource(this, pDecodeInfo->dstPictureResource);
    if (dst_resource) {
        auto hazard = context->DetectHazard(*vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dst_resource.image_view_state->image_view(),
                             decode_info_loc.dot(Field::dstPictureResource), "Hazard %s for decode output picture. Access info %s.",
                             string_SyncHazard(hazard.Hazard()), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    if (pDecodeInfo->pSetupReferenceSlot != nullptr && pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource && (setup_resource != dst_resource)) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), setup_resource.image_view_state->image_view(),
                                 decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                 "Hazard %s for reconstructed picture. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }

    for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
        if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ);
                if (hazard.IsHazard()) {
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), reference_resource.image_view_state->image_view(),
                                     decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                     "Hazard %s for reference picture #%u. Access info %s.", string_SyncHazard(hazard.Hazard()), i,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                   const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    auto src_buffer = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange);
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, SyncOrdering::kNonAttachment, src_range, tag);
    }

    auto dst_resource = vvl::VideoPictureResource(this, pDecodeInfo->dstPictureResource);
    if (dst_resource) {
        context->UpdateAccessState(*vs_state, dst_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
    }

    if (pDecodeInfo->pSetupReferenceSlot != nullptr && pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource && (setup_resource != dst_resource)) {
            context->UpdateAccessState(*vs_state, setup_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
        if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(*vs_state, reference_resource, SYNC_VIDEO_DECODE_VIDEO_DECODE_READ, tag);
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return skip;

    const Location encode_info_loc = error_obj.location.dot(Field::pEncodeInfo);

    auto dst_buffer = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (dst_buffer) {
        const ResourceAccessRange src_range = MakeRange(*dst_buffer, pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, src_range);
        if (hazard.IsHazard()) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dst_buffer->buffer(), encode_info_loc.dot(Field::dstBuffer),
                             "Hazard %s for bitstream buffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pEncodeInfo->dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    auto src_resource = vvl::VideoPictureResource(this, pEncodeInfo->srcPictureResource);
    if (src_resource) {
        auto hazard = context->DetectHazard(*vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), src_resource.image_view_state->image_view(),
                             encode_info_loc.dot(Field::srcPictureResource), "Hazard %s for encode input picture. Access info %s.",
                             string_SyncHazard(hazard.Hazard()), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    if (pEncodeInfo->pSetupReferenceSlot != nullptr && pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource) {
            auto hazard = context->DetectHazard(*vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), setup_resource.image_view_state->image_view(),
                                 encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                 "Hazard %s for reconstructed picture. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }

    for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
        if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                auto hazard = context->DetectHazard(*vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ);
                if (hazard.IsHazard()) {
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), reference_resource.image_view_state->image_view(),
                                     encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                     "Hazard %s for reference picture #%u. Access info %s.", string_SyncHazard(hazard.Hazard()), i,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                   const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    auto src_buffer = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (src_buffer) {
        const ResourceAccessRange src_range = MakeRange(*src_buffer, pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange);
        context->UpdateAccessState(*src_buffer, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, SyncOrdering::kNonAttachment, src_range, tag);
    }

    auto src_resource = vvl::VideoPictureResource(this, pEncodeInfo->srcPictureResource);
    if (src_resource) {
        context->UpdateAccessState(*vs_state, src_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
    }

    if (pEncodeInfo->pSetupReferenceSlot != nullptr && pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
        auto setup_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
        if (setup_resource) {
            context->UpdateAccessState(*vs_state, setup_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE, tag);
        }
    }

    for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
        if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
            auto reference_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
            if (reference_resource) {
                context->UpdateAccessState(*vs_state, reference_resource, SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ, tag);
            }
        }
    }
}

bool SyncValidator::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

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
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
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
    StateTracker::PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event,
                                             *pDependencyInfo, cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

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
    StateTracker::PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

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
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

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
    StateTracker::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
                                              pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                              imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

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
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, pDependencyInfos);
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<vvl::Buffer>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, error_obj);
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    // Heavyweight, but we need a proxy copy of the active command buffer access context
    CommandBufferAccessContext proxy_cb_context(*cb_context, CommandBufferAccessContext::AsProxyContext());

    // Make working copies of the access and events contexts
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        proxy_cb_context.NextIndexedCommandTag(error_obj.location.function, cb_index);

        const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        const auto *recorded_cb_context = &recorded_cb->access_context;
        assert(recorded_cb_context);

        skip |= ReplayState(proxy_cb_context, *recorded_cb_context, error_obj, cb_index).ValidateFirstUse();

        // The barriers have already been applied in ValidatFirstUse
        ResourceUsageRange tag_range = proxy_cb_context.ImportRecordedAccessLog(*recorded_cb_context);
        proxy_cb_context.ResolveExecutedCommandBuffer(*recorded_cb_context->GetCurrentAccessContext(), tag_range.begin);
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        const ResourceUsageTag cb_tag = cb_context->NextIndexedCommandTag(record_obj.location.function, cb_index);
        const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        cb_context->AddHandle(cb_tag, "pCommandBuffers", recorded_cb->Handle(), cb_index);
        const auto *recorded_cb_context = &recorded_cb->access_context;
        cb_context->RecordExecutedCommandBuffer(*recorded_cb_context);
    }
}

void SyncValidator::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                                  const RecordObject &record_obj) {
    StateTracker::PostCallRecordBindImageMemory(device, image, mem, memoryOffset, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    const VkBindImageMemoryInfo bind_info = ConvertImageMemoryInfo(device, image, mem, memoryOffset);
    UpdateSyncImageMemoryBindState(1, &bind_info);
}

void SyncValidator::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   const RecordObject &record_obj) {
    StateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    UpdateSyncImageMemoryBindState(bindInfoCount, pBindInfos);
}

void SyncValidator::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo *pBindInfos, const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void SyncValidator::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueWaitIdle(queue, record_obj);
    if ((record_obj.result != VK_SUCCESS) || (disabled[sync_validation_queue_submit]) || (queue == VK_NULL_HANDLE)) return;

    const auto queue_state = GetQueueSyncStateShared(queue);
    if (!queue_state) return;  // Invalid queue
    QueueId waited_queue = queue_state->GetQueueId();
    ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);

    // Eliminate waitable fences from the current queue.
    vvl::EraseIf(waitable_fences_, [waited_queue](const SignaledFence &sf) { return sf.second.queue_id == waited_queue; });
}

void SyncValidator::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    StateTracker::PostCallRecordDeviceWaitIdle(device, record_obj);

    // We need to treat this a fence waits for all queues... noting that present engine ops will be preserved.
    ForAllQueueBatchContexts(
        [](const std::shared_ptr<QueueBatchContext> &batch) { batch->ApplyTaggedWait(kQueueAny, ResourceUsageRecord::kMaxIndex); });

    // As we we've waited for everything on device, any waits are mooted. (except for acquires)
    vvl::EraseIf(waitable_fences_, [](SignaledFences::value_type &waitable) { return waitable.second.acquired.Invalid(); });
}

struct QueuePresentCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    std::shared_ptr<QueueBatchContext> present_batch;
    SignaledSemaphores signaled;
    PresentedImages presented_images;
    QueuePresentCmdState(const SignaledSemaphores &parent_semaphores) : signaled(parent_semaphores) {}
};

bool SyncValidator::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (disabled[sync_validation_queue_submit]) return skip;

    vvl::TlsGuard<QueuePresentCmdState> cmd_state(&skip, signaled_semaphores_);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    std::shared_ptr<const QueueBatchContext> last_batch = cmd_state->queue->LastBatch();
    std::shared_ptr<QueueBatchContext> batch(std::make_shared<QueueBatchContext>(*this, *cmd_state->queue, submit_id, 0));

    ResourceUsageRange tag_range = SetupPresentInfo(*pPresentInfo, batch, cmd_state->presented_images);
    batch->SetupAccessContext(last_batch, *pPresentInfo, cmd_state->presented_images, cmd_state->signaled);
    batch->SetupBatchTags(tag_range);
    // Update the present tags
    for (auto &presented : cmd_state->presented_images) {
        presented.tag += batch->GetTagRange().begin;
    }

    skip |= batch->DoQueuePresentValidate(error_obj.location, cmd_state->presented_images);
    batch->DoPresentOperations(cmd_state->presented_images);
    batch->LogPresentOperations(cmd_state->presented_images);
    batch->Cleanup();

    if (!skip) {
        cmd_state->present_batch = std::move(batch);
    }
    return skip;
}

ResourceUsageRange SyncValidator::SetupPresentInfo(const VkPresentInfoKHR &present_info, std::shared_ptr<QueueBatchContext> &batch,
                                                   PresentedImages &presented_images) const {
    const VkSwapchainKHR *const swapchains = present_info.pSwapchains;
    const uint32_t *const image_indices = present_info.pImageIndices;
    const uint32_t swap_count = present_info.swapchainCount;

    // Create the working list of presented images
    presented_images.reserve(swap_count);
    for (uint32_t present_index = 0; present_index < swap_count; present_index++) {
        // Note: Given the "EraseIf" implementation for acquire fence waits, each presentation needs a unique tag.
        const ResourceUsageTag tag = presented_images.size();
        presented_images.emplace_back(*this, batch, swapchains[present_index], image_indices[present_index], present_index, tag);
        if (presented_images.back().Invalid()) {
            presented_images.pop_back();
        }
    }

    // Present is tagged for each swap.
    return ResourceUsageRange(0, presented_images.size());
}

void SyncValidator::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                  const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
    if (disabled[sync_validation_queue_submit]) return;

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueuePresentCmdState> cmd_state;

    // See ValidationStateTracker::PostCallRecordQueuePresentKHR for spec excerpt supporting
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }

    // Update the state with the data from the validate phase
    cmd_state->signaled.Resolve(signaled_semaphores_, cmd_state->present_batch);
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    for (auto &presented : cmd_state->presented_images) {
        presented.ExportToSwapchain(*this);
    }
    queue_state->UpdateLastBatch(std::move(cmd_state->present_batch));
}

void SyncValidator::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                      const RecordObject &record_obj) {
    StateTracker::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
    if (disabled[sync_validation_queue_submit]) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
}

void SyncValidator::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                       uint32_t *pImageIndex, const RecordObject &record_obj) {
    StateTracker::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, record_obj);
    if (disabled[sync_validation_queue_submit]) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj);
}

void SyncValidator::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;

    // Get the image out of the presented list and create apppropriate fences/semaphores.
    auto swapchain_state = Get<syncval_state::Swapchain>(swapchain);
    if (vvl::StateObject::Invalid(swapchain_state)) return;  // Invalid acquire calls to be caught in CoreCheck/Parameter validation

    PresentedImage presented = swapchain_state->MovePresentedImage(*pImageIndex);
    if (presented.Invalid()) return;

    // No way to make access safe, so nothing to record
    if ((semaphore == VK_NULL_HANDLE) && (fence == VK_NULL_HANDLE)) return;

    // We create a queue-less QBC for the Semaphore and fences to wait on

    // Note: this is a heavyweight way to deal with the fact that all operation logs live in the QueueBatchContext... and
    // acquire doesn't happen on a queue, but we need a place to put the acquire operation access record.
    auto batch = std::make_shared<QueueBatchContext>(*this);
    batch->SetupAccessContext(presented);
    ResourceUsageRange acquire_tag_range(0, 1);
    batch->SetupBatchTags(ResourceUsageRange(0, 1));
    const ResourceUsageTag acquire_tag = batch->GetTagRange().begin;
    batch->DoAcquireOperation(presented);
    batch->LogAcquireOperation(presented, record_obj.location.function);

    // Now swap out the present queue batch with the acquired one.
    // Note that fence and signal will read the acquire batch from presented, so this needs to be done before
    // setting up the synchronization
    presented.batch = std::move(batch);

    if (semaphore != VK_NULL_HANDLE) {
        std::shared_ptr<const vvl::Semaphore> sem_state = Get<vvl::Semaphore>(semaphore);

        if (bool(sem_state)) {
            signaled_semaphores_.SignalSemaphore(sem_state, presented, acquire_tag);
        }
    }
    if (fence != VK_NULL_HANDLE) {
        UpdateFenceWaitInfo(fence, presented, acquire_tag);
    }
}

bool SyncValidator::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                               const ErrorObject &error_obj) const {
    auto queue_state = GetQueueSyncStateShared(queue);
    if (!bool(queue_state)) return false;
    SubmitInfoConverter submit_info(submitCount, pSubmits, queue_state->GetQueueFlags());
    return ValidateQueueSubmit(queue, submitCount, submit_info.info2s.data(), fence, error_obj);
}

bool SyncValidator::ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (disabled[sync_validation_queue_submit]) return skip;

    vvl::TlsGuard<QueueSubmitCmdState> cmd_state(&skip, error_obj, signaled_semaphores_);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    // verify each submit batch
    // Since the last batch from the queue state is const, we need to track the last_batch separately from the
    // most recently created batch
    std::shared_ptr<const QueueBatchContext> last_batch = cmd_state->queue->LastBatch();
    std::shared_ptr<QueueBatchContext> batch;
    for (uint32_t batch_idx = 0; batch_idx < submitCount; batch_idx++) {
        const VkSubmitInfo2 &submit = pSubmits[batch_idx];
        batch = std::make_shared<QueueBatchContext>(*this, *cmd_state->queue, submit_id, batch_idx);
        batch->SetupCommandBufferInfo(submit);
        batch->SetupAccessContext(last_batch, submit, cmd_state->signaled);

        // Skip import and validation of empty batches
        if (batch->GetTagRange().size()) {
            batch->SetupBatchTags();
            skip |= batch->DoQueueSubmitValidate(*this, *cmd_state, submit);
        }

        // Empty batches could have semaphores, though.
        for (uint32_t sem_idx = 0; sem_idx < submit.signalSemaphoreInfoCount; ++sem_idx) {
            const VkSemaphoreSubmitInfo &semaphore_info = submit.pSignalSemaphoreInfos[sem_idx];
            // Make a copy of the state, signal the copy and pend it...
            auto sem_state = Get<vvl::Semaphore>(semaphore_info.semaphore);
            if (!sem_state) continue;
            cmd_state->signaled.SignalSemaphore(sem_state, batch, semaphore_info);
        }
        // Unless the previous batch was referenced by a signal, the QueueBatchContext will self destruct, but as
        // we ResolvePrevious as we can let any contexts we've fully referenced go.
        batch->Cleanup();  // Clear the temporaries that the batch holds.
        last_batch = batch;
    }
    // The most recently created batch will become the queue's "last batch" in the record phase
    if (batch) {
        cmd_state->last_batch = std::move(batch);
    }

    // Note that if we skip, guard cleans up for us, but cannot release the reserved tag range
    return skip;
}

void SyncValidator::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                              const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    RecordQueueSubmit(queue, fence, record_obj);
}

void SyncValidator::RecordQueueSubmit(VkQueue queue, VkFence fence, const RecordObject &record_obj) {
    // If this return is above the TlsGuard, then the Validate phase return must also be.
    if (disabled[sync_validation_queue_submit]) return;  // Queue submit validation disabled

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state;

    if (VK_SUCCESS != record_obj.result) return;  // dispatched QueueSubmit failed
    if (!cmd_state->queue) return;     // Validation couldn't find a valid queue object

    // Don't need to look up the queue state again, but we need a non-const version
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));

    cmd_state->signaled.Resolve(signaled_semaphores_, cmd_state->last_batch);
    queue_state->UpdateLastBatch(std::move(cmd_state->last_batch));

    ResourceUsageRange fence_tag_range = ReserveGlobalTagRange(1U);
    UpdateFenceWaitInfo(fence, queue_state->GetQueueId(), fence_tag_range.begin);
}

bool SyncValidator::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                   VkFence fence, const ErrorObject &error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool SyncValidator::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                VkFence fence, const ErrorObject &error_obj) const {
    return ValidateQueueSubmit(queue, submitCount, pSubmits, fence, error_obj);
}

void SyncValidator::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                  VkFence fence, const RecordObject &record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}
void SyncValidator::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                               const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit(queue, fence, record_obj);
}

void SyncValidator::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    StateTracker::PostCallRecordGetFenceStatus(device, fence, record_obj);
    if (disabled[sync_validation_queue_submit]) return;
    if (record_obj.result == VK_SUCCESS) {
        // fence is signalled, mark it as waited for
        WaitForFence(fence);
    }
}

void SyncValidator::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject &record_obj) {
    StateTracker::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, record_obj);
    if (disabled[sync_validation_queue_submit]) return;
    if ((record_obj.result == VK_SUCCESS) && ((VK_TRUE == waitAll) || (1 == fenceCount))) {
        // We can only know the pFences have signal if we waited for all of them, or there was only one of them
        for (uint32_t i = 0; i < fenceCount; i++) {
            WaitForFence(pFences[i]);
        }
    }
}

void SyncValidator::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                        VkImage *pSwapchainImages, const RecordObject &record_obj) {
    StateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, record_obj);
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    auto swapchain_state = Get<vvl::Swapchain>(swapchain);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            vvl::SwapchainImage &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) {
                auto *sync_image = static_cast<ImageState *>(swapchain_image.image_state);
                assert(sync_image->IsTiled());  // This is the assumption from the spec, and the implementation relies on it
                sync_image->SetOpaqueBaseAddress(*this);
            }
        }
    }
}

void SyncValidator::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo,
                                                            const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo, record_obj);
    if (auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer)) {
        cb_state->access_context.PushDebugRegion(pLabelInfo ? pLabelInfo->pLabelName : nullptr);
    }
}

void SyncValidator::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer, record_obj);
    if (auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer)) {
        cb_state->access_context.PopDebugRegion();
    }
}

bool syncval_state::ImageState::IsSimplyBound() const {
    bool simple = SimpleBinding(static_cast<const vvl::Bindable &>(*this)) || IsSwapchainImage() || bind_swapchain;

    // If it's not simple we must have an encoder.
    assert(!simple || fragment_encoder.get());

    return simple;
}

void syncval_state::ImageState::SetOpaqueBaseAddress(ValidationStateTracker &dev_data) {
    // This is safe to call if already called to simplify caller logic
    // NOTE: Not asserting IsTiled, as there could in future be other reasons for opaque representations
    if (opaque_base_address_) return;

    VkDeviceSize opaque_base = 0U;  // Fakespace Allocator starts > 0
    auto get_opaque_base = [&opaque_base](const vvl::Image &other) {
        const ImageState &other_sync = static_cast<const ImageState &>(other);
        opaque_base = other_sync.opaque_base_address_;
        return true;
    };
    if (IsSwapchainImage()) {
        AnyAliasBindingOf(bind_swapchain->ObjectBindings(), get_opaque_base);
    } else {
        AnyImageAliasOf(get_opaque_base);
    }
    if (!opaque_base) {
        // The size of the opaque range is based on the SyncVal *internal* representation of the tiled resource, unrelated
        // to the acutal size of the the resource in device memory. If differing representations become possible, the allocated
        // size would need to be changed to those representation's size requirements.
        opaque_base = dev_data.AllocFakeMemory(fragment_encoder->TotalSize());
    }
    opaque_base_address_ = opaque_base;
}

VkDeviceSize syncval_state::ImageState::GetResourceBaseAddress() const {
    if (HasOpaqueMapping()) {
        return GetOpaqueBaseAddress();
    }
    return GetFakeBaseAddress();
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    ImageRangeGen range_gen(*fragment_encoder.get(), subresource_range, base_address, is_depth_sliced);
    return range_gen;
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           const VkOffset3D &offset, const VkExtent3D &extent,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    subresource_adapter::ImageRangeGenerator range_gen(*fragment_encoder.get(), subresource_range, offset, extent, base_address,
                                                       is_depth_sliced);
    return range_gen;
}

syncval_state::ImageViewState::ImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv,
                                              const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                                              const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props)
    : vvl::ImageView(image_state, iv, ci, ff, cubic_props), view_range_gen(MakeImageRangeGen()) {}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen() const {
    return GetImageState()->MakeImageRangeGen(normalized_subresource_range, IsDepthSliced());
}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen(const VkOffset3D &offset, const VkExtent3D &extent,
                                                               const VkImageAspectFlags aspect_mask) const {
    if (Invalid()) ImageRangeGen();

    // Intentional copy
    VkImageSubresourceRange subresource_range = normalized_subresource_range;
    if (aspect_mask) {
        subresource_range.aspectMask &= aspect_mask;
        assert(subresource_range.aspectMask);
    }

    return GetImageState()->MakeImageRangeGen(subresource_range, offset, extent, IsDepthSliced());
}

