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
#include <cassert>
#include "subresource_adapter.h"

namespace subresource_adapter {
Subresource::Subresource(const RangeEncoder& encoder, const VkImageSubresource& subres)
    : VkImageSubresource({0, subres.mipLevel, subres.arrayLayer}), aspect_index() {
    aspect_index = encoder.LowerBoundFromMask(subres.aspectMask);
    aspectMask = encoder.AspectBit(aspect_index);
}

IndexType RangeEncoder::Encode1AspectArrayOnly(const Subresource& pos) const { return pos.arrayLayer; }
IndexType RangeEncoder::Encode1AspectMipArray(const Subresource& pos) const { return pos.arrayLayer + pos.mipLevel * mip_size_; }
IndexType RangeEncoder::Encode1AspectMipOnly(const Subresource& pos) const { return pos.mipLevel; }

IndexType RangeEncoder::EncodeAspectArrayOnly(const Subresource& pos) const {
    return pos.arrayLayer + aspect_base_[pos.aspect_index];
}
IndexType RangeEncoder::EncodeAspectMipArray(const Subresource& pos) const {
    return pos.arrayLayer + pos.mipLevel * mip_size_ + aspect_base_[pos.aspect_index];
}
IndexType RangeEncoder::EncodeAspectMipOnly(const Subresource& pos) const { return pos.mipLevel + aspect_base_[pos.aspect_index]; }

uint32_t RangeEncoder::LowerBoundImpl1(VkImageAspectFlags aspect_mask) const {
    assert(aspect_mask & aspect_bits_[0]);
    return 0;
}
uint32_t RangeEncoder::LowerBoundWithStartImpl1(VkImageAspectFlags aspect_mask, uint32_t start) const {
    assert(start == 0);
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    }
    return limits_.aspect_index;
}

uint32_t RangeEncoder::LowerBoundImpl2(VkImageAspectFlags aspect_mask) const {
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    }
    assert(aspect_mask & aspect_bits_[1]);
    return 1;
}
uint32_t RangeEncoder::LowerBoundWithStartImpl2(VkImageAspectFlags aspect_mask, uint32_t start) const {
    if (start == 0) {
        if (aspect_mask & aspect_bits_[0]) {
            return 0;
        }
    } else {
        assert(start == 1);
        if (aspect_mask & aspect_bits_[1]) {
            return 1;
        }
    }
    return limits_.aspect_index;
}

uint32_t RangeEncoder::LowerBoundImpl3(VkImageAspectFlags aspect_mask) const {
    if (aspect_mask & aspect_bits_[0]) {
        return 0;
    } else if (aspect_mask & aspect_bits_[1]) {
        return 1;
    } else {
        assert(aspect_mask & aspect_bits_[2]);
        return 2;
    }
}

uint32_t RangeEncoder::LowerBoundWithStartImpl3(VkImageAspectFlags aspect_mask, uint32_t start) const {
    if (start == 0) {
        if (aspect_mask & aspect_bits_[0]) {
            return 0;
        }
    } else if (start == 1) {
        if ((aspect_mask & aspect_bits_[1])) {
            return 1;
        }
    } else {
        assert(start == 2);
        if ((aspect_mask & aspect_bits_[2])) {
            return 2;
        }
    }
    return limits_.aspect_index;
}

