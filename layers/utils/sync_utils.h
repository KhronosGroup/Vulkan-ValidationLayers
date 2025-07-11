/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Copyright (C) 2025 Arm Limited.
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

#include "hash_util.h"
#include "generated/error_location_helper.h"
#include <vulkan/vulkan.h>
#include <string>

struct DeviceFeatures;
struct DeviceExtensions;

namespace vvl {
class CommandBuffer;
}  // namespace vvl

struct SyncMemoryBarrier {
    VkPipelineStageFlags2 srcStageMask;
    VkAccessFlags2 srcAccessMask;
    VkPipelineStageFlags2 dstStageMask;
    VkAccessFlags2 dstAccessMask;

    explicit SyncMemoryBarrier(const VkMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit SyncMemoryBarrier(const VkBufferMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit SyncMemoryBarrier(const VkImageMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit SyncMemoryBarrier(const VkTensorMemoryBarrierARM& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    SyncMemoryBarrier(const VkMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    SyncMemoryBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                      VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    SyncMemoryBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    SyncMemoryBarrier(const VkTensorMemoryBarrierARM& barrier, VkPipelineStageFlags src_stage_mask,
                      VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
};

enum class OwnershipTransferOp { none, release, acquire };

// OwnershipTransferBarrier is not a standalone barrier type; it is part of a buffer/image barrier.
// Similar to MemoryBarrier, it can be used when buffer/image specific information is not needed.
struct OwnershipTransferBarrier : SyncMemoryBarrier {
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;

    OwnershipTransferBarrier(const VkBufferMemoryBarrier2& barrier)
        : SyncMemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                             VkPipelineStageFlags dst_stage_mask)
        : SyncMemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkImageMemoryBarrier2& barrier)
        : SyncMemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                             VkPipelineStageFlags dst_stage_mask)
        : SyncMemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkTensorMemoryBarrierARM& barrier)
        : SyncMemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}

    OwnershipTransferOp TransferOp(uint32_t command_pool_queue_family) const {
        if (srcQueueFamilyIndex != dstQueueFamilyIndex) {
            if (command_pool_queue_family == srcQueueFamilyIndex) {
                return OwnershipTransferOp::release;
            } else if (command_pool_queue_family == dstQueueFamilyIndex) {
                return OwnershipTransferOp::acquire;
            }
        }
        return OwnershipTransferOp::none;
    }
};

struct BufferBarrier : OwnershipTransferBarrier {
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;

    explicit BufferBarrier(const VkBufferMemoryBarrier2& barrier)
        : OwnershipTransferBarrier(barrier), buffer(barrier.buffer), offset(barrier.offset), size(barrier.size) {}
    BufferBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : OwnershipTransferBarrier(barrier, src_stage_mask, dst_stage_mask),
          buffer(barrier.buffer),
          offset(barrier.offset),
          size(barrier.size) {}
};

struct ImageBarrier : OwnershipTransferBarrier {
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkImage image;
    VkImageSubresourceRange subresourceRange;

    explicit ImageBarrier(const VkImageMemoryBarrier2& barrier)
        : OwnershipTransferBarrier(barrier),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}
    ImageBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : OwnershipTransferBarrier(barrier, src_stage_mask, dst_stage_mask),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}
};

struct TensorBarrier : OwnershipTransferBarrier {
    VkTensorARM tensor;
    explicit TensorBarrier(const VkTensorMemoryBarrierARM& barrier) : OwnershipTransferBarrier(barrier), tensor(barrier.tensor) {}
};

struct ExecScopes {
    VkPipelineStageFlags2 src;
    VkPipelineStageFlags2 dst;
};

//
// Types to store queue family ownership (QFO) Transfers
//
static inline bool IsOwnershipTransfer(const OwnershipTransferBarrier& barrier) {
    return barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex;
}

static inline bool IsQueueFamilyExternal(const uint32_t queue_family_index) {
    return (queue_family_index == VK_QUEUE_FAMILY_EXTERNAL) || (queue_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT);
}

