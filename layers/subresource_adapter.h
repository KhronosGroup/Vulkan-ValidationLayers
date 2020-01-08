/* Copyright (c) 2019-2020 The Khronos Group Inc.
 * Copyright (c) 2019-2020 Valve Corporation
 * Copyright (c) 2019-2020 LunarG, Inc.
 * Copyright (C) 2019-2020 Google Inc.
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
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#pragma once

#ifndef SUBRESOURCE_ADAPTER_H_
#define SUBRESOURCE_ADAPTER_H_

#include <algorithm>
#include <array>
#include "range_vector.h"
#ifndef SPARSE_CONTAINER_UNIT_TEST
#include "vulkan/vulkan.h"
#else
#include "vk_snippets.h"
#endif

namespace subresource_adapter {

class RangeEncoder;
using IndexType = uint64_t;
template <typename Element>
using Range = sparse_container::range<Element>;
using IndexRange = Range<IndexType>;
using WritePolicy = sparse_container::value_precedence;
using split_op_keep_both = sparse_container::split_op_keep_both;
using split_op_keep_lower = sparse_container::split_op_keep_lower;
using split_op_keep_upper = sparse_container::split_op_keep_upper;

// Interface for aspect specific traits objects (now isolated in the cpp file)
class AspectParameters {
  public:
    static const AspectParameters* Get(VkImageAspectFlags);
    typedef uint32_t (*MaskIndexFunc)(VkImageAspectFlags);
    virtual VkImageAspectFlags AspectMask() const = 0;
    virtual MaskIndexFunc MaskToIndexFunction() const = 0;
    virtual uint32_t AspectCount() const = 0;
    virtual const VkImageAspectFlagBits* AspectBits() const = 0;
};

struct Subresource : public VkImageSubresource {
    uint32_t aspect_index;
    Subresource() : VkImageSubresource({0, 0, 0}), aspect_index(0) {}

    Subresource(const Subresource& from) = default;
    Subresource(const RangeEncoder& encoder, const VkImageSubresource& subres);
    Subresource(VkImageAspectFlags aspect_mask_, uint32_t mip_level_, uint32_t array_layer_, uint32_t aspect_index_)
        : VkImageSubresource({aspect_mask_, mip_level_, array_layer_}), aspect_index(aspect_index_) {}
    Subresource(VkImageAspectFlagBits aspect_, uint32_t mip_level_, uint32_t array_layer_, uint32_t aspect_index_)
        : Subresource(static_cast<VkImageAspectFlags>(aspect_), mip_level_, array_layer_, aspect_index_) {}
};

// Subresource is encoded in (from slowest varying to fastest)
//    aspect_index
//    mip_level_index
//    array_layer_index
// into continuous index ranges
class RangeEncoder {
  public:
    static constexpr uint32_t kMaxSupportedAspect = 3;

    // The default constructor for default iterators
    RangeEncoder()
        : full_range_(),
          limits_(),
          mip_size_(0),
          aspect_size_(0),
          aspect_bits_(nullptr),
          mask_index_function_(nullptr),
          encode_function_(nullptr),
          decode_function_(nullptr),
          lower_bound_function_(nullptr),
          lower_bound_with_start_function_(nullptr),
          aspect_base_{0, 0, 0} {}

    RangeEncoder(const VkImageSubresourceRange& full_range, const AspectParameters* param);
    // Create the encoder suitable to the full range (aspect mask *must* be canonical)
    RangeEncoder(const VkImageSubresourceRange& full_range)
        : RangeEncoder(full_range, AspectParameters::Get(full_range.aspectMask)) {}
    RangeEncoder(const RangeEncoder& from);

    inline bool InRange(const VkImageSubresource& subres) const {
        bool in_range = (subres.mipLevel < limits_.mipLevel) && (subres.arrayLayer < limits_.arrayLayer) &&
                        (subres.aspectMask & limits_.aspectMask);
        return in_range;
    }
    inline bool InRange(const VkImageSubresourceRange& range) const {
        bool in_range = (range.baseMipLevel < limits_.mipLevel) && ((range.baseMipLevel + range.levelCount) <= limits_.mipLevel) &&
                        (range.baseArrayLayer < limits_.arrayLayer) &&
                        ((range.baseArrayLayer + range.layerCount) <= limits_.arrayLayer) &&
                        (range.aspectMask & limits_.aspectMask);
        return in_range;
    }

    inline IndexType Encode(const Subresource& pos) const { return (this->*(encode_function_))(pos); }
    inline IndexType Encode(const VkImageSubresource& subres) const { return Encode(Subresource(*this, subres)); }

    Subresource Decode(const IndexType& index) const { return (this->*decode_function_)(index); }

    inline Subresource BeginSubresource(const VkImageSubresourceRange& range) const {
        const auto aspect_index = LowerBoundFromMask(range.aspectMask);
        Subresource begin(aspect_bits_[aspect_index], range.baseMipLevel, range.baseArrayLayer, aspect_index);
        return begin;
    }

    // This version assumes the mask must have at least one bit matching limits_.aspectMask
    // Suitable for getting a starting value from a range
    inline uint32_t LowerBoundFromMask(VkImageAspectFlags mask) const {
        assert(mask & limits_.aspectMask);
        return (this->*(lower_bound_function_))(mask);
    }

    // This version allows for a mask that can (starting at start) not have any bits set matching limits_.aspectMask
    // Suitable for seeking the *next* value for a range
    inline uint32_t LowerBoundFromMask(VkImageAspectFlags mask, uint32_t start) const {
        if (start < limits_.aspect_index) {
            return (this->*(lower_bound_with_start_function_))(mask, start);
        }
        return limits_.aspect_index;
    }

    inline IndexType AspectSize() const { return aspect_size_; }
    inline IndexType MipSize() const { return mip_size_; }
    inline const Subresource& Limits() const { return limits_; }
    inline const VkImageSubresourceRange& FullRange() const { return full_range_; }
    inline IndexType SubresourceCount() const { return AspectSize() * Limits().aspect_index; }
    inline VkImageAspectFlags AspectMask() const { return limits_.aspectMask; }
    inline VkImageAspectFlagBits AspectBit(uint32_t aspect_index) const {
        RANGE_ASSERT(aspect_index < limits_.aspect_index);
        return aspect_bits_[aspect_index];
    }
    inline IndexType AspectBase(uint32_t aspect_index) const {
        RANGE_ASSERT(aspect_index < limits_.aspect_index);
        return aspect_base_[aspect_index];
    }

    inline VkImageSubresource MakeVkSubresource(const Subresource& subres) const {
        VkImageSubresource vk_subres = {static_cast<VkImageAspectFlags>(aspect_bits_[subres.aspect_index]), subres.mipLevel,
                                        subres.arrayLayer};
        return vk_subres;
    }

  protected:
    void PopulateFunctionPointers();

    IndexType Encode1AspectArrayOnly(const Subresource& pos) const;
    IndexType Encode1AspectMipArray(const Subresource& pos) const;
    IndexType Encode1AspectMipOnly(const Subresource& pos) const;
    IndexType EncodeAspectArrayOnly(const Subresource& pos) const;
    IndexType EncodeAspectMipArray(const Subresource& pos) const;
    IndexType EncodeAspectMipOnly(const Subresource& pos) const;

    // Use compiler to create the aspect count variants...
    // For ranges that only have a single mip level...
    template <uint32_t N>
    Subresource DecodeAspectArrayOnly(const IndexType& index) const {
        if ((N > 2) && (index >= aspect_base_[2])) {
            return Subresource(aspect_bits_[2], 0, static_cast<uint32_t>(index - aspect_base_[2]), 2);
        } else if ((N > 1) && (index >= aspect_base_[1])) {
            return Subresource(aspect_bits_[1], 0, static_cast<uint32_t>(index - aspect_base_[1]), 1);
        }
        // NOTE: aspect_base_[0] is always 0... here and below
        return Subresource(aspect_bits_[0], 0, static_cast<uint32_t>(index), 0);
    }

    // For ranges that only have a single array layer...
    template <uint32_t N>
    Subresource DecodeAspectMipOnly(const IndexType& index) const {
        if ((N > 2) && (index >= aspect_base_[2])) {
            return Subresource(aspect_bits_[2], static_cast<uint32_t>(index - aspect_base_[2]), 0, 2);
        } else if ((N > 1) && (index >= aspect_base_[1])) {
            return Subresource(aspect_bits_[1], static_cast<uint32_t>(index - aspect_base_[1]), 0, 1);
        }
        return Subresource(aspect_bits_[0], static_cast<uint32_t>(index), 0, 0);
    }

    // For ranges that only have both > 1 layer and level
    template <uint32_t N>
    Subresource DecodeAspectMipArray(const IndexType& index) const {
        assert(limits_.aspect_index <= N);
        uint32_t aspect_index = 0;
        if ((N > 2) && (index >= aspect_base_[2])) {
            aspect_index = 2;
        } else if ((N > 1) && (index >= aspect_base_[1])) {
            aspect_index = 1;
        }

        // aspect_base_[0] is always zero, so use the template to cheat
        const IndexType base_index = index - ((N == 1) ? 0 : aspect_base_[aspect_index]);

        const IndexType mip_level = base_index / mip_size_;
        const IndexType mip_start = mip_level * mip_size_;
        const IndexType array_offset = base_index - mip_start;

        return Subresource(aspect_bits_[aspect_index], static_cast<uint32_t>(mip_level), static_cast<uint32_t>(array_offset),
                           aspect_index);
    }

    uint32_t LowerBoundImpl1(VkImageAspectFlags aspect_mask) const;
    uint32_t LowerBoundImpl2(VkImageAspectFlags aspect_mask) const;
    uint32_t LowerBoundImpl3(VkImageAspectFlags aspect_mask) const;
    uint32_t LowerBoundWithStartImpl1(VkImageAspectFlags aspect_mask, uint32_t start) const;
    uint32_t LowerBoundWithStartImpl2(VkImageAspectFlags aspect_mask, uint32_t start) const;
    uint32_t LowerBoundWithStartImpl3(VkImageAspectFlags aspect_mask, uint32_t start) const;

  private:
    VkImageSubresourceRange full_range_;
    Subresource limits_;
    const size_t mip_size_;
    const size_t aspect_size_;
    const VkImageAspectFlagBits* const aspect_bits_;
    uint32_t (*const mask_index_function_)(VkImageAspectFlags);
    IndexType (RangeEncoder::*encode_function_)(const Subresource&) const;
    Subresource (RangeEncoder::*decode_function_)(const IndexType&) const;
    uint32_t (RangeEncoder::*lower_bound_function_)(VkImageAspectFlags aspect_mask) const;
    uint32_t (RangeEncoder::*lower_bound_with_start_function_)(VkImageAspectFlags aspect_mask, uint32_t start) const;
    IndexType aspect_base_[kMaxSupportedAspect];
};

class SubresourceGenerator : public Subresource {
  public:
    SubresourceGenerator() : Subresource(), encoder_(nullptr), limits_(){};
    SubresourceGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& range)
        : Subresource(encoder.BeginSubresource(range)), encoder_(&encoder), limits_(range) {}

    const VkImageSubresourceRange& Limits() const { return limits_; }

    // Seek functions are used by generators to force synchronization, as callers may have altered the position
    // to iterater between calls to the generator increment or Seek functions
    void SeekAspect(uint32_t seek_index) {
        arrayLayer = limits_.baseArrayLayer;
        mipLevel = limits_.baseMipLevel;
        const auto aspect_index_limit = encoder_->Limits().aspect_index;
        if (seek_index < aspect_index_limit) {
            aspect_index = seek_index;
            // Seeking to bit outside of the limit will set a "empty" subresource
            aspectMask = encoder_->AspectBit(aspect_index) & limits_.aspectMask;
        } else {
            // This is an "end" tombstone
            aspect_index = aspect_index_limit;
            aspectMask = 0;
        }
    }

    void SeekMip(uint32_t mip_level) {
        arrayLayer = limits_.baseArrayLayer;
        mipLevel = mip_level;
    }

    // Next and and ++ functions are for iteration from a base with the bounds, this may be additionally
    // controlled/updated by an owning generator (like RangeGenerator using Seek functions)
    inline void NextAspect() { SeekAspect(encoder_->LowerBoundFromMask(limits_.aspectMask, aspect_index + 1)); }

    void NextMip() {
        arrayLayer = limits_.baseArrayLayer;
        mipLevel++;
        if (mipLevel >= (limits_.baseMipLevel + limits_.levelCount)) {
            NextAspect();
        }
    }

    SubresourceGenerator& operator++() {
        arrayLayer++;
        if (arrayLayer >= (limits_.baseArrayLayer + limits_.layerCount)) {
            NextMip();
        }
        return *this;
    }

    // General purpose and slow, when we have no other information to update the generator
    void Seek(IndexType index) {
        // skip forward past discontinuities
        *static_cast<Subresource* const>(this) = encoder_->Decode(index);
    }

    const VkImageSubresource& operator*() const { return *this; }
    const VkImageSubresource* operator->() const { return this; }

  private:
    const RangeEncoder* encoder_;
    const VkImageSubresourceRange limits_;
};

// Like an iterator for ranges...
class RangeGenerator {
  public:
    RangeGenerator() : encoder_(nullptr), isr_pos_(), pos_(), aspect_base_() {}
    bool operator!=(const RangeGenerator& rhs) { return (pos_ != rhs.pos_) || (&encoder_ != &rhs.encoder_); }
    RangeGenerator(const RangeEncoder& encoder);
    RangeGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range);
    inline const IndexRange& operator*() const { return pos_; }
    inline const IndexRange* operator->() const { return &pos_; }
    // Returns a generator suitable for iterating within a range, is modified by operator ++ to bring
    // it in line with sync.
    SubresourceGenerator& GetSubresourceGenerator() { return isr_pos_; }
    Subresource& GetSubresource() { return isr_pos_; }
    RangeGenerator& operator++();

  private:
    const RangeEncoder* encoder_;
    SubresourceGenerator isr_pos_;
    IndexRange pos_;
    IndexRange aspect_base_;
    uint32_t mip_count_ = 0;
    uint32_t mip_index_ = 0;
    uint32_t aspect_count_ = 0;
    uint32_t aspect_index_ = 0;
};

// double wrapped map variants.. to avoid needing to templatize on the range map type.  The underlying maps are available for
// use in performance sensitive places that are *already* templatized (for example update_range_value).
// In STL style.  Note that N must be < uint8_t max
enum BothRangeMapMode { kTristate, kSmall, kBig };
template <typename T, size_t N>
class BothRangeMap {
    using BigMap = sparse_container::range_map<IndexType, T>;
    using RangeType = sparse_container::range<IndexType>;
    using SmallMap = sparse_container::small_range_map<IndexType, T, RangeType, N>;
    using SmallMapIterator = typename SmallMap::iterator;
    using SmallMapConstIterator = typename SmallMap::const_iterator;
    using BigMapIterator = typename BigMap::iterator;
    using BigMapConstIterator = typename BigMap::const_iterator;

  public:
    using value_type = typename SmallMap::value_type;
    using key_type = typename SmallMap::key_type;
    using index_type = typename SmallMap::index_type;
    using mapped_type = typename SmallMap::mapped_type;
    using small_map = SmallMap;
    using big_map = BigMap;

    template <typename Map, typename Value, typename SmallIt, typename BigIt>
    class IteratorImpl {
      protected:
        friend BothRangeMap;

      public:
        Value* operator->() const {
            assert(!Tristate());
            if (SmallMode()) {
                return small_it_.operator->();
            } else {
                return big_it_.operator->();
            }
        }

        Value& operator*() const {
            assert(!Tristate());
            if (SmallMode()) {
                return small_it_.operator*();
            } else {
                return big_it_.operator*();
            }
        }
        IteratorImpl& operator++() {
            assert(!Tristate());
            if (SmallMode()) {
                small_it_.operator++();
            } else {
                big_it_.operator++();
            }
            return *this;
        }
        IteratorImpl& operator--() {
            assert(!Tristate());
            if (SmallMode()) {
                small_it_.operator--();
            } else {
                big_it_.operator--();
            }
            return *this;
        }
        IteratorImpl& operator=(const IteratorImpl& other) {
            if (other.Tristate()) {
                // Tranisition to tristate
                *this = IteratorImpl();
            } else {
                if (other.SmallMode()) {
                    small_it_ = other.small_it_;
                } else {
                    big_it_ = other.big_it_;
                }
                mode_ = other.mode_;  // For transitions from Tristate.
            }
            return *this;
        }
        bool operator==(const IteratorImpl& other) const {
            if (other.Tristate()) return Tristate();  // both Tristate -> equal, any other comparison !equal
            if (Tristate()) return false;

            // Since we know neither are tristate....
            assert(mode_ == other.mode_);
            if (SmallMode()) {
                return small_it_ == other.small_it_;
            } else {
                return big_it_ == other.big_it_;
            }
        }
        bool operator!=(const IteratorImpl& other) const { return !(*this == other); }
        IteratorImpl() : small_it_(), big_it_(), mode_(BothRangeMapMode::kTristate) {}

      private:
        IteratorImpl(BothRangeMapMode mode) : small_it_(), big_it_(), mode_(mode) {}
        IteratorImpl(const SmallIt& it) : small_it_(it), big_it_(), mode_(BothRangeMapMode::kSmall) {}
        IteratorImpl(const BigIt& it) : small_it_(), big_it_(it), mode_(BothRangeMapMode::kBig) {}
        inline bool SmallMode() const { return BothRangeMapMode::kSmall == mode_; }
        inline bool BigMode() const { return BothRangeMapMode::kBig == mode_; }
        inline bool Tristate() const { return BothRangeMapMode::kTristate == mode_; }
        SmallIt small_it_;  // only one of these will be initialized non trivially (and they should be small)
        BigIt big_it_;
        BothRangeMapMode mode_;
    };

    using iterator = IteratorImpl<BothRangeMap, value_type, SmallMapIterator, BigMapIterator>;
    // TODO change const iterator to derived class if iterator -> const_iterator constructor is needed
    using const_iterator = IteratorImpl<const BothRangeMap, const value_type, SmallMapConstIterator, BigMapConstIterator>;

    inline iterator begin() {
        if (SmallMode()) {
            return iterator(small_map_.begin());
        } else {
            return iterator(big_map_.begin());
        }
    }
    inline const_iterator cbegin() const {
        if (SmallMode()) {
            return const_iterator(small_map_.begin());
        } else {
            return const_iterator(big_map_.begin());
        }
    }
    inline const_iterator begin() const { return cbegin(); }

    inline iterator end() {
        if (SmallMode()) {
            return iterator(small_map_.end());
        } else {
            return iterator(big_map_.end());
        }
    }
    inline const_iterator cend() const {
        if (SmallMode()) {
            return const_iterator(small_map_.end());
        } else {
            return const_iterator(big_map_.end());
        }
    }
    inline const_iterator end() const { return cend(); }

    inline iterator find(const key_type& key) {
        assert(!Tristate());
        if (SmallMode()) {
            return iterator(small_map_.find(key));
        } else {
            return iterator(big_map_.find(key));
        }
    }

    inline const_iterator find(const key_type& key) const {
        assert(!Tristate());
        if (SmallMode()) {
            return const_iterator(small_map_.find(key));
        } else {
            return const_iterator(big_map_.find(key));
        }
    }

    inline iterator find(const index_type& index) {
        assert(!Tristate());
        if (SmallMode()) {
            return iterator(small_map_.find(index));
        } else {
            return iterator(big_map_.find(index));
        }
    }

    inline const_iterator find(const index_type& index) const {
        assert(!Tristate());
        if (SmallMode()) {
            return const_iterator(const_small_map_.find(index));
        } else {
            return const_iterator(const_big_map_.find(index));
        }
    }

    // TODO -- this is supposed to be a const_iterator, which is constructable from an iterator
    inline void insert(const iterator& hint, const value_type& value) {
        assert(!Tristate());
        if (SmallMode()) {
            assert(hint.SmallMode());
            small_map_.insert(hint.small_it_, value);
        } else {
            assert(hint.BigMode());
            big_map_.insert(hint.big_it_, value);
        }
    }

    template <typename SplitOp>
    iterator split(const iterator whole_it, const index_type& index, const SplitOp& split_op) {
        assert(!Tristate());
        if (SmallMode()) {
            return small_map_.split(whole_it.small_it_, index, split_op);
        } else {
            return big_map_.split(whole_it.big_it_, index, split_op);
        }
    }

    inline iterator lower_bound(const key_type& key) {
        if (SmallMode()) {
            return iterator(small_map_.lower_bound(key));
        } else {
            return iterator(big_map_.lower_bound(key));
        }
    }

    inline const_iterator lower_bound(const key_type& key) const {
        if (SmallMode()) {
            return const_iterator(small_map_.lower_bound(key));
        } else {
            return const_iterator(big_map_.lower_bound(key));
        }
    }

    template <typename Value>
    inline iterator overwrite_range(const iterator& lower, Value&& value) {
        if (SmallMode()) {
            assert(lower.SmallMode());
            return small_map_.overwrite_range(lower.small_it_, std::forward<Value>(value));
        } else {
            assert(lower.BigMode());
            return big_map_.overwrite_range(lower.big_it_, std::forward<Value>(value));
        }
    }

    // With power comes responsibility.  You can get to the underlying maps, s.t. in inner loops, the "SmallMode" checks can be
    // avoided per call, just be sure and Get the correct one.
    BothRangeMapMode GetMode() const { return mode_; }
    const small_map& GetSmallMap() const {
        assert(SmallMode());
        return small_map_;
    }
    small_map& GetSmallMap() {
        assert(SmallMode());
        return small_map_;
    }
    const big_map& GetBigMap() const {
        assert(BigMode());
        return big_map_;
    }
    big_map& GetBigMap() {
        assert(BigMode());
        return big_map_;
    }
    BothRangeMap() : const_big_map_(big_map_), const_small_map_(small_map_), mode_(BothRangeMapMode::kBig) {}
    BothRangeMap(index_type limit)
        : big_map_(),
          small_map_(limit <= N ? limit : 0),
          const_big_map_(big_map_),
          const_small_map_(small_map_),
          mode_(limit <= N ? BothRangeMapMode::kSmall : BothRangeMapMode::kBig) {}

    inline bool empty() const {
        if (SmallMode()) {
            return small_map_.empty();
        } else {
            return big_map_.empty();
        }
    }

    inline size_t size() const {
        if (SmallMode()) {
            return small_map_.size();
        } else {
            return big_map_.size();
        }
    }

    inline bool SmallMode() const { return BothRangeMapMode::kSmall == mode_; }
    inline bool BigMode() const { return BothRangeMapMode::kBig == mode_; }
    inline bool Tristate() const { return BothRangeMapMode::kTristate == mode_; }

  private:
    BigMap big_map_;
    SmallMap small_map_;
    const BigMap& const_big_map_;
    const SmallMap& const_small_map_;
    BothRangeMapMode mode_;
};

}  // namespace subresource_adapter

#endif
