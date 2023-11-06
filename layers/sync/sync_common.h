/*
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
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

#include "containers/range_vector.h"
#include "generated/sync_validation_types.h"

// The resource tag index is relative to the command buffer or queue in which it's found
using QueueId = uint32_t;
constexpr static QueueId kQueueIdBase = QueueId(0);
constexpr static QueueId kQueueIdInvalid = ~kQueueIdBase;
constexpr static QueueId kQueueAny = kQueueIdInvalid - 1;

using ResourceUsageTag = size_t;
constexpr static ResourceUsageTag kMaxIndex = std::numeric_limits<ResourceUsageTag>::max();
constexpr static ResourceUsageTag kInvalidTag = kMaxIndex;

using ResourceUsageRange = sparse_container::range<ResourceUsageTag>;

enum SyncHazard {
    NONE = 0,
    READ_AFTER_WRITE,
    WRITE_AFTER_READ,
    WRITE_AFTER_WRITE,
    READ_RACING_WRITE,
    WRITE_RACING_WRITE,
    WRITE_RACING_READ,
    WRITE_AFTER_PRESENT,  // Once presented, an image may not be used until acquired
    READ_AFTER_PRESENT,
    PRESENT_AFTER_READ,  // Must be unreferenced and visible to present
    PRESENT_AFTER_WRITE,
};

enum class SyncOrdering : uint8_t {
    kNonAttachment = 0,
    kColorAttachment = 1,
    kDepthStencilAttachment = 2,
    kRaster = 3,
    kNumOrderings = 4,
};

// Useful Utilites for manipulating StageAccess parameters, suitable as base class to save typing
struct SyncStageAccess {
    static inline const SyncStageAccessInfoType &UsageInfo(SyncStageAccessIndex stage_access_index) {
        return syncStageAccessInfoByStageAccessIndex()[stage_access_index];
    }
    static inline SyncStageAccessFlags FlagBit(SyncStageAccessIndex stage_access) {
        return syncStageAccessInfoByStageAccessIndex()[stage_access].stage_access_bit;
    }

    static bool IsRead(SyncStageAccessIndex stage_access_index) { return syncStageAccessReadMask[stage_access_index]; }
    static bool IsRead(const SyncStageAccessInfoType &info) { return IsRead(info.stage_access_index); }
    static bool IsWrite(SyncStageAccessIndex stage_access_index) { return syncStageAccessWriteMask[stage_access_index]; }
    static bool IsWrite(const SyncStageAccessInfoType &info) { return IsWrite(info.stage_access_index); }

    static VkPipelineStageFlags2KHR PipelineStageBit(SyncStageAccessIndex stage_access_index) {
        return syncStageAccessInfoByStageAccessIndex()[stage_access_index].stage_mask;
    }
    static SyncStageAccessFlags AccessScopeByStage(VkPipelineStageFlags2KHR stages);
    static SyncStageAccessFlags AccessScopeByAccess(VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(VkPipelineStageFlags2KHR stages, VkAccessFlags2KHR access);
    static SyncStageAccessFlags AccessScope(const SyncStageAccessFlags &stage_scope, VkAccessFlags2KHR accesses) {
        return stage_scope & AccessScopeByAccess(accesses);
    }
};

// Notes:
//  * Design goal is performance optimized set creation during specific SyncVal operations
//  * Key must be integral.
//  * We aren't interested as of this implementation in caching lookups, only inserts
//  * using a raw C-style array instead of std::array intentionally for size/performance reasons
//
// The following were shown to not improve hit rate for current usage (tag set gathering).  For general use YMMV.
//  * More complicated index construction (at >> LogSize ^ at)
//  * Multi-way LRU eviction caching (equivalent hit rate to 1-way direct replacement of same total cache slots) but with
//    higher complexity.
template <typename IntegralKey, size_t LogSize = 4U, IntegralKey kInvalidKey = IntegralKey(0)>
class CachedInsertSet : public std::set<IntegralKey> {
  public:
    using Base = std::set<IntegralKey>;
    using key_type = typename Base::key_type;
    using Index = unsigned;
    static constexpr Index kSize = 1 << LogSize;
    static constexpr key_type kMask = static_cast<key_type>(kSize) - 1;

    void CachedInsert(const key_type key) {
        // 1-way direct replacement
        const Index index = static_cast<Index>(key & kMask);  // Simplest

        if (entries_[index] != key) {
            entries_[index] = key;
            Base::insert(key);
        }
    }

    CachedInsertSet() { std::fill(entries_, entries_ + kSize, kInvalidKey); }

  private:
    key_type entries_[kSize];
};
