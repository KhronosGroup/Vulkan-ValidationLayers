/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "best_practices/bp_state.h"
#include "state_tracker/queue_state.h"
#include "generated/dispatch_functions.h"

bool BestPractices::CheckDependencyInfo(const LogObjectList& objlist, const Location& dep_loc,
                                        const VkDependencyInfo& dep_info, VkCommandBuffer commandBuffer) const {
    bool skip = false;
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(
            dep_loc.dot(Field::pImageMemoryBarriers, i), commandBuffer, dep_info.pImageMemoryBarriers[i].image,
            dep_info.pImageMemoryBarriers[i].oldLayout, dep_info.pImageMemoryBarriers[i].newLayout,
            dep_info.pImageMemoryBarriers[i].srcAccessMask, dep_info.pImageMemoryBarriers[i].dstAccessMask,
            dep_info.pImageMemoryBarriers[i].subresourceRange.aspectMask, dep_info.pImageMemoryBarriers[i].srcQueueFamilyIndex,
            dep_info.pImageMemoryBarriers[i].dstQueueFamilyIndex);
    }
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; ++i) {
        skip |= ValidateBufferMemoryBarrier(
            dep_loc.dot(Field::pBufferMemoryBarriers, i), commandBuffer, dep_info.pBufferMemoryBarriers[i].buffer,
            dep_info.pBufferMemoryBarriers[i].srcQueueFamilyIndex, dep_info.pBufferMemoryBarriers[i].dstQueueFamilyIndex);
    }

    return skip;
}

bool BestPractices::CheckEventSignalingState(const bp_state::CommandBufferSubState& command_buffer, VkEvent event,
                                             const Location& cb_loc) const {
    bool skip = false;
    if (auto* signaling_info = vvl::Find(command_buffer.event_signaling_state, event); signaling_info && signaling_info->signaled) {
        const LogObjectList objlist(command_buffer.VkHandle(), event);
        skip |= LogWarning("BestPractices-Event-SignalSignaledEvent", objlist, cb_loc,
                           "%s sets event %s which was already set (in this command buffer or in the executed secondary command "
                           "buffers). If this is not the desired behavior, the event must be reset before it is set again.",
                           FormatHandle(command_buffer.VkHandle()).c_str(), FormatHandle(event).c_str());
    }
    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    auto& sub_state = bp_state::SubState(*cb_state);
    skip |= CheckEventSignalingState(sub_state, event, error_obj.location.dot(Field::commandBuffer));
    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR* pDependencyInfo, const ErrorObject& error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= CheckDependencyInfo(commandBuffer, error_obj.location.dot(Field::pDependencyInfo), *pDependencyInfo, commandBuffer);
    auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
    auto& sub_state = bp_state::SubState(*cb_state);
    skip |= CheckEventSignalingState(sub_state, event, error_obj.location.dot(Field::commandBuffer));
    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask, const ErrorObject& error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

bool BestPractices::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                     const VkDependencyInfoKHR* pDependencyInfos,
                                                     const ErrorObject& error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool BestPractices::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip |= CheckDependencyInfo(commandBuffer, error_obj.location.dot(Field::pDependencyInfos, i), pDependencyInfos[i],
                                    commandBuffer);
    }

    return skip;
}

bool BestPractices::ValidateAccessLayoutCombination(const Location& loc, VkImage image, VkAccessFlags2 access, VkImageLayout layout,
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
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
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
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                           VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                           VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
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
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                           VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                           VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
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
        skip |= LogWarning("BestPractices-ImageBarrierAccessLayout", image, loc,
                           "image is %s and accessMask is %s, but for layout %s expected accessMask are %s.",
                           FormatHandle(image).c_str(), string_VkAccessFlags2(access).c_str(), string_VkImageLayout(layout),
                           string_VkAccessFlags2(allowed).c_str());
    }

    return skip;
}

