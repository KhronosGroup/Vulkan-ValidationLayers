/* Copyright (c) 2019-2026 The Khronos Group Inc.
 * Copyright (c) 2019-2026 Valve Corporation
 * Copyright (c) 2019-2026 LunarG, Inc.
 * Copyright (C) 2019-2026 Google Inc.
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

#include "containers/small_vector.h"
#include "state_tracker/subresource_encoding.h"
#include <optional>
#include <vector>

namespace vvl {
class Image;
}  // namespace vvl

namespace syncval {

class ImageRangeEncoder : public vvl::SubresourceEncoder {
  public:
    // Byte offsets and ranges in the fake address space. The base class indexes subresources
    using IndexType = VkDeviceSize;
    using IndexRange = vvl::range<IndexType>;

    struct SubresInfo {
        VkSubresourceLayout layout;
        VkExtent3D extent;
        VkDeviceSize y_step_pitch;
        VkDeviceSize z_step_pitch;
        VkDeviceSize layer_span;

        SubresInfo(const VkSubresourceLayout& layout, const VkExtent3D& extent);
    };

    // The default constructor for default iterators
    ImageRangeEncoder() {}

    ImageRangeEncoder(const vvl::Image& image, const vvl::AspectParameters* param);
    explicit ImageRangeEncoder(const vvl::Image& image);
    ImageRangeEncoder(const ImageRangeEncoder& from) = default;

    IndexType Encode2D(const VkSubresourceLayout& layout, uint32_t layer, uint32_t aspect_index, const VkOffset3D& offset) const;
    IndexType Encode3D(const VkSubresourceLayout& layout, uint32_t aspect_index, const VkOffset3D& offset) const;

    uint32_t GetSubresourceIndex(uint32_t aspect_index, uint32_t mip_level) const {
        return mip_level + (aspect_index ? (aspect_index * limits_.mipLevel) : 0U);
    }
    const SubresInfo& GetSubresourceInfo(uint32_t index) const { return subres_info_[index]; }

    IndexType GetAspectSize(uint32_t aspect_index) const { return aspect_sizes_[aspect_index]; }
    VkExtent2D GetAspectExtentDivisors(uint32_t aspect_index) const { return aspect_extent_divisors_[aspect_index]; }
    bool IsLinearImage() const { return linear_image_; }
    IndexType TotalSize() const { return total_size_; }
    bool Is3D() const { return is_3_d_; }
    bool IsInterleaveY() const { return y_interleave_; }
    const VkExtent3D& TexelBlockExtent() const { return texel_block_extent_; }

    uint32_t TexelBlockSize(uint32_t aspect_index) const { return texel_block_sizes_[aspect_index]; }

  private:
    std::vector<uint32_t> texel_block_sizes_;
    std::vector<SubresInfo> subres_info_;
    small_vector<IndexType, 4, uint32_t> aspect_sizes_;
    small_vector<VkExtent2D, 4, uint32_t> aspect_extent_divisors_;
    IndexType total_size_;
    VkExtent3D texel_block_extent_;
    bool is_3_d_;
    bool linear_image_;
    bool y_interleave_;
};

// Sequence of equally sized segments separated by gaps
struct StridedRange {
    using IndexType = ImageRangeEncoder::IndexType;
    using IndexRange = ImageRangeEncoder::IndexRange;

    IndexType base = 0;    // start address of the first segment
    IndexType width = 0;   // byte size of each segment
    IndexType stride = 0;  // byte distance between consecutive segment starts, greater than width
    uint32_t count = 0;    // number of segments

    bool operator==(const StridedRange& other) const {
        return base == other.base && width == other.width && stride == other.stride && count == other.count;
    }

    IndexRange Bounds() const {
        assert(count > 0);
        return IndexRange(base, base + (count - 1) * stride + width);
    }

    // Return the part of range up to the next segment boundary and set inside accordingly.
    // The input range may extend beyond Bounds()
    IndexRange NextSpan(IndexRange range, bool& inside) const;

    bool Intersects(IndexRange range) const;
    bool Covers(IndexRange range) const;
};

class ImageRangeGenerator {
  public:
    using IndexType = ImageRangeEncoder::IndexType;
    using IndexRange = ImageRangeEncoder::IndexRange;
    using RangeType = IndexRange;
    ImageRangeGenerator(const ImageRangeGenerator&) = default;
    ImageRangeGenerator() : encoder_(nullptr), subres_range_(), offset_(), extent_(), base_address_(), pos_() {}
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, const VkOffset3D& offset,
                        const VkExtent3D& extent, VkDeviceSize base_address, bool is_depth_sliced);
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, VkDeviceSize base_address,
                        bool is_depth_sliced);
    ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range, VkDeviceSize base_address,
                        bool is_depth_sliced, uint32_t view_mask);

    const IndexRange& operator*() const { return pos_; }
    const IndexRange* operator->() const { return &pos_; }
    ImageRangeGenerator& operator++();
    ImageRangeGenerator& operator=(const ImageRangeGenerator&) = default;
    // Return a strided representation of this generator's ranges, when supported
    std::optional<StridedRange> GetStridedRange() const;

  private:
    bool Convert2DCompatibleTo3D();
    void SetUpSubresInfo();
    void SetUpIncrementerDefaults();
    void SetUpSubresIncrementer();
    void SetUpIncrementer(bool all_width, bool all_height, bool all_depth);

    using SetInitialPosFn = void (ImageRangeGenerator::*)(uint32_t, uint32_t);
    void SetInitialPos(uint32_t layer, uint32_t aspect_index) { (this->*(set_initial_pos_fn_))(layer, aspect_index); }

    void SetInitialPosFullOffset(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullWidth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullHeight(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosSomeDepth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosFullDepth(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosAllLayers(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosOneAspect(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosAllSubres(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosSomeLayers(uint32_t layer, uint32_t aspect_index);
    void SetInitialPosMultiviewLayers(uint32_t layer, uint32_t aspect_index);

    VkOffset3D GetOffset(uint32_t aspect_index) const;
    VkExtent3D GetExtent(uint32_t aspect_index) const;

  private:
    const ImageRangeEncoder* encoder_;
    VkImageSubresourceRange subres_range_;
    VkOffset3D offset_;
    VkExtent3D extent_;
    VkDeviceSize base_address_;
    uint32_t view_mask_ = 0;

    uint32_t mip_index_ = 0U;
    uint32_t incr_mip_ = 0U;
    uint32_t aspect_index_ = 0U;
    uint32_t subres_index_ = 0U;
    const ImageRangeEncoder::SubresInfo* subres_info_ = nullptr;

    SetInitialPosFn set_initial_pos_fn_ = nullptr;

    IndexRange pos_;

    struct IncrementerState {
        // These vary per mip at least...
        uint32_t y_count = 0U;
        uint32_t layer_z_count = 0U;
        uint32_t y_index = 0U;
        uint32_t layer_z_index = 0U;
        IndexRange y_base = {0U, 0U};
        IndexRange layer_z_base = {0U, 0U};
        IndexType incr_y = 0U;
        IndexType incr_layer_z = 0U;

        uint32_t view_mask_ = 0;

        void Set(uint32_t y_count_, uint32_t layer_z_count_, IndexType base, IndexType span, IndexType y_step, IndexType z_step);

        // When multiview is disabled returns:
        //      layer_z_index + 1
        // When multiview is enabled returns:
        //      the next value after layer_z_index that corresponds to the next set bit in view mask.
        //      When all view bits are iterated or layer_z_count is reach then returns layer_z_count.
        uint32_t GetNextLayerZIndex() const;
    };
    IncrementerState incr_state_;
    bool single_full_size_range_ = true;
    bool is_depth_sliced_ = false;
};

}  // namespace syncval
