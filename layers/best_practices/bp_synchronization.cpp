/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"
#include "sync/sync_utils.h"

bool BestPractices::CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags2KHR flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) {
        skip |= LogWarning(device, kVUID_BestPractices_PipelineStageFlags,
                           "You are using VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::CheckDependencyInfo(const std::string& api_name, const VkDependencyInfoKHR& dep_info) const {
    bool skip = false;
    auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);

    skip |= CheckPipelineStageFlags(api_name, stage_masks.src);
    skip |= CheckPipelineStageFlags(api_name, stage_masks.dst);
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(
            api_name, dep_info.pImageMemoryBarriers[i].oldLayout, dep_info.pImageMemoryBarriers[i].newLayout,
            dep_info.pImageMemoryBarriers[i].srcAccessMask, dep_info.pImageMemoryBarriers[i].dstAccessMask,
            dep_info.pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdSetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR* pDependencyInfo) const {
    return CheckDependencyInfo("vkCmdSetEvent2KHR", *pDependencyInfo);
}

bool BestPractices::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo) const {
    return CheckDependencyInfo("vkCmdSetEvent2", *pDependencyInfo);
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                 VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent2KHR", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                  VkPipelineStageFlags2 stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent2", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                     const VkDependencyInfoKHR* pDependencyInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip = CheckDependencyInfo("vkCmdWaitEvents2KHR", pDependencyInfos[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip = CheckDependencyInfo("vkCmdWaitEvents2", pDependencyInfos[i]);
    }

    return skip;
}

bool BestPractices::ValidateAccessLayoutCombination(const std::string& api_name, VkAccessFlags2 access, VkImageLayout layout,
                                                    VkImageAspectFlags aspect) const {
    bool skip = false;

    const VkAccessFlags2 all = vvl::kU64Max;  // core validation is responsible for detecting undefined flags.
    VkAccessFlags2 allowed = 0;

    // Combinations taken from https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2918
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                      VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            allowed = VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            allowed = VK_ACCESS_NONE;  // PR table says "Must be 0"
            break;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            allowed = all;
            break;
        // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            // alias VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV
            allowed = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            allowed = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
            break;
        default:
            // If a new layout is added, will need to manually add it
            return false;
    }

    if ((allowed | access) != allowed) {
        skip |=
            LogWarning(device, kVUID_BestPractices_ImageBarrierAccessLayout,
                       "%s: accessMask is %s, but for layout %s expected accessMask are %s.", api_name.c_str(),
                       string_VkAccessFlags2(access).c_str(), string_VkImageLayout(layout), string_VkAccessFlags2(allowed).c_str());
    }

    return skip;
}