bool BestPractices::ValidateImageMemoryBarrier(const Location& loc, VkCommandBuffer commandBuffer, VkImage image,
                                               VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask,
                                               VkAccessFlags2 dstAccessMask, VkImageAspectFlags aspectMask,
                                               uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) const {
    bool skip = false;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && IsImageLayoutReadOnly(newLayout)) {
        skip |= LogWarning("BestPractices-ImageMemoryBarrier-TransitionUndefinedToReadOnly", image, loc,
                           "VkImageMemoryBarrier is being submitted with oldLayout VK_IMAGE_LAYOUT_UNDEFINED and the contents "
                           "may be discarded, but the newLayout is %s, which is read only.",
                           string_VkImageLayout(newLayout));
    }

    if (device_state->special_supported.has_maintenance9 && srcQueueFamilyIndex != dstQueueFamilyIndex &&
        srcQueueFamilyIndex != VK_QUEUE_FAMILY_FOREIGN_EXT && srcQueueFamilyIndex != VK_QUEUE_FAMILY_EXTERNAL &&
        dstQueueFamilyIndex != VK_QUEUE_FAMILY_FOREIGN_EXT && dstQueueFamilyIndex != VK_QUEUE_FAMILY_EXTERNAL) {
        auto image_state = Get<vvl::Image>(image);
        const char* warning = enabled_features.maintenance9
                                  ? "is not required, because maintenance9 is enabled"
                                  : "could be omitted, if maintenance9 (which is supported by the physical device) were enabled";
        if (image_state->create_info.tiling == VK_IMAGE_TILING_LINEAR) {
            skip |= LogPerformanceWarning("BestPractices-PipelineBarrier-unneeded-QFOT", image, loc,
                                          "A queue family ownership transfer is being performed on %s, but this %s. Image was "
                                          "created with VK_IMAGE_TILING_LINEAR.",
                                          FormatHandle(image).c_str(), warning);
        } else if ((image_state->create_info.usage &
                    (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) ==
                   0) {
            auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
            VkQueueFamilyOwnershipTransferPropertiesKHR qfot_props = vku::InitStructHelper();
            uint32_t qf_count = dstQueueFamilyIndex + 1;
            std::vector<VkQueueFamilyProperties2> qf_props(qf_count);
            qf_props.back().pNext = &qfot_props;
            DispatchGetPhysicalDeviceQueueFamilyProperties2(cb_state->dev_data.physical_device, &qf_count, qf_props.data());
            // The list of image usages not allowed comes from
            // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSharingMode.html
            if (qfot_props.optimalImageTransferToQueueFamilies) {
                skip |= LogPerformanceWarning(
                    "BestPractices-PipelineBarrier-unneeded-QFOT", image, loc,
                    "A queue family ownership transfer is being performed on %s, but this %s. Image was created with "
                    "VK_IMAGE_TILING_OPTIMAL, image usage does not contain any of VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | "
                    "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | "
                    "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT | "
                    "VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR and "
                    "VkQueueFamilyOwnershipTransferPropertiesKHR::optimalImageTransferToQueueFamilies (%" PRIu32
                    ") has bit set for destination queue family index %" PRIu32 ".",
                    FormatHandle(image).c_str(), warning, qfot_props.optimalImageTransferToQueueFamilies, dstQueueFamilyIndex);
            }
        }
    }

    skip |= ValidateAccessLayoutCombination(loc, image, srcAccessMask, oldLayout, aspectMask);
    skip |= ValidateAccessLayoutCombination(loc, image, dstAccessMask, newLayout, aspectMask);

    return skip;
}

