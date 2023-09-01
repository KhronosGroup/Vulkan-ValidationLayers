/*
 * Copyright (c) 2019-2021, 2023 Valve Corporation
 * Copyright (c) 2019-2021, 2023 LunarG, Inc.
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

#pragma once
#include "generated/sync_validation_types.h"
#include "generated/vk_object_types.h"
#include <vulkan/vulkan.h>
#include <string>

// Remove Windows trojan macro that prevents usage of this name in any scope of the program.
// For example, nested namespace type sync_utils::MemoryBarrier won't compile because of this.
#if defined(VK_USE_PLATFORM_WIN32_KHR) && defined(MemoryBarrier)
#undef MemoryBarrier
#endif

struct DeviceFeatures;
class ValidationStateTracker;
class BUFFER_STATE;
class IMAGE_STATE;

namespace sync_utils {

static constexpr VkQueueFlags kAllQueueTypes = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

VkPipelineStageFlags2KHR DisabledPipelineStages(const DeviceFeatures& features);

// Expand all pipeline stage bits. If queue_flags and disabled_feature_mask is provided, the expansion of ALL_COMMANDS_BIT
// and ALL_GRAPHICS_BIT will be limited to what is supported.
VkPipelineStageFlags2KHR ExpandPipelineStages(VkPipelineStageFlags2KHR stage_mask, VkQueueFlags queue_flags = kAllQueueTypes,
                                              const VkPipelineStageFlags2KHR disabled_feature_mask = 0);

VkAccessFlags2KHR ExpandAccessFlags(VkAccessFlags2KHR access_mask);

VkAccessFlags2KHR CompatibleAccessMask(VkPipelineStageFlags2KHR stage_mask);

VkPipelineStageFlags2KHR WithEarlierPipelineStages(VkPipelineStageFlags2KHR stage_mask);

VkPipelineStageFlags2KHR WithLaterPipelineStages(VkPipelineStageFlags2KHR stage_mask);

std::string StringPipelineStageFlags(VkPipelineStageFlags2KHR mask);

std::string StringAccessFlags(VkAccessFlags2KHR mask);

struct ExecScopes {
    VkPipelineStageFlags2KHR src;
    VkPipelineStageFlags2KHR dst;
};
ExecScopes GetGlobalStageMasks(const VkDependencyInfoKHR& dep_info);

struct ShaderStageAccesses {
    SyncStageAccessIndex sampled_read;
    SyncStageAccessIndex storage_read;
    SyncStageAccessIndex storage_write;
    SyncStageAccessIndex uniform_read;
};
ShaderStageAccesses GetShaderStageAccesses(VkShaderStageFlagBits shader_stage);

struct MemoryBarrier {
    VkPipelineStageFlags2 srcStageMask;
    VkAccessFlags2 srcAccessMask;
    VkPipelineStageFlags2 dstStageMask;
    VkAccessFlags2 dstAccessMask;

    explicit MemoryBarrier(const VkMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit MemoryBarrier(const VkBufferMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit MemoryBarrier(const VkImageMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
};

// QueueFamilyBarrier is not a real barrier (there are no queue family barriers in vulkan),
// but is part of a buffer/image barrier structure (still can be created separately if needed).
// This type (and also MemoryBarrier) can be used by the functionality that does not need
// buffer/image specific information.
struct QueueFamilyBarrier : MemoryBarrier {
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;

    QueueFamilyBarrier(const VkBufferMemoryBarrier2& barrier)
        : MemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    QueueFamilyBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                       VkPipelineStageFlags dst_stage_mask)
        : MemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    QueueFamilyBarrier(const VkImageMemoryBarrier2& barrier)
        : MemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    QueueFamilyBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                       VkPipelineStageFlags dst_stage_mask)
        : MemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
};

struct BufferBarrier : QueueFamilyBarrier {
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;

    explicit BufferBarrier(const VkBufferMemoryBarrier2& barrier)
        : QueueFamilyBarrier(barrier), buffer(barrier.buffer), offset(barrier.offset), size(barrier.size) {}
    BufferBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : QueueFamilyBarrier(barrier, src_stage_mask, dst_stage_mask),
          buffer(barrier.buffer),
          offset(barrier.offset),
          size(barrier.size) {}

    VulkanTypedHandle GetTypedHandle() const { return VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer); }
    const std::shared_ptr<const BUFFER_STATE> GetResourceState(const ValidationStateTracker& state_tracker) const;
};

struct ImageBarrier : QueueFamilyBarrier {
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkImage image;
    VkImageSubresourceRange subresourceRange;

    explicit ImageBarrier(const VkImageMemoryBarrier2& barrier)
        : QueueFamilyBarrier(barrier),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}
    ImageBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : QueueFamilyBarrier(barrier, src_stage_mask, dst_stage_mask),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}

    VulkanTypedHandle GetTypedHandle() const { return VulkanTypedHandle(image, kVulkanObjectTypeImage); }
    const std::shared_ptr<const IMAGE_STATE> GetResourceState(const ValidationStateTracker& state_tracker) const;
};

}  // namespace sync_utils
