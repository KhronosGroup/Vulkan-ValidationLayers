/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
#pragma once
#include "vk_layer_data.h"
#include "hash_util.h"

// Types to store queue family ownership (QFO) Transfers
class COMMAND_POOL_STATE;

template <typename Barrier>
inline bool IsTransferOp(const Barrier &barrier) {
    return barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex;
}

// specializations for barriers that cannot do queue family ownership transfers
template <>
constexpr bool IsTransferOp(const VkMemoryBarrier &barrier) {
    return false;
}
template <>
constexpr bool IsTransferOp(const VkMemoryBarrier2KHR &barrier) {
    return false;
}
template <>
constexpr bool IsTransferOp(const VkSubpassDependency2 &barrier) {
    return false;
}

static inline bool QueueFamilyIsExternal(const uint32_t queue_family_index) {
    return (queue_family_index == VK_QUEUE_FAMILY_EXTERNAL) || (queue_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT);
}

// Caution: Section 7.7.4 states that "If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer
// is performed, and the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED."; this does not handle that case.
static inline bool QueueFamilyIsIgnored(uint32_t queue_family_index) { return queue_family_index == VK_QUEUE_FAMILY_IGNORED; }

// Common to image and buffer memory barriers
template <typename Handle>
struct QFOTransferBarrierBase {
    using HandleType = Handle;
    Handle handle = VK_NULL_HANDLE;
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    QFOTransferBarrierBase() = default;
    QFOTransferBarrierBase(const Handle &resource_handle, uint32_t src, uint32_t dst)
        : handle(resource_handle), srcQueueFamilyIndex(src), dstQueueFamilyIndex(dst) {}

    hash_util::HashCombiner base_hash_combiner() const {
        hash_util::HashCombiner hc;
        hc << srcQueueFamilyIndex << dstQueueFamilyIndex << handle;
        return hc;
    }

    bool operator==(const QFOTransferBarrierBase<Handle> &rhs) const {
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
    QFOImageTransferBarrier(const VkImageMemoryBarrier &barrier)
        : BaseType(barrier.image, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    QFOImageTransferBarrier(const VkImageMemoryBarrier2KHR &barrier)
        : BaseType(barrier.image, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          subresourceRange(barrier.subresourceRange) {}
    size_t hash() const {
        // Ignoring the layout information for the purpose of the hash, as we're interested in QFO release/acquisition w.r.t.
        // the subresource affected, an layout transitions are current validated on another path
        auto hc = base_hash_combiner() << subresourceRange;
        return hc.Value();
    }
    bool operator==(const QFOImageTransferBarrier &rhs) const {
        // Ignoring layout w.r.t. equality. See comment in hash above.
        return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (subresourceRange == rhs.subresourceRange);
    }
    // TODO: codegen a comprehensive complie time type -> string (and or other traits) template family
    static const char *BarrierName() { return "VkImageMemoryBarrier"; }
    static const char *HandleName() { return "VkImage"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00001 QFO transfer image barrier must not duplicate QFO recorded in command buffer
    static const char *ErrMsgDuplicateQFOInCB() { return "UNASSIGNED-VkImageMemoryBarrier-image-00001"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00002 QFO transfer image barrier must not duplicate QFO submitted in batch
    static const char *ErrMsgDuplicateQFOInSubmit() { return "UNASSIGNED-VkImageMemoryBarrier-image-00002"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00003 QFO transfer image barrier must not duplicate QFO submitted previously
    static const char *ErrMsgDuplicateQFOSubmitted() { return "UNASSIGNED-VkImageMemoryBarrier-image-00003"; }
    // UNASSIGNED-VkImageMemoryBarrier-image-00004 QFO acquire image barrier must have matching QFO release submitted previously
    static const char *ErrMsgMissingQFOReleaseInSubmit() { return "UNASSIGNED-VkImageMemoryBarrier-image-00004"; }
};

// Buffer barrier specific implementation
struct QFOBufferTransferBarrier : public QFOTransferBarrierBase<VkBuffer> {
    using BaseType = QFOTransferBarrierBase<VkBuffer>;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    QFOBufferTransferBarrier() = default;
    QFOBufferTransferBarrier(const VkBufferMemoryBarrier &barrier)
        : BaseType(barrier.buffer, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          offset(barrier.offset),
          size(barrier.size) {}
    QFOBufferTransferBarrier(const VkBufferMemoryBarrier2KHR &barrier)
        : BaseType(barrier.buffer, barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex),
          offset(barrier.offset),
          size(barrier.size) {}
    size_t hash() const {
        auto hc = base_hash_combiner() << offset << size;
        return hc.Value();
    }
    bool operator==(const QFOBufferTransferBarrier &rhs) const {
        return (static_cast<BaseType>(*this) == static_cast<BaseType>(rhs)) && (offset == rhs.offset) && (size == rhs.size);
    }
    static const char *BarrierName() { return "VkBufferMemoryBarrier"; }
    static const char *HandleName() { return "VkBuffer"; }
    // UNASSIGNED-VkImageMemoryBarrier-buffer-00001 QFO transfer buffer barrier must not duplicate QFO recorded in command buffer
    static const char *ErrMsgDuplicateQFOInCB() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00002 QFO transfer buffer barrier must not duplicate QFO submitted in batch
    static const char *ErrMsgDuplicateQFOInSubmit() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00003 QFO transfer buffer barrier must not duplicate QFO submitted previously
    static const char *ErrMsgDuplicateQFOSubmitted() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00003"; }
    // UNASSIGNED-VkBufferMemoryBarrier-buffer-00004 QFO acquire buffer barrier must have matching QFO release submitted previously
    static const char *ErrMsgMissingQFOReleaseInSubmit() { return "UNASSIGNED-VkBufferMemoryBarrier-buffer-00004"; }
};

template <typename TransferBarrier>
using QFOTransferBarrierHash = hash_util::HasHashMember<TransferBarrier>;

// Command buffers store the set of barriers recorded
template <typename TransferBarrier>
using QFOTransferBarrierSet = layer_data::unordered_set<TransferBarrier, QFOTransferBarrierHash<TransferBarrier>>;

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
    vl_concurrent_unordered_map<typename TransferBarrier::HandleType, QFOTransferBarrierSet<TransferBarrier>>;

// Submit queue uses the Scoreboard to track all release/acquire operations in a batch.
template <typename TransferBarrier>
using QFOTransferCBScoreboard =
    layer_data::unordered_map<TransferBarrier, const CMD_BUFFER_STATE *, QFOTransferBarrierHash<TransferBarrier>>;

template <typename TransferBarrier>
struct QFOTransferCBScoreboards {
    QFOTransferCBScoreboard<TransferBarrier> acquire;
    QFOTransferCBScoreboard<TransferBarrier> release;
};