// Common to image and buffer memory barriers
template <typename Handle>
struct QFOTransferBarrierBase {
    using HandleType = Handle;
    Handle handle = VK_NULL_HANDLE;
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    QFOTransferBarrierBase() = default;
    QFOTransferBarrierBase(const Handle& resource_handle, uint32_t src, uint32_t dst)
        : handle(resource_handle), srcQueueFamilyIndex(src), dstQueueFamilyIndex(dst) {}

    hash_util::HashCombiner base_hash_combiner() const {
        hash_util::HashCombiner hc;
        hc << srcQueueFamilyIndex << dstQueueFamilyIndex << handle;
        return hc;
    }

    bool operator==(const QFOTransferBarrierBase<Handle>& rhs) const {
        return (srcQueueFamilyIndex == rhs.srcQueueFamilyIndex) && (dstQueueFamilyIndex == rhs.dstQueueFamilyIndex) &&
               (handle == rhs.handle);
    }
};

// Image barrier specific implementation
struct QFOImageTransferBarrier : public QFOTransferBarrierBase<VkImage> {
    using BaseType = QFOTransferBarrierBase<VkImage>;
    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageSubresourceRange subresourceRange;

    QFOImageTransferBarrier() = default;
    QFOImageTransferBarrier(const ImageBarrier& barrier)
        : BaseType(barrier.image, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    size_t hash() const;
    bool operator==(const QFOImageTransferBarrier& rhs) const;
    static vvl::Struct BarrierName() { return vvl::Struct::VkImageMemoryBarrier; }
};

// Buffer barrier specific implementation
struct QFOBufferTransferBarrier : public QFOTransferBarrierBase<VkBuffer> {
    using BaseType = QFOTransferBarrierBase<VkBuffer>;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    QFOBufferTransferBarrier() = default;
    QFOBufferTransferBarrier(const BufferBarrier& barrier)
        : BaseType(barrier.buffer, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          offset(barrier.offset),
          size(barrier.size) {}
    size_t hash() const;
    bool operator==(const QFOBufferTransferBarrier& rhs) const;
    static vvl::Struct BarrierName() { return vvl::Struct::VkBufferMemoryBarrier; }
};

template <typename TransferBarrier>
using QFOTransferBarrierHash = hash_util::HasHashMember<TransferBarrier>;

// Command buffers store the set of barriers recorded
template <typename TransferBarrier>
using QFOTransferBarrierSet = vvl::unordered_set<TransferBarrier, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferBarrierSets {
    QFOTransferBarrierSet<TransferBarrier> release;
    QFOTransferBarrierSet<TransferBarrier> acquire;
    void Reset() {
        acquire.clear();
        release.clear();
    }
};

// The layer_data stores the map of pending release barriers
template <typename TransferBarrier>
using GlobalQFOTransferBarrierMap =
    vvl::concurrent_unordered_map<typename TransferBarrier::HandleType, QFOTransferBarrierSet<TransferBarrier>>;

// Submit queue uses the Scoreboard to track all release/acquire operations in a batch.
template <typename TransferBarrier>
using QFOTransferCBScoreboard =
    vvl::unordered_map<TransferBarrier, const vvl::CommandBuffer*, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferCBScoreboards {
    QFOTransferCBScoreboard<TransferBarrier> acquire;
    QFOTransferCBScoreboard<TransferBarrier> release;
};

namespace sync_utils {
VkPipelineStageFlags2 DisabledPipelineStages(const DeviceFeatures& features, const DeviceExtensions& device_extensions);
VkAccessFlags2 DisabledAccesses(const DeviceExtensions& device_extensions);

std::string StringPipelineStageFlags(VkPipelineStageFlags2 mask);

// Expand all pipeline stage bits. If queue_flags and disabled_feature_mask is provided, the expansion of ALL_COMMANDS_BIT
// and ALL_GRAPHICS_BIT will be limited to what is supported.
VkPipelineStageFlags2 ExpandPipelineStages(VkPipelineStageFlags2 stage_mask, VkQueueFlags queue_flags,
                                           const VkPipelineStageFlags2 disabled_feature_mask = 0);

VkAccessFlags2 CompatibleAccessMask(VkPipelineStageFlags2 stage_mask);

std::string StringAccessFlags(VkAccessFlags2 mask);

ExecScopes GetExecScopes(const VkDependencyInfo& dep_info);
}  // namespace sync_utils
