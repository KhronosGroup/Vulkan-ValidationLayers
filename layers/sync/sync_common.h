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
#include "error_message/error_location.h"
#include "containers/subresource_adapter.h"
#include "containers/range_vector.h"
#include "generated/sync_validation_types.h"
#include "state_tracker/image_state.h"

namespace vvl {
class Buffer;
class BufferView;
}  // namespace vvl

struct BufferBinding;
class HazardResult;
class SyncValidator;

using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

// The resource tag index is relative to the command buffer or queue in which it's found
using QueueId = uint32_t;
constexpr static QueueId kQueueIdBase = QueueId(0);
constexpr static QueueId kQueueIdInvalid = ~kQueueIdBase;
constexpr static QueueId kQueueAny = kQueueIdInvalid - 1;

using ResourceUsageTag = size_t;
constexpr static ResourceUsageTag kMaxIndex = std::numeric_limits<ResourceUsageTag>::max();
constexpr static ResourceUsageTag kInvalidTag = kMaxIndex;

using ResourceUsageRange = sparse_container::range<ResourceUsageTag>;
using ResourceAddress = VkDeviceSize;
using ResourceAccessRange = sparse_container::range<ResourceAddress>;

template <typename T>
ResourceAccessRange MakeRange(const T &has_offset_and_size) {
    return ResourceAccessRange(has_offset_and_size.offset, (has_offset_and_size.offset + has_offset_and_size.size));
}
ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size);
ResourceAccessRange MakeRange(const vvl::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size);
ResourceAccessRange MakeRange(const vvl::BufferView &buf_view_state);
ResourceAccessRange MakeRange(VkDeviceSize offset, uint32_t first_index, uint32_t count, uint32_t stride);
ResourceAccessRange MakeRange(const BufferBinding &binding, uint32_t first_index, const std::optional<uint32_t> &count,
                              uint32_t stride);

extern const ResourceAccessRange kFullRange;

constexpr VkImageAspectFlags kColorAspects =
    VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
constexpr VkImageAspectFlags kDepthStencilAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

class SyncValidationInfo {
  public:
    SyncValidationInfo(const SyncValidator* sync_validator) : sync_state_(sync_validator) {}
    const SyncValidator& GetSyncState() const {
        assert(sync_state_);
        return *sync_state_;
    }
    std::string FormatHazard(const HazardResult& hazard) const;
    virtual std::string FormatUsage(ResourceUsageTag tag) const = 0;

  protected:
    const SyncValidator* sync_state_;
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
namespace syncval_state {
class CommandBuffer;
class Swapchain;

class ImageState : public vvl::Image {
  public:
    ImageState(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo,
               VkFormatFeatureFlags2KHR features)
        : vvl::Image(dev_data, img, pCreateInfo, features), opaque_base_address_(0U) {}

    ImageState(const ValidationStateTracker *dev_data, VkImage img, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
               uint32_t swapchain_index, VkFormatFeatureFlags2KHR features)
        : vvl::Image(dev_data, img, pCreateInfo, swapchain, swapchain_index, features), opaque_base_address_(0U) {}
    bool IsLinear() const { return fragment_encoder->IsLinearImage(); }
    bool IsTiled() const { return !IsLinear(); }
    bool IsSimplyBound() const;

    void SetOpaqueBaseAddress(ValidationStateTracker &dev_data);

    VkDeviceSize GetOpaqueBaseAddress() const { return opaque_base_address_; }
    bool HasOpaqueMapping() const { return 0U != opaque_base_address_; }
    VkDeviceSize GetResourceBaseAddress() const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                    const VkExtent3D &extent, bool is_depth_sliced) const;

  protected:
    VkDeviceSize opaque_base_address_ = 0U;
};

class ImageViewState : public vvl::ImageView {
  public:
    ImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                   VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    const ImageState *GetImageState() const { return static_cast<const syncval_state::ImageState *>(image_state.get()); }
    ImageRangeGen MakeImageRangeGen(const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask = 0) const;
    const ImageRangeGen &GetFullViewImageRangeGen() const { return view_range_gen; }

  protected:
    ImageRangeGen MakeImageRangeGen() const;
    // All data members needs for MakeImageRangeGen() must be set before initializing view_range_gen... i.e. above this line.
    const ImageRangeGen view_range_gen;
};

// Utilities to DRY up Get... calls
template <typename Map, typename Key = typename Map::key_type, typename RetVal = std::optional<typename Map::mapped_type>>
RetVal GetMappedOptional(const Map &map, const Key &key) {
    RetVal ret_val;
    auto it = map.find(key);
    if (it != map.cend()) {
        ret_val.emplace(it->second);
    }
    return ret_val;
}
template <typename Map, typename Fn>
typename Map::mapped_type GetMapped(const Map &map, const typename Map::key_type &key, Fn &&default_factory) {
    auto value = GetMappedOptional(map, key);
    return (value) ? *value : default_factory();
}

template <typename Map, typename Key = typename Map::key_type, typename Mapped = typename Map::mapped_type,
          typename Value = typename Mapped::element_type>
Value *GetMappedPlainFromShared(const Map &map, const Key &key) {
    auto value = GetMappedOptional<Map, Key>(map, key);
    if (value) return value->get();
    return nullptr;
}

}  // namespace syncval_state