void RangeEncoder::PopulateFunctionPointers() {
    // Select the encode/decode specialists
    if (limits_.aspect_index == 1) {
        // One aspect use simplified encode/decode math
        if (limits_.arrayLayer == 1) {  // Same as mip_size_ == 1
            encode_function_ = &RangeEncoder::Encode1AspectMipOnly;
            decode_function_ = &RangeEncoder::DecodeAspectMipOnly<1>;
        } else if (limits_.mipLevel == 1) {
            encode_function_ = &RangeEncoder::Encode1AspectArrayOnly;
            decode_function_ = &RangeEncoder::DecodeAspectArrayOnly<1>;
        } else {
            encode_function_ = &RangeEncoder::Encode1AspectMipArray;
            decode_function_ = &RangeEncoder::DecodeAspectMipArray<1>;
        }
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl1;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl1;
    } else if (limits_.aspect_index == 2) {
        // Two aspect use simplified encode/decode math
        if (limits_.arrayLayer == 1) {  // Same as mip_size_ == 1
            encode_function_ = &RangeEncoder::EncodeAspectMipOnly;
            decode_function_ = &RangeEncoder::DecodeAspectMipOnly<2>;
        } else if (limits_.mipLevel == 1) {
            encode_function_ = &RangeEncoder::EncodeAspectArrayOnly;
            decode_function_ = &RangeEncoder::DecodeAspectArrayOnly<2>;
        } else {
            encode_function_ = &RangeEncoder::EncodeAspectMipArray;
            decode_function_ = &RangeEncoder::DecodeAspectMipArray<2>;
        }
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl2;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl2;
    } else {
        encode_function_ = &RangeEncoder::EncodeAspectMipArray;
        decode_function_ = &RangeEncoder::DecodeAspectMipArray<3>;
        lower_bound_function_ = &RangeEncoder::LowerBoundImpl3;
        lower_bound_with_start_function_ = &RangeEncoder::LowerBoundWithStartImpl3;
    }

    // Initialize the offset array
    aspect_base_[0] = 0;
    for (uint32_t i = 1; i < limits_.aspect_index; ++i) {
        aspect_base_[i] = aspect_base_[i - 1] + aspect_size_;
    }
}

RangeEncoder::RangeEncoder(const VkImageSubresourceRange& full_range, const AspectParameters* param)
    : limits_(param->AspectMask(), full_range.levelCount, full_range.layerCount, param->AspectCount()),
      mip_size_(full_range.layerCount),
      aspect_size_(mip_size_ * full_range.levelCount),
      aspect_bits_(param->AspectBits()),
      mask_index_function_(param->MaskToIndexFunction()),
      encode_function_(nullptr),
      decode_function_(nullptr) {
    // Only valid to create an encoder for a *whole* image (i.e. base must be zero, and the specified limits_.selected_aspects
    // *must* be equal to the traits aspect mask. (Encoder range assumes zero bases)
    assert(full_range.aspectMask == limits_.aspectMask);
    assert(full_range.baseArrayLayer == 0);
    assert(full_range.baseMipLevel == 0);
    // TODO: should be some static assert
    assert(param->AspectCount() <= kMaxSupportedAspect);
    PopulateFunctionPointers();
}

static bool IsValid(const RangeEncoder& encoder, const VkImageSubresourceRange& bounds) {
    const auto& limits = encoder.Limits();
    return (((bounds.aspectMask & limits.aspectMask) == bounds.aspectMask) &&
            (bounds.baseMipLevel + bounds.levelCount <= limits.mipLevel) &&
            (bounds.baseArrayLayer + bounds.layerCount <= limits.arrayLayer));
}