bool BestPractices::ValidateImageMemoryBarrier(const std::string& api_name, VkImageLayout oldLayout, VkImageLayout newLayout,
                                               VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                               VkImageAspectFlags aspectMask) const {
    bool skip = false;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && IsImageLayoutReadOnly(newLayout)) {
        skip |= LogWarning(device, kVUID_BestPractices_TransitionUndefinedToReadOnly,
                           "VkImageMemoryBarrier is being submitted with oldLayout VK_IMAGE_LAYOUT_UNDEFINED and the contents "
                           "may be discarded, but the newLayout is %s, which is read only.",
                           string_VkImageLayout(newLayout));
    }

    skip |= ValidateAccessLayoutCombination(api_name, srcAccessMask, oldLayout, aspectMask);
    skip |= ValidateAccessLayoutCombination(api_name, dstAccessMask, newLayout, aspectMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", dstStageMask);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |=
            ValidateImageMemoryBarrier("vkCmdPipelineBarrier", pImageMemoryBarriers[i].oldLayout, pImageMemoryBarriers[i].newLayout,
                                       pImageMemoryBarriers[i].srcAccessMask, pImageMemoryBarriers[i].dstAccessMask,
                                       pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto num = num_barriers_objects_.load();
        if (num + imageMemoryBarrierCount + bufferMemoryBarrierCount > kMaxRecommendedBarriersSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_CmdBuffer_highBarrierCount,
                                          "%s Performance warning: In this frame, %" PRIu32
                                          " barriers were already submitted. Barriers have a high cost and can "
                                          "stall the GPU. "
                                          "Consider consolidating and re-organizing the frame to use fewer barriers.",
                                          VendorSpecificTag(kBPVendorAMD), num);
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        static constexpr std::array<VkImageLayout, 3> read_layouts = {
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        };

        for (uint32_t i = 0; i < imageMemoryBarrierCount; i++) {
            // read to read barriers
            const auto& image_barrier = pImageMemoryBarriers[i];
            const bool old_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.oldLayout) != read_layouts.end();
            const bool new_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.newLayout) != read_layouts.end();

            if (old_is_read_layout && new_is_read_layout) {
                skip |= LogPerformanceWarning(device, kVUID_BestPractices_PipelineBarrier_readToReadBarrier,
                                              "%s %s Performance warning: Don't issue read-to-read barriers. "
                                              "Get the resource in the right state the first time you use it.",
                                              VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
            }

            // general with no storage
            if (VendorCheckEnabled(kBPVendorAMD) && image_barrier.newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                auto image_state = Get<IMAGE_STATE>(pImageMemoryBarriers[i].image);
                if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                    skip |= LogPerformanceWarning(device, kVUID_BestPractices_vkImage_AvoidGeneral,
                                                  "%s Performance warning: VK_IMAGE_LAYOUT_GENERAL should only be used with "
                                                  "VK_IMAGE_USAGE_STORAGE_BIT images.",
                                                  VendorSpecificTag(kBPVendorAMD));
                }
            }
        }
    }

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                          const VkDependencyInfoKHR* pDependencyInfo) const {
    bool skip = false;

    skip |= CheckDependencyInfo("vkCmdPipelineBarrier2KHR", *pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                       const VkDependencyInfo* pDependencyInfo) const {
    bool skip = false;

    skip |= CheckDependencyInfo("vkCmdPipelineBarrier2", *pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }

    return skip;
}

template <typename ImageMemoryBarrier>
bool BestPractices::ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) const {
    bool skip = false;

    const auto cmd_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    assert(cmd_state);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && barrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= ValidateZcull(*cmd_state, barrier.image, barrier.subresourceRange);
        }
    }

    return skip;
}

template <typename Func>
static void ForEachSubresource(const IMAGE_STATE& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layerCount =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t levelCount =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layerCount; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < levelCount; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

template <typename ImageMemoryBarrier>
void BestPractices::RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) {
    auto cb = Get<bp_state::CommandBuffer>(commandBuffer);
    assert(cb);

    // Is a queue ownership acquisition barrier
    if (barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex &&
        barrier.dstQueueFamilyIndex == cb->command_pool->queueFamilyIndex) {
        auto image = Get<bp_state::Image>(barrier.image);
        auto subresource_range = barrier.subresourceRange;
        cb->queue_submit_functions.push_back([image, subresource_range](const ValidationStateTracker& vst, const QUEUE_STATE& qs,
                                                                        const CMD_BUFFER_STATE& cbs) -> bool {
            ForEachSubresource(*image, subresource_range, [&](uint32_t layer, uint32_t level) {
                // Update queue family index without changing usage, signifying a correct queue family transfer
                image->UpdateUsage(layer, level, image->GetUsageType(layer, level), qs.queueFamilyIndex);
            });
            return false;
        });
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordResetZcullDirection(*cb, barrier.image, barrier.subresourceRange);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                     VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                     uint32_t bufferMemoryBarrierCount,
                                                     const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                     uint32_t imageMemoryBarrierCount,
                                                     const VkImageMemoryBarrier* pImageMemoryBarriers) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags,
                                                             memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                             pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

    num_barriers_objects_ += (memoryBarrierCount + imageMemoryBarrierCount + bufferMemoryBarrierCount);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    ValidationStateTracker::PostCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }
}

bool BestPractices::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (Count<SEMAPHORE_STATE>() > kMaxRecommendedSemaphoreObjectsSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_SyncObjects_HighNumberOfSemaphores,
                                          "%s %s Performance warning: High number of vkSemaphore objects created. "
                                          "Minimize the amount of queue synchronization that is used. "
                                          "Semaphores and fences have overhead. Each fence has a CPU and GPU cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (Count<FENCE_STATE>() > kMaxRecommendedFenceObjectsSizeAMD) {
            skip |= LogPerformanceWarning(device, kVUID_BestPractices_SyncObjects_HighNumberOfFences,
                                          "%s %s Performance warning: High number of VkFence objects created."
                                          "Minimize the amount of CPU-GPU synchronization that is used. "
                                          "Semaphores and fences have overhead. Each fence has a CPU and GPU cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}
