/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Copyright (C) 2019-2025 Google Inc.
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

#include <cstddef>
#include <variant>
#include <vector>
#include "containers/range.h"
#include "containers/range_map.h"
#include "containers/small_vector.h"
#include "vulkan/vulkan.h"

namespace vvl {
class Image;
}  // namespace vvl

namespace subresource_adapter {

class RangeEncoder;
using IndexType = uint64_t;  // TODO: just update to 32 bit, but before collect memory usage stats, perf stats
using IndexRange = vvl::range<IndexType>;
using split_op_keep_both = sparse_container::split_op_keep_both;

// Interface for aspect specific traits objects (now isolated in the cpp file)
class AspectParameters {
  public:
    virtual ~AspectParameters() {}
    static const AspectParameters* Get(VkImageAspectFlags);
    virtual VkImageAspectFlags AspectMask() const = 0;
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

    Subresource& operator=(const Subresource&) = default;
};

// Subresource is encoded in (from slowest varying to fastest)
//    aspect_index
//    mip_level_index
//    array_layer_index
// into continuous index ranges
class RangeEncoder {
  public:
    static constexpr uint32_t kMaxSupportedAspect = 4;

    // The default constructor for default iterators
    RangeEncoder()
        : limits_(),
          full_range_(),
          mip_size_(0),
          aspect_size_(0),
          aspect_bits_(nullptr),
          encode_function_(nullptr),
          decode_function_(nullptr),
          lower_bound_function_(nullptr),
          lower_bound_with_start_function_(nullptr),
          aspect_base_{0, 0, 0} {}

    // Create the encoder suitable to the full range (aspect mask *must* be canonical)
    explicit RangeEncoder(const VkImageSubresourceRange& full_range)
         : RangeEncoder(full_range, AspectParameters::Get(full_range.aspectMask)) {}
    RangeEncoder(const RangeEncoder& from) = default;

    inline bool InRange(const VkImageSubresource& subres) const {
        return (subres.mipLevel < limits_.mipLevel) && (subres.arrayLayer < limits_.arrayLayer) &&
               (subres.aspectMask & limits_.aspectMask);
    }
    inline bool InRange(const VkImageSubresourceRange& range) const {
        return (range.baseMipLevel < limits_.mipLevel) && ((range.baseMipLevel + range.levelCount) <= limits_.mipLevel) &&
               (range.baseArrayLayer < limits_.arrayLayer) && ((range.baseArrayLayer + range.layerCount) <= limits_.arrayLayer) &&
               (range.aspectMask & limits_.aspectMask);
    }

    inline IndexType Encode(const Subresource& pos) const { return (this->*(encode_function_))(pos); }
    inline IndexType Encode(const VkImageSubresource& subres) const { return Encode(Subresource(*this, subres)); }

    Subresource Decode(const IndexType& index) const { return (this->*decode_function_)(index); }

    inline Subresource BeginSubresource(const VkImageSubresourceRange& range) const {
        if (!InRange(range)) {
            return limits_;
        }
        const auto aspect_index = LowerBoundFromMask(range.aspectMask);
        Subresource begin(aspect_bits_[aspect_index], range.baseMipLevel, range.baseArrayLayer, aspect_index);
        return begin;
    }

    inline Subresource Begin() const {
        Subresource begin(aspect_bits_[0], 0, 0, 0);
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
    inline VkImageSubresource IndexToVkSubresource(const IndexType& index) const { return MakeVkSubresource(Decode(index)); }

  protected:
    RangeEncoder(const VkImageSubresourceRange& full_range, const AspectParameters* param);

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
        if constexpr (N > 2) {
            if (index >= aspect_base_[2]) {
                return Subresource(aspect_bits_[2], 0, static_cast<uint32_t>(index - aspect_base_[2]), 2);
            }
        } else if constexpr (N > 1) {
            if (index >= aspect_base_[1]) {
                return Subresource(aspect_bits_[1], 0, static_cast<uint32_t>(index - aspect_base_[1]), 1);
            }
        }
        // NOTE: aspect_base_[0] is always 0... here and below
        return Subresource(aspect_bits_[0], 0, static_cast<uint32_t>(index), 0);
    }

    // For ranges that only have a single array layer...
    template <uint32_t N>
    Subresource DecodeAspectMipOnly(const IndexType& index) const {
        if constexpr (N > 2) {
            if (index >= aspect_base_[2]) {
                return Subresource(aspect_bits_[2], static_cast<uint32_t>(index - aspect_base_[2]), 0, 2);
            }
        } else if constexpr (N > 1) {
            if (index >= aspect_base_[1]) {
                return Subresource(aspect_bits_[1], static_cast<uint32_t>(index - aspect_base_[1]), 0, 1);
            }
        }
        return Subresource(aspect_bits_[0], static_cast<uint32_t>(index), 0, 0);
    }

