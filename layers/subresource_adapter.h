/* Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
 * Copyright (C) 2019 Google Inc.
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

#include <array>
#include "vulkan/vulkan.h"

#include "range_vector.h"

namespace subresource_adapter {
using IndexType = uint64_t;

template <typename Element>
using Range = sparse_container::range<Element>;

using IndexRange = Range<IndexType>;

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

struct Subresource {
    uint32_t array_layer;
    uint32_t mip_level;
    uint32_t aspect_index;
    VkImageAspectFlags selected_aspects;
    Subresource() : array_layer(0), mip_level(0), aspect_index(0), selected_aspects(0) {}

    Subresource(uint32_t array_layer_, uint32_t mip_level_, uint32_t aspect_index_, VkImageAspectFlags selected_aspects_)
        : array_layer(array_layer_), mip_level(mip_level_), aspect_index(aspect_index_), selected_aspects(selected_aspects_) {}
};
using SubresourceRange = Range<Subresource>;

// Subresource is encoded in (from slowest varying to fastest)
//    aspect_index
//    mip_level_index
//    array_layer_index
// into continuous index ranges
class RangeEncoder {
    static constexpr uint32_t kMaxSupportedAspect = 3;

  public:
    using IndexRange = sparse_container::range<IndexType>;

  protected:
    Subresource limits_;
    const size_t mip_size_;
    const size_t aspect_size_;
    const VkImageAspectFlagBits* const aspect_bits_;
    uint32_t (*const mask_index_function_)(VkImageAspectFlags);
    IndexType (RangeEncoder::*encode_function_)(const Subresource&) const;
    Subresource (RangeEncoder::*decode_function_)(const IndexType&) const;
    IndexType aspect_base_[kMaxSupportedAspect];

    IndexType Encode1AspectArrayOnly(const Subresource& pos) const;
    IndexType Encode1AspectMipArray(const Subresource& pos) const;
    IndexType Encode1AspectMipOnly(const Subresource& pos) const;

    IndexType EncodeAspectArrayOnly(const Subresource& pos) const;
    IndexType EncodeAspectMipArray(const Subresource& pos) const;
    IndexType EncodeAspectMipOnly(const Subresource& pos) const;

    // Use compiler to create the aspect count variants...
    template <uint32_t N>
    Subresource DecodeAspectArrayOnly(const IndexType& index) const {
        if ((N > 2) && (index >= aspect_base_[2])) {
            return Subresource(static_cast<uint32_t>(index - aspect_base_[2]), 0, 2, aspect_bits_[2]);
        } else if ((N > 1) && (index >= aspect_base_[1])) {
            return Subresource(static_cast<uint32_t>(index - aspect_base_[1]), 0, 1, aspect_bits_[1]);
        }
        return Subresource(static_cast<uint32_t>(index), 0, 0, aspect_bits_[0]);
    }

    template <uint32_t N>
    Subresource DecodeAspectMipOnly(const IndexType& index) const {
        if ((N > 2) && (index >= aspect_base_[2])) {
            return Subresource(0, static_cast<uint32_t>(index - aspect_base_[2]), 2, aspect_bits_[2]);
        } else if ((N > 1) && (index >= aspect_base_[1])) {
            return Subresource(0, static_cast<uint32_t>(index - aspect_base_[1]), 1, aspect_bits_[1]);
        }
        return Subresource(0, static_cast<uint32_t>(index), 0, aspect_bits_[0]);
    }

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

        return Subresource(static_cast<uint32_t>(array_offset), static_cast<uint32_t>(mip_level), aspect_index,
                           aspect_bits_[aspect_index]);
    }

  public:
    // The default constructor for default iterators
    RangeEncoder()
        : limits_(),
          mip_size_(0),
          aspect_size_(0),
          aspect_bits_(nullptr),
          mask_index_function_(nullptr),
          encode_function_(nullptr),
          decode_function_(nullptr),
          aspect_base_{0, 0, 0} {}

    RangeEncoder(const VkImageSubresourceRange& full_range, const AspectParameters* param);
    // Create the encoder suitable to the full range (aspect mask *must* be canonical)
    RangeEncoder(const VkImageSubresourceRange& full_range)
        : RangeEncoder(full_range, AspectParameters::Get(full_range.aspectMask)) {}

    bool InRange(const VkImageSubresource& subres) const;
    bool InRange(const VkImageSubresourceRange& range) const;
    inline IndexType Encode(const Subresource& pos) const { return (this->*(encode_function_))(pos); }
    inline IndexType Encode(const VkImageSubresource& subres) const { return Encode(MakeSubresource(subres)); }

    Subresource Decode(const IndexType& index) const { return (this->*decode_function_)(index); }

    uint32_t LowerBoundFromMask(VkImageAspectFlags mask, uint32_t starting_index = 0) const;
    uint32_t UpperBoundFromMask(VkImageAspectFlags mask) const;
    inline IndexType AspectSize() const { return aspect_size_; }
    inline IndexType MipSize() const { return mip_size_; }
    inline const Subresource& Limits() const { return limits_; }
    inline VkImageAspectFlags AspectMask() const { return limits_.selected_aspects; }
    inline VkImageAspectFlagBits AspectBit(uint32_t aspect_index) const {
        RANGE_ASSERT(aspect_index < limits_.aspect_index);
        return aspect_bits_[aspect_index];
    }
    inline IndexType AspectBase(uint32_t aspect_index) const {
        RANGE_ASSERT(aspect_index < limits_.aspect_index);
        return aspect_base_[aspect_index];
    }

    inline VkImageSubresource MakeVkSubresource(const Subresource& subres) const {
        VkImageSubresource vk_subres = {static_cast<VkImageAspectFlags>(aspect_bits_[subres.aspect_index]), subres.mip_level,
                                        subres.array_layer};
        return vk_subres;
    }

  protected:
    void PopulateFunctionPointers();

  public:
    Subresource MakeSubresource(const VkImageSubresource&) const;
    SubresourceRange MakeSubresourceRange(const VkImageSubresourceRange& range) const;
};

class VkImageSubresourceGenerator {
    const RangeEncoder* encoder_;
    const SubresourceRange limits_;
    VkImageSubresource pos_;
    uint32_t aspect_index_;

  public:
    VkImageSubresourceGenerator() : encoder_(nullptr), limits_(), pos_(), aspect_index_(0){};
    VkImageSubresourceGenerator(const RangeEncoder& encoder, const SubresourceRange& bounds)
        : encoder_(&encoder),
          limits_(bounds),
          pos_(encoder_->MakeVkSubresource(bounds.begin)),
          aspect_index_(bounds.begin.aspect_index) {}
    VkImageSubresourceGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range)
        : VkImageSubresourceGenerator(encoder, encoder.MakeSubresourceRange(subres_range)) {}

    const SubresourceRange& Limits() const { return limits_; }

    // Seek functions are used by generators to force synchronization, as callers may have altered the position
    // to iterater between calls to the generator increment, offset, or seek functions
    void seek_aspect(uint32_t aspect_index) {
        pos_.arrayLayer = limits_.begin.array_layer;
        pos_.mipLevel = limits_.begin.mip_level;
        if (aspect_index < limits_.end.aspect_index) {
            aspect_index_ = aspect_index;
            pos_.aspectMask = encoder_->AspectBit(aspect_index_);
        } else {
            // This is an "end" tombstone
            aspect_index_ = limits_.end.aspect_index;
            pos_.aspectMask = 0;
        }
    }

    void seek_mip(uint32_t mip_level) {
        pos_.arrayLayer = limits_.begin.array_layer;
        pos_.mipLevel = mip_level;
    }

    // Next and and ++ functions are for iteration from a base with the bounds, this may be additionally
    // controlled/updated by an owning generator (like RangeGenerator using seek functions)
    void next_aspect() { seek_aspect(aspect_index_ + 1); }

    void next_mip() {
        pos_.arrayLayer = limits_.begin.array_layer;
        pos_.mipLevel++;
        if (pos_.mipLevel >= limits_.end.mip_level) {
            next_aspect();
        }
    }

    VkImageSubresourceGenerator& operator++() {
        pos_.arrayLayer++;
        if (pos_.arrayLayer >= limits_.end.array_layer) {
            next_mip();
        }
        return *this;
    }

    // TODO -- optimize this if it becomes a hotspot.
    // General purpose and slow, when we have no other information to update the generator
    void seek(IndexType index) {
        // skip forward past discontinuities
        pos_ = encoder_->MakeVkSubresource(encoder_->Decode(index));
    }

    // TODO -- optimize this if it becomes a hotspot.
    void offset(IndexType offset) {
        // skip forward past discontinuities
        Subresource subres(pos_.arrayLayer, pos_.mipLevel, aspect_index_, pos_.aspectMask);
        IndexType index = encoder_->Encode(subres);
        seek(index + offset);
    }

    const VkImageSubresource& operator*() const { return pos_; }
    const VkImageSubresource* operator->() const { return &pos_; }
};

// Like an iterator for ranges...
class RangeGenerator {
    const RangeEncoder* encoder_;
    VkImageSubresourceGenerator isr_pos_;
    IndexRange pos_;
    IndexRange aspect_base_;
    uint32_t mip_count_ = 0;
    uint32_t mip_index_ = 0;
    uint32_t selected_aspects_ = 0;
    uint32_t aspect_index_ = 0;
    uint32_t mask_limit_ = 0;

  public:
    RangeGenerator() : encoder_(nullptr), isr_pos_(), pos_(), aspect_base_() {}
    bool operator!=(const RangeGenerator& rhs) { return (pos_ != rhs.pos_) || (&encoder_ != &rhs.encoder_); }
    RangeGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range);
    inline const IndexRange& operator*() const { return pos_; }
    inline const IndexRange* operator->() const { return &pos_; }
    // Returns a generator suitable for iterating within a range, is modified by operator ++ to bring
    // it in line with sync.
    VkImageSubresourceGenerator& SubresourceGenerator() { return isr_pos_; }
    RangeGenerator& operator++();
};

};  // namespace subresource_adapter

#endif
