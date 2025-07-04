/*
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
#pragma once
#include "containers/subresource_adapter.h"
#include "containers/range.h"
#include "generated/sync_validation_types.h"
#include "containers/limits.h"
#include <set>

namespace vvl {
class Buffer;
}  // namespace vvl

using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

// The resource tag index is relative to the command buffer or queue in which it's found
using QueueId = uint32_t;
constexpr static QueueId kQueueIdInvalid = QueueId(vvl::kU32Max);
constexpr static QueueId kQueueAny = kQueueIdInvalid - 1;

using ResourceUsageTag = size_t;

// TODO: in the current implementation invalid tag is used not only as initial value
// but also in some other scenarios (e.g. error reporting classifies layout transition
// based on tag validity). Clarify when tag can be invalid and document this.
constexpr static ResourceUsageTag kInvalidTag = std::numeric_limits<ResourceUsageTag>::max();

using ResourceUsageRange = vvl::range<ResourceUsageTag>;
using ResourceAddress = VkDeviceSize;
using ResourceAccessRange = vvl::range<ResourceAddress>;

// Usage tag extended with resource handle information
struct ResourceUsageTagEx {
    ResourceUsageTag tag = kInvalidTag;
    uint32_t handle_index = vvl::kNoIndex32;
};

ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size);
ResourceAccessRange MakeRange(const vvl::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size);
inline const SyncAccessInfo &GetAccessInfo(SyncAccessIndex access) { return GetSyncAccessInfos()[access]; }

extern const ResourceAccessRange kFullRange;
constexpr VkImageAspectFlags kDepthStencilAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

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

// A wrapper for a single range with the same semantics as other non-trivial range generators
template <typename KeyType>
class SingleRangeGenerator {
  public:
    using RangeType = KeyType;
    SingleRangeGenerator(const KeyType &range) : current_(range) {}
    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    SingleRangeGenerator &operator++() {
        current_ = KeyType();  // just one real range
        return *this;
    }

    bool operator==(const SingleRangeGenerator &other) const { return current_ == other.current_; }

  private:
    SingleRangeGenerator() = default;
    const KeyType range_;
    KeyType current_;
};