// Create an iterator like "generator" that for each increment produces the next index range matching the
// next contiguous (in index space) section of the VkImageSubresourceRange
// Ranges will always span the layerCount layers, and if the layerCount is the full range of the image (as known by
// the encoder) will span the levelCount mip levels as weill.
RangeGenerator::RangeGenerator(const RangeEncoder& encoder, const VkImageSubresourceRange& subres_range)
    : encoder_(&encoder), isr_pos_(encoder, subres_range), pos_(), aspect_base_() {
    assert(IsValid(encoder, isr_pos_.Limits()));

    // To see if we have a full range special case, need to compare the subres_range against the *encoders* limits
    const auto& limits = encoder.Limits();
    if ((subres_range.baseArrayLayer == 0 && subres_range.layerCount == limits.arrayLayer)) {
        if ((subres_range.baseMipLevel == 0) && (subres_range.levelCount == limits.mipLevel)) {
            if (subres_range.aspectMask == limits.aspectMask) {
                // Full range
                pos_.begin = 0;
                pos_.end = encoder.AspectSize() * limits.aspect_index;
                aspect_count_ = 1;  // Flag this to never advance aspects.
            } else {
                // All mips all layers but not all aspect
                pos_.begin = encoder.AspectBase(isr_pos_.aspect_index);
                pos_.end = pos_.begin + encoder.AspectSize();
                aspect_count_ = limits.aspect_index;
            }
        } else {
            // All array layers, but not all levels
            pos_.begin = encoder.AspectBase(isr_pos_.aspect_index) + subres_range.baseMipLevel * encoder.MipSize();
            pos_.end = pos_.begin + subres_range.levelCount * encoder.MipSize();
            aspect_count_ = limits.aspect_index;
        }

        // Full set of array layers at a time, thus we can span across all selected mip levels
        mip_count_ = 1;  // we don't ever advance across mips, as we do all of then in one range
    } else {
        // Each range covers all included array_layers for each selected mip_level for each given selected aspect
        // so we'll use the general purpose encode and smallest range size
        pos_.begin = encoder.Encode(isr_pos_);
        pos_.end = pos_.begin + subres_range.layerCount;

        // we do have to traverse across mips, though (other than Encode abover), we don't have to know which one we are on.
        mip_count_ = subres_range.levelCount;
        aspect_count_ = limits.aspect_index;
    }

    // To get to the next aspect range we offset from the last base
    aspect_base_ = pos_;
    mip_index_ = 0;
    aspect_index_ = isr_pos_.aspect_index;
}

RangeGenerator& RangeGenerator::operator++() {
    mip_index_++;
    // NOTE: If all selected mip levels are done at once, mip_count_ is set to one, not the number of selected mip_levels
    if (mip_index_ >= mip_count_) {
        const auto last_aspect_index = aspect_index_;
        // Seek the next value aspect (if any)
        aspect_index_ = encoder_->LowerBoundFromMask(isr_pos_.Limits().aspectMask, aspect_index_ + 1);
        if (aspect_index_ < aspect_count_) {
            // Force isr_pos to the beginning of this found aspect
            isr_pos_.seek_aspect(aspect_index_);
            // SubresourceGenerator should never be at tombstones we we aren't
            assert(isr_pos_.aspectMask != 0);

            // Offset by the distance between the last start of aspect and *this* start of aspect
            aspect_base_ += (encoder_->AspectBase(isr_pos_.aspect_index) - encoder_->AspectBase(last_aspect_index));
            pos_ = aspect_base_;
            mip_index_ = 0;
        } else {
            // Tombstone both index range and subresource positions to "At end" convention
            pos_ = {0, 0};
            isr_pos_.aspectMask = 0;
        }
    } else {
        // Note: for the layerCount < full_range.layerCount case, because the generated ranges per mip_level are discontinuous
        // we have to do each individual array of ranges
        pos_ += encoder_->MipSize();
        isr_pos_.seek_mip(isr_pos_.Limits().baseMipLevel + mip_index_);
    }
    return *this;
}

template <typename AspectTraits>
class AspectParametersImpl : public AspectParameters {
  public:
    VkImageAspectFlags AspectMask() const override { return AspectTraits::kAspectMask; }
    MaskIndexFunc MaskToIndexFunction() const override { return &AspectTraits::MaskIndex; }
    uint32_t AspectCount() const override { return AspectTraits::kAspectCount; };
    const VkImageAspectFlagBits* AspectBits() const override { return AspectTraits::AspectBits().data(); }
};

struct NullAspectTraits {
    static constexpr uint32_t kAspectCount = 0;
    static constexpr VkImageAspectFlags kAspectMask = 0;
    static uint32_t MaskIndex(VkImageAspectFlags mask) { return 0; };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{};
        return kAspectBits;
    }
};

