/* Copyright (c) 2019-2026 The Khronos Group Inc.
 * Copyright (c) 2019-2026 Valve Corporation
 * Copyright (c) 2019-2026 LunarG, Inc.
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
 */

#include "sync_image_encoding.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/image_state.h"
#include "utils/image_utils.h"
#include "utils/math_utils.h"
#include <vulkan/utility/vk_format_utils.h>
#include <cassert>

namespace syncval {

#ifndef NDEBUG
static bool IsValid(const vvl::SubresourceEncoder& encoder, const VkImageSubresourceRange& bounds) {
    const auto& limits = encoder.Limits();
    return (((bounds.aspectMask & limits.aspectMask) == bounds.aspectMask) &&
            (bounds.baseMipLevel + bounds.levelCount <= limits.mipLevel) &&
            (bounds.baseArrayLayer + bounds.layerCount <= limits.arrayLayer));
}
#endif

static uint32_t FormatTexelBlockSizeWithAspect(VkFormat format, VkImageAspectFlagBits aspect_flag) {
    if (aspect_flag == VK_IMAGE_ASPECT_STENCIL_BIT) {
        return vkuFormatStencilSize(format) / 8;
    }
    if (aspect_flag == VK_IMAGE_ASPECT_DEPTH_BIT) {
        return vkuFormatDepthSize(format) / 8;
    }
    if (vkuFormatIsMultiplane(format)) {
        const VkFormat compatible_format = vkuFindMultiplaneCompatibleFormat(format, aspect_flag);
        return vkuFormatTexelBlockSize(compatible_format);
    }
    return vkuFormatTexelBlockSize(format);
}

ImageRangeEncoder::ImageRangeEncoder(const vvl::Image& image)
    : ImageRangeEncoder(image, vvl::AspectParameters::Get(image.full_range.aspectMask)) {}

ImageRangeEncoder::ImageRangeEncoder(const vvl::Image& image, const vvl::AspectParameters* param)
    : SubresourceEncoder(image.full_range, param), total_size_(0U) {
    if (image.GetExtent().depth > 1) {
        limits_.arrayLayer = image.GetExtent().depth;
    }

    linear_image_ = false;
    if (image.GetTiling() == VK_IMAGE_TILING_LINEAR) {
        linear_image_ = true;

        // WORKAROUND for profile and mock_icd not containing valid VkSubresourceLayout yet. Treat it as optimal image.
        const VkImageAspectFlags first_aspect = AspectBit(0);  // AspectBit returns aspects by index
        VkImageSubresource mock_test_subres = {first_aspect, 0, 0};
        VkSubresourceLayout mock_test_layout = {};
        DispatchGetImageSubresourceLayout(image.store_device_as_workaround, image.VkHandle(), &mock_test_subres, &mock_test_layout);
        if (mock_test_layout.size == 0) {  // Happens only for mock icd
            linear_image_ = false;
        }
    }

    // WORKAROUND for not being able to handle general linear images without resulting in non-monotonically increasing ranges
    // Need to clean this up to correctly detect aliasing conflicts between linear image(s) and buffers
    // Issues:
    //     * Lower resolution MIP levels are sometimes hidden in unused space of image rows smaller than minimum stride
    //     * Mutliplane YUV formats may interleave UV rows
    //     * Ranges treat row size as synonymous with row stride, causing ranges to include interleaved content when
    //       checking for hazards or updating state.
    //
    // Needs a rework on how linear range generation is done to ensure correct sizing and monotonicity, before detection of
    // aliased resources can be done correctly.
    linear_image_ = false;

    const VkFormat image_format = image.GetFormat();
    texel_block_extent_ = vkuFormatTexelBlockExtent(image_format);

    is_3_d_ = image.GetImageType() == VK_IMAGE_TYPE_3D;
    y_interleave_ = false;

    VkSubresourceLayout layout = {};
    VkImageSubresourceLayers subres_layers = {limits_.aspectMask, 0, 0, limits_.arrayLayer};

    for (uint32_t aspect_index = 0; aspect_index < limits_.aspect_index; ++aspect_index) {
        const VkImageAspectFlagBits aspect_flag = AspectBit(aspect_index);
        const uint32_t texel_block_size = FormatTexelBlockSizeWithAspect(image_format, aspect_flag);

        texel_block_sizes_.push_back(texel_block_size);
        subres_layers.aspectMask = aspect_flag;

        IndexType aspect_size = 0;
        for (uint32_t mip_index = 0; mip_index < limits_.mipLevel; ++mip_index) {
            subres_layers.mipLevel = mip_index;
            const VkExtent3D subres_extent = image.GetEffectiveSubresourceExtent(subres_layers);

            if (linear_image_) {
                VkImageSubresource subres = {};
                subres.aspectMask = aspect_flag;
                subres.mipLevel = mip_index;
                DispatchGetImageSubresourceLayout(image.store_device_as_workaround, image.VkHandle(), &subres, &layout);
                if (is_3_d_) {
                    if ((layout.depthPitch == 0) && (subres_extent.depth == 1)) {
                        layout.depthPitch = layout.size;  // Certain implmentations don't supply pitches when size is 1
                    }
                    y_interleave_ = y_interleave_ || (layout.rowPitch > layout.depthPitch);
                } else {
                    if ((layout.arrayPitch == 0) && (limits_.arrayLayer == 1)) {
                        layout.arrayPitch = layout.size;  // Certain implmentations don't supply pitches when size is 1
                    }
                    y_interleave_ = y_interleave_ || (layout.rowPitch > layout.arrayPitch);
                }
            } else {
                const VkExtent3D layout_extent = RoundUpToFullTexelBlocks(subres_extent, texel_block_extent_);
                const uint32_t texel_block_x = layout_extent.width / texel_block_extent_.width;
                const uint32_t texel_block_y = layout_extent.height / texel_block_extent_.height;
                const uint32_t texel_block_z = layout_extent.depth / texel_block_extent_.depth;

                layout.offset += layout.size;
                layout.rowPitch = texel_block_x * texel_block_sizes_[aspect_index];
                layout.arrayPitch = layout.rowPitch * texel_block_y;
                layout.depthPitch = layout.arrayPitch;
                if (is_3_d_) {
                    layout.size = layout.depthPitch * texel_block_z;
                } else {
                    layout.size = layout.arrayPitch * limits_.arrayLayer;
                }
            }
            subres_info_.emplace_back(layout, subres_extent);
            aspect_size += layout.size;
            total_size_ += layout.size;
        }
        aspect_sizes_.emplace_back(aspect_size);
        aspect_extent_divisors_.emplace_back(vkuFindMultiplaneExtentDivisors(image_format, aspect_flag));
    }
}

ImageRangeEncoder::IndexType ImageRangeEncoder::Encode2D(const VkSubresourceLayout& layout, uint32_t layer, uint32_t aspect_index,
                                                         const VkOffset3D& offset) const {
    assert(offset.z == 0);

    const uint32_t texel_block_x = uint32_t(offset.x) / texel_block_extent_.width;
    const uint32_t texel_block_y = uint32_t(offset.y) / texel_block_extent_.height;

    return layout.offset + layer * layout.arrayPitch + texel_block_y * layout.rowPitch +
           texel_block_x * texel_block_sizes_[aspect_index];
}

ImageRangeEncoder::IndexType ImageRangeEncoder::Encode3D(const VkSubresourceLayout& layout, uint32_t aspect_index,
                                                         const VkOffset3D& offset) const {
    const uint32_t texel_block_x = uint32_t(offset.x) / texel_block_extent_.width;
    const uint32_t texel_block_y = uint32_t(offset.y) / texel_block_extent_.height;
    const uint32_t texel_block_z = uint32_t(offset.z) / texel_block_extent_.depth;

    return layout.offset + texel_block_z * layout.depthPitch + texel_block_y * layout.rowPitch +
           texel_block_x * texel_block_sizes_[aspect_index];
}

StridedRange::IndexRange StridedRange::NextSpan(IndexRange range, bool& inside) const {
    assert(count > 0);
    assert(width < stride);
    inside = false;
    if (range.empty()) {
        return {};
    }
    if (range.begin < base) {
        return {range.begin, std::min(range.end, base)};
    }
    const IndexType end = Bounds().end;
    if (range.begin >= end) {
        return range;
    }
    const IndexType row = (range.begin - base) / stride;
    const IndexType row_begin = base + row * stride;
    const IndexType row_end = row_begin + width;

    inside = range.begin < row_end;

    const IndexType span_end = inside ? row_end : row_begin + stride;
    return {range.begin, std::min(range.end, span_end)};
}

bool StridedRange::Intersects(IndexRange range) const {
    bool inside;
    const IndexRange first = NextSpan(range, inside);
    return first.non_empty() && (inside || first.end < range.end);
}

bool StridedRange::Covers(IndexRange range) const {
    bool inside;
    const IndexRange first = NextSpan(range, inside);
    return range.empty() || (inside && first.end == range.end);
}

inline VkImageSubresourceRange GetRemaining(const VkImageSubresourceRange& full_range, VkImageSubresourceRange subres_range) {
    subres_range.levelCount = GetEffectiveLevelCount(subres_range, full_range.levelCount);
    subres_range.layerCount = GetEffectiveLayerCount(subres_range, full_range.layerCount);
    return subres_range;
}
inline bool CoversAllLayers(const VkImageSubresourceRange& full_range, VkImageSubresourceRange subres_range) {
    return (subres_range.baseArrayLayer == 0) && (subres_range.layerCount == full_range.layerCount);
}
static bool SubresourceRangeIsEmpty(const VkImageSubresourceRange& range) {
    return (0 == range.aspectMask) || (0 == range.levelCount) || (0 == range.layerCount);
}
static bool ExtentIsEmpty(const VkExtent3D& extent) { return (0 == extent.width) || (0 == extent.height) || (0 == extent.depth); }

VkOffset3D ImageRangeGenerator::GetOffset(uint32_t aspect_index) const {
    // Return the effective offset taking into account the multiplane extent divisor
    auto offset = offset_;
    auto divisor = encoder_->GetAspectExtentDivisors(aspect_index);
    offset.x /= divisor.width;
    offset.y /= divisor.height;
    return offset;
}

VkExtent3D ImageRangeGenerator::GetExtent(uint32_t aspect_index) const {
    // Return the effective extent taking into account the multiplane extent divisor
    // Note that we also have to look at the offset, because it affects the rounding
    auto offset = GetOffset(aspect_index);
    auto extent = extent_;
    auto divisor = encoder_->GetAspectExtentDivisors(aspect_index);
    extent.width = ((offset_.x + extent.width + divisor.width - 1) / divisor.width) - offset.x;
    extent.height = ((offset_.y + extent.height + divisor.height - 1) / divisor.height) - offset.y;
    return extent;
}

static VkExtent3D GetAccessBlockCount(const VkOffset3D& offset, VkExtent3D extent, const VkExtent3D& block_extent) {
    // Account for the offset within the first block
    extent.width += static_cast<uint32_t>(offset.x) % block_extent.width;
    extent.height += static_cast<uint32_t>(offset.y) % block_extent.height;
    extent.depth += static_cast<uint32_t>(offset.z) % block_extent.depth;
    return GetTexelBlocks(extent, block_extent);
}

void ImageRangeGenerator::SetInitialPosFullOffset(uint32_t layer, uint32_t aspect_index) {
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const VkExtent3D access_blocks = GetAccessBlockCount(offset, extent, encoder_->TexelBlockExtent());
    const bool is_3D = encoder_->Is3D();
    const auto& subres_layout = subres_info_->layout;
    const IndexType encode_base = is_3D ? encoder_->Encode3D(subres_layout, aspect_index, offset)
                                        : encoder_->Encode2D(subres_layout, layer, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;

    const IndexType span = IndexType(access_blocks.width) * encoder_->TexelBlockSize(aspect_index);

    const uint32_t z_count = is_3D ? access_blocks.depth : subres_range_.layerCount;
    const IndexType z_pitch = is_3D ? subres_info_->z_step_pitch : subres_layout.arrayPitch;
    incr_state_.Set(access_blocks.height, z_count, base, span, subres_info_->y_step_pitch, z_pitch);
}

void ImageRangeGenerator::SetInitialPosFullWidth(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsInterleaveY() && (offset_.x == 0));
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const VkExtent3D access_blocks = GetAccessBlockCount(offset, extent, encoder_->TexelBlockExtent());
    const bool is_3D = encoder_->Is3D();
    const VkSubresourceLayout& subres_layout = subres_info_->layout;
    const IndexType encode_base = is_3D ? encoder_->Encode3D(subres_layout, aspect_index, offset)
                                        : encoder_->Encode2D(subres_layout, layer, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;
    const IndexType span = subres_layout.rowPitch * access_blocks.height;

    const uint32_t z_count = is_3D ? access_blocks.depth : subres_range_.layerCount;
    const IndexType z_pitch = is_3D ? subres_info_->z_step_pitch : subres_layout.arrayPitch;
    incr_state_.Set(1U, z_count, base, span, subres_info_->y_step_pitch, z_pitch);
}

void ImageRangeGenerator::SetInitialPosFullHeight(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset + subres_range_.baseArrayLayer * subres_layout.arrayPitch;
    const IndexType span = subres_info_->layer_span;
    const IndexType z_step = subres_layout.arrayPitch;

    incr_state_.Set(1, subres_range_.layerCount, base, span, span, z_step);
}

void ImageRangeGenerator::SetInitialPosSomeDepth(uint32_t layer, uint32_t aspect_index) {
    assert(encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (layer == 0));
    const auto offset = GetOffset(aspect_index);
    const auto extent = GetExtent(aspect_index);
    const VkExtent3D access_blocks = GetAccessBlockCount(offset, extent, encoder_->TexelBlockExtent());
    const auto& subres_layout = subres_info_->layout;
    const IndexType encode_base = encoder_->Encode3D(subres_layout, aspect_index, offset);
    const IndexType base = base_address_ + encode_base;
    const IndexType span = subres_layout.depthPitch * access_blocks.depth;

    incr_state_.Set(1, 1, base, span, span, subres_layout.size);
}

void ImageRangeGenerator::SetInitialPosFullDepth(uint32_t layer, uint32_t aspect_index) {
    assert(encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0) && (layer == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    const IndexType span = subres_layout.depthPitch * GetTexelBlocks(extent_, encoder_->TexelBlockExtent()).depth;

    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosSomeLayers(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset + layer * subres_layout.arrayPitch;
    const IndexType span = subres_layout.arrayPitch * subres_range_.layerCount;
    const IndexType z_step = subres_layout.arrayPitch * encoder_->Limits().arrayLayer;
    incr_state_.Set(1, 1, base, span, span, z_step);
}

void ImageRangeGenerator::SetInitialPosMultiviewLayers(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset + (subres_range_.baseArrayLayer + layer) * subres_layout.arrayPitch;
    const IndexType span = subres_layout.arrayPitch * 1;
    const IndexType z_step = subres_layout.arrayPitch * 1;
    incr_state_.Set(1, subres_range_.layerCount, base, span, span, z_step);
    incr_state_.layer_z_index = layer;
    incr_state_.view_mask_ = view_mask_;
}

void ImageRangeGenerator::SetInitialPosAllLayers(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->Is3D() && (offset_.x == 0) && (offset_.y == 0) && (offset_.z == 0) && (layer == 0));
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    const IndexType span = subres_layout.arrayPitch * subres_range_.layerCount;
    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosOneAspect(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsLinearImage());  // Requires the major minor of "idealized/non-linear" images
    const auto& subres_layout = subres_info_->layout;
    const IndexType base = base_address_ + subres_layout.offset;
    IndexType span = 0;
    if (subres_range_.levelCount == encoder_->Limits().mipLevel) {
        span = encoder_->GetAspectSize(aspect_index);
    } else {
        // Add up the mip sizes...
        // Assumes subres_info is pointing to index(baseMipLevel, aspect_index)
        // Assumes mip major order...
        for (uint32_t level = 0; level < subres_range_.levelCount; level++) {
            span += subres_info_[level].layout.size;
        }
    }
    incr_mip_ = subres_range_.levelCount;
    incr_state_.Set(1, 1, base, span, span, span);
}

void ImageRangeGenerator::SetInitialPosAllSubres(uint32_t layer, uint32_t aspect_index) {
    assert(!encoder_->IsLinearImage());
    const IndexType base = base_address_;
    const IndexType span = encoder_->TotalSize();

    // Just one range... everything, ++ will short circuit to "end"
    single_full_size_range_ = true;
    // We don't need to set up the rest of the incrementer, just the starting position
    incr_state_.y_base = {base, base + span};
}

bool ImageRangeGenerator::Convert2DCompatibleTo3D() {
    if (encoder_->Is3D() && is_depth_sliced_) {
        // This is only valid for 2D compatible 3D images.
        // Touch up the extent and the subres to make this look like a depth extent
        offset_.z = subres_range_.baseArrayLayer;
        subres_range_.baseArrayLayer = 0;
        extent_.depth = subres_range_.layerCount;
        subres_range_.layerCount = 1;
        return true;
    }
    return false;
}
ImageRangeGenerator::ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range,
                                         VkDeviceSize base_address, bool is_depth_sliced)
    : ImageRangeGenerator(encoder, subres_range, base_address, is_depth_sliced, 0) {}

ImageRangeGenerator::ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range,
                                         VkDeviceSize base_address, bool is_depth_sliced, uint32_t view_mask)
    : encoder_(&encoder),
      subres_range_(GetRemaining(encoder.FullRange(), subres_range)),
      offset_(),
      extent_(),
      base_address_(base_address),
      view_mask_(view_mask),
      is_depth_sliced_(is_depth_sliced) {
    assert(IsValid(*encoder_, subres_range_));
    if (SubresourceRangeIsEmpty(subres_range)) {
        // Not robust to empty ranges, so for to "at end" condition.
        pos_ = {0, 0};
        return;
    }
    SetUpSubresInfo();
    extent_ = subres_info_->extent;
    const bool converted = Convert2DCompatibleTo3D();
    SetUpIncrementerDefaults();
    if (converted && (extent_.depth != subres_info_->extent.depth)) {
        SetUpIncrementer(true, true, false);
    } else {
        SetUpSubresIncrementer();
    }

    // With multiview, baseArrayLayer is taken into account inside SetInitialPosMultiviewLayers.
    // Otherwise, pass baseArrayLayer directly.
    const uint32_t initial_layer = view_mask ? static_cast<uint32_t>(LeastSignificantBit(view_mask)) : subres_range_.baseArrayLayer;
    SetInitialPos(initial_layer, aspect_index_);
    pos_ = incr_state_.y_base;
}

ImageRangeGenerator::ImageRangeGenerator(const ImageRangeEncoder& encoder, const VkImageSubresourceRange& subres_range,
                                         const VkOffset3D& offset, const VkExtent3D& extent, VkDeviceSize base_address,
                                         bool is_depth_sliced)
    : encoder_(&encoder),
      subres_range_(GetRemaining(encoder.FullRange(), subres_range)),
      offset_(offset),
      extent_(extent),
      base_address_(base_address),
      is_depth_sliced_(is_depth_sliced) {
#ifndef NDEBUG
    assert(IsValid(*encoder_, subres_range_));
#endif

    assert(subres_range_.levelCount == 1);
    if (SubresourceRangeIsEmpty(subres_range)) {
        // Empty range forces empty position -- no operations other than deref for empty check are valid
        pos_ = {0, 0};
        return;
    }

    // When passing in an offset and extent, *must* only specify *one* mip level
    SetUpSubresInfo();
    Convert2DCompatibleTo3D();

    const VkExtent3D& subres_extent = subres_info_->extent;
    if (ExtentIsEmpty(extent_) || ((extent_.width + offset_.x) > subres_extent.width) ||
        ((extent_.height + offset_.y) > subres_extent.height) || ((extent_.depth + offset_.z) > subres_extent.depth)) {
        // Empty range forces empty position -- no operations other than deref for empty check are valid
        pos_ = {0, 0};
        return;
    }

    const bool all_width = (offset.x == 0) && (extent_.width == subres_extent.width);
    const bool all_height = (offset.y == 0) && (extent_.height == subres_extent.height);
    const bool all_depth = !encoder_->Is3D() || ((offset.z == 0) && (extent_.depth == subres_extent.depth));

    SetUpIncrementerDefaults();
    SetUpIncrementer(all_width, all_height, all_depth);
    SetInitialPos(subres_range_.baseArrayLayer, aspect_index_);
    pos_ = incr_state_.y_base;
}

void ImageRangeGenerator::SetUpSubresInfo() {
    mip_index_ = 0;
    aspect_index_ = encoder_->LowerBoundFromMask(subres_range_.aspectMask);
    subres_index_ = encoder_->GetSubresourceIndex(aspect_index_, subres_range_.baseMipLevel);
    subres_info_ = &encoder_->GetSubresourceInfo(subres_index_);
}

void ImageRangeGenerator::SetUpIncrementerDefaults() {
    // These are safe defaults that most SetInitialPos* will use.  Those that need to change them, do.
    incr_state_.y_step = 1;
    incr_state_.layer_z_step = 1;
    incr_mip_ = 1;
    single_full_size_range_ = false;
}

// Assumes full extent in width/height/depth(if present)
void ImageRangeGenerator::SetUpSubresIncrementer() {
    const auto& full_range = encoder_->FullRange();
    const bool linear_image = encoder_->IsLinearImage();
    const bool is_3d = encoder_->Is3D();
    const bool layers_interleave = linear_image && (subres_info_->layout.arrayPitch > subres_info_->layout.size);
    if (layers_interleave) {
        // The implementation can interleave arrays, aspects, and mips arbitrarily
        if (encoder_->Is3D()) {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullDepth;
        } else {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullHeight;
        }
    } else if (!is_3d && view_mask_ != 0) {
        // TODO: handle 3d case too
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosMultiviewLayers;
    } else if (is_3d || CoversAllLayers(full_range, subres_range_)) {
        if (!linear_image) {
            // Linear images are defined by the implementation and so we can't assume the ordering we use here
            const bool all_mips = (subres_range_.baseMipLevel == 0) && (subres_range_.levelCount == full_range.levelCount);
            const bool all_aspects = subres_range_.aspectMask == full_range.aspectMask;
            if (all_aspects && all_mips) {
                set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosAllSubres;
            } else {
                set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosOneAspect;
            }
        } else if (is_3d) {
            // 3D implies CoversAllLayers
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullDepth;
        } else {
            set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosAllLayers;
        }
    } else {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosSomeLayers;
    }
}

void ImageRangeGenerator::SetUpIncrementer(bool all_width, bool all_height, bool all_depth) {
    if (!all_width || encoder_->IsInterleaveY()) {
        // Dimensional majority is not guaranteed for Linear images except in X
        // For tiled images we can use "idealized" addresses
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullOffset;
    } else if (!all_height) {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosFullWidth;
    } else if (encoder_->Is3D() && !all_depth) {
        set_initial_pos_fn_ = &ImageRangeGenerator::SetInitialPosSomeDepth;
    } else {
        SetUpSubresIncrementer();
    }
}

std::optional<StridedRange> ImageRangeGenerator::GetStridedRange() const {
    if (set_initial_pos_fn_ != &ImageRangeGenerator::SetInitialPosFullOffset) {
        return {};
    }
    // Return a strided range only if it reproduces every range this generator emits:
    // a) the generator must be at the start of the strided sequence
    if (mip_index_ || incr_state_.y_index || incr_state_.layer_z_index) {
        return {};
    }
    // b) only one aspect and one layer or slice form a single strided sequence
    if (!IsSingleBitSet(subres_range_.aspectMask) || incr_state_.layer_z_count > incr_state_.layer_z_step || !incr_state_.y_step) {
        return {};
    }
    const IndexType width = pos_.size();
    const IndexType stride = incr_state_.incr_y;
    // c) segments must be non-empty and separated by gaps
    if (width == 0 || width >= stride) {
        return {};
    }

    const uint32_t count = (incr_state_.y_count + incr_state_.y_step - 1) / incr_state_.y_step;
    if (count < 4) {
        return {};  // use regular ranges if we have only a few rows
    }
    return StridedRange{pos_.begin, width, stride, count};
}

ImageRangeGenerator& ImageRangeGenerator::operator++() {
    // Short circuit
    if (single_full_size_range_) {
        // Advance directly to end
        pos_ = {0, 0};
        return *this;
    }

    incr_state_.y_index += incr_state_.y_step;
    if (incr_state_.y_index < incr_state_.y_count) {
        incr_state_.y_base += incr_state_.incr_y;
        pos_ = incr_state_.y_base;
    } else {
        uint32_t old_z_index = incr_state_.layer_z_index;
        incr_state_.layer_z_index = incr_state_.GetNextLayerZIndex();
        if (incr_state_.layer_z_index < incr_state_.layer_z_count) {
            uint32_t z_count = incr_state_.layer_z_index - old_z_index;
            incr_state_.layer_z_base += z_count * incr_state_.incr_layer_z;
            incr_state_.y_index = 0;
            incr_state_.y_base = incr_state_.layer_z_base;
            pos_ = incr_state_.y_base;
        } else {
            // For aspects and mips we need to move to a new subresource layer info
            mip_index_ += incr_mip_;
            if (mip_index_ < subres_range_.levelCount) {
                // NOTE: This means that ImageRangeGenerator is relying on the major/minor ordering of mip and aspect in the
                subres_index_ += incr_mip_;
                extent_ = subres_info_->extent;  // Overwrites input extent, but > 1 MIP isn't valid with input extent
            } else {
                const auto next_aspect_index = encoder_->LowerBoundFromMask(subres_range_.aspectMask, aspect_index_ + 1);
                if (next_aspect_index < encoder_->Limits().aspect_index) {
                    //       SubresourceLayout info in ImageRangeEncoder... it's a cheat, but it was a hotspot.
                    aspect_index_ = next_aspect_index;
                    mip_index_ = 0;
                    subres_index_ = encoder_->GetSubresourceIndex(aspect_index_, subres_range_.baseMipLevel);
                } else {
                    // At End
                    pos_ = {0, 0};
                    return *this;
                }
            }

            subres_info_ = &encoder_->GetSubresourceInfo(subres_index_);
            SetInitialPos(subres_range_.baseArrayLayer, aspect_index_);
            pos_ = incr_state_.y_base;
        }
    }

    return *this;
}

ImageRangeEncoder::SubresInfo::SubresInfo(const VkSubresourceLayout& layout, const VkExtent3D& extent)
    : layout(layout),
      extent(extent),
      y_step_pitch(layout.rowPitch),
      z_step_pitch(layout.depthPitch),
      layer_span(layout.arrayPitch) {}

void ImageRangeGenerator::IncrementerState::Set(uint32_t y_count_, uint32_t layer_z_count_, IndexType base, IndexType span,
                                                IndexType y_step, IndexType z_step) {
    y_count = y_count_;
    layer_z_count = layer_z_count_;
    y_index = 0;
    layer_z_index = 0;
    y_base.begin = base;
    y_base.end = base + span;
    layer_z_base = y_base;
    incr_y = y_step;
    incr_layer_z = z_step;
}

uint32_t ImageRangeGenerator::IncrementerState::GetNextLayerZIndex() const {
    if (view_mask_ == 0) {
        return layer_z_index + layer_z_step;
    } else {
        uint32_t next_z = layer_z_index + 1;
        while (next_z < layer_z_count) {
            if ((1 << next_z) & view_mask_) {
                break;
            }
            next_z++;
        }
        return next_z;
    }
}

}  // namespace syncval