    // For ranges that only have both > 1 layer and level
    template <uint32_t N>
    Subresource DecodeAspectMipArray(const IndexType& index) const {
        assert(limits_.aspect_index <= N);
        uint32_t aspect_index = 0;
        if constexpr (N > 2) {
            if (index >= aspect_base_[2]) {
                aspect_index = 2;
            }
        } else if constexpr (N > 1) {
            if (index >= aspect_base_[1]) {
                aspect_index = 1;
            }
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

    explicit SubresourceGenerator(const RangeEncoder& encoder)
        : Subresource(encoder.Begin()), encoder_(&encoder), limits_(encoder.FullRange()) {}

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
    explicit RangeGenerator(const RangeEncoder& encoder) : RangeGenerator(encoder, encoder.FullRange()) {}
    RangeGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range);
    const IndexRange& operator*() const { return pos_; }
    const IndexRange* operator->() const { return &pos_; }
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
    struct SubresInfo {
        VkSubresourceLayout layout;
        VkExtent3D extent;
        SubresInfo(const VkSubresourceLayout& layout_, const VkExtent3D& extent_, const VkExtent3D& texel_extent,
                   double texel_size);
        SubresInfo(const SubresInfo&);
        SubresInfo() = default;
        VkDeviceSize y_step_pitch;
        VkDeviceSize z_step_pitch;
        VkDeviceSize layer_span;
    };

    // The default constructor for default iterators
    ImageRangeEncoder() {}

    ImageRangeEncoder(const vvl::Image& image, const AspectParameters* param);
    explicit ImageRangeEncoder(const vvl::Image& image);
    ImageRangeEncoder(const ImageRangeEncoder& from) = default;

    inline IndexType Encode2D(const VkSubresourceLayout& layout, uint32_t layer, uint32_t aspect_index,
                              const VkOffset3D& offset) const;
    inline IndexType Encode3D(const VkSubresourceLayout& layout, uint32_t aspect_index, const VkOffset3D& offset) const;
    void Decode(const VkImageSubresource& subres, const IndexType& encode, uint32_t& out_layer, VkOffset3D& out_offset) const;

    inline uint32_t GetSubresourceIndex(uint32_t aspect_index, uint32_t mip_level) const {
        return mip_level + (aspect_index ? (aspect_index * limits_.mipLevel) : 0U);
    }
    inline const SubresInfo& GetSubresourceInfo(uint32_t index) const { return subres_info_[index]; }

    inline IndexType GetAspectSize(uint32_t aspect_index) const { return aspect_sizes_[aspect_index]; }
    inline VkExtent2D GetAspectExtentDivisors(uint32_t aspect_index) const { return aspect_extent_divisors_[aspect_index]; }
    inline const double& TexelSize(int aspect_index) const { return texel_sizes_[aspect_index]; }
    inline bool IsLinearImage() const { return linear_image_; }
    inline IndexType TotalSize() const { return total_size_; }
    inline bool Is3D() const { return is_3_d_; }
    inline bool IsInterleaveY() const { return y_interleave_; }
    inline bool IsCompressed() const { return is_compressed_; }
    const VkExtent3D& TexelBlockExtent() const { return texel_block_extent_; }

    using SubresInfoVector = std::vector<SubresInfo>;

  private:
    std::vector<double> texel_sizes_;
    SubresInfoVector subres_info_;
    small_vector<IndexType, 4, uint32_t> aspect_sizes_;
    small_vector<VkExtent2D, 4, uint32_t> aspect_extent_divisors_;
    IndexType total_size_;
    VkExtent3D texel_block_extent_;
    bool is_3_d_;
    bool linear_image_;
    bool y_interleave_;
    bool is_compressed_;
};

class ImageRangeGenerator {
  public:
    using RangeType = IndexRange;
    ImageRangeGenerator(const ImageRangeGenerator&) = default;
    ImageRangeGenerator() : encoder_(nullptr), subres_range_(), offset_(), extent_(), base_address_(), pos_() {}
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, const VkOffset3D& offset,
                        const VkExtent3D& extent, VkDeviceSize base_address, bool is_depth_sliced);
    void SetInitialPosFullOffset(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullWidth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullHeight(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosSomeDepth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullDepth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosAllLayers(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosOneAspect(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosAllSubres(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosSomeLayers(uint32_t layer, uint32_t aspect_index);
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, VkDeviceSize base_address,
                        bool is_depth_sliced);
    inline const IndexRange& operator*() const { return pos_; }
    inline const IndexRange* operator->() const { return &pos_; }
    ImageRangeGenerator& operator++();
    ImageRangeGenerator& operator=(const ImageRangeGenerator&) = default;

  private:
    bool Convert2DCompatibleTo3D();
    void SetUpSubresInfo();
    void SetUpIncrementerDefaults();
    void SetUpSubresIncrementer();
    void SetUpIncrementer(bool all_width, bool all_height, bool all_depth);
    typedef void (ImageRangeGenerator::*SetInitialPosFn)(uint32_t, uint32_t);
    inline void SetInitialPos(uint32_t layer, uint32_t aspect_index) { (this->*(set_initial_pos_fn_))(layer, aspect_index); }

    VkOffset3D GetOffset(uint32_t aspect_index) const;
    VkExtent3D GetExtent(uint32_t aspect_index) const;

    const ImageRangeEncoder* encoder_;
    VkImageSubresourceRange subres_range_;
    VkOffset3D offset_;
    VkExtent3D extent_;
    VkDeviceSize base_address_;

    uint32_t mip_index_ = 0U;
    uint32_t incr_mip_ = 0U;
    uint32_t aspect_index_ = 0U;
    uint32_t subres_index_ = 0U;
    const ImageRangeEncoder::SubresInfo* subres_info_ = nullptr;

    SetInitialPosFn set_initial_pos_fn_ = nullptr;
    IndexRange pos_;

    struct IncrementerState {
        // These should be invariant across subresources (mip/aspect)
        uint32_t y_step = 0U;
        uint32_t layer_z_step = 0U;

        // These vary per mip at least...
        uint32_t y_count = 0U;
        uint32_t layer_z_count = 0U;
        uint32_t y_index = 0U;
        uint32_t layer_z_index = 0U;
        IndexRange y_base = {0U, 0U};
        IndexRange layer_z_base = {0U, 0U};
        IndexType incr_y = 0U;
        IndexType incr_layer_z = 0U;
        void Set(uint32_t y_count_, uint32_t layer_z_count_, IndexType base, IndexType span, IndexType y_step, IndexType z_step);
    };
    IncrementerState incr_state_;
    bool single_full_size_range_ = true;
    bool is_depth_sliced_ = false;
};

// double wrapped map variants.. to avoid needing to templatize on the range map type.  The underlying maps are available for
// use in performance sensitive places that are *already* templatized (for example update_range_value).
// In STL style.  Note that N must be < uint8_t max
template <typename T, size_t N>
class BothRangeMap {
    using SmallMap = sparse_container::small_range_map<IndexType, T, vvl::range<IndexType>, N>;
    using SmallMapIterator = typename SmallMap::iterator;
    using SmallMapConstIterator = typename SmallMap::const_iterator;

    using BigMap = sparse_container::range_map<IndexType, T>;
    using BigMapIterator = typename BigMap::iterator;
    using BigMapConstIterator = typename BigMap::const_iterator;

  public:
    using index_type = IndexType;
    using key_type = vvl::range<IndexType>;
    using mapped_type = T;
    using value_type = std::pair<const key_type, mapped_type>;

    template <typename Value, typename SmallIt, typename BigIt>
    class IteratorImpl {
      public:
        Value* operator->() const {
            if (is_small_it_) {
                return small_it_.operator->();
            } else {
                return big_it_.operator->();
            }
        }
        Value& operator*() const {
            if (is_small_it_) {
                return small_it_.operator*();
            } else {
                return big_it_.operator*();
            }
        }
        IteratorImpl& operator++() {
            if (is_small_it_) {
                small_it_.operator++();
            } else {
                big_it_.operator++();
            }
            return *this;
        }
        IteratorImpl& operator--() {
            if (is_small_it_) {
                small_it_.operator--();
            } else {
                big_it_.operator--();
            }
            return *this;
        }
        IteratorImpl& operator=(const IteratorImpl& other) {
            is_small_it_ = other.is_small_it_;
            small_it_ = other.small_it_;
            big_it_ = other.big_it_;
            return *this;
        }
        bool operator==(const IteratorImpl& other) const {
            // It's enough just to compare both iterators.
            return small_it_ == other.small_it_ && big_it_ == other.big_it_;
        }
        bool operator!=(const IteratorImpl& other) const { return !(*this == other); }

        IteratorImpl() = default;
        IteratorImpl(const IteratorImpl& other) = default;
        IteratorImpl(const SmallIt& it) : is_small_it_(true), small_it_(it) {}
        IteratorImpl(const BigIt& it) : is_small_it_(false), big_it_(it) {}

      private:
        friend BothRangeMap;

        bool is_small_it_ = false;
        SmallIt small_it_;
        BigIt big_it_;
    };

    using iterator = IteratorImpl<value_type, SmallMapIterator, BigMapIterator>;
    // TODO change const iterator to derived class if iterator -> const_iterator constructor is needed
    using const_iterator = IteratorImpl<const value_type, SmallMapConstIterator, BigMapConstIterator>;

    iterator begin() {
        if (UsesSmallMap()) {
            return iterator(GetSmallMap().begin());
        } else {
            return iterator(GetBigMap().begin());
        }
    }
    const_iterator cbegin() const {
        if (UsesSmallMap()) {
            return const_iterator(GetSmallMap().begin());
        } else {
            return const_iterator(GetBigMap().begin());
        }
    }
    const_iterator begin() const { return cbegin(); }

    iterator end() {
        if (UsesSmallMap()) {
            return iterator(GetSmallMap().end());
        } else {
            return iterator(GetBigMap().end());
        }
    }
    const_iterator cend() const {
        if (UsesSmallMap()) {
            return const_iterator(GetSmallMap().end());
        } else {
            return const_iterator(GetBigMap().end());
        }
    }
    const_iterator end() const { return cend(); }

    iterator find(const key_type& key) {
        if (UsesSmallMap()) {
            return iterator(GetSmallMap().find(key));
        } else {
            return iterator(GetBigMap().find(key));
        }
    }

    const_iterator find(const key_type& key) const {
        if (UsesSmallMap()) {
            return const_iterator(GetSmallMap().find(key));
        } else {
            return const_iterator(GetBigMap().find(key));
        }
    }

    iterator find(const index_type& index) {
        if (UsesSmallMap()) {
            return iterator(GetSmallMap().find(index));
        } else {
            return iterator(GetBigMap().find(index));
        }
    }

    const_iterator find(const index_type& index) const {
        if (UsesSmallMap()) {
            return const_iterator(GetSmallMap().find(index));
        } else {
            return const_iterator(GetBigMap().find(index));
        }
    }

    // TODO -- this is supposed to be a const_iterator, which is constructable from an iterator
    void insert(const iterator& hint, const value_type& value) {
        if (UsesSmallMap()) {
            assert(hint.is_small_it_);
            GetSmallMap().insert(hint.small_it_, value);
        } else {
            assert(!hint.is_small_it_);
            GetBigMap().insert(hint.big_it_, value);
        }
    }

    iterator lower_bound(const key_type& key) {
        if (UsesSmallMap()) {
            return iterator(GetSmallMap().lower_bound(key));
        } else {
            return iterator(GetBigMap().lower_bound(key));
        }
    }

    const_iterator lower_bound(const key_type& key) const {
        if (UsesSmallMap()) {
            return const_iterator(GetSmallMap().lower_bound(key));
        } else {
            return const_iterator(GetBigMap().lower_bound(key));
        }
    }

    template <typename Value>
    iterator overwrite_range(const iterator& lower, Value&& value) {
        if (UsesSmallMap()) {
            assert(lower.is_small_it_);
            return GetSmallMap().overwrite_range(lower.small_it_, std::forward<Value>(value));
        } else {
            assert(!lower.is_small_it_);
            return GetBigMap().overwrite_range(lower.big_it_, std::forward<Value>(value));
        }
    }

    // With power comes responsibility (🕷).  You can get to the underlying maps, s.t. in inner loops, the "SmallMode" checks can be
    // avoided per call, just be sure and Get the correct one.
    const SmallMap& GetSmallMap() const {
        assert(UsesSmallMap());
        return std::get<SmallMap>(map_);
    }
    SmallMap& GetSmallMap() {
        assert(UsesSmallMap());
        return std::get<SmallMap>(map_);
    }
    const BigMap& GetBigMap() const {
        assert(!UsesSmallMap());
        return std::get<BigMap>(map_);
    }
    BigMap& GetBigMap() {
        assert(!UsesSmallMap());
        return std::get<BigMap>(map_);
    }

    BothRangeMap() = delete;

    BothRangeMap(index_type limit) {
        if (limit <= N) {
            map_ = SmallMap(limit);
        } else {
            map_ = BigMap();
        }
    }

    bool empty() const {
        if (UsesSmallMap()) {
            return GetSmallMap().empty();
        } else {
            return GetBigMap().empty();
        }
    }

    size_t size() const {
        if (UsesSmallMap()) {
            return GetSmallMap().size();
        } else {
            return GetBigMap().size();
        }
    }

    bool UsesSmallMap() const { return std::holds_alternative<SmallMap>(map_); }

  private:
    std::variant<SmallMap, BigMap> map_;
};

}  // namespace subresource_adapter