struct ColorAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    static uint32_t MaskIndex(VkImageAspectFlags mask) { return 0; };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_COLOR_BIT}};
        return kAspectBits;
    }
};

struct DepthAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    static uint32_t MaskIndex(VkImageAspectFlags mask) { return 0; };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_DEPTH_BIT}};
        return kAspectBits;
    }
};

struct StencilAspectTraits {
    static constexpr uint32_t kAspectCount = 1;
    static constexpr VkImageAspectFlags kAspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    static uint32_t MaskIndex(VkImageAspectFlags mask) { return 0; };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{{VK_IMAGE_ASPECT_STENCIL_BIT}};
        return kAspectBits;
    }
};

struct DepthStencilAspectTraits {
    // VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,  >> 1 -> 1 -1 -> 0
    // VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004, >> 1 -> 2 -1 = 1
    static constexpr uint32_t kAspectCount = 2;
    static constexpr VkImageAspectFlags kAspectMask = (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    static uint32_t MaskIndex(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 1) - 1;
        assert((index == 0) || (index == 1));
        return index;
    };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT}};
        return kAspectBits;
    }
};

struct Multiplane2AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    static constexpr uint32_t kAspectCount = 2;
    static constexpr VkImageAspectFlags kAspectMask = (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
    static uint32_t MaskIndex(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 4) - 1;
        assert((index == 0) || (index == 1));
        return index;
    };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT}};
        return kAspectBits;
    }
};

struct Multiplane3AspectTraits {
    // VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010, >> 4 - 1 -> 0
    // VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020, >> 4 - 1 -> 1
    // VK_IMAGE_ASPECT_PLANE_2_BIT = 0x00000040, >> 4 - 1 -> 3
    static constexpr uint32_t kAspectCount = 3;
    static constexpr VkImageAspectFlags kAspectMask =
        (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT);
    static uint32_t MaskIndex(VkImageAspectFlags mask) {
        uint32_t index = (mask >> 4) - 1;
        index = index > 2 ? 2 : index;
        assert((index == 0) || (index == 1) || (index == 2));
        return index;
    };
    static const std::array<VkImageAspectFlagBits, kAspectCount>& AspectBits() {
        static std::array<VkImageAspectFlagBits, kAspectCount> kAspectBits{
            {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT}};
        return kAspectBits;
    }
};

// Create the encoder parameter suitable to the full range aspect mask (*must* be canonical)
const AspectParameters* AspectParameters::Get(VkImageAspectFlags aspect_mask) {
    // We need a persitent instance of each specialist containing only a VTABLE each
    static const AspectParametersImpl<ColorAspectTraits> kColorParam;
    static const AspectParametersImpl<DepthAspectTraits> kDepthParam;
    static const AspectParametersImpl<StencilAspectTraits> kStencilParam;
    static const AspectParametersImpl<DepthStencilAspectTraits> kDepthStencilParam;
    static const AspectParametersImpl<Multiplane2AspectTraits> kMutliplane2Param;
    static const AspectParametersImpl<Multiplane3AspectTraits> kMutliplane3Param;
    static const AspectParametersImpl<NullAspectTraits> kNullAspect;

    const AspectParameters* param;
    switch (aspect_mask) {
        case ColorAspectTraits::kAspectMask:
            param = &kColorParam;
            break;
        case DepthAspectTraits::kAspectMask:
            param = &kDepthParam;
            break;
        case StencilAspectTraits::kAspectMask:
            param = &kStencilParam;
            break;
        case DepthStencilAspectTraits::kAspectMask:
            param = &kDepthStencilParam;
            break;
        case Multiplane2AspectTraits::kAspectMask:
            param = &kMutliplane2Param;
            break;
        case Multiplane3AspectTraits::kAspectMask:
            param = &kMutliplane3Param;
            break;
        default:
            assert(false);
            param = &kNullAspect;
    }
    return param;
}

};  // namespace subresource_adapter
