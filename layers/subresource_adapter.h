/* Copyright (c) 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2019-2021 Valve Corporation
 * Copyright (c) 2019-2021 LunarG, Inc.
 * Copyright (C) 2019-2021 Google Inc.
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
#include <vector>
#include "range_vector.h"
#ifndef SPARSE_CONTAINER_UNIT_TEST
#include "vulkan/vulkan.h"
#else
#include "vk_snippets.h"
#endif

class IMAGE_STATE;

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
    virtual ~AspectParameters() {}
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
        : limits_(),
          full_range_(),
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
    RangeEncoder(const RangeEncoder& from) = default;
    ;

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

    Subresource limits_;

  private:
    VkImageSubresourceRange full_range_;
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
        *static_cast<Subresource*>(this) = encoder_->Decode(index);
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

class ImageRangeEncoder : public RangeEncoder {
  public:
    // The default constructor for default iterators
    ImageRangeEncoder() : image_(nullptr) {}

    ImageRangeEncoder(const IMAGE_STATE& image, const AspectParameters* param);
    ImageRangeEncoder(const IMAGE_STATE& image);
    ImageRangeEncoder(const ImageRangeEncoder& from) = default;

    inline IndexType Encode(const VkImageSubresource& subres, uint32_t layer, VkOffset3D offset) const;
    void Decode(const VkImageSubresource& subres, const IndexType& encode, uint32_t& out_layer, VkOffset3D& out_offset) const;

    const VkSubresourceLayout& SubresourceLayout(const VkImageSubresource& subres) const;
    inline const VkExtent3D& SubresourceExtent(int mip_level, int aspect_index_) const {
        return subres_extents_[mip_level * limits_.aspect_index + aspect_index_];
    }
    inline const double& TexelSize(int aspect_index) const { return texel_sizes_[aspect_index]; }
    inline bool IsLinearImage() const { return linear_image; }

  private:
    bool linear_image;
    const IMAGE_STATE* image_;
    std::vector<double> texel_sizes_;
    std::vector<VkExtent3D> subres_extents_;
    std::vector<VkSubresourceLayout> subres_layouts_;
};

class ImageRangeGenerator {
  public:
    ImageRangeGenerator() : encoder_(nullptr), subres_range_(), offset_(), extent_(), base_address_(), pos_() {}
    bool operator!=(const ImageRangeGenerator& rhs) { return (pos_ != rhs.pos_) || (&encoder_ != &rhs.encoder_); }
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, const VkOffset3D& offset,
                        const VkExtent3D& extent, VkDeviceSize base_address);
    inline const IndexRange& operator*() const { return pos_; }
    inline const IndexRange* operator->() const { return &pos_; }
    ImageRangeGenerator* operator++();
    void SetPos();

  private:
    const ImageRangeEncoder* encoder_;
    const VkImageSubresourceRange subres_range_;
    const VkOffset3D offset_;
    const VkExtent3D extent_;
    VkDeviceSize base_address_;
    uint32_t range_arraylayer_base_;
    uint32_t range_layer_count_;

    IndexRange pos_;
    IndexRange offset_offset_y_base_;
    IndexRange offset_layer_base_;
    uint32_t offset_y_index_;
    uint32_t offset_y_count_;
    uint32_t aspect_count_ = 0;
    uint32_t aspect_index_ = 0;

    // It doesn't have offset_z. If the z > 1, it will be used in arrayLayer.
    uint32_t arrayLayer_index_;
    uint32_t layer_count_;
    uint32_t mip_level_index_;
    uint32_t mip_count_;
    const VkSubresourceLayout* subres_layout_;
};

// Designed for use with RangeMap of MappedType
template <typename Map>
class ConstMapView {
  public:
    using KeyType = typename Map::key_type;
    using MappedType = typename Map::mapped_type;
    using MapValueType = typename Map::mapped_type;
    using MapIterator = typename Map::const_iterator;
    using CachedLowerBound = typename sparse_container::cached_lower_bound_impl<const Map>;

    struct ValueType {
        const VkImageSubresource& subresource;
        MapIterator it;
        ValueType(const VkImageSubresource& subresource_) : subresource(subresource_), it(){};
    };
    class ConstIterator {
      public:
        ConstIterator()
            : view_(nullptr),
              range_gen_(),
              cached_it_(),
              pos_(range_gen_.GetSubresource()),
              current_index_(),
              constant_value_bound_() {}
        ConstIterator& operator++() {
            Increment();
            return *this;
        }
        const ValueType* operator->() const { return &pos_; }
        const ValueType& operator*() const { return pos_; }
        // Only for comparisons to end()
        // Note: if a fully function == is needed, the AtEnd needs to be maintained, as end_iterator is a static.
        bool AtEnd() const { return pos_.subresource.aspectMask == 0; }
        bool operator==(const ConstIterator& other) const { return AtEnd() && other.AtEnd(); };
        bool operator!=(const ConstIterator& other) const { return AtEnd() != other.AtEnd(); };

      protected:
        friend ConstMapView;
        ConstIterator(const ConstMapView& view, const VkImageSubresourceRange& range)
            : view_(&view),
              range_gen_(view.GetEncoder(), range),
              cached_it_(view.GetMap(), range_gen_->begin),
              pos_(range_gen_.GetSubresource()),
              current_index_(range_gen_->begin),
              constant_value_bound_(current_index_) {
            UpdateRangeAndValue();
        }

        void Increment() {
            ++current_index_;
            ++(range_gen_.GetSubresourceGenerator());
            if (constant_value_bound_ <= current_index_) {
                UpdateRangeAndValue();
            }
        }

        void ForceEndCondition() { range_gen_.GetSubresource().aspectMask = 0; }

        // Constant value range logice, subreource / lower bound position advance logic
        // TODO: convert this piece into a template _impl function suitable for const and non-const view iterators
        void UpdateRangeAndValue() {
            bool not_found = true;
            while (range_gen_->non_empty() && not_found) {
                if (!cached_it_.includes(current_index_)) {
                    // The result of the seek can be invalid, valid, or end...
                    cached_it_.seek(current_index_);
                }

                if (cached_it_->lower_bound == view_->GetMap().end()) {
                    // We're past the end of mapped data. Set end condtion.
                    ForceEndCondition();
                    not_found = false;
                } else {
                    // Search within the current range_ for a constant valid constant value interval
                    // The while condition allows the parallel iterator to advance constant value ranges as needed.
                    while (range_gen_->includes(current_index_) && not_found) {
                        if (cached_it_->valid) {
                            // Our position with in the map is valid so we can update our value
                            pos_.it = cached_it_->lower_bound;
                            constant_value_bound_ = std::min(cached_it_->lower_bound->first.end, range_gen_->end);
                            not_found = false;
                        } else {
                            // We're skipping this gap in Map, set the index to the exclusive end and look again
                            // Note that we ONLY need to Seek the Subresource generator on a skip condition.
                            current_index_ = std::min(cached_it_->lower_bound->first.begin, range_gen_->end);
                            constant_value_bound_ = current_index_;
                            // Move the subresource to the end of the skipped range
                            range_gen_.GetSubresourceGenerator().Seek(current_index_);
                            cached_it_.seek(current_index_);
                        }
                    }

                    if (not_found) {
                        // We need to advance the index range to search as the current cached_it_ lies outside it, and there's
                        // no easy way to seek RangeGen
                        // ++range_gen will update Subresource.
                        ++range_gen_;
                        current_index_ = range_gen_->begin;
                    }
                }
            }

            if (range_gen_->empty()) {
                ForceEndCondition();
            }
        }

      private:
        const ConstMapView* view_;
        RangeGenerator range_gen_;
        CachedLowerBound cached_it_;
        ValueType pos_;
        IndexType current_index_;
        IndexType constant_value_bound_;
    };

    const Map& GetMap() const { return *map_; }
    const RangeEncoder& GetEncoder() const { return *encoder_; }

    inline ConstIterator Begin(const VkImageSubresourceRange& range) const { return ConstIterator(*this, range); }
    inline const ConstIterator& End() const { return end_; }

    // Enable range based for....
    inline ConstIterator begin() const { return Begin(encoder_->FullRange()); }
    inline const ConstIterator& end() const { return End(); }

    ConstMapView() : map_(nullptr), encoder_(nullptr), end_() {}
    ConstMapView(const Map& map, const RangeEncoder& encoder) : map_(&map), encoder_(&encoder), end_() {}

  private:
    const Map* map_;
    const RangeEncoder* encoder_;
    const ConstIterator end_;
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
                // Transition to tristate
                small_it_ = SmallIt();
                big_it_ = BigIt();
            } else if (other.SmallMode()) {
                small_it_ = other.small_it_;
                if (mode_ != other.mode_) {
                    big_it_ = BigIt();
                }
            } else {
                big_it_ = other.big_it_;
                if (mode_ != other.mode_) {
                    small_it_ = SmallIt();
                }
            }
            mode_ = other.mode_;
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
        IteratorImpl(const IteratorImpl& other)
            : small_it_(other.SmallMode() ? other.small_it_ : SmallIt()),
              big_it_(other.BigMode() ? other.big_it_ : BigIt()),
              mode_(other.mode_){};

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
            return iterator(small_map_->begin());
        } else {
            return iterator(big_map_->begin());
        }
    }
    inline const_iterator cbegin() const {
        if (SmallMode()) {
            return const_iterator(small_map_->begin());
        } else {
            return const_iterator(big_map_->begin());
        }
    }
    inline const_iterator begin() const { return cbegin(); }

    inline iterator end() {
        if (SmallMode()) {
            return iterator(small_map_->end());
        } else {
            return iterator(big_map_->end());
        }
    }
    inline const_iterator cend() const {
        if (SmallMode()) {
            return const_iterator(small_map_->end());
        } else {
            return const_iterator(big_map_->end());
        }
    }
    inline const_iterator end() const { return cend(); }

    inline iterator find(const key_type& key) {
        assert(!Tristate());
        if (SmallMode()) {
            return iterator(small_map_->find(key));
        } else {
            return iterator(big_map_->find(key));
        }
    }

    inline const_iterator find(const key_type& key) const {
        assert(!Tristate());
        if (SmallMode()) {
            return const_iterator(small_map_->find(key));
        } else {
            return const_iterator(big_map_->find(key));
        }
    }

    inline iterator find(const index_type& index) {
        assert(!Tristate());
        if (SmallMode()) {
            return iterator(small_map_->find(index));
        } else {
            return iterator(big_map_->find(index));
        }
    }

    inline const_iterator find(const index_type& index) const {
        assert(!Tristate());
        if (SmallMode()) {
            return const_iterator(static_cast<const SmallMap*>(small_map_)->find(index));
        } else {
            return const_iterator(static_cast<const BigMap*>(big_map_)->find(index));
        }
    }

    // TODO -- this is supposed to be a const_iterator, which is constructable from an iterator
    inline void insert(const iterator& hint, const value_type& value) {
        assert(!Tristate());
        if (SmallMode()) {
            assert(hint.SmallMode());
            small_map_->insert(hint.small_it_, value);
        } else {
            assert(hint.BigMode());
            big_map_->insert(hint.big_it_, value);
        }
    }

    template <typename SplitOp>
    iterator split(const iterator whole_it, const index_type& index, const SplitOp& split_op) {
        assert(!Tristate());
        if (SmallMode()) {
            return small_map_->split(whole_it.small_it_, index, split_op);
        } else {
            return big_map_->split(whole_it.big_it_, index, split_op);
        }
    }

    inline iterator lower_bound(const key_type& key) {
        if (SmallMode()) {
            return iterator(small_map_->lower_bound(key));
        } else {
            return iterator(big_map_->lower_bound(key));
        }
    }

    inline const_iterator lower_bound(const key_type& key) const {
        if (SmallMode()) {
            return const_iterator(small_map_->lower_bound(key));
        } else {
            return const_iterator(big_map_->lower_bound(key));
        }
    }

    template <typename Value>
    inline iterator overwrite_range(const iterator& lower, Value&& value) {
        if (SmallMode()) {
            assert(lower.SmallMode());
            return small_map_->overwrite_range(lower.small_it_, std::forward<Value>(value));
        } else {
            assert(lower.BigMode());
            return big_map_->overwrite_range(lower.big_it_, std::forward<Value>(value));
        }
    }

    // With power comes responsibility.  You can get to the underlying maps, s.t. in inner loops, the "SmallMode" checks can be
    // avoided per call, just be sure and Get the correct one.
    BothRangeMapMode GetMode() const { return mode_; }
    const small_map& GetSmallMap() const {
        assert(SmallMode());
        return *small_map_;
    }
    small_map& GetSmallMap() {
        assert(SmallMode());
        return *small_map_;
    }
    const big_map& GetBigMap() const {
        assert(BigMode());
        return *big_map_;
    }
    big_map& GetBigMap() {
        assert(BigMode());
        return *big_map_;
    }
    BothRangeMap() = delete;
    BothRangeMap(index_type limit) : mode_(ComputeMode(limit)), big_map_(MakeBigMap()), small_map_(MakeSmallMap(limit)) {}

    inline bool empty() const {
        if (SmallMode()) {
            return small_map_->empty();
        } else {
            assert(BigMode());
            return big_map_->empty();
        }
    }

    inline size_t size() const {
        if (SmallMode()) {
            return small_map_->size();
        } else {
            assert(BigMode());
            return big_map_->size();
        }
    }

    inline bool SmallMode() const { return BothRangeMapMode::kSmall == mode_; }
    inline bool BigMode() const { return BothRangeMapMode::kBig == mode_; }
    inline bool Tristate() const { return BothRangeMapMode::kTristate == mode_; }

  private:
    static BothRangeMapMode ComputeMode(index_type size_limit) {
        return size_limit <= N ? BothRangeMapMode::kSmall : BothRangeMapMode::kBig;
    }
    BigMap* MakeBigMap() {
        if (BigMode()) {
            return new (&backing_store) BigMap();
        }
        return nullptr;
    }
    SmallMap* MakeSmallMap(index_type limit) {
        if (SmallMode()) {
            return new (&backing_store) SmallMap(limit);
        }
        return nullptr;
    }

    BothRangeMapMode mode_ = BothRangeMapMode::kTristate;
    // Must be after mode_ as they use mode for initialization logic
    BigMap* big_map_ = nullptr;
    SmallMap* small_map_ = nullptr;

    using Storage = typename std::aligned_union<0, SmallMap, BigMap>::type;
    Storage backing_store;
};

}  // namespace subresource_adapter

#endif
