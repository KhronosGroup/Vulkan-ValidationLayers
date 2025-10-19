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

namespace syncval {

using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

// The resource tag index is relative to the command buffer or queue in which it's found
using QueueId = uint32_t;
constexpr static QueueId kQueueIdInvalid = QueueId(vvl::kNoIndex32);
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

// The ThreadSafeLookupTable supports fast object lookup in multithreaded environment.
// The insertions are slow. The idea is that you have relatively small amount of
// objects and you need to put them into a container in multithreaded environment.
// In return you get an index of the inserted object. After that initial insertion all
// further operations are the queries and they are fast (single atomic load in addition to
// regular vector/hashmap lookup). You can query an object given its index or you can get
// an index of already registered object.
template <typename ObjectType>
class ThreadSafeLookupTable {
  public:
    ThreadSafeLookupTable() { std::atomic_store(&snapshot_, std::make_shared<const Snapshot>()); }

    // Returns the object with the given index.
    // The object index is from the previous call to GetOrInsert.
    // This operation uses single atomic load.
    ObjectType GetObject(uint32_t object_index) const {
        auto snapshot = std::atomic_load(&snapshot_);
        return snapshot->objects[object_index];
    }

    // Returns the index of the given object. If the object is seen for the first time, it is registered.
    // For already registerd objects the function performs single atomic load and hash map access (fast path).
    // In order to register new object the follow expensive operations are performed (slow path):
    // mutex lock, repeat the search, allocate new snapshot object, copy all data from the old snapshot.
    uint32_t GetIndexAndMaybeInsert(const ObjectType &object) {
        //
        // Fast path: object was already registered
        //
        auto snapshot = std::atomic_load(&snapshot_);
        if (auto it = snapshot->object_to_index.find(object); it != snapshot->object_to_index.end()) {
            return it->second;
        }

        //
        // Slow path: register new object
        //
        std::unique_lock<std::mutex> lock(snapshot_mutex_);

        // Search again since another thread could have registered the object just before we locked the mutex
        snapshot = std::atomic_load(&snapshot_);
        if (auto it = snapshot->object_to_index.find(object); it != snapshot->object_to_index.end()) {
            return it->second;
        }

        // Create a new snapshot. Copy constructor copies data from the old snapshot.
        // The old snapshot is not allowed to be modified (so no move).
        auto new_snapshot = std::make_shared<Snapshot>(*snapshot);

        // Add new object
        new_snapshot->objects.emplace_back(object);
        const uint32_t index = uint32_t(new_snapshot->objects.size()) - 1;
        new_snapshot->object_to_index.insert(std::make_pair(object, index));

        // Update snapshot holder
        std::atomic_store(&snapshot_, std::shared_ptr<const Snapshot>(std::move(new_snapshot)));

        return index;
    }

    uint32_t ObjectCount() const {
        auto snapshot = std::atomic_load(&snapshot_);
        return (uint32_t)snapshot->objects.size();
    }

  private:
    struct Snapshot {
        std::vector<ObjectType> objects;
        vvl::unordered_map<ObjectType, uint32_t> object_to_index;
    };

    // Once snapshot is created it must never be modified (other threads can access it at any time).
    // New objects are added by replacing entire snapshot with an updated version.
    // TODO: C++ 20: use std::atomic<std::shared_ptr<T>. Until then we use std::atomic_load/atomic_store.
    std::shared_ptr<const Snapshot> snapshot_;

    // Locks snapshot during rare insert events
    std::mutex snapshot_mutex_;
};

}  // namespace syncval