bool BestPractices::ValidateBufferMemoryBarrier(const Location& loc, VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) const {
    bool skip = false;

    if (device_state->special_supported.has_maintenance9 && srcQueueFamilyIndex != dstQueueFamilyIndex &&
        srcQueueFamilyIndex != VK_QUEUE_FAMILY_FOREIGN_EXT && srcQueueFamilyIndex != VK_QUEUE_FAMILY_EXTERNAL &&
        dstQueueFamilyIndex != VK_QUEUE_FAMILY_FOREIGN_EXT && dstQueueFamilyIndex != VK_QUEUE_FAMILY_EXTERNAL) {
        const char* warning = enabled_features.maintenance9
                                  ? "is not required, because maintenance9 is enabled"
                                  : "could be omitted, if maintenance9 (which is supported by the physical device) were enabled";
        skip |= LogPerformanceWarning("BestPractices-PipelineBarrier-unneeded-QFOT", buffer, loc,
                                      "A queue family ownership transfer is being performed on %s, but this %s.",
                                      FormatHandle(buffer).c_str(), warning);
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(
            error_obj.location.dot(Field::pImageMemoryBarriers, i), commandBuffer, pImageMemoryBarriers[i].image,
            pImageMemoryBarriers[i].oldLayout, pImageMemoryBarriers[i].newLayout, pImageMemoryBarriers[i].srcAccessMask,
            pImageMemoryBarriers[i].dstAccessMask, pImageMemoryBarriers[i].subresourceRange.aspectMask,
            pImageMemoryBarriers[i].srcQueueFamilyIndex, pImageMemoryBarriers[i].dstQueueFamilyIndex);
    }
    for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i) {
        skip |= ValidateBufferMemoryBarrier(error_obj.location.dot(Field::pBufferMemoryBarriers, i), commandBuffer,
                                            pBufferMemoryBarriers[i].buffer, pBufferMemoryBarriers[i].srcQueueFamilyIndex,
                                            pBufferMemoryBarriers[i].dstQueueFamilyIndex);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        const uint32_t num = num_barriers_objects_.load();
        const uint32_t total_barriers = num + imageMemoryBarrierCount + bufferMemoryBarrierCount;
        if (total_barriers > kMaxRecommendedBarriersSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CmdBuffer-highBarrierCount", commandBuffer, error_obj.location,
                                          "%s In this frame, %" PRIu32 " barriers were already submitted (%" PRIu32
                                          " if you include image and buffer barriers too). Barriers have a high cost and can "
                                          "stall the GPU.\nTotal recommended max is %" PRIu32
                                          ". Consider consolidating and re-organizing the frame to use fewer barriers.",
                                          VendorSpecificTag(kBPVendorAMD), num, total_barriers, kMaxRecommendedBarriersSizeAMD);
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
                skip |= LogPerformanceWarning("BestPractices-PipelineBarrier-readToReadBarrier", commandBuffer, error_obj.location,
                                              "%s %s Don't issue read-to-read barriers. "
                                              "Get the resource in the right state the first time you use it.",
                                              VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
            }

            // general with no storage
            if (VendorCheckEnabled(kBPVendorAMD) && image_barrier.newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                auto image_state = Get<vvl::Image>(pImageMemoryBarriers[i].image);
                if (image_state && !(image_state->create_info.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                    const LogObjectList objlist(commandBuffer, pImageMemoryBarriers[i].image);
                    skip |= LogPerformanceWarning("BestPractices-AMD-vkImage-AvoidGeneral", objlist,
                                                  error_obj.location.dot(Field::pImageMemoryBarriers, i).dot(Field::image),
                                                  "%s VK_IMAGE_LAYOUT_GENERAL should only be used with "
                                                  "VK_IMAGE_USAGE_STORAGE_BIT images.",
                                                  VendorSpecificTag(kBPVendorAMD));
                }
            }
        }
    }

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i],
                                                       error_obj.location.dot(Field::pImageMemoryBarriers, i));
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                                          const ErrorObject& error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    skip |= CheckDependencyInfo(commandBuffer, dep_info_loc, *pDependencyInfo, commandBuffer);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i],
                                                       dep_info_loc.dot(Field::pImageMemoryBarriers, i));
    }

    return skip;
}

template <typename ImageMemoryBarrier>
bool BestPractices::ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier,
                                                           const Location& loc) const {
    bool skip = false;

    const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && barrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            const auto& sub_state = bp_state::SubState(*cb_state);
            skip |= ValidateZcull(sub_state, barrier.image, barrier.subresourceRange, loc);
        }
    }

    return skip;
}

template <typename Func>
static void ForEachSubresource(const vvl::Image& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layer_count =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t level_count =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layer_count; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < level_count; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    num_barriers_objects_ += (memoryBarrierCount + imageMemoryBarrierCount + bufferMemoryBarrierCount);
}

bool BestPractices::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        const size_t count = Count<vvl::Semaphore>();
        if (count > kMaxRecommendedSemaphoreObjectsSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-SyncObjects-HighNumberOfSemaphores", device, error_obj.location,
                                          "%s %s High number of vkSemaphore objects created. "
                                          "%zu created, but recommended max is %" PRIu32
                                          ".\nMinimize the amount of queue synchronization that is used. "
                                          "Each semaphore has a CPU and GPU overhead cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), count,
                                          kMaxRecommendedSemaphoreObjectsSizeAMD);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                               const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        const size_t count = Count<vvl::Fence>();
        if (count > kMaxRecommendedFenceObjectsSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-SyncObjects-HighNumberOfFences", device, error_obj.location,
                                          "%s %s High number of VkFence objects created. "
                                          "%zu created, but recommended max is %" PRIu32
                                          ".\nMinimize the amount of CPU-GPU synchronization that is used. "
                                          "Each fence has a CPU and GPU overhead cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), count,
                                          kMaxRecommendedFenceObjectsSizeAMD);
        }
    }

    return skip;
}
