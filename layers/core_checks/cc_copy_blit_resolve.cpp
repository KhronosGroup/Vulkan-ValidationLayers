/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include "core_checks/cc_buffer_address.h"
#include "core_validation.h"
#include "core_checks/cc_state_tracker.h"
#include "cc_vuid_maps.h"
#include "error_message/error_location.h"
#include "error_message/error_strings.h"
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "utils/math_utils.h"
#include "utils/image_utils.h"

struct ImageRegionIntersection {
    VkImageSubresourceLayers subresource = {};
    VkOffset3D offset = {0, 0, 0};
    VkExtent3D extent = {1, 1, 1};
    bool has_intersection = false;
    std::string String() const noexcept {
        std::stringstream ss;
        ss << "\nsubresource : { aspectMask: " << string_VkImageAspectFlags(subresource.aspectMask)
           << ", mipLevel: " << subresource.mipLevel << ", baseArrayLayer: " << subresource.baseArrayLayer
           << ", layerCount: " << subresource.layerCount << " },\noffset : {" << string_VkOffset3D(offset) << "},\nextent : {"
           << string_VkExtent3D(extent) << "}\n";
        return ss.str();
    }
};

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static ImageRegionIntersection GetRegionIntersection(const RegionType &region0, const RegionType &region1, VkImageType type,
                                                     bool is_multiplane) {
    ImageRegionIntersection result = {};

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (region0.srcSubresource.aspectMask != region1.dstSubresource.aspectMask)) {
        return result;
    }
    auto intersection = GetRangeIntersection(region0.srcSubresource.baseArrayLayer, region0.srcSubresource.layerCount,
                                             region1.dstSubresource.baseArrayLayer, region1.dstSubresource.layerCount);
    if ((region0.srcSubresource.mipLevel == region1.dstSubresource.mipLevel) && intersection.non_empty()) {
        result.subresource.aspectMask = region0.srcSubresource.aspectMask;
        result.subresource.baseArrayLayer = static_cast<uint32_t>(intersection.begin);
        result.subresource.layerCount = static_cast<uint32_t>(intersection.distance());
        result.subresource.mipLevel = region0.srcSubresource.mipLevel;
        result.has_intersection = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.z, region0.extent.depth, region1.dstOffset.z, region1.extent.depth);
                if (intersection.non_empty()) {
                    result.offset.z = static_cast<int32_t>(intersection.begin);
                    result.extent.depth = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_intersection = false;
                    return result;
                }
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.y, region0.extent.height, region1.dstOffset.y, region1.extent.height);
                if (intersection.non_empty()) {
                    result.offset.y = static_cast<int32_t>(intersection.begin);
                    result.extent.height = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_intersection = false;
                    return result;
                }
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.x, region0.extent.width, region1.dstOffset.x, region1.extent.width);
                if (intersection.non_empty()) {
                    result.offset.x = static_cast<int32_t>(intersection.begin);
                    result.extent.width = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_intersection = false;
                    return result;
                }
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static bool RegionIntersects(const RegionType *region0, const RegionType *region1, VkImageType type, bool is_multiplane) {
    return GetRegionIntersection(region0, region1, type, is_multiplane).has_instersection;
}

template <typename RegionType>
static bool RegionIntersectsBlit(const RegionType *region0, const RegionType *region1, VkImageType type, bool is_multiplane) {
    bool result = false;

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (region0->srcSubresource.aspectMask != region1->dstSubresource.aspectMask)) {
        return result;
    }

    if ((region0->srcSubresource.mipLevel == region1->dstSubresource.mipLevel) &&
        (RangesIntersect(region0->srcSubresource.baseArrayLayer, region0->srcSubresource.layerCount,
                         region1->dstSubresource.baseArrayLayer, region1->dstSubresource.layerCount))) {
        result = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                result &= RangesIntersect(region0->srcOffsets[0].z, region0->srcOffsets[1].z - region0->srcOffsets[0].z,
                                          region1->dstOffsets[0].z, region1->dstOffsets[1].z - region1->dstOffsets[0].z);
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                result &= RangesIntersect(region0->srcOffsets[0].y, region0->srcOffsets[1].y - region0->srcOffsets[0].y,
                                          region1->dstOffsets[0].y, region1->dstOffsets[1].y - region1->dstOffsets[0].y);
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                result &= RangesIntersect(region0->srcOffsets[0].x, region0->srcOffsets[1].x - region0->srcOffsets[0].x,
                                          region1->dstOffsets[0].x, region1->dstOffsets[1].x - region1->dstOffsets[0].x);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

static inline bool IsExtentEqual(const VkExtent3D &extent, const VkExtent3D &other_extent) {
    return (extent.width == other_extent.width) && (extent.height == other_extent.height) && (extent.depth == other_extent.depth);
}

static inline bool IsExtentAllZeroes(const VkExtent3D &extent) {
    return ((extent.width == 0) && (extent.height == 0) && (extent.depth == 0));
}

static inline bool IsExtentAllOne(const VkExtent3D &extent) {
    return ((extent.width == 1) && (extent.height == 1) && (extent.depth == 1));
}

// Test elements of a VkExtent3D structure against alignment constraints contained in another VkExtent3D structure
static inline bool IsExtentAligned(const VkExtent3D &extent, const VkExtent3D &granularity) {
    bool valid = true;
    if ((SafeModulo(extent.depth, granularity.depth) != 0) || (SafeModulo(extent.width, granularity.width) != 0) ||
        (SafeModulo(extent.height, granularity.height) != 0)) {
        valid = false;
    }
    return valid;
}

VkExtent3D CoreChecks::GetImageTransferGranularity(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state) const {
    if (cb_state.command_pool) {
        return physical_device_state->queue_family_properties[cb_state.command_pool->queueFamilyIndex].minImageTransferGranularity;
    }
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    return {0, 0, 0};
}

// Check elements of a VkOffset3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::ValidateTransferGranularityOffset(const LogObjectList &objlist, const VkOffset3D &offset,
                                                   const VkExtent3D &granularity, const Location &offset_loc,
                                                   const char *vuid) const {
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset.x));
    offset_extent.height = static_cast<uint32_t>(abs(offset.y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset.z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (!IsExtentAllZeroes(offset_extent)) {
            skip |= LogError(vuid, objlist, offset_loc,
                             "(%s) must be (0, 0, 0) when the command buffer's queue family "
                             "minImageTransferGranularity is (0, 0, 0) as this queue doesn't allow for any offset.",
                             string_VkOffset3D(offset).c_str());
        }
    } else if (!IsExtentAligned(offset_extent, granularity)) {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        skip |= LogError(vuid, objlist, offset_loc,
                         "(%s) dimensions must be even integer multiples of this command "
                         "buffer's queue family minImageTransferGranularity (%s).",
                         string_VkOffset3D(offset).c_str(), string_VkExtent3D(granularity).c_str());
    }
    return skip;
}

// Check elements of a VkExtent3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::ValidateTransferGranularityExtent(const LogObjectList &objlist, const VkExtent3D &region_extent,
                                                   const VkOffset3D &region_offset, const VkExtent3D &granularity,
                                                   const VkExtent3D &subresource_extent, const vvl::Image &image_state,
                                                   const Location &extent_loc, const char *vuid) const {
    bool skip = false;

    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (!IsExtentEqual(region_extent, subresource_extent)) {
            skip |= LogError(vuid, objlist, extent_loc,
                             "(%s) must match the image subresource extent (%s) when the command buffer's queue family "
                             "minImageTransferGranularity is (0, 0, 0) as this queue only allows full image copies.",
                             string_VkExtent3D(region_extent).c_str(), string_VkExtent3D(subresource_extent).c_str());
        }
    } else {
        const bool is_compressed = vkuFormatIsCompressed(image_state.create_info.format);
        // "minImageTransferGranularity" is unit of compressed texel blocks for images having a block-compressed format, and a unit
        // of texels otherwise.
        //
        // Either the effective region extent must be a multiple of minImageTransferGranularity or extent+offset must be the entire
        // image subresource
        VkExtent3D effective_region_extent = region_extent;
        VkExtent3D block_extent = {1, 1, 1};
        if (is_compressed) {
            block_extent = vkuFormatTexelBlockExtent(image_state.create_info.format);
            effective_region_extent = GetTexelBlocks(region_extent, block_extent);
        }

        // We keep this in texels regardless of compressed or not
        const VkExtent3D offset_extent_sum = {static_cast<uint32_t>(abs(region_offset.x)) + region_extent.width,
                                              static_cast<uint32_t>(abs(region_offset.y)) + region_extent.height,
                                              static_cast<uint32_t>(abs(region_offset.z)) + region_extent.depth};

        // This is an annoying VU, so provide granular (pun intended) error messages
        bool x_ok = true;
        bool y_ok = true;
        bool z_ok = true;
        switch (image_state.create_info.imageType) {
            case VK_IMAGE_TYPE_3D:
                z_ok = (SafeModulo(effective_region_extent.depth, granularity.depth) == 0) ||
                       (subresource_extent.depth == offset_extent_sum.depth);
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                y_ok = (SafeModulo(effective_region_extent.height, granularity.height) == 0) ||
                       (subresource_extent.height == offset_extent_sum.height);
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                x_ok = (SafeModulo(effective_region_extent.width, granularity.width) == 0) ||
                       (subresource_extent.width == offset_extent_sum.width);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }

        if (!(x_ok && y_ok && z_ok)) {
            std::stringstream ss;
            ss << "(" << string_VkExtent3D(region_extent)
               << ") is invalid with this command buffer's queue family minImageTransferGranularity ("
               << string_VkExtent3D(granularity) << ") for copying to/from " << FormatHandle(image_state) << " ("
               << string_VkFormat(image_state.create_info.format) << ") because both:\n";
            if (is_compressed) {
                ss << "1) The image is in texel blocks of size (" << string_VkExtentDimensions(block_extent)
                   << ") so the texel block extent of (" << string_VkExtent3D(effective_region_extent)
                   << ") is not an even integer multiple of minImageTransferGranularity.\n";
            } else {
                ss << "1) The extent is not an even integer multiple of minImageTransferGranularity.\n";
            }
            ss << "2) The extent plus the offset (" << string_VkOffset3D(region_offset) << ") is ("
               << string_VkExtentDimensions(offset_extent_sum) << ") which is not the entire image subresource ("
               << string_VkExtent3D(subresource_extent) << ").";

            skip |= LogError(vuid, objlist, extent_loc, "%s", ss.str().c_str());
        }
    }
    return skip;
}

bool IsValidAspectMaskForFormat(VkImageAspectFlags aspect_mask, VkFormat format) {
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        if (!(vkuFormatIsColor(format) || vkuFormatIsMultiplane(format))) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        if (!vkuFormatHasDepth(format)) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        if (!vkuFormatHasStencil(format)) return false;
    }
    if (0 != (aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
        if (vkuFormatPlaneCount(format) == 1) return false;
    }
    return true;
}

static std::string DescribeValidAspectMaskForFormat(VkFormat format) {
    VkImageAspectFlags aspect_mask = 0;
    if (vkuFormatIsColor(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (vkuFormatHasDepth(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (vkuFormatHasStencil(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    const uint32_t plane_count = vkuFormatPlaneCount(format);
    if (plane_count > 1) {
        // Color bit is "techically" valid, other VUs will warn if/when not allowed to use
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
    }
    if (plane_count > 2) {
        aspect_mask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
    }

    std::stringstream ss;
    ss << "Valid VkImageAspectFlags are " << string_VkImageAspectFlags(aspect_mask);
    return ss.str();
}

// This is a wrapper around VkImageCopy and VkImageCopy2 to both reduce reduntant information gathered looping each region, but also
// to be a single source to format better error messages
struct ImageCopyRegion {
    VkExtent3D extent;

    const vvl::Image &src_state;
    VkOffset3D src_offset;
    VkImageSubresourceLayers src_subresource;
    VkExtent3D src_subresource_extent;    // represented by texels (not texel blocks)
    uint32_t normalized_src_layer_count;  // fills in VK_REMAINING_ARRAY_LAYERS

    const vvl::Image &dst_state;
    VkOffset3D dst_offset;
    VkImageSubresourceLayers dst_subresource;
    VkExtent3D dst_subresource_extent;    // represented by texels (not texel blocks)
    uint32_t normalized_dst_layer_count;  // fills in VK_REMAINING_ARRAY_LAYERS

    bool is_adjusted_extent = false;
    // extent that is adjusted if copying compressed<-->uncompressed
    VkExtent3D dst_adjusted_extent;
    // This "just works" for Single Planar YCbCr formats because it used to catch height/depth, but the YCbCr formats only extend in
    // the x-dimension (width)
    bool src_dst_both_compressed = false;

    void Init() {
        src_subresource_extent = src_state.GetEffectiveSubresourceExtent(src_subresource);
        normalized_src_layer_count = src_state.NormalizeLayerCount(src_subresource);
        const VkFormat src_format = src_state.create_info.format;
        const VkExtent3D src_block_extent = vkuFormatTexelBlockExtent(src_format);
        const bool src_is_in_blocks = !IsExtentAllOne(src_block_extent);

        dst_subresource_extent = dst_state.GetEffectiveSubresourceExtent(dst_subresource);
        normalized_dst_layer_count = dst_state.NormalizeLayerCount(dst_subresource);
        const VkFormat dst_format = dst_state.create_info.format;
        const VkExtent3D dst_block_extent = vkuFormatTexelBlockExtent(dst_format);
        const bool dst_is_in_blocks = !IsExtentAllOne(dst_block_extent);

        // For image copies between compressed/uncompressed formats, the extent is provided in source image texels
        // Destination image texel extents must be adjusted by block size for the dest validation checks
        // vkspec.html#formats-size-compatibility
        //
        // Note: Single Planar are also included, they are similar to compressed formats... fun
        if (src_is_in_blocks && !dst_is_in_blocks) {
            dst_adjusted_extent = GetTexelBlocks(extent, src_block_extent);
            is_adjusted_extent = true;
        } else if (!src_is_in_blocks && dst_is_in_blocks) {
            dst_adjusted_extent = {extent.width * dst_block_extent.width, extent.height * dst_block_extent.height,
                                   extent.depth * dst_block_extent.depth};
            // One final edge case, if the compressed image is in 1D, the height is only actually 1 texel
            if (dst_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
                dst_adjusted_extent.height = 1;
                dst_adjusted_extent.depth = 1;
            } else if (dst_state.create_info.imageType == VK_IMAGE_TYPE_2D) {
                dst_adjusted_extent.depth = 1;
            }
            is_adjusted_extent = true;
        } else if (src_is_in_blocks && dst_is_in_blocks) {
            src_dst_both_compressed = true;
            dst_adjusted_extent = extent;
        } else {
            // both uncompressed
            dst_adjusted_extent = extent;
        }
    }

    ImageCopyRegion(const vvl::Image &src_state, const vvl::Image &dst_state, const VkImageCopy &region)
        : src_state(src_state), dst_state(dst_state) {
        extent = region.extent;
        src_subresource = region.srcSubresource;
        src_offset = region.srcOffset;
        dst_offset = region.dstOffset;
        dst_subresource = region.dstSubresource;
        Init();
    }

    ImageCopyRegion(const vvl::Image &src_state, const vvl::Image &dst_state, const VkImageCopy2 &region)
        : src_state(src_state), dst_state(dst_state) {
        extent = region.extent;
        src_offset = region.srcOffset;
        src_subresource = region.srcSubresource;
        dst_offset = region.dstOffset;
        dst_subresource = region.dstSubresource;
        Init();
    }

    std::string DescribeSrcAndDstImage() const {
        std::stringstream ss;
        ss << "srcImage: " << src_state.DescribeSubresourceLayers(src_subresource)
           << "dstImage: " << dst_state.DescribeSubresourceLayers(dst_subresource);
        return ss.str();
    }

    std::string DescribeAdjustedExtent() const {
        std::stringstream ss;
        if (is_adjusted_extent) {
            ss << "The VkImageCopy::extent [" << string_VkExtent3D(extent) << "] is adjusted to ["
               << string_VkExtent3D(dst_adjusted_extent) << "] because it is going from ";
            if (vkuFormatIsCompressed(src_state.create_info.format)) {
                ss << "compressed to uncompressed";
            } else if (vkuFormatIsSinglePlane_422(src_state.create_info.format)) {
                ss << "single-planar YCbCr (2x1 compressed) to uncompressed";
            } else if (vkuFormatIsSinglePlane_422(dst_state.create_info.format)) {
                ss << "uncompressed to single-planar YCbCr (2x1 compressed)";
            } else {
                ss << "uncompressed to compressed";
            }
            ss << '\n';
        }
        return ss.str();
    }
};

template <typename T>
uint32_t GetRowLength(T data) {
    return data.bufferRowLength;
}
template <>
uint32_t GetRowLength<VkMemoryToImageCopy>(VkMemoryToImageCopy data) {
    return data.memoryRowLength;
}
template <>
uint32_t GetRowLength<VkImageToMemoryCopy>(VkImageToMemoryCopy data) {
    return data.memoryRowLength;
}
template <typename T>
uint32_t GetImageHeight(T data) {
    return data.bufferImageHeight;
}
template <>
uint32_t GetImageHeight<VkMemoryToImageCopy>(VkMemoryToImageCopy data) {
    return data.memoryImageHeight;
}
template <>
uint32_t GetImageHeight<VkImageToMemoryCopy>(VkImageToMemoryCopy data) {
    return data.memoryImageHeight;
}

// Common between non-image to/from image copies
// ex) BufferToImage, ImageToBuffer, MemoryToImage, and ImageToMemory
template <typename RegionType>
bool CoreChecks::ValidateHeterogeneousCopyData(const RegionType &region, const vvl::Image &image_state,
                                               const LogObjectList &objlist, const Location &region_loc) const {
    bool skip = false;
    const bool is_memory = IsValueIn(region_loc.function, {Func::vkCopyMemoryToImage, Func::vkCopyMemoryToImageEXT,
                                                           Func::vkCopyImageToMemory, Func::vkCopyImageToMemoryEXT});
    skip |= ValidateHeterogeneousCopyImageless(region, objlist, region_loc, is_memory);

    if (image_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
        if (region.imageOffset.y != 0 || region.imageExtent.height != 1) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image1D_07979), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::y),
                             "is %" PRId32 " and imageExtent.height is %" PRIu32
                             ". For VK_IMAGE_TYPE_1D images imageOffset.y must be 0 and imageExtent.height must be 1.",
                             region.imageOffset.y, region.imageExtent.height);
        }
    }

    if ((image_state.create_info.imageType == VK_IMAGE_TYPE_1D) || (image_state.create_info.imageType == VK_IMAGE_TYPE_2D)) {
        if (region.imageOffset.z != 0 || region.imageExtent.depth != 1) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image1D_07980), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z),
                             "is %" PRId32 " and imageExtent.depth is %" PRIu32
                             ". For %s images imageOffset.z must be 0 and imageExtent.depth must be 1.",
                             region.imageOffset.z, region.imageExtent.depth, string_VkImageType(image_state.create_info.imageType));
        }
    }

    const uint32_t normalized_layer_count = image_state.NormalizeLayerCount(region.imageSubresource);
    if (image_state.create_info.imageType == VK_IMAGE_TYPE_3D) {
        if (region.imageSubresource.baseArrayLayer != 0 || normalized_layer_count != 1) {
            skip |=
                LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image3D_07983), objlist,
                         region_loc.dot(Field::imageSubresource).dot(Field::baseArrayLayer),
                         "is %" PRIu32
                         " and layerCount is %s. For VK_IMAGE_TYPE_3D images baseArrayLayer must be 0 and layerCount must be 1.",
                         region.imageSubresource.baseArrayLayer,
                         string_LayerCount(image_state.create_info, region.imageSubresource).c_str());
        }
    }

    const VkExtent3D effective_image_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
    // check range of imageOffset and imageExtent
    {
        if (region.imageOffset.x < 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07971), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::x), "(%" PRId32 ") must be greater than zero.",
                             region.imageOffset.x);
        } else if ((uint64_t)region.imageOffset.x + (uint64_t)region.imageExtent.width > (uint64_t)effective_image_extent.width) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07971), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::x),
                             "(%" PRId32 ") + extent.width (%" PRIu32 ") exceeds imageSubresource width extent (%" PRIu32 ").\n%s",
                             region.imageOffset.x, region.imageExtent.width, effective_image_extent.width,
                             image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        } else if (region.imageOffset.y < 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07972), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::y), "(%" PRId32 ") must be greater than zero.",
                             region.imageOffset.y);
        } else if ((uint64_t)region.imageOffset.y + (uint64_t)region.imageExtent.height > (uint64_t)effective_image_extent.height) {
            skip |=
                LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07972), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::y),
                         "(%" PRId32 ") + extent.height (%" PRIu32 ") exceeds imageSubresource height extent (%" PRIu32 ").\n%s",
                         region.imageOffset.y, region.imageExtent.height, effective_image_extent.height,
                         image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        } else if (region.imageOffset.z < 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_09104), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z), "(%" PRId32 ") must be greater than zero.",
                             region.imageOffset.z);
        } else if ((uint64_t)region.imageOffset.z + (uint64_t)region.imageExtent.depth > (uint64_t)effective_image_extent.depth) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_09104), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z),
                             "(%" PRId32 ") + extent.depth (%" PRIu32 ") exceeds imageSubresource depth extent (%" PRIu32 ").\n%s",
                             region.imageOffset.z, region.imageExtent.depth, effective_image_extent.depth,
                             image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        }
    }

    const VkFormat image_format = image_state.create_info.format;

    // if uncompressed, extent is {1,1,1} and non of this will matter
    const VkExtent3D block_extent = vkuFormatTexelBlockExtent(image_format);
    if (!IsExtentAllOne(block_extent)) {
        if ((SafeModulo(region.imageExtent.width, block_extent.width) != 0) &&
            (region.imageExtent.width + region.imageOffset.x != effective_image_extent.width)) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentWidth_00207), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::width),
                             "(%" PRIu32
                             ") must be a multiple of the image format (%s) texel block width "
                             "(%" PRIu32 "), or when added to imageOffset.x (%" PRId32
                             ") must equal the image subresource width (%" PRIu32 ").\n%s",
                             region.imageExtent.width, string_VkFormat(image_format), block_extent.width, region.imageOffset.x,
                             effective_image_extent.width, image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        } else if ((SafeModulo(region.imageExtent.height, block_extent.height) != 0) &&
                   (region.imageExtent.height + region.imageOffset.y != effective_image_extent.height)) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentHeight_00208), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::height),
                             "(%" PRIu32
                             ") must be a multiple of the image format (%s) texel block height "
                             "(%" PRIu32 "), or when added to imageOffset.y (%" PRId32
                             ") must equal the image subresource height (%" PRIu32 ").\n%s",
                             region.imageExtent.height, string_VkFormat(image_format), block_extent.height, region.imageOffset.y,
                             effective_image_extent.height, image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        } else if ((SafeModulo(region.imageExtent.depth, block_extent.depth) != 0) &&
                   (region.imageExtent.depth + region.imageOffset.z != effective_image_extent.depth)) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentDepth_00209), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::depth),
                             "(%" PRIu32
                             ") must be a multiple of the image format (%s) texel block depth "
                             "(%" PRIu32 "), or when added to imageOffset.z (%" PRId32
                             ") must equal the image subresource depth (%" PRIu32 ").\n%s",
                             region.imageExtent.depth, string_VkFormat(image_format), block_extent.depth, region.imageOffset.z,
                             effective_image_extent.depth, image_state.DescribeSubresourceLayers(region.imageSubresource).c_str());
        }

        if (SafeModulo(region.imageOffset.x, block_extent.width) != 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentWidth_07274), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::x),
                             "(%" PRId32
                             ") must be a multiple of the image format (%s) texel block extent "
                             "width (%" PRIu32 ").",
                             region.imageOffset.x, string_VkFormat(image_format), block_extent.width);
        } else if (SafeModulo(region.imageOffset.y, block_extent.height) != 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentHeight_07275), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::y),
                             "(%" PRId32 ") must be a multiple of the image format (%s) texel block height (%" PRIu32 ").",
                             region.imageOffset.y, string_VkFormat(image_format), block_extent.height);
        } else if (SafeModulo(region.imageOffset.z, block_extent.depth) != 0) {
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentDepth_07276), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z),
                             "(%" PRId32 ") must be a multiple of the image format (%s) texel block depth (%" PRIu32 ").",
                             region.imageOffset.z, string_VkFormat(image_format), block_extent.depth);
        }

        const uint32_t row_length = GetRowLength(region);
        if (SafeModulo(row_length, block_extent.width) != 0) {
            Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
            skip |=
                LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferRowLength_09106), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") must be a multiple of the image format (%s) texel block extent width (%" PRIu32 ").",
                         row_length, string_VkFormat(image_format), block_extent.width);
        }

        const uint32_t image_height = GetImageHeight(region);
        if (SafeModulo(image_height, block_extent.height) != 0) {
            Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferImageHeight_09107), objlist,
                             region_loc.dot(field),
                             "(%" PRIu32 ") must be a multiple of the image format (%s) texel block extent height (%" PRIu32 ").",
                             image_height, string_VkFormat(image_format), block_extent.height);
        }

        // *RowLength divided by the texel block extent width and then multiplied by the texel block size of the image must be
        // less than or equal to 2^31-1
        const uint32_t texel_block_size = vkuFormatTexelBlockSize(image_format);
        double test_value = row_length / block_extent.width;
        test_value = test_value * texel_block_size;
        const auto two_to_31_minus_1 = static_cast<double>((1u << 31) - 1);
        if (test_value > two_to_31_minus_1) {
            Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
            skip |= LogError(
                GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferRowLength_09108), objlist, region_loc.dot(field),
                "(%" PRIu32 ") divided by the image format (%s) texel block width (%" PRIu32
                ") then multiplied by the "
                "texel block size of image (%" PRIu32 ") is (%" PRIu64 ") which is greater than 2^31 - 1",
                row_length, string_VkFormat(image_format), block_extent.width, texel_block_size, static_cast<uint64_t>(test_value));
        }
    }

    const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
    // image subresource aspect bit must match format
    if (!IsValidAspectMaskForFormat(region_aspect_mask, image_format)) {
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::AspectMask_09105), objlist,
                         region_loc.dot(Field::imageSubresource).dot(Field::aspectMask),
                         "(%s) is invalid for image format %s. (%s)", string_VkImageAspectFlags(region_aspect_mask).c_str(),
                         string_VkFormat(image_format), DescribeValidAspectMaskForFormat(image_format).c_str());
    }

    // Checks that apply only to multi-planar format images
    if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, region_aspect_mask)) {
        skip |=
            LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::MultiPlaneAspectMask_07981), objlist,
                     region_loc.dot(Field::imageSubresource).dot(Field::aspectMask), "(%s) is invalid for multi-planar format %s.",
                     string_VkImageAspectFlags(region_aspect_mask).c_str(), string_VkFormat(image_format));
    }

    return skip;
}

// This is just a way to put the "imageless" (really stateless) checks in a seperate area
// (We don't do this in the stateless validation object because pain of duplicating the logic to route the various functions here)
template <typename RegionType>
bool CoreChecks::ValidateHeterogeneousCopyImageless(const RegionType &region, const LogObjectList &objlist,
                                                    const Location &region_loc, bool is_memory) const {
    bool skip = false;

    // Make sure not a empty region
    if (region.imageExtent.width == 0) {
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentWidthZero_06659), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::width), "is zero (empty copies are not allowed).");
    } else if (region.imageExtent.height == 0) {
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentHeightZero_06660), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::height), "is zero (empty copies are not allowed).");
    } else if (region.imageExtent.depth == 0) {
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentDepthZero_06661), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::depth), "is zero (empty copies are not allowed).");
    }

    const uint32_t row_length = GetRowLength(region);
    if (row_length != 0 && row_length < region.imageExtent.width) {
        Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
        skip |=
            LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentRowLength_09101), objlist,
                     region_loc.dot(field), "(%" PRIu32 ") must be zero or greater than or equal imageExtent.width (%" PRIu32 ").",
                     row_length, region.imageExtent.width);
    }

    const uint32_t image_height = GetImageHeight(region);
    if (image_height != 0 && image_height < region.imageExtent.height) {
        Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
        skip |=
            LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentImageHeight_09102), objlist,
                     region_loc.dot(field), "(%" PRIu32 ") must be zero or greater than or equal imageExtent.height (%" PRIu32 ").",
                     image_height, region.imageExtent.height);
    }

    const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
    if (GetBitSetCount(region_aspect_mask) != 1) {
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::AspectMaskSingleBit_09103), objlist,
                         region_loc.dot(Field::imageSubresource).dot(Field::aspectMask), "is %s (only one bit allowed).",
                         string_VkImageAspectFlags(region_aspect_mask).c_str());
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferImageCopyData(const vvl::CommandBuffer &cb_state, const RegionType &region,
                                             const vvl::Image &image_state, const LogObjectList &objlist,
                                             const Location &region_loc) const {
    bool skip = false;

    skip |= ValidateHeterogeneousCopyData(region, image_state, objlist, region_loc);

    // bufferOffset must be a certain multiple depending if
    // - Depth Stencil format
    // - Multi-Planar format
    // - everything else
    const VkFormat image_format = image_state.create_info.format;
    if (vkuFormatIsDepthOrStencil(image_format)) {
        if (SafeModulo(region.bufferOffset, 4) != 0) {
            skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::BufferOffset_07978), objlist,
                             region_loc.dot(Field::bufferOffset),
                             "(%" PRIu64 ") must be a multiple 4 if using a depth/stencil format (%s).", region.bufferOffset,
                             string_VkFormat(image_format));
        }
    } else if (vkuFormatIsMultiplane(image_format)) {
        // MultiPlaneAspectMask_07981 will validate if aspect mask is bad
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (IsAnyPlaneAspect(region_aspect_mask)) {
            const VkFormat compatible_format =
                vkuFindMultiplaneCompatibleFormat(image_format, static_cast<VkImageAspectFlagBits>(region_aspect_mask));
            const uint32_t texel_block_size = vkuFormatTexelBlockSize(compatible_format);
            if (SafeModulo(region.bufferOffset, texel_block_size) != 0) {
                skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::MultiPlaneCompatible_07976), objlist,
                                 region_loc.dot(Field::bufferOffset),
                                 "(%" PRIu64 ") is not a multiple of texel block size (%" PRIu32
                                 ") for %s (which is the compatible format for plane %" PRIu32 " of %s).",
                                 region.bufferOffset, texel_block_size, string_VkFormat(compatible_format),
                                 vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(region_aspect_mask)),
                                 string_VkFormat(image_format));
            }
        }
    } else {
        const uint32_t texel_block_size = vkuFormatTexelBlockSize(image_format);
        if (SafeModulo(region.bufferOffset, texel_block_size) != 0) {
            skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::TexelBlockSize_07975), objlist,
                             region_loc.dot(Field::bufferOffset),
                             "(%" PRIu64 ") must be a multiple texel block size (%" PRIu32 ") for %s.", region.bufferOffset,
                             texel_block_size, string_VkFormat(image_format));
        }
    }

    if (SafeModulo(region.bufferOffset, 4) != 0) {
        const VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        if (!HasRequiredQueueFlags(cb_state, *physical_device_state, required_flags)) {
            const char *vuid = GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::BufferOffset_07737).c_str();
            const LogObjectList objlist(cb_state.Handle(), cb_state.command_pool->Handle());
            skip |=
                LogError(vuid, objlist, region_loc.dot(Field::bufferOffset), "(%" PRIu64 ") is not a multiple of 4, but is %s",
                         region.bufferOffset, DescribeRequiredQueueFlag(cb_state, *physical_device_state, required_flags).c_str());
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferBounds(VkCommandBuffer commandBuffer, const vvl::Buffer &src_buffer_state,
                                             const vvl::Buffer &dst_buffer_state, uint32_t regionCount, const RegionType *pRegions,
                                             const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;

    const auto *src_binding = src_buffer_state.Binding();
    const auto *dst_binding = dst_buffer_state.Binding();

    const bool are_buffers_sparse = src_buffer_state.sparse || dst_buffer_state.sparse;
    const bool validate_no_memory_overlaps = !are_buffers_sparse && (regionCount > 0) && src_binding && dst_binding &&
                                             (src_binding->memory_state == dst_binding->memory_state);

    using MemoryRange = vvl::BindableMemoryTracker::BufferRange;

    std::vector<MemoryRange> src_memory_ranges;
    std::vector<MemoryRange> dst_memory_ranges;
    if (validate_no_memory_overlaps) {
        src_memory_ranges.reserve(regionCount);
        dst_memory_ranges.reserve(regionCount);
    }

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType &region = pRegions[i];

        // src
        {
            if (region.srcOffset >= src_buffer_state.create_info.size) {
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcOffset-00113" : "VUID-vkCmdCopyBuffer-srcOffset-00113";
                const LogObjectList objlist(commandBuffer, src_buffer_state.Handle());
                skip |= LogError(vuid, objlist, region_loc.dot(Field::srcOffset),
                                 "(%" PRIuLEAST64 ") is greater than size of srcBuffer (%" PRIuLEAST64 ").", region.srcOffset,
                                 src_buffer_state.create_info.size);
            }

            if (region.size > (src_buffer_state.create_info.size - region.srcOffset)) {
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00115" : "VUID-vkCmdCopyBuffer-size-00115";
                const LogObjectList objlist(commandBuffer, src_buffer_state.Handle());
                skip |= LogError(vuid, objlist, region_loc.dot(Field::size),
                                 "(%" PRIuLEAST64 ") is greater than the source buffer size (%" PRIuLEAST64
                                 ") minus srcOffset (%" PRIuLEAST64 ").",
                                 region.size, src_buffer_state.create_info.size, region.srcOffset);
            }
        }

        // dst
        {
            if (region.dstOffset >= dst_buffer_state.create_info.size) {
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstOffset-00114" : "VUID-vkCmdCopyBuffer-dstOffset-00114";
                const LogObjectList objlist(commandBuffer, dst_buffer_state.Handle());
                skip |= LogError(vuid, objlist, region_loc.dot(Field::dstOffset),
                                 "(%" PRIuLEAST64 ") is greater than size of dstBuffer (%" PRIuLEAST64 ").", region.dstOffset,
                                 dst_buffer_state.create_info.size);
            }

            if (region.size > (dst_buffer_state.create_info.size - region.dstOffset)) {
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00116" : "VUID-vkCmdCopyBuffer-size-00116";
                const LogObjectList objlist(commandBuffer, dst_buffer_state.Handle());
                skip |= LogError(vuid, objlist, region_loc.dot(Field::size),
                                 "(%" PRIuLEAST64 ") is greater than the destination buffer size (%" PRIuLEAST64
                                 ") minus dstOffset (%" PRIuLEAST64 ").",
                                 region.size, dst_buffer_state.create_info.size, region.dstOffset);
            }
        }

        // The union of the source regions, and the union of the destination regions, must not overlap in memory
        if (validate_no_memory_overlaps) {
            // Sort copy ranges
            {
                MemoryRange src_buffer_memory_range(src_binding->memory_offset + region.srcOffset,
                                                    src_binding->memory_offset + region.srcOffset + region.size);
                auto insert_pos = std::lower_bound(src_memory_ranges.begin(), src_memory_ranges.end(), src_buffer_memory_range);
                src_memory_ranges.insert(insert_pos, src_buffer_memory_range);
            }

            {
                MemoryRange dst_buffer_memory_range(dst_binding->memory_offset + region.dstOffset,
                                                    dst_binding->memory_offset + region.dstOffset + region.size);
                auto insert_pos = std::lower_bound(dst_memory_ranges.begin(), dst_memory_ranges.end(), dst_buffer_memory_range);
                dst_memory_ranges.insert(insert_pos, dst_buffer_memory_range);
            }
        }
    }

    if (validate_no_memory_overlaps) {
        // Memory ranges are sorted, so looking for overlaps can be done in linear time
        auto src_ranges_it = src_memory_ranges.cbegin();
        auto dst_ranges_it = dst_memory_ranges.cbegin();

        while (src_ranges_it != src_memory_ranges.cend() && dst_ranges_it != dst_memory_ranges.cend()) {
            if (src_ranges_it->intersects(*dst_ranges_it)) {
                auto memory_range_overlap = *src_ranges_it & *dst_ranges_it;

                const LogObjectList objlist(commandBuffer, src_binding->memory_state->Handle(), src_buffer_state.Handle(),
                                            dst_buffer_state.Handle());
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";
                skip |= LogError(
                    vuid, objlist, loc,
                    "Copy source buffer range %s (from buffer %s) and destination buffer range %s (from buffer %s) are bound to "
                    "the same memory (%s), "
                    "and end up overlapping on memory range %s.",
                    vvl::string_range(*src_ranges_it).c_str(), FormatHandle(src_buffer_state.VkHandle()).c_str(),
                    vvl::string_range(*dst_ranges_it).c_str(), FormatHandle(dst_buffer_state.VkHandle()).c_str(),
                    FormatHandle(src_binding->memory_state->VkHandle()).c_str(), vvl::string_range(memory_range_overlap).c_str());
            }

            if (*src_ranges_it < *dst_ranges_it) {
                ++src_ranges_it;
            } else {
                ++dst_ranges_it;
            }
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                       const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_buffer_state = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    if (!cb_state_ptr || !src_buffer_state || !dst_buffer_state) {
        return skip;
    }
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid;

    skip |= ValidateCmd(cb_state, loc);
    skip |= ValidateCmdCopyBufferBounds(commandBuffer, *src_buffer_state, *dst_buffer_state, regionCount, pRegions, loc);

    // src buffer
    {
        const Location src_buffer_loc = loc.dot(Field::srcBuffer);
        vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00119" : "VUID-vkCmdCopyBuffer-srcBuffer-00119";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00118" : "VUID-vkCmdCopyBuffer-srcBuffer-00118";
        skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, srcBuffer), *src_buffer_state,
                                         VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT, true, vuid, src_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01822" : "VUID-vkCmdCopyBuffer-commandBuffer-01822";
        skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    }

    // dst buffer
    {
        const Location dst_buffer_loc = loc.dot(Field::dstBuffer);
        vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00121" : "VUID-vkCmdCopyBuffer-dstBuffer-00121";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00120" : "VUID-vkCmdCopyBuffer-dstBuffer-00120";
        skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, dstBuffer), *dst_buffer_state,
                                         VK_BUFFER_USAGE_2_TRANSFER_DST_BIT, true, vuid, dst_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01823" : "VUID-vkCmdCopyBuffer-commandBuffer-01823";
        skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01824" : "VUID-vkCmdCopyBuffer-commandBuffer-01824";
        skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy *pRegions,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                               const ErrorObject &error_obj) const {
    return ValidateCmdCopyBuffer(commandBuffer, pCopyBufferInfo->srcBuffer, pCopyBufferInfo->dstBuffer,
                                 pCopyBufferInfo->regionCount, pCopyBufferInfo->pRegions,
                                 error_obj.location.dot(Field::pCopyBufferInfo));
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkBufferImageCopy/VkBufferImageCopy2 structure
template <typename RegionType>
bool CoreChecks::ValidateCopyBufferImageTransferGranularityRequirements(const vvl::CommandBuffer &cb_state,
                                                                        const vvl::Image &image_state, const RegionType &region,
                                                                        const LogObjectList &objlist,
                                                                        const Location &region_loc) const {
    bool skip = false;
    std::string vuid = GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::TransferGranularity_07747);
    const VkExtent3D granularity = GetImageTransferGranularity(cb_state, image_state);
    skip |= ValidateTransferGranularityOffset(objlist, region.imageOffset, granularity, region_loc.dot(Field::imageOffset),
                                              vuid.c_str());
    VkExtent3D subresource_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
    skip |= ValidateTransferGranularityExtent(objlist, region.imageExtent, region.imageOffset, granularity, subresource_extent,
                                              image_state, region_loc.dot(Field::imageExtent), vuid.c_str());
    return skip;
}

template <typename HandleT>
bool CoreChecks::ValidateImageSubresourceLayers(HandleT handle, const vvl::Image &image_state,
                                                const VkImageSubresourceLayers &subresource_layers,
                                                const Location &subresource_loc) const {
    bool skip = false;
    if (subresource_layers.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        if (!enabled_features.maintenance5) {
            skip |= LogError("VUID-VkImageSubresourceLayers-layerCount-09243", handle, subresource_loc.dot(Field::layerCount),
                             "is VK_REMAINING_ARRAY_LAYERS.");
        }
    } else if (subresource_layers.layerCount == 0) {
        skip |=
            LogError("VUID-VkImageSubresourceLayers-layerCount-01700", handle, subresource_loc.dot(Field::layerCount), "is zero.");
    }

    const VkImageAspectFlags aspect_mask = subresource_layers.aspectMask;
    // aspectMask must not contain VK_IMAGE_ASPECT_METADATA_BIT
    if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-00168", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    // if aspectMask contains COLOR, it must not contain either DEPTH or STENCIL
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) && (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-00167", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT
    if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                       VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-02247", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }

    if (subresource_layers.mipLevel >= image_state.create_info.mipLevels) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(vvl::GetImageMipLevelVUID(subresource_loc), objlist, subresource_loc.dot(Field::mipLevel),
                         "is %" PRIu32 ", but provided %s has only %" PRIu32 " mip levels.%s", subresource_layers.mipLevel,
                         FormatHandle(image_state).c_str(), image_state.create_info.mipLevels,
                         subresource_layers.mipLevel == image_state.create_info.mipLevels ? " (mip level are 0 indexed)" : "");
    }

    const uint32_t base_layer = subresource_layers.baseArrayLayer;
    const uint32_t layer_count = subresource_layers.layerCount;
    if (base_layer >= image_state.create_info.arrayLayers || layer_count > image_state.create_info.arrayLayers ||
        (base_layer + layer_count) > image_state.create_info.arrayLayers) {
        if (layer_count != VK_REMAINING_ARRAY_LAYERS) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(vvl::GetImageArrayLayerRangeVUID(subresource_loc), objlist, subresource_loc.dot(Field::baseArrayLayer),
                             "is %" PRIu32 " and layerCount is %" PRIu32 ", but provided %s has %" PRIu32
                             " array layers (and %" PRIu32 " layers are being requested).",
                             base_layer, layer_count, FormatHandle(image_state).c_str(), image_state.create_info.arrayLayers,
                             base_layer + layer_count);
        }
    }

    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy/VkImageCopy2KHR structure
bool CoreChecks::ValidateCopyImageTransferGranularityRequirements(const vvl::CommandBuffer &cb_state, const ImageCopyRegion &region,
                                                                  const Location &region_loc) const {
    bool skip = false;
    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const char *vuid;

    {
        // Source image checks
        const LogObjectList objlist(cb_state.Handle(), region.src_state.Handle());
        const VkExtent3D granularity = GetImageTransferGranularity(cb_state, region.src_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= ValidateTransferGranularityOffset(objlist, region.src_offset, granularity, region_loc.dot(Field::srcOffset), vuid);
        skip |=
            ValidateTransferGranularityExtent(objlist, region.extent, region.src_offset, granularity, region.src_subresource_extent,
                                              region.src_state, region_loc.dot(Field::extent), vuid);
    }

    {
        // Destination image checks
        const LogObjectList objlist(cb_state.Handle(), region.dst_state.Handle());
        const VkExtent3D granularity = GetImageTransferGranularity(cb_state, region.dst_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= ValidateTransferGranularityOffset(objlist, region.dst_offset, granularity, region_loc.dot(Field::dstOffset), vuid);
        skip |=
            ValidateTransferGranularityExtent(objlist, region.extent, region.dst_offset, granularity, region.dst_subresource_extent,
                                              region.dst_state, region_loc.dot(Field::extent), vuid);
    }
    return skip;
}

// Validate contents of a VkImageCopy or VkImageCopy2 struct
template <typename HandleT>
bool CoreChecks::ValidateCopyImageRegionCommon(HandleT handle, const ImageCopyRegion &region, const Location &region_loc) const {
    bool skip = false;
    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const bool is_host = region_loc.function == Func::vkCopyImageToImage || region_loc.function == Func::vkCopyImageToImageEXT;
    const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
    const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);

    // Make sure not a empty region (src extent is always un-modified)
    if (region.extent.width == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06668" : "VUID-VkImageCopy-extent-06668";
        const LogObjectList src_objlist(handle, region.src_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::width),
                         "is zero. (empty copies are not allowed).");
    } else if (region.extent.height == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06669" : "VUID-VkImageCopy-extent-06669";
        const LogObjectList src_objlist(handle, region.src_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::height),
                         "is zero. (empty copies are not allowed).");
    } else if (region.extent.depth == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06670" : "VUID-VkImageCopy-extent-06670";
        const LogObjectList src_objlist(handle, region.src_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::depth),
                         "is zero. (empty copies are not allowed).");
    }

    const VkImageType src_image_type = region.src_state.create_info.imageType;
    const VkFormat src_format = region.src_state.create_info.format;
    const VkImageType dst_image_type = region.dst_state.create_info.imageType;
    const VkFormat dst_format = region.dst_state.create_info.format;

    // comparing src vs dst subresource
    {
        if (api_version < VK_API_VERSION_1_1) {
            if (!IsExtEnabled(extensions.vk_khr_maintenance1)) {
                // For each region the layerCount member of srcSubresource and dstSubresource must match
                if (region.normalized_src_layer_count != region.normalized_dst_layer_count) {
                    const LogObjectList objlist(handle, region.src_state.VkHandle(), region.dst_state.VkHandle());
                    const char *vuid =
                        (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07941" : "VUID-VkImageCopy-apiVersion-07941";
                    skip |=
                        LogError(vuid, objlist, src_subresource_loc.dot(Field::layerCount),
                                 "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.normalized_src_layer_count,
                                 dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.normalized_dst_layer_count);
                }
            }
            if (!IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion)) {
                // For each region the aspectMask member of srcSubresource and dstSubresource must match
                if (region.src_subresource.aspectMask != region.dst_subresource.aspectMask) {
                    const LogObjectList objlist(handle, region.src_state.VkHandle(), region.dst_state.VkHandle());
                    const char *vuid =
                        (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07940" : "VUID-VkImageCopy-apiVersion-07940";
                    skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                                     string_VkImageAspectFlags(region.src_subresource.aspectMask).c_str(),
                                     dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                                     string_VkImageAspectFlags(region.dst_subresource.aspectMask).c_str());
                }
            }
        }

        // Handle difference between Maintenance 1
        if (IsExtEnabled(extensions.vk_khr_maintenance1) || is_host) {
            if (src_image_type == VK_IMAGE_TYPE_3D) {
                const LogObjectList objlist(handle, region.src_state.Handle());
                if (region.src_subresource.baseArrayLayer != 0 || region.normalized_src_layer_count != 1) {
                    skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage3D_04443), objlist,
                                     region_loc.dot(Field::srcSubresource).dot(Field::baseArrayLayer),
                                     "is %" PRIu32
                                     " and srcSubresource.layerCount is %s. For VK_IMAGE_TYPE_3D images baseArrayLayer must be 0 "
                                     "and layerCount must be 1.",
                                     region.src_subresource.baseArrayLayer,
                                     string_LayerCount(region.src_state.create_info, region.src_subresource).c_str());
                }
            }
            if (dst_image_type == VK_IMAGE_TYPE_3D) {
                const LogObjectList objlist(handle, region.dst_state.Handle());
                if (region.dst_subresource.baseArrayLayer != 0 || region.normalized_dst_layer_count != 1) {
                    skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage3D_04444), objlist,
                                     region_loc.dot(Field::dstSubresource).dot(Field::baseArrayLayer),
                                     "is %" PRIu32
                                     " and dstSubresource.layerCount is %s. For VK_IMAGE_TYPE_3D images baseArrayLayer must be 0 "
                                     "and layerCount must be 1.",
                                     region.dst_subresource.baseArrayLayer,
                                     string_LayerCount(region.dst_state.create_info, region.dst_subresource).c_str());
                }
            }
        } else {  // Pre maint 1
            if (src_image_type == VK_IMAGE_TYPE_3D || dst_image_type == VK_IMAGE_TYPE_3D) {
                if (region.src_subresource.baseArrayLayer != 0 || region.normalized_src_layer_count != 1) {
                    const LogObjectList objlist(handle, region.src_state.Handle());
                    const char *vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                    skip |= LogError(vuid, objlist, region_loc.dot(Field::srcSubresource).dot(Field::baseArrayLayer),
                                     "is %" PRIu32
                                     " and srcSubresource.layerCount is %s. For copies with either src or dst of type "
                                     "VK_IMAGE_TYPE_3D, baseArrayLayer must be 0 and layerCount must be 1.",
                                     region.src_subresource.baseArrayLayer,
                                     string_LayerCount(region.src_state.create_info, region.src_subresource).c_str());
                }
                if (region.dst_subresource.baseArrayLayer != 0 || region.normalized_dst_layer_count != 1) {
                    const LogObjectList objlist(handle, region.dst_state.Handle());
                    const char *vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                    skip |= LogError(vuid, objlist, region_loc.dot(Field::dstSubresource).dot(Field::baseArrayLayer),
                                     "is %" PRIu32
                                     " and dstSubresource.layerCount is %s For copies with either src or dst of type "
                                     "VK_IMAGE_TYPE_3D, baseArrayLayer must be 0 and layerCount must be 1.",
                                     region.dst_subresource.baseArrayLayer,
                                     string_LayerCount(region.dst_state.create_info, region.dst_subresource).c_str());
                }
            }
        }
    }

    // src
    {
        const LogObjectList src_objlist(handle, region.src_state.VkHandle());

        skip |= ValidateImageSubresourceLayers(handle, region.src_state, region.src_subresource, src_subresource_loc);
        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!IsValidAspectMaskForFormat(region.src_subresource.aspectMask, src_format)) {
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcSubresource_00142), src_objlist,
                             src_subresource_loc.dot(Field::aspectMask), "(%s) is invalid for source image format %s. (%s)",
                             string_VkImageAspectFlags(region.src_subresource.aspectMask).c_str(), string_VkFormat(src_format),
                             DescribeValidAspectMaskForFormat(src_format).c_str());
        }

        {
            if (region.src_offset.x < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00144), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::x), "(%" PRId32 ") must be greater than zero.",
                                 region.src_offset.x);
            } else if ((uint64_t)region.src_offset.x + (uint64_t)region.extent.width >
                       (uint64_t)region.src_subresource_extent.width) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00144), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::x),
                                 "(%" PRId32 ") + extent.width (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a width of %" PRIu32 ".\n%s",
                                 region.src_offset.x, region.extent.width, region.src_subresource.mipLevel,
                                 region.src_subresource_extent.width, region.DescribeSrcAndDstImage().c_str());
            } else if (region.src_offset.y < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00145), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::y), "(%" PRId32 ") must be greater than zero.",
                                 region.src_offset.y);
            } else if ((uint64_t)region.src_offset.y + (uint64_t)region.extent.height >
                       (uint64_t)region.src_subresource_extent.height) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00145), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::y),
                                 "(%" PRId32 ") + extent.height (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a height of %" PRIu32 ".\n%s",
                                 region.src_offset.y, region.extent.height, region.src_subresource.mipLevel,
                                 region.src_subresource_extent.height, region.DescribeSrcAndDstImage().c_str());
            } else if (region.src_offset.z < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00147), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::z), "(%" PRId32 ") must be greater than zero.",
                                 region.src_offset.z);
            } else if ((uint64_t)region.src_offset.z + (uint64_t)region.extent.depth >
                       (uint64_t)region.src_subresource_extent.depth) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00147), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::z),
                                 "(%" PRId32 ") + extent.depth (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a depth of %" PRIu32 ".\n%s",
                                 region.src_offset.z, region.extent.depth, region.src_subresource.mipLevel,
                                 region.src_subresource_extent.depth, region.DescribeSrcAndDstImage().c_str());
            }
        }

        if (src_image_type == VK_IMAGE_TYPE_1D) {
            if (region.src_offset.y != 0 || region.extent.height != 1) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage1D_00146), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::y),
                                 "is %" PRId32 " and extent.height is %" PRIu32
                                 ". For VK_IMAGE_TYPE_1D images the srcOffset.y must be 0 and extent.height must be 1.\n%s",
                                 region.src_offset.y, region.extent.height, region.DescribeSrcAndDstImage().c_str());
            }
        }

        if (((src_image_type == VK_IMAGE_TYPE_1D) || ((src_image_type == VK_IMAGE_TYPE_2D) && is_host)) &&
            ((region.src_offset.z != 0) || (region.extent.depth != 1))) {
            const char *image_type = is_host ? "1D or 2D" : "1D";
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage1D_01785), src_objlist,
                             region_loc.dot(Field::srcOffset).dot(Field::z),
                             "is %" PRId32 " and extent.depth is %" PRIu32
                             ". For %s images the srcOffset.z must be 0 and extent.depth must be 1.\n%s",
                             region.src_offset.z, region.extent.depth, image_type, region.DescribeSrcAndDstImage().c_str());
        }

        if ((src_image_type == VK_IMAGE_TYPE_2D) && (region.src_offset.z != 0) && (!is_host)) {
            const char *vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01787" : "VUID-vkCmdCopyImage-srcImage-01787";
            skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::z),
                             "is %" PRId32 ", but for VK_IMAGE_TYPE_2D images this must be 0.\n%s", region.src_offset.z,
                             region.DescribeSrcAndDstImage().c_str());
        }

        // if uncompressed, extent is {1,1,1} and non of this will matter
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(src_format);
        if (!IsExtentAllOne(block_extent)) {
            if ((SafeModulo(region.extent.width, block_extent.width) != 0) &&
                (region.extent.width + region.src_offset.x != region.src_subresource_extent.width)) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01728), src_objlist,
                                 region_loc.dot(Field::extent).dot(Field::width),
                                 "(%" PRIu32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "width (%" PRIu32 "), or when added to srcOffset.x (%" PRId32
                                 ") must equal the image subresource width (%" PRIu32 ").\n%s",
                                 region.extent.width, string_VkFormat(src_format), block_extent.width, region.src_offset.x,
                                 region.src_subresource_extent.width, region.DescribeSrcAndDstImage().c_str());
            } else if ((SafeModulo(region.extent.height, block_extent.height) != 0) &&
                       (region.extent.height + region.src_offset.y != region.src_subresource_extent.height)) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01729), src_objlist,
                                 region_loc.dot(Field::extent).dot(Field::height),
                                 "(%" PRIu32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "height (%" PRIu32 "), or when added to srcOffset.y (%" PRId32
                                 ") must equal the image subresource height (%" PRIu32 ").\n%s",
                                 region.extent.height, string_VkFormat(src_format), block_extent.height, region.src_offset.y,
                                 region.src_subresource_extent.height, region.DescribeSrcAndDstImage().c_str());
            } else if ((SafeModulo(region.extent.depth, block_extent.depth) != 0) &&
                       (region.extent.depth + region.src_offset.z != region.src_subresource_extent.depth)) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01730), src_objlist,
                                 region_loc.dot(Field::extent).dot(Field::depth),
                                 "(%" PRIu32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "depth (%" PRIu32 "), or when added to srcOffset.z (%" PRId32
                                 ") must equal the image subresource depth (%" PRIu32 ").\n%s",
                                 region.extent.depth, string_VkFormat(src_format), block_extent.depth, region.src_offset.z,
                                 region.src_subresource_extent.depth, region.DescribeSrcAndDstImage().c_str());
            }

            if (SafeModulo(region.src_offset.x, block_extent.width) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07278), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::x),
                                 "(%" PRId32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "width (%" PRIu32 ").\n%s",
                                 region.src_offset.x, string_VkFormat(src_format), block_extent.width,
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (SafeModulo(region.src_offset.y, block_extent.height) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07279), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::y),
                                 "(%" PRId32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "height (%" PRIu32 ").\n%s",
                                 region.src_offset.y, string_VkFormat(src_format), block_extent.height,
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (SafeModulo(region.src_offset.z, block_extent.depth) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07280), src_objlist,
                                 region_loc.dot(Field::srcOffset).dot(Field::z),
                                 "(%" PRId32
                                 ") must be a multiple of the image format (%s) texel block "
                                 "depth (%" PRIu32 ").\n%s",
                                 region.src_offset.z, string_VkFormat(src_format), block_extent.depth,
                                 region.DescribeSrcAndDstImage().c_str());
            }
        }
    }

    // dst
    {
        const LogObjectList dst_objlist(handle, region.dst_state.Handle());

        skip |= ValidateImageSubresourceLayers(handle, region.dst_state, region.dst_subresource, dst_subresource_loc);
        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!IsValidAspectMaskForFormat(region.dst_subresource.aspectMask, dst_format)) {
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstSubresource_00143), dst_objlist,
                             dst_subresource_loc.dot(Field::aspectMask), "(%s) is invalid for destination image format %s. (%s)",
                             string_VkImageAspectFlags(region.dst_subresource.aspectMask).c_str(), string_VkFormat(dst_format),
                             DescribeValidAspectMaskForFormat(dst_format).c_str());
        }

        // if uncompressed, extent is {1,1,1} and non of this will matter
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(dst_format);
        if (!IsExtentAllOne(block_extent)) {
            if (SafeModulo(region.dst_offset.x, block_extent.width) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07281), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::x),
                                 "(%" PRId32 ") must be a multiple of the image format (%s) texel block width (%" PRIu32 ").\n%s",
                                 region.dst_offset.x, string_VkFormat(dst_format), block_extent.width,
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (SafeModulo(region.dst_offset.y, block_extent.height) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07282), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::y),
                                 "(%" PRId32 ") must be a multiple of the image format (%s) texel block height (%" PRIu32 ").\n%s",
                                 region.dst_offset.y, string_VkFormat(dst_format), block_extent.height,
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (SafeModulo(region.dst_offset.z, block_extent.depth) != 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07283), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::z),
                                 "(%" PRId32 ") must be a multiple of the image format (%s) texel block depth (%" PRIu32 ").\n%s",
                                 region.dst_offset.z, string_VkFormat(dst_format), block_extent.depth,
                                 region.DescribeSrcAndDstImage().c_str());
            }
        }

        {
            // If you have a compressed block that is not full, as a dst image, we want to round it up to "look" like a full texel
            // block
            const VkExtent3D dst_subresource = IsExtentAllOne(block_extent)
                                                   ? region.dst_subresource_extent
                                                   : RoundUpToFullTexelBlocks(region.dst_subresource_extent, block_extent);

            if (region.dst_offset.x < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00150), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::x), "(%" PRId32 ") must be greater than zero.",
                                 region.dst_offset.x);
            } else if ((uint64_t)region.dst_offset.x + (uint64_t)region.dst_adjusted_extent.width >
                       (uint64_t)dst_subresource.width) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00150), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::x),
                                 "(%" PRId32 ") + extent.width (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a width of %" PRIu32 ".\n%s%s",
                                 region.dst_offset.x, region.dst_adjusted_extent.width, region.dst_subresource.mipLevel,
                                 dst_subresource.width, region.DescribeAdjustedExtent().c_str(),
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (region.dst_offset.y < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00151), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::y), "(%" PRId32 ") must be greater than zero.",
                                 region.dst_offset.y);
            } else if ((uint64_t)region.dst_offset.y + (uint64_t)region.dst_adjusted_extent.height >
                       (uint64_t)dst_subresource.height) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00151), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::y),
                                 "(%" PRId32 ") + extent.height (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a height of %" PRIu32 ".\n%s%s",
                                 region.dst_offset.y, region.dst_adjusted_extent.height, region.dst_subresource.mipLevel,
                                 dst_subresource.height, region.DescribeAdjustedExtent().c_str(),
                                 region.DescribeSrcAndDstImage().c_str());
            } else if (region.dst_offset.z < 0) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00153), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::z), "(%" PRId32 ") must be greater than zero.",
                                 region.dst_offset.z);
            } else if ((uint64_t)region.dst_offset.z + (uint64_t)region.dst_adjusted_extent.depth >
                       (uint64_t)dst_subresource.depth) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00153), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::z),
                                 "(%" PRId32 ") + extent.depth (%" PRIu32 ") exceeds miplevel %" PRIu32
                                 " which has a depth of %" PRIu32 ".\n%s%s",
                                 region.dst_offset.z, region.dst_adjusted_extent.depth, region.dst_subresource.mipLevel,
                                 dst_subresource.depth, region.DescribeAdjustedExtent().c_str(),
                                 region.DescribeSrcAndDstImage().c_str());
            }
        }

        if (region.dst_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
            if (region.dst_offset.y != 0) {
                skip |=
                    LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_00152), dst_objlist,
                             region_loc.dot(Field::dstOffset).dot(Field::y),
                             "is %" PRId32 ", for VK_IMAGE_TYPE_1D images the dstOffset.y must be 0.\n%s%s", region.dst_offset.y,
                             region.DescribeAdjustedExtent().c_str(), region.DescribeSrcAndDstImage().c_str());
            } else if (region.dst_adjusted_extent.height != 1 && !region.src_dst_both_compressed) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_10908), dst_objlist,
                                 region_loc.dot(Field::extent).dot(Field::height),
                                 "is %" PRIu32 ", for VK_IMAGE_TYPE_1D images the extent.height must be 1.\n%s%s",
                                 region.dst_adjusted_extent.height, region.DescribeAdjustedExtent().c_str(),
                                 region.DescribeSrcAndDstImage().c_str());
            }
        }

        if (region.dst_state.create_info.imageType == VK_IMAGE_TYPE_1D ||
            (region.dst_state.create_info.imageType == VK_IMAGE_TYPE_2D && is_host)) {
            if (region.dst_offset.z != 0) {
                const char *image_type = is_host ? "1D or 2D" : "1D";
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_01786), dst_objlist,
                                 region_loc.dot(Field::dstOffset).dot(Field::z),
                                 "is %" PRId32 ", for %s images the dstOffset.z must be 0\n%s%s", region.dst_offset.z, image_type,
                                 region.DescribeAdjustedExtent().c_str(), region.DescribeSrcAndDstImage().c_str());

            } else if (region.dst_adjusted_extent.depth != 1 && !region.src_dst_both_compressed) {
                const char *image_type = is_host ? "1D or 2D" : "1D";
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_10907), dst_objlist,
                                 region_loc.dot(Field::extent).dot(Field::depth),
                                 "is %" PRIu32 ", for %s images the extent.depth must be 1\n%s%s", region.dst_adjusted_extent.depth,
                                 image_type, region.DescribeAdjustedExtent().c_str(), region.DescribeSrcAndDstImage().c_str());
            }
        }

        if ((region.dst_state.create_info.imageType == VK_IMAGE_TYPE_2D) && (region.dst_offset.z != 0) && !is_host) {
            const char *vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01788" : "VUID-vkCmdCopyImage-dstImage-01788";
            skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::z),
                             "is %" PRId32 ", but for VK_IMAGE_TYPE_2D images this must be 0.\n%s", region.dst_offset.z,
                             region.DescribeSrcAndDstImage().c_str());
        }
    }

    return skip;
}

template <typename HandleT>
bool CoreChecks::ValidateCopyImageCommon(HandleT handle, const vvl::Image &src_image_state, const vvl::Image &dst_image_state,
                                         const Location &loc) const {
    bool skip = false;

    // src
    {
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        const Location src_image_loc = loc.dot(Field::srcImage);
        skip |= ValidateMemoryIsBoundToImage(src_objlist, src_image_state, src_image_loc,
                                             GetCopyImageVUID(loc, vvl::CopyError::SrcImageContiguous_07966).c_str());
        if (src_image_state.create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            skip |= LogError(GetCopyImageVUID(loc, vvl::CopyError::SrcImageSubsampled_07969), src_objlist, src_image_loc,
                             "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }
    }

    // dst
    {
        const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
        const Location dst_image_loc = loc.dot(Field::dstImage);
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, dst_image_state, dst_image_loc,
                                             GetCopyImageVUID(loc, vvl::CopyError::DstImageContiguous_07966).c_str());
        if (dst_image_state.create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            skip |= LogError(GetCopyImageVUID(loc, vvl::CopyError::DstImageSubsampled_07969), dst_objlist, dst_image_loc,
                             "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    const VkFormat src_format = src_image_state->create_info.format;
    const VkFormat dst_format = dst_image_state->create_info.format;
    const VkImageType src_image_type = src_image_state->create_info.imageType;
    const VkImageType dst_image_type = dst_image_state->create_info.imageType;
    const bool src_is_2d = (VK_IMAGE_TYPE_2D == src_image_type);
    const bool src_is_3d = (VK_IMAGE_TYPE_3D == src_image_type);
    const bool dst_is_2d = (VK_IMAGE_TYPE_2D == dst_image_type);
    const bool dst_is_3d = (VK_IMAGE_TYPE_3D == dst_image_type);
    const bool is_2 = loc.function == Func::vkCmdCopyImage2 || loc.function == Func::vkCmdCopyImage2KHR;

    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);

    skip |= ValidateCopyImageCommon(commandBuffer, *src_image_state, *dst_image_state, loc);

    bool has_stencil_aspect = false;
    bool has_non_stencil_aspect = false;
    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        ImageCopyRegion region(*src_image_state, *dst_image_state, pRegions[i]);
        const VkImageSubresourceLayers &src_subresource = region.src_subresource;
        const VkImageSubresourceLayers &dst_subresource = region.dst_subresource;

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            const uint32_t src_layer_count = region.src_subresource.layerCount;
            const uint32_t dst_layer_count = region.dst_subresource.layerCount;
            if (src_layer_count == VK_REMAINING_ARRAY_LAYERS || dst_layer_count == VK_REMAINING_ARRAY_LAYERS) {
                if (src_layer_count != VK_REMAINING_ARRAY_LAYERS) {
                    if (src_layer_count != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08794" : "VUID-vkCmdCopyImage-srcImage-08794";
                        skip |= LogError(vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                                         "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32
                                         ") minus baseArrayLayer (%" PRIu32 ").%s",
                                         src_layer_count, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer,
                                         src_is_3d ? " (srcImage is 3D so it won't be possible to match array layers, but you can "
                                                     "set srcSubresource.layerCount also to VK_REMAINING_ARRAY_LAYERS)"
                                                   : "");
                    }
                } else if (dst_layer_count != VK_REMAINING_ARRAY_LAYERS) {
                    if (dst_layer_count != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08794" : "VUID-vkCmdCopyImage-srcImage-08794";
                        skip |= LogError(vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                                         "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32
                                         ") minus baseArrayLayer (%" PRIu32 ")%s.",
                                         dst_layer_count, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer,
                                         dst_is_3d ? " (dstImage is 3D so it won't be possible to match array layers, but you can "
                                                     "set srcSubresource.layerCount also to VK_REMAINING_ARRAY_LAYERS)"
                                                   : "");
                    }
                }
            } else if (src_image_type == dst_image_type && src_layer_count != dst_layer_count) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08793" : "VUID-vkCmdCopyImage-srcImage-08793";
                skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                                 "(%" PRIu32 ") is different from dstSubresource.layerCount (%" PRIu32 "). Both images are %s.",
                                 src_layer_count, dst_layer_count, string_VkImageType(src_image_type));
            }

            // Maintenance 1 requires both while prior only required one to be 2D
            if ((src_is_2d && dst_is_2d) && (region.extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01790" : "VUID-vkCmdCopyImage-srcImage-01790";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "both srcImage and dstImage are 2D and extent.depth is %" PRIu32 " and has to be 1",
                                 region.extent.depth);
            }

            if (src_image_type != dst_image_type) {
                const bool valid = (src_is_2d && dst_is_3d) || (src_is_3d && dst_is_2d) || enabled_features.maintenance5;
                if (!valid) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07743" : "VUID-vkCmdCopyImage-srcImage-07743";
                    skip |= LogError(vuid, all_objlist, region_loc,
                                     "srcImage type (%s) is not equal to dstImage type (%s). You can't copy between 1D and 2D/3D. "
                                     "(The maintenance5 feature allows you to copy between any image type).",
                                     string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
                }
            }

            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01995" : "VUID-vkCmdCopyImage-srcImage-01995";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, region.src_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    src_image_loc, vuid);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01996" : "VUID-vkCmdCopyImage-dstImage-01996";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, region.dst_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    dst_image_loc, vuid);

            // Check if 2D with 3D and depth not equal to 2D layerCount
            if (src_is_2d && dst_is_3d && (region.extent.depth != region.normalized_src_layer_count)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01791" : "VUID-vkCmdCopyImage-srcImage-01791";
                skip |=
                    LogError(vuid, all_objlist, region_loc,
                             "srcImage is 2D, dstImage is 3D, but the extent.depth (%" PRIu32
                             ") is not equal to the srcSubresource.layerCount (%s)",
                             region.extent.depth, string_LayerCount(region.src_state.create_info, region.src_subresource).c_str());
            } else if (src_is_3d && dst_is_2d && (region.extent.depth != region.normalized_dst_layer_count)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01792" : "VUID-vkCmdCopyImage-dstImage-01792";
                skip |=
                    LogError(vuid, all_objlist, region_loc,
                             "srcImage is 3D, dstImage is 2D, but the extent.depth (%" PRIu32
                             ") is not equal to the dstSubresource.layerCount (%s)",
                             region.extent.depth, string_LayerCount(region.dst_state.create_info, region.dst_subresource).c_str());
            }
        } else {  // !vk_khr_maintenance1
            if ((src_is_2d || dst_is_2d) && (region.extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-08969" : "VUID-vkCmdCopyImage-apiVersion-08969";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage is %s and dstImage is %s, but extent.depth is %" PRIu32 " (should be 1).",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type), region.extent.depth);
            }

            if (src_image_type != dst_image_type) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07933" : "VUID-vkCmdCopyImage-apiVersion-07933";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage (%s) must be equal to dstImage (%s) without VK_KHR_maintenance1 enabled",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
            }
        }

        const VkImageAspectFlags src_aspect = src_subresource.aspectMask;
        const VkImageAspectFlags dst_aspect = dst_subresource.aspectMask;
        const bool is_src_multiplane = vkuFormatIsMultiplane(src_format);
        const bool is_dst_multiplane = vkuFormatIsMultiplane(dst_format);
        if (!is_src_multiplane && !is_dst_multiplane) {
            // If neither image is multi-plane the aspectMask member of src and dst must match
            if (src_aspect != dst_aspect && !enabled_features.maintenance8) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01551" : "VUID-vkCmdCopyImage-srcImage-01551";
                skip |= LogError(
                    vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                    "(%s) does not match %s (%s). (This can be allowed in some cases if maintenance8 feature is enabled)",
                    string_VkImageAspectFlags(src_aspect).c_str(), dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                    string_VkImageAspectFlags(dst_aspect).c_str());
            }

            // If the aspect is wrong, it will be caught elsewhere, so use the default
            if (!AreFormatsSizeCompatible(dst_format, src_format)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-01548";
                skip |=
                    LogError(vuid, all_objlist, loc, "srcImage format (%s) is not size-compatible with dstImage format (%s). %s",
                             string_VkFormat(src_format), string_VkFormat(dst_format),
                             DescribeFormatsSizeCompatible(src_format, dst_format).c_str());
            }
        } else {
            // Here we might be copying between 2 multi-planar formats, or color to/from multi-planar

            // Source image multiplane checks
            if (is_src_multiplane && !IsOnlyOneValidPlaneAspect(src_format, src_aspect)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08713" : "VUID-vkCmdCopyImage-srcImage-08713";
                skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(src_aspect).c_str(),
                                 string_VkFormat(src_format));
            }
            // Single-plane to multi-plane
            if (!is_src_multiplane && is_dst_multiplane && VK_IMAGE_ASPECT_COLOR_BIT != src_aspect) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01557" : "VUID-vkCmdCopyImage-dstImage-01557";
                skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                                 string_VkImageAspectFlags(src_aspect).c_str(), string_VkFormat(src_format),
                                 string_VkFormat(dst_format));
            }

            // Dest image multiplane checks
            if (is_dst_multiplane && !IsOnlyOneValidPlaneAspect(dst_format, dst_aspect)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-08714" : "VUID-vkCmdCopyImage-dstImage-08714";
                skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(dst_aspect).c_str(),
                                 string_VkFormat(dst_format));
            }
            // Multi-plane to single-plane
            if (is_src_multiplane && !is_dst_multiplane && VK_IMAGE_ASPECT_COLOR_BIT != dst_aspect) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01556" : "VUID-vkCmdCopyImage-srcImage-01556";
                skip |= LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                                 string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                 string_VkFormat(dst_format));
            }

            const VkFormat src_plane_format =
                is_src_multiplane ? vkuFindMultiplaneCompatibleFormat(src_format, static_cast<VkImageAspectFlagBits>(src_aspect))
                                  : src_format;
            const VkFormat dst_plane_format =
                is_dst_multiplane ? vkuFindMultiplaneCompatibleFormat(dst_format, static_cast<VkImageAspectFlagBits>(dst_aspect))
                                  : dst_format;
            const uint32_t src_format_size = vkuFormatTexelBlockSize(src_plane_format);
            const uint32_t dst_format_size = vkuFormatTexelBlockSize(dst_plane_format);

            // If size is still zero, then format is invalid and will be caught in another VU
            if ((src_format_size != dst_format_size) && (src_format_size != 0) && (dst_format_size != 0)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-None-01549" : "VUID-vkCmdCopyImage-None-01549";
                std::stringstream ss;
                ss << "srcImage format " << string_VkFormat(src_plane_format);
                if (is_src_multiplane) {
                    ss << " (which is the compatible format for plane "
                       << vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(src_aspect)) << " of " << string_VkFormat(src_format)
                       << ")";
                }
                ss << " has texel block size of " << src_format_size << " which is different than the dstImage format "
                   << string_VkFormat(dst_plane_format);
                if (is_dst_multiplane) {
                    ss << " (which is the compatible format for plane "
                       << vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(dst_aspect)) << " of " << string_VkFormat(dst_format)
                       << ")";
                }
                ss << " which has texel block size of " << dst_format_size;
                skip |= LogError(vuid, all_objlist, region_loc, "%s", ss.str().c_str());
            }
        }

        if (enabled_features.maintenance8) {
            const VkImageAspectFlags both_depth_and_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (src_aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
                if ((dst_aspect & both_depth_and_stencil) == both_depth_and_stencil) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10214" : "VUID-vkCmdCopyImage-srcSubresource-10214";
                    skip |=
                        LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "is VK_IMAGE_ASPECT_COLOR_BIT but dstSubresource.aspectMask contains both Depth and Stencil (%s).",
                                 string_VkImageAspectFlags(dst_aspect).c_str());
                } else if (dst_aspect == VK_IMAGE_ASPECT_DEPTH_BIT || dst_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                    if (!vkuFormatIsDepthStencilWithColorSizeCompatible(src_format, dst_format, dst_aspect)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10211" : "VUID-vkCmdCopyImage-srcSubresource-10211";
                        skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                         "is VK_IMAGE_ASPECT_COLOR_BIT and dstSubresource.aspectMask is %s, but the src color "
                                         "format (%s) is not "
                                         "compatible with the dst depth/stencil format (%s).",
                                         string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                         string_VkFormat(dst_format));
                    }
                }
            }

            if (dst_aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
                if ((src_aspect & both_depth_and_stencil) == both_depth_and_stencil) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstSubresource-10215" : "VUID-vkCmdCopyImage-dstSubresource-10215";
                    skip |=
                        LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "is VK_IMAGE_ASPECT_COLOR_BIT but srcSubresource.aspectMask contains both Depth and Stencil (%s).",
                                 string_VkImageAspectFlags(src_aspect).c_str());
                } else if (src_aspect == VK_IMAGE_ASPECT_DEPTH_BIT || src_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                    if (!vkuFormatIsDepthStencilWithColorSizeCompatible(dst_format, src_format, src_aspect)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10212" : "VUID-vkCmdCopyImage-srcSubresource-10212";
                        skip |= LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                         "is VK_IMAGE_ASPECT_COLOR_BIT and srcSubresource.aspectMask is %s, but the src "
                                         "depth/stencil format (%s) is not "
                                         "compatible with the dst color format (%s).",
                                         string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                         string_VkFormat(dst_format));
                    }
                }
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        // Validation is only performed when source image is the same as destination image.
        // In the general case, the mapping between an image and its underlying memory is undefined,
        // so checking for memory overlaps is not possible.
        if (srcImage == dstImage) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (auto intersection = GetRegionIntersection(pRegions[i], pRegions[j], src_image_type, is_src_multiplane);
                    intersection.has_intersection) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-00124" : "VUID-vkCmdCopyImage-pRegions-00124";
                    skip |= LogError(vuid, all_objlist, loc,
                                     "pRegion[%" PRIu32 "] copy source overlaps with pRegions[%" PRIu32
                                     "] copy destination. Overlap info, with respect to image (%s):%s",
                                     i, j, FormatHandle(srcImage).c_str(), intersection.String().c_str());
                }
            }
        }

        // track aspect mask in loop through regions
        if ((src_aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            has_stencil_aspect = true;
        }
        if ((src_aspect & (~VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            has_non_stencil_aspect = true;
        }

        // When performing copy from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                       RangesIntersect(src_subresource.baseArrayLayer, src_subresource.layerCount,
                                                       dst_subresource.baseArrayLayer, dst_subresource.layerCount));
        if (same_subresource) {
            if (!IsValueIn(srcImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL}) ||
                !IsValueIn(dstImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL})) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-09460" : "VUID-vkCmdCopyImage-srcImage-09460";
                skip |= LogError(vuid, src_objlist, loc,
                                 "copying to same VkImage at miplevel = %" PRIu32
                                 "\n"
                                 "srcSubresource baseArrayLayer = %" PRIu32
                                 ", layerCount = %s\n"
                                 "dstSubresource baseArrayLayer = %" PRIu32
                                 ", layerCount = %s\n"
                                 "but srcImageLayout is %s and is dstImageLayout is %s",
                                 src_subresource.mipLevel, src_subresource.baseArrayLayer,
                                 string_LayerCount(src_image_state->create_info, src_subresource).c_str(),
                                 dst_subresource.baseArrayLayer,
                                 string_LayerCount(src_image_state->create_info, dst_subresource).c_str(),
                                 string_VkImageLayout(srcImageLayout), string_VkImageLayout(dstImageLayout));
            }
        }

        auto validate_copy_color_to_or_from_depth =
            [&, this](const VkImageSubresourceLayers &subresource_1, const Location &subresource_1_loc,
                      const VkImageSubresourceLayers &subresource_2, const Location &subresource_2_loc, const vvl::Image &depth_img,
                      Field depth_img_field) {
                const bool is_subresource_1_aspect_color = subresource_1.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT;
                const bool is_depth_copy = subresource_2.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT;
                const bool is_stencil_copy = subresource_2.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT;

                if (is_subresource_1_aspect_color && (is_depth_copy || is_stencil_copy)) {
                    const VkFormatFeatureFlags2 img_format_features = GetPotentialFormatFeatures(depth_img.create_info.format);

                    const bool invalid_depth_copy_on_compute =
                        is_depth_copy && !(img_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR);
                    const bool invalid_stencil_copy_on_compute =
                        is_stencil_copy && !(img_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR);

                    const bool invalid_depth_copy_on_transfer =
                        is_depth_copy && !(img_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR);
                    const bool invalid_stencil_copy_on_transfer =
                        is_stencil_copy && !(img_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANSFER_QUEUE_BIT_KHR);

                    if (!HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_GRAPHICS_BIT)) {
                        if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_COMPUTE_BIT)) {
                            if (invalid_depth_copy_on_compute || invalid_stencil_copy_on_compute) {
                                if (is_2) {
                                    if (subresource_1_loc.field == Field::srcSubresource) {
                                        vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImage2-commandBuffer-11782"
                                                                             : "VUID-vkCmdCopyImage2-commandBuffer-11784";
                                    } else {
                                        vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImage2-commandBuffer-11786"
                                                                             : "VUID-vkCmdCopyImage2-commandBuffer-11788";
                                    }
                                } else {
                                    if (subresource_1_loc.field == Field::srcSubresource) {
                                        vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImage-commandBuffer-11782"
                                                                             : "VUID-vkCmdCopyImage-commandBuffer-11784";
                                    } else {
                                        vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImage-commandBuffer-11786"
                                                                             : "VUID-vkCmdCopyImage-commandBuffer-11788";
                                    }
                                }

                                const LogObjectList objlist(cb_state.Handle(), depth_img.Handle());
                                skip |= LogError(
                                    vuid, objlist, subresource_1_loc.dot(Field::aspectMask),
                                    "is %s and %s is %s, commandBuffer was created with a VkCommandPool that does not "
                                    "support VK_QUEUE_GRAPHICS_BIT but "
                                    "supports VK_QUEUE_COMPUTE_BIT, yet %s (%s) does not have the %s feature\n(features: %s).",
                                    string_VkImageAspectFlags(subresource_1.aspectMask).c_str(),
                                    subresource_2_loc.dot(Field::aspectMask).Fields().c_str(),
                                    string_VkImageAspectFlags(subresource_2.aspectMask).c_str(), String(depth_img_field),
                                    string_VkFormat(depth_img.create_info.format),
                                    invalid_depth_copy_on_compute ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR"
                                                                  : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR",
                                    string_VkFormatFeatureFlags2(img_format_features).c_str());
                            }

                        } else if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_TRANSFER_BIT)) {
                            if (invalid_depth_copy_on_transfer || invalid_stencil_copy_on_transfer) {
                                if (is_2) {
                                    if (subresource_1_loc.field == Field::srcSubresource) {
                                        vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImage2-commandBuffer-11783"
                                                                              : "VUID-vkCmdCopyImage2-commandBuffer-11785";
                                    } else {
                                        vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImage2-commandBuffer-11787"
                                                                              : "VUID-vkCmdCopyImage2-commandBuffer-11789";
                                    }
                                } else {
                                    if (subresource_1_loc.field == Field::srcSubresource) {
                                        vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImage-commandBuffer-11783"
                                                                              : "VUID-vkCmdCopyImage-commandBuffer-11785";
                                    } else {
                                        vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImage-commandBuffer-11787"
                                                                              : "VUID-vkCmdCopyImage-commandBuffer-11789";
                                    }
                                }

                                const LogObjectList objlist(cb_state.Handle(), depth_img.Handle());
                                skip |= LogError(vuid, objlist, subresource_1_loc.dot(Field::aspectMask),
                                                 "is %s and %s is %s, commandBuffer was created with a VkCommandPool that does "
                                                 "supports neither "
                                                 "VK_QUEUE_GRAPHICS_BIT nor VK_QUEUE_COMPUTE_BIT but supports "
                                                 "VK_QUEUE_TRANSFER_BIT, yet %s (%s) does not have the %s feature\n(features: %s).",
                                                 string_VkImageAspectFlags(subresource_1.aspectMask).c_str(),
                                                 subresource_2_loc.dot(Field::aspectMask).Fields().c_str(),
                                                 string_VkImageAspectFlags(subresource_2.aspectMask).c_str(),
                                                 String(depth_img_field), string_VkFormat(depth_img.create_info.format),
                                                 invalid_depth_copy_on_transfer
                                                     ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANFER_QUEUE_BIT_KHR"
                                                     : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANFER_QUEUE_BIT_KHR",
                                                 string_VkFormatFeatureFlags2(img_format_features).c_str());
                            }
                        }
                    }
                }
            };

        validate_copy_color_to_or_from_depth(src_subresource, region_loc.dot(Field::srcSubresource), dst_subresource,
                                             region_loc.dot(Field::dstSubresource), *dst_image_state, Field::dstImage);

        validate_copy_color_to_or_from_depth(dst_subresource, region_loc.dot(Field::dstSubresource), src_subresource,
                                             region_loc.dot(Field::srcSubresource), *src_image_state, Field::srcImage);

        // src
        {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-00128" : "VUID-vkCmdCopyImage-srcImageLayout-00128";
            skip |= ValidateSubresourceImageLayout(cb_state, region.src_state, src_subresource, region.src_offset.z,
                                                   region.extent.depth, srcImageLayout, src_image_loc, vuid);

            if (src_aspect == VK_IMAGE_ASPECT_COLOR_BIT && !enabled_features.maintenance10) {
                vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-10217" : "VUID-vkCmdCopyImage-commandBuffer-10217";
                skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, dst_aspect, region.src_state,
                                                   region_loc.dot(Field::srcSubresource).dot(Field::aspectMask), vuid);
            }
        }

        // dst
        {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-00133" : "VUID-vkCmdCopyImage-dstImageLayout-00133";
            skip |= ValidateSubresourceImageLayout(cb_state, region.dst_state, dst_subresource, region.dst_offset.z,
                                                   region.extent.depth, dstImageLayout, dst_image_loc, vuid);

            if (dst_aspect == VK_IMAGE_ASPECT_COLOR_BIT && !enabled_features.maintenance10) {
                vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-10218" : "VUID-vkCmdCopyImage-commandBuffer-10218";
                skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, src_aspect, region.dst_state,
                                                   region_loc.dot(Field::dstSubresource).dot(Field::aspectMask), vuid);
            }
        }

        skip |= ValidateCopyImageTransferGranularityRequirements(cb_state, region, region_loc);
        skip |= ValidateCopyImageRegionCommon(commandBuffer, region, region_loc);
    }

    if (vkuFormatIsCompressed(src_format) && vkuFormatIsCompressed(dst_format)) {
        const VkExtent3D src_block_extent = vkuFormatTexelBlockExtent(src_format);
        const VkExtent3D dst_block_extent = vkuFormatTexelBlockExtent(dst_format);
        if (src_block_extent.width != dst_block_extent.width || src_block_extent.height != dst_block_extent.height ||
            src_block_extent.depth != dst_block_extent.depth) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-09247" : "VUID-vkCmdCopyImage-srcImage-09247";
            skip |= LogError(vuid, all_objlist, loc,
                             "srcImage format %s has texel block extent (%s) and dstImage format %s has texel block extent (%s).",
                             string_VkFormat(src_format), string_VkExtent3D(src_block_extent).c_str(), string_VkFormat(dst_format),
                             string_VkExtent3D(dst_block_extent).c_str());
        }
    }

    // Validate that SRC & DST images have correct usage flags set
    if (!IsExtEnabled(extensions.vk_ext_separate_stencil_usage)) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid, src_image_loc);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid, dst_image_loc);
    } else {
        auto src_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(src_image_state->create_info.pNext);
        if (src_separate_stencil && has_stencil_aspect &&
            ((src_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06664" : "VUID-vkCmdCopyImage-aspect-06664";
            skip =
                LogError(vuid, src_objlist, src_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_SRC_BIT.",
                         FormatHandle(src_image_state->Handle()).c_str(),
                         string_VkImageUsageFlags(src_separate_stencil->stencilUsage).c_str());
        }
        if (!src_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
            skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid,
                                            src_image_loc);
        }

        auto dst_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(dst_image_state->create_info.pNext);
        if (dst_separate_stencil && has_stencil_aspect &&
            ((dst_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06665" : "VUID-vkCmdCopyImage-aspect-06665";
            skip =
                LogError(vuid, dst_objlist, dst_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
                         FormatHandle(dst_image_state->Handle()).c_str(),
                         string_VkImageUsageFlags(dst_separate_stencil->stencilUsage).c_str());
        }
        if (!dst_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-vkCmdCopyImage-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
            skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid,
                                            dst_image_loc);
        }
    }

    // Source and dest image sample counts must match
    if (src_image_state->create_info.samples != dst_image_state->create_info.samples) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00136" : "VUID-vkCmdCopyImage-srcImage-00136";
        skip |= LogError(vuid, all_objlist, src_image_loc, "was created with (%s) but the dstImage was created with (%s).",
                         string_VkSampleCountFlagBits(src_image_state->create_info.samples),
                         string_VkSampleCountFlagBits(dst_image_state->create_info.samples));
    }

    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01825" : "VUID-vkCmdCopyImage-commandBuffer-01825";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01826" : "VUID-vkCmdCopyImage-commandBuffer-01826";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01827" : "VUID-vkCmdCopyImage-commandBuffer-01827";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

    skip |= ValidateCmd(cb_state, loc);

    if (!IsValueIn(srcImageLayout,
                   {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-01917" : "VUID-vkCmdCopyImage-srcImageLayout-01917";
        skip |= LogError(vuid, src_objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
    }

    if (!IsValueIn(dstImageLayout,
                   {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-01395" : "VUID-vkCmdCopyImage-dstImageLayout-01395";
        skip |= LogError(vuid, dst_objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdCopyImage(commandBuffer, pCopyImageInfo->srcImage, pCopyImageInfo->srcImageLayout, pCopyImageInfo->dstImage,
                                pCopyImageInfo->dstImageLayout, pCopyImageInfo->regionCount, pCopyImageInfo->pRegions,
                                error_obj.location.dot(Field::pCopyImageInfo));
}

template <typename RegionType>
bool CoreChecks::ValidateBufferBounds(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                      const vvl::Buffer &buffer_state, const RegionType &region, const Location &region_loc) const {
    bool skip = false;

    const uint32_t normalized_layer_count = image_state.NormalizeLayerCount(region.imageSubresource);
    const uint32_t z_copies = std::max(region.imageExtent.depth, normalized_layer_count);
    // Invalid if copy size is 0 and other validation checks will catch it. Returns zero as the caller should have fallback already
    // to ignore.
    if (region.imageExtent.width == 0 || region.imageExtent.height == 0 || region.imageExtent.depth == 0 || z_copies == 0) {
        return skip;
    }

    VkDeviceSize block_size = 0;
    const VkFormat format = image_state.create_info.format;
    bool is_multiplane = vkuFormatIsMultiplane(format);
    const VkFormat compatible_format =
        is_multiplane
            ? vkuFindMultiplaneCompatibleFormat(format, static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask))
            : format;

    if (region.imageSubresource.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
        // Spec in VkBufferImageCopy section list special cases for each format
        if (region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            block_size = 1;
        } else {
            // VK_IMAGE_ASPECT_DEPTH_BIT
            switch (format) {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                    block_size = 2;
                    break;
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // packed with the D24 value in the LSBs of the word, and undefined values in the eight MSBs
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                    block_size = 4;
                    break;
                default:
                    // Any misuse of formats vs aspect mask should be caught before here
                    return false;
            }
        }
    } else {
        block_size = vkuFormatTexelBlockSize(compatible_format);
    }

    const VkExtent3D block_extent = vkuFormatTexelBlockExtent(format);
    // From spec:
    // bytes in the buffer are accessed at offsets in the range [texelOffset, texelOffset + blockSize), where:
    // texelOffset = bufferOffset +
    //              (⌊x / blockWidth⌋ × blockSize) +
    //              (⌊y / blockHeight⌋ × rowExtent) +
    //              (⌊z / blockDepth⌋ × sliceExtent) +
    //              ((layer - imageSubresource.baseArrayLayer) × layerExtent)
    //
    // rowExtent = ⌈ max(bufferRowLength, imageExtent.width) / blockWidth ⌉ × blockSize
    // sliceExtent = ⌈ max(bufferImageHeight, imageExtent.height) / blockHeight ⌉ × rowExtent
    // layerExtent = ⌈ imageExtent.depth / blockDepth ⌉ × sliceExtent
    //
    // So first find texelOffset and return (texelOffset + blockSize)

    // if you have an extent of {1, 1, 1}, the texelOffset should become zero
    const uint32_t last_x = region.imageExtent.width - 1;
    const uint32_t last_y = region.imageExtent.height - 1;
    const uint32_t last_z = region.imageExtent.depth - 1;
    const uint32_t last_layer = normalized_layer_count - 1;

    const VkDeviceSize row_extent =
        static_cast<VkDeviceSize>(std::ceil(std::max(region.bufferRowLength, region.imageExtent.width) / block_extent.width)) *
        block_size;
    const VkDeviceSize slice_extent =
        static_cast<VkDeviceSize>(std::ceil(std::max(region.bufferImageHeight, region.imageExtent.height) / block_extent.height)) *
        row_extent;
    const VkDeviceSize layer_extent =
        static_cast<VkDeviceSize>(std::ceil(region.imageExtent.depth / block_extent.height)) * slice_extent;

    const VkDeviceSize x_value = static_cast<VkDeviceSize>(std::floor(last_x / block_extent.width)) * block_size;
    const VkDeviceSize y_value = static_cast<VkDeviceSize>(std::floor(last_y / block_extent.height)) * row_extent;
    const VkDeviceSize z_value = static_cast<VkDeviceSize>(std::floor(last_z / block_extent.depth)) * slice_extent;
    const VkDeviceSize layer_value = static_cast<VkDeviceSize>(last_layer) * layer_extent;
    VkDeviceSize texel_offset = x_value + y_value + z_value + layer_value;

    const VkDeviceSize buffer_copy_size = region.bufferOffset + texel_offset + block_size;

    if (buffer_state.create_info.size < buffer_copy_size) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle(), image_state.Handle());
        std::stringstream ss;
        ss << "is trying to copy " << buffer_copy_size << " bytes to/from the (" << FormatHandle(buffer_state).c_str()
           << ") which exceeds the VkBuffer total size of " << buffer_state.create_info.size
           << " bytes.\nLast texel coordinate of the image is at {x = " << last_x << ", y = " << last_y << ", z = " << last_z
           << ", layer = " << last_layer << "}\nrowExtent = " << row_extent << ", sliceExtent = " << slice_extent
           << ", layerExtent = " << layer_extent << "\nThe final byte found is at bufferOffset (" << region.bufferOffset
           << ") + texelOffset (" << texel_offset << ") + blockSize (" << block_size << ") ";

        if (is_multiplane) {
            ss << "(" << string_VkFormat(compatible_format) << ", the compatible format for plane "
               << vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask)) << " of "
               << string_VkFormat(format) << ")";
        } else {
            ss << "(" << string_VkFormat(format) << ")";
        }
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ExceedBufferBounds_00171), objlist, region_loc,
                         "%s", ss.str().c_str());
    }

    return skip;
}

template <typename HandleT>
// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(const HandleT handle, const vvl::Image &image_state, VkSampleCountFlagBits sample_count,
                                          const Location &loc, const std::string &vuid) const {
    bool skip = false;
    if (image_state.create_info.samples != sample_count) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "%s was created with a sample count of %s but must be %s.",
                         FormatHandle(image_state).c_str(), string_VkSampleCountFlagBits(image_state.create_info.samples),
                         string_VkSampleCountFlagBits(sample_count));
    }
    return skip;
}

bool CoreChecks::ValidateQueueFamilySupport(const vvl::CommandBuffer &cb_state, const vvl::PhysicalDevice &physical_device_state,
                                            VkImageAspectFlags aspectMask, const vvl::Image &image_state,
                                            const Location &aspect_mask_loc, const char *vuid) const {
    bool skip = false;

    if (!HasRequiredQueueFlags(cb_state, physical_device_state, VK_QUEUE_GRAPHICS_BIT) &&
        ((aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, aspect_mask_loc, "is %s, but command is %s", string_VkImageAspectFlags(aspectMask).c_str(),
                         DescribeRequiredQueueFlag(cb_state, physical_device_state, VK_QUEUE_GRAPHICS_BIT).c_str());
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_buffer_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyImageToBuffer2 || loc.function == Func::vkCmdCopyImageToBuffer2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_buffer_loc = loc.dot(Field::dstBuffer);
    // Even if the error is only dealing with the image or buffer, helpful to know incase there are many copies and the other handle
    // may be a helpful hint which one it is
    const LogObjectList objlist(commandBuffer, srcImage, dstBuffer);

    skip |= ValidateCmd(cb_state, loc);

    // dst buffer
    {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00192" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00192";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191";
        skip |=
            ValidateBufferUsageFlags(objlist, *dst_buffer_state, VK_BUFFER_USAGE_2_TRANSFER_DST_BIT, true, vuid, dst_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01832" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832";
        skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01833" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833";
        skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    }

    // src image
    {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07973" : "VUID-vkCmdCopyImageToBuffer-srcImage-07973";
        skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07966" : "VUID-vkCmdCopyImageToBuffer-srcImage-07966";
        skip |= ValidateMemoryIsBoundToImage(objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00186" : "VUID-vkCmdCopyImageToBuffer-srcImage-00186";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01831" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07969" : "VUID-vkCmdCopyImageToBuffer-srcImage-07969";
            skip |= LogError(vuid, objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-01998" : "VUID-vkCmdCopyImageToBuffer-srcImage-01998";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    src_image_loc, vuid);
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-01397" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397";
            skip |= LogError(vuid, objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }
    }

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];

        skip |= ValidateBufferImageCopyData(cb_state, region, *src_image_state, objlist, region_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, *src_image_state, region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-00189" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189";
        skip |= ValidateSubresourceImageLayout(cb_state, *src_image_state, region.imageSubresource, region.imageOffset.z,
                                               region.imageExtent.depth, srcImageLayout, src_image_loc, vuid);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *src_image_state, region, objlist, region_loc);

        skip |= ValidateBufferBounds(cb_state, *src_image_state, *dst_buffer_state, region, region_loc);

        if (!enabled_features.maintenance10) {
            vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-10216" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-10216";
            skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, region.imageSubresource.aspectMask,
                                               *src_image_state, subresource_loc.dot(Field::aspectMask), vuid);
        }

        const bool has_depth_aspect = region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT;
        const bool has_stencil_aspect = region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT;
        if (has_depth_aspect || has_stencil_aspect) {
            const VkFormatFeatureFlags2 src_image_format_features = GetPotentialFormatFeatures(src_image_state->create_info.format);

            const bool invalid_depth_copy_on_compute =
                has_depth_aspect && !(src_image_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR);
            const bool invalid_stencil_copy_on_compute =
                has_stencil_aspect && !(src_image_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR);

            const bool invalid_depth_copy_on_transfer =
                has_depth_aspect && !(src_image_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR);
            const bool invalid_stencil_copy_on_transfer =
                has_stencil_aspect && !(src_image_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANSFER_QUEUE_BIT_KHR);

            if (!HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_GRAPHICS_BIT)) {
                if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_COMPUTE_BIT)) {
                    if (invalid_depth_copy_on_compute || invalid_stencil_copy_on_compute) {
                        if (is_2) {
                            vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-11778"
                                                                 : "VUID-vkCmdCopyBufferToImage2-commandBuffer-11780";
                        } else {
                            vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyBufferToImage-commandBuffer-11778"
                                                                 : "VUID-vkCmdCopyBufferToImage-commandBuffer-11780";
                        }
                        const LogObjectList src_img_objlist(cb_state.Handle(), src_image_state->Handle());
                        skip |= LogError(
                            vuid, src_img_objlist, subresource_loc.dot(Field::aspectMask),
                            "is %s, commandBuffer was created with a VkCommandPool that does not support VK_QUEUE_GRAPHICS_BIT but "
                            "supports VK_QUEUE_COMPUTE_BIT, yet srcImage (%s) does not have the %s feature\n(features: %s).",
                            string_VkImageAspectFlags(region.imageSubresource.aspectMask).c_str(),
                            string_VkFormat(src_image_state->create_info.format),
                            invalid_depth_copy_on_compute ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR"
                                                          : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR",
                            string_VkFormatFeatureFlags2(src_image_format_features).c_str());
                    }
                } else if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_TRANSFER_BIT)) {
                    if (invalid_depth_copy_on_transfer || invalid_stencil_copy_on_transfer) {
                        if (is_2) {
                            vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-11779"
                                                                  : "VUID-vkCmdCopyBufferToImage2-commandBuffer-11781";
                        } else {
                            vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyBufferToImage-commandBuffer-11779"
                                                                  : "VUID-vkCmdCopyBufferToImage-commandBuffer-11781";
                        }
                        const LogObjectList src_img_objlist(cb_state.Handle(), src_image_state->Handle());
                        skip |=
                            LogError(vuid, src_img_objlist, subresource_loc.dot(Field::aspectMask),
                                     "is %s, commandBuffer was created with a VkCommandPool that supports neither "
                                     "VK_QUEUE_GRAPHICS_BIT nor VK_QUEUE_COMPUTE_BIT but supports VK_QUEUE_TRANSFER_BIT, yet "
                                     "srcImage (%s) does not have the %s feature\n(features: %s).",
                                     string_VkImageAspectFlags(region.imageSubresource.aspectMask).c_str(),
                                     string_VkFormat(src_image_state->create_info.format),
                                     invalid_depth_copy_on_transfer ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR"
                                                                    : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANSFER_QUEUE_BIT_KHR",
                                     string_VkFormatFeatureFlags2(src_image_format_features).c_str());
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                     const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                      const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, error_obj.location.dot(Field::pCopyImageToBufferInfo));
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_buffer_state = Get<vvl::Buffer>(srcBuffer);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_buffer_state);
    ASSERT_AND_RETURN_SKIP(dst_image_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBufferToImage2 || loc.function == Func::vkCmdCopyBufferToImage2KHR;
    const char *vuid;
    const Location src_buffer_loc = loc.dot(Field::srcBuffer);
    const Location dst_image_loc = loc.dot(Field::dstImage);
    // Even if the error is only dealing with the image or buffer, helpful to know incase there are many copies and the other handle
    // may be a helpful hint which one it is
    const LogObjectList objlist(commandBuffer, srcBuffer, dstImage);

    skip |= ValidateCmd(cb_state, loc);

    // src buffer
    {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00176" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00176";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00174";
        skip |=
            ValidateBufferUsageFlags(objlist, *src_buffer_state, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT, true, vuid, src_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01828" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01828";
        skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    }

    // dst image
    {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07973" : "VUID-vkCmdCopyBufferToImage-dstImage-07973";
        skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07966" : "VUID-vkCmdCopyBufferToImage-dstImage-07966";
        skip |= ValidateMemoryIsBoundToImage(objlist, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00177" : "VUID-vkCmdCopyBufferToImage-dstImage-00177";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01829" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01829";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage-commandBuffer-01830" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01830";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07969" : "VUID-vkCmdCopyBufferToImage-dstImage-07969";
            skip |= LogError(vuid, objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-01997" : "VUID-vkCmdCopyBufferToImage-dstImage-01997";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    dst_image_loc, vuid);
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-01396" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396";
            skip |= LogError(vuid, objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }
    }

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];

        skip |= ValidateBufferImageCopyData(cb_state, region, *dst_image_state, objlist, region_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, *dst_image_state, region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-00180" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180";
        skip |= ValidateSubresourceImageLayout(cb_state, *dst_image_state, region.imageSubresource, region.imageOffset.z,
                                               region.imageExtent.depth, dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *dst_image_state, region, objlist, region_loc);

        skip |= ValidateBufferBounds(cb_state, *dst_image_state, *src_buffer_state, region, region_loc);

        if (!enabled_features.maintenance10) {
            vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-07739" : "VUID-vkCmdCopyBufferToImage-commandBuffer-07739";
            skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, region.imageSubresource.aspectMask,
                                               *dst_image_state, subresource_loc.dot(Field::aspectMask), vuid);
        }

        const bool has_depth_aspect = region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT;
        const bool has_stencil_aspect = region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT;
        if (has_depth_aspect || has_stencil_aspect) {
            const VkFormatFeatureFlags2 dst_image_format_features = GetPotentialFormatFeatures(dst_image_state->create_info.format);

            const bool invalid_depth_copy_on_compute =
                has_depth_aspect && !(dst_image_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR);
            const bool invalid_stencil_copy_on_compute =
                has_stencil_aspect && !(dst_image_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR);

            const bool invalid_depth_copy_on_transfer =
                has_depth_aspect && !(dst_image_format_features & VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR);
            const bool invalid_stencil_copy_on_transfer =
                has_stencil_aspect && !(dst_image_format_features & VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANSFER_QUEUE_BIT_KHR);

            if (!HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_GRAPHICS_BIT)) {
                if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_COMPUTE_BIT)) {
                    if (invalid_depth_copy_on_compute || invalid_stencil_copy_on_compute) {
                        if (is_2) {
                            vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-11790"
                                                                 : "VUID-vkCmdCopyImageToBuffer2-commandBuffer-11792";
                        } else {
                            vuid = invalid_depth_copy_on_compute ? "VUID-vkCmdCopyImageToBuffer-commandBuffer-11790"
                                                                 : "VUID-vkCmdCopyImageToBuffer-commandBuffer-11792";
                        }
                        const LogObjectList src_img_objlist(cb_state.Handle(), dst_image_state->Handle());
                        skip |= LogError(
                            vuid, src_img_objlist, subresource_loc.dot(Field::aspectMask),
                            "is %s, commandBuffer was created with a VkCommandPool that does not support VK_QUEUE_GRAPHICS_BIT but "
                            "supports VK_QUEUE_COMPUTE_BIT, yet dstImage (%s) does not have the %s feature\n(features: %s).",
                            string_VkImageAspectFlags(region.imageSubresource.aspectMask).c_str(),
                            string_VkFormat(dst_image_state->create_info.format),
                            invalid_depth_copy_on_compute ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_COMPUTE_QUEUE_BIT_KHR"
                                                          : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_COMPUTE_QUEUE_BIT_KHR",
                            string_VkFormatFeatureFlags2(dst_image_format_features).c_str());
                    }
                } else if (HasRequiredQueueFlags(cb_state, *physical_device_state, VK_QUEUE_TRANSFER_BIT)) {
                    if (invalid_depth_copy_on_transfer || invalid_stencil_copy_on_transfer) {
                        if (is_2) {
                            vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-11791"
                                                                  : "VUID-vkCmdCopyImageToBuffer2-commandBuffer-11793";
                        } else {
                            vuid = invalid_depth_copy_on_transfer ? "VUID-vkCmdCopyImageToBuffer-commandBuffer-11791"
                                                                  : "VUID-vkCmdCopyImageToBuffer-commandBuffer-11793";
                        }
                        const LogObjectList src_img_objlist(cb_state.Handle(), dst_image_state->Handle());
                        skip |=
                            LogError(vuid, src_img_objlist, subresource_loc.dot(Field::aspectMask),
                                     "is %s, commandBuffer was created with a VkCommandPool that does does supports neither "
                                     "VK_QUEUE_GRAPHICS_BIT nor VK_QUEUE_COMPUTE_BIT but supports VK_QUEUE_TRANSFER_BIT, yet "
                                     "dstImage (%s) does not have the %s feature\n(features: %s).",
                                     string_VkImageAspectFlags(region.imageSubresource.aspectMask).c_str(),
                                     string_VkFormat(dst_image_state->create_info.format),
                                     invalid_depth_copy_on_transfer ? "VK_FORMAT_FEATURE_2_DEPTH_COPY_ON_TRANSFER_QUEUE_BIT_KHR"
                                                                    : "VK_FORMAT_FEATURE_2_STENCIL_COPY_ON_TRANSFER_QUEUE_BIT_KHR",
                                     string_VkFormatFeatureFlags2(dst_image_format_features).c_str());
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                      const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, error_obj.location.dot(Field::pCopyBufferToImageInfo));
}

bool CoreChecks::UsageHostTransferCheck(const vvl::Image &image_state, const VkImageAspectFlags aspect_mask, const char *vuid_09111,
                                        const char *vuid_09112, const char *vuid_09113, const Location &subresource_loc) const {
    bool skip = false;
    const bool has_stencil = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT);
    const bool has_non_stencil = (aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT);

    if (has_stencil) {
        if (const auto image_stencil_struct =
                vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state.create_info.pNext)) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
                skip |= LogError(vuid_09112, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                                 "(%s) includes VK_IMAGE_ASPECT_STENCIL_BIT and the image was created with "
                                 "VkImageStencilUsageCreateInfo, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included in "
                                 "VkImageStencilUsageCreateInfo::stencilUsage (%s) used to create image",
                                 string_VkImageAspectFlags(aspect_mask).c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            }
        } else {
            if ((image_state.create_info.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
                skip |= LogError(vuid_09111, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                                 "(%s) includes VK_IMAGE_ASPECT_STENCIL_BIT and the image was not created with "
                                 "VkImageStencilUsageCreateInfo, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included in "
                                 "VkImageCreateInfo::usage (%s) used to create image",
                                 string_VkImageAspectFlags(aspect_mask).c_str(),
                                 string_VkImageUsageFlags(image_state.create_info.usage).c_str());
            }
        }
    }
    if (has_non_stencil) {
        if ((image_state.create_info.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
            skip |= LogError(vuid_09113, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                             "(%s) includes aspects other than VK_IMAGE_ASPECT_STENCIL_BIT, but "
                             "VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included "
                             "in VkImageCreateInfo::usage (%s) used to create image",
                             string_VkImageAspectFlags(aspect_mask).c_str(),
                             string_VkImageUsageFlags(image_state.create_info.usage).c_str());
        }
    }
    return skip;
}

VkImageLayout GetImageLayout(VkCopyMemoryToImageInfo data) { return data.dstImageLayout; }
VkImageLayout GetImageLayout(VkCopyImageToMemoryInfo data) { return data.srcImageLayout; }

VkImage GetImage(VkCopyMemoryToImageInfo data) { return data.dstImage; }
VkImage GetImage(VkCopyImageToMemoryInfo data) { return data.srcImage; }

template <typename InfoPointer>
bool CoreChecks::ValidateMemoryImageCopyCommon(InfoPointer info_ptr, const Location &loc) const {
    bool skip = false;
    VkImage image = GetImage(*info_ptr);
    auto image_state = Get<vvl::Image>(image);
    ASSERT_AND_RETURN_SKIP(image_state);
    const VkImageLayout image_layout = GetImageLayout(*info_ptr);
    const bool from_image = loc.function == Func::vkCopyImageToMemory || loc.function == Func::vkCopyImageToMemoryEXT;
    const Location image_loc = loc.dot(from_image ? Field::srcImage : Field::dstImage);
    const Field info_type = from_image ? Field::pCopyImageToMemoryInfo : Field::pCopyMemoryToImageInfo;
    const Field image_layout_field = from_image ? Field::srcImageLayout : Field::dstImageLayout;

    const LogObjectList objlist(image_state->Handle());

    skip |= ValidateMemoryIsBoundToImage(
        image, *image_state, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07966" : "VUID-VkCopyMemoryToImageInfo-dstImage-07966");

    if (image_state->sparse && (!image_state->HasFullRangeBound())) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09109" : "VUID-VkCopyMemoryToImageInfo-dstImage-09109";
        skip |= LogError(vuid, objlist, image_loc, "is a sparse image with no memory bound");
    }

    if (image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07969" : "VUID-VkCopyMemoryToImageInfo-dstImage-07969";
        skip |= LogError(vuid, objlist, image_loc,
                         "must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }

    skip |= ValidateImageSampleCount(
        device, *image_state, VK_SAMPLE_COUNT_1_BIT, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07973" : "VUID-VkCopyMemoryToImageInfo-dstImage-07973");

    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY);
    const char *vuid_09111 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09111" : "VUID-VkCopyMemoryToImageInfo-dstImage-09111";
    const char *vuid_09112 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09112" : "VUID-VkCopyMemoryToImageInfo-dstImage-09112";
    const char *vuid_09113 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09113" : "VUID-VkCopyMemoryToImageInfo-dstImage-09113";
    for (uint32_t i = 0; i < info_ptr->regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const auto region = info_ptr->pRegions[i];

        skip |= ValidateHeterogeneousCopyData(region, *image_state, objlist, region_loc);
        skip |= ValidateImageSubresourceLayers(device, *image_state, region.imageSubresource, subresource_loc);
        skip |= UsageHostTransferCheck(*image_state, region.imageSubresource.aspectMask, vuid_09111, vuid_09112, vuid_09113,
                                       subresource_loc);

        if (check_memcpy) {
            if (region.imageOffset.x != 0 || region.imageOffset.y != 0 || region.imageOffset.z != 0) {
                const char *vuid = from_image ? "VUID-VkCopyImageToMemoryInfo-imageOffset-09114"
                                              : "VUID-VkCopyMemoryToImageInfo-imageOffset-09114";
                skip |= LogError(vuid, objlist, loc.dot(info_type).dot(Field::flags),
                                 "contains VK_HOST_IMAGE_COPY_MEMCPY which "
                                 "means that pRegions[%" PRIu32 "].imageOffset (%s) must all be zero",
                                 i, string_VkOffset3D(region.imageOffset).c_str());
            }
            const VkExtent3D subresource_extent = image_state->GetEffectiveSubresourceExtent(region.imageSubresource);
            if (!IsExtentEqual(region.imageExtent, subresource_extent)) {
                const char *vuid =
                    from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09115" : "VUID-VkCopyMemoryToImageInfo-dstImage-09115";
                skip |= LogError(
                    vuid, objlist, region_loc.dot(Field::imageExtent),
                    "(%s) must match the imageSubresource extents (%s) because %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY\n%s",
                    string_VkExtent3D(region.imageExtent).c_str(), string_VkExtent3D(subresource_extent).c_str(), String(info_type),
                    image_state->DescribeSubresourceLayers(region.imageSubresource).c_str());
            }
            if ((region.memoryRowLength != 0) || (region.memoryImageHeight != 0)) {
                const char *vuid =
                    from_image ? "VUID-VkCopyImageToMemoryInfo-flags-09394" : "VUID-VkCopyMemoryToImageInfo-flags-09393";
                skip |= LogError(vuid, objlist, region_loc.dot(Field::memoryRowLength),
                                 "(%" PRIu32 "), and memoryImageHeight (%" PRIu32
                                 ") must both be zero if %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                                 region.memoryRowLength, region.memoryImageHeight, String(info_type));
            }
        }

        skip |=
            ValidateHostCopyCurrentLayout(image_layout, region.imageSubresource, *image_state, region_loc.dot(image_layout_field));
    }

    return skip;
}

bool CoreChecks::ValidateHostCopyImageCreateInfos(const vvl::Image &src_image_state, const vvl::Image &dst_image_state,
                                                  const Location &loc) const {
    bool skip = false;
    std::stringstream mismatch_stream{};
    const VkImageCreateInfo &src_info = src_image_state.create_info;
    const VkImageCreateInfo &dst_info = dst_image_state.create_info;

    if (src_info.flags != dst_info.flags) {
        mismatch_stream << "srcImage flags = " << string_VkImageCreateFlags(src_info.flags)
                        << " and dstImage flags = " << string_VkImageCreateFlags(dst_info.flags) << '\n';
    }
    if (src_info.imageType != dst_info.imageType) {
        mismatch_stream << "srcImage imageType = " << string_VkImageType(src_info.imageType)
                        << " and dstImage imageType = " << string_VkImageType(dst_info.imageType) << '\n';
    }
    if (src_info.format != dst_info.format) {
        mismatch_stream << "srcImage format = " << string_VkFormat(src_info.format)
                        << " and dstImage format = " << string_VkFormat(dst_info.format) << '\n';
    }
    if ((src_info.extent.width != dst_info.extent.width) || (src_info.extent.height != dst_info.extent.height) ||
        (src_info.extent.depth != dst_info.extent.depth)) {
        mismatch_stream << "srcImage extent = (" << string_VkExtent3D(src_info.extent) << ") but dstImage exten = ("
                        << string_VkExtent3D(dst_info.extent) << ")\n";
    }
    if (src_info.mipLevels != dst_info.mipLevels) {
        mismatch_stream << "srcImage mipLevels = " << src_info.mipLevels << "and dstImage mipLevels = " << dst_info.mipLevels
                        << '\n';
    }
    if (src_info.arrayLayers != dst_info.arrayLayers) {
        mismatch_stream << "srcImage arrayLayers = " << src_info.arrayLayers
                        << " and dstImage arrayLayers = " << dst_info.arrayLayers << '\n';
    }
    if (src_info.samples != dst_info.samples) {
        mismatch_stream << "srcImage samples = " << string_VkSampleCountFlagBits(src_info.samples)
                        << " and dstImage samples = " << string_VkSampleCountFlagBits(dst_info.samples) << '\n';
    }
    if (src_info.tiling != dst_info.tiling) {
        mismatch_stream << "srcImage tiling = " << string_VkImageTiling(src_info.tiling)
                        << " and dstImage tiling = " << string_VkImageTiling(dst_info.tiling) << '\n';
    }
    if (src_info.usage != dst_info.usage) {
        mismatch_stream << "srcImage usage = " << string_VkImageUsageFlags(src_info.usage)
                        << " and dstImage usage = " << string_VkImageUsageFlags(dst_info.usage) << '\n';
    }
    if (src_info.sharingMode != dst_info.sharingMode) {
        mismatch_stream << "srcImage sharingMode = " << string_VkSharingMode(src_info.sharingMode)
                        << " and dstImage sharingMode = " << string_VkSharingMode(dst_info.sharingMode) << '\n';
    }
    if (src_info.initialLayout != dst_info.initialLayout) {
        mismatch_stream << "srcImage initialLayout = " << string_VkImageLayout(src_info.initialLayout)
                        << " and dstImage initialLayout = " << string_VkImageLayout(dst_info.initialLayout) << '\n';
    }

    if (mismatch_stream.str().length() > 0) {
        std::stringstream ss;
        ss << "The creation parameters for srcImage and dstImage differ:\n" << mismatch_stream.str();
        LogObjectList objlist(src_image_state.Handle(), dst_image_state.Handle());
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-09069", objlist, loc, "%s.", ss.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyImageLayout(const VkImage image, const VkImageLayout image_layout, const Location &loc,
                                             vvl::Field supported_name, const char *vuid) const {
    bool skip = false;
    const bool is_src = supported_name == Field::pCopySrcLayouts;
    const uint32_t layout_count = is_src ? phys_dev_props_core14.copySrcLayoutCount : phys_dev_props_core14.copyDstLayoutCount;
    const VkImageLayout *supported_image_layouts =
        is_src ? phys_dev_props_core14.pCopySrcLayouts : phys_dev_props_core14.pCopyDstLayouts;

    for (uint32_t i = 0; i < layout_count; ++i) {
        if (supported_image_layouts[i] == image_layout) {
            return false;
        }
    }

    std::stringstream ss;
    ss << "is " << string_VkImageLayout(image_layout)
       << " which is not one of the layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::" << String(supported_name)
       << "[" << layout_count << "]\nList of supported layouts:\n";
    for (uint32_t i = 0; i < layout_count; ++i) {
        ss << string_VkImageLayout(supported_image_layouts[i]) << "\n";
    }
    skip |= LogError(vuid, image, loc, "%s", ss.str().c_str());

    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyMemoryToImageInfo);
    auto dst_image = pCopyMemoryToImageInfo->dstImage;

    skip |= ValidateMemoryImageCopyCommon(pCopyMemoryToImageInfo, copy_loc);
    skip |= ValidateHostCopyImageLayout(dst_image, pCopyMemoryToImageInfo->dstImageLayout, copy_loc.dot(Field::dstImageLayout),
                                        Field::pCopyDstLayouts, "VUID-VkCopyMemoryToImageInfo-dstImageLayout-09060");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT *pCopyMemoryToImageInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCopyMemoryToImage(device, pCopyMemoryToImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyImageToMemoryInfo);
    auto src_image = pCopyImageToMemoryInfo->srcImage;

    skip |= ValidateMemoryImageCopyCommon(pCopyImageToMemoryInfo, copy_loc);
    skip |= ValidateHostCopyImageLayout(src_image, pCopyImageToMemoryInfo->srcImageLayout, copy_loc.dot(Field::srcImageLayout),
                                        Field::pCopySrcLayouts, "VUID-VkCopyImageToMemoryInfo-srcImageLayout-09065");
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT *pCopyImageToMemoryInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCopyImageToMemory(device, pCopyImageToMemoryInfo, error_obj);
}

bool CoreChecks::ValidateMemcpyExtents(const ImageCopyRegion &region, const Location &region_loc) const {
    bool skip = false;
    if (region.src_offset.x != 0 || region.src_offset.y != 0 || region.src_offset.z != 0) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcOffset-09114", device, region_loc.dot(Field::srcOffset),
                         "is (%s) but flags contains VK_HOST_IMAGE_COPY_MEMCPY.", string_VkOffset3D(region.src_offset).c_str());
    }
    if (!IsExtentEqual(region.extent, region.src_state.create_info.extent)) {
        skip |=
            LogError("VUID-VkCopyImageToImageInfo-srcImage-09115", region.src_state.Handle(), region_loc.dot(Field::imageExtent),
                     "(%s) must match the image's subresource "
                     "extents (%s) when VkCopyImageToImageInfo->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                     string_VkExtent3D(region.extent).c_str(), string_VkExtent3D(region.src_state.create_info.extent).c_str());
    }

    if (region.dst_offset.x != 0 || region.dst_offset.y != 0 || region.dst_offset.z != 0) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstOffset-09114", device, region_loc.dot(Field::dstOffset),
                         "is (%s) but flags contains VK_HOST_IMAGE_COPY_MEMCPY.", string_VkOffset3D(region.dst_offset).c_str());
    }
    if (!IsExtentEqual(region.extent, region.dst_state.create_info.extent)) {
        skip |=
            LogError("VUID-VkCopyImageToImageInfo-dstImage-09115", region.dst_state.Handle(), region_loc.dot(Field::imageExtent),
                     "(%s) must match the image's subresource "
                     "extents (%s) when VkCopyImageToImageInfo->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                     string_VkExtent3D(region.extent).c_str(), string_VkExtent3D(region.dst_state.create_info.extent).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyMultiplane(const ImageCopyRegion &region, const Location &region_loc) const {
    bool skip = false;
    const VkImageAspectFlags src_aspect_mask = region.src_subresource.aspectMask;
    if (vkuFormatPlaneCount(region.src_state.create_info.format) == 2 &&
        (src_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && src_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-07981", region.src_state.Handle(),
                         region_loc.dot(Field::srcSubresource), "is %s but srcImage has 2-plane format (%s).",
                         string_VkImageAspectFlags(src_aspect_mask).c_str(), string_VkFormat(region.src_state.create_info.format));
    }
    if (vkuFormatPlaneCount(region.src_state.create_info.format) == 3 &&
        (src_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && src_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT &&
         src_aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-07981", region.src_state.Handle(),
                         region_loc.dot(Field::srcSubresource), "is %s but srcImage has 3-plane format (%s).",
                         string_VkImageAspectFlags(src_aspect_mask).c_str(), string_VkFormat(region.src_state.create_info.format));
    }

    const VkImageAspectFlags dst_aspect_mask = region.dst_subresource.aspectMask;
    if (vkuFormatPlaneCount(region.dst_state.create_info.format) == 2 &&
        (dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-07981", region.dst_state.Handle(),
                         region_loc.dot(Field::dstSubresource), "is %s but dstImage has 2-plane format (%s).",
                         string_VkImageAspectFlags(dst_aspect_mask).c_str(), string_VkFormat(region.dst_state.create_info.format));
    }
    if (vkuFormatPlaneCount(region.dst_state.create_info.format) == 3 &&
        (dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT &&
         dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-07981", region.dst_state.Handle(),
                         region_loc.dot(Field::dstSubresource), "is %s but dstImage has 3-plane format (%s).",
                         string_VkImageAspectFlags(dst_aspect_mask).c_str(), string_VkFormat(region.dst_state.create_info.format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto info_ptr = pCopyImageToImageInfo;
    const Location loc = error_obj.location.dot(Field::pCopyImageToImageInfo);
    auto src_image_state = Get<vvl::Image>(info_ptr->srcImage);
    auto dst_image_state = Get<vvl::Image>(info_ptr->dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    // Formats are required to match, but check each image anyway
    const uint32_t src_plane_count = vkuFormatPlaneCount(src_image_state->create_info.format);
    const uint32_t dst_plane_count = vkuFormatPlaneCount(dst_image_state->create_info.format);
    bool check_multiplane = ((src_plane_count == 2 || src_plane_count == 3) || (dst_plane_count == 2 || dst_plane_count == 3));
    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY);

    skip |= ValidateHostCopyImageCreateInfos(*src_image_state, *dst_image_state, error_obj.location);
    skip |= ValidateCopyImageCommon(device, *src_image_state, *dst_image_state, error_obj.location);
    skip |= ValidateHostCopyImageLayout(info_ptr->srcImage, info_ptr->srcImageLayout, loc.dot(Field::srcImageLayout),
                                        Field::pCopySrcLayouts, "VUID-VkCopyImageToImageInfo-srcImageLayout-09072");
    skip |= ValidateHostCopyImageLayout(info_ptr->dstImage, info_ptr->dstImageLayout, loc.dot(Field::dstImageLayout),
                                        Field::pCopyDstLayouts, "VUID-VkCopyImageToImageInfo-dstImageLayout-09073");

    if (src_image_state->sparse && (!src_image_state->HasFullRangeBound())) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-09109", src_image_state->Handle(), loc.dot(Field::srcImage),
                         "is a sparse image with no memory bound");
    }
    if (dst_image_state->sparse && (!dst_image_state->HasFullRangeBound())) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-09109", dst_image_state->Handle(), loc.dot(Field::dstImage),
                         "is a sparse image with no memory bound");
    }

    const uint32_t region_count = info_ptr->regionCount;
    for (uint32_t i = 0; i < region_count; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        ImageCopyRegion region(*src_image_state, *dst_image_state, info_ptr->pRegions[i]);

        if (check_memcpy) {
            skip |= ValidateMemcpyExtents(region, region_loc);
        }
        if (check_multiplane) {
            skip |= ValidateHostCopyMultiplane(region, region_loc);
        }

        skip |= ValidateCopyImageRegionCommon(device, region, region_loc);

        // src
        {
            skip |=
                UsageHostTransferCheck(region.src_state, region.src_subresource.aspectMask,
                                       "VUID-VkCopyImageToImageInfo-srcImage-09111", "VUID-VkCopyImageToImageInfo-srcImage-09112",
                                       "VUID-VkCopyImageToImageInfo-srcImage-09113", region_loc.dot(Field::srcSubresource));
            skip |= ValidateHostCopyCurrentLayout(info_ptr->srcImageLayout, region.src_subresource, region.src_state,
                                                  region_loc.dot(Field::srcImageLayout));
        }

        // dst
        {
            skip |=
                UsageHostTransferCheck(region.dst_state, region.dst_subresource.aspectMask,
                                       "VUID-VkCopyImageToImageInfo-dstImage-09111", "VUID-VkCopyImageToImageInfo-dstImage-09112",
                                       "VUID-VkCopyImageToImageInfo-dstImage-09113", region_loc.dot(Field::dstSubresource));
            skip |= ValidateHostCopyCurrentLayout(info_ptr->dstImageLayout, region.dst_subresource, region.dst_state,
                                                  region_loc.dot(Field::dstImageLayout));
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT *pCopyImageToImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCopyImageToImage(device, pCopyImageToImageInfo, error_obj);
}

template <typename RegionType>
bool CoreChecks::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    const bool is_2 = loc.function == Func::vkCmdBlitImage2 || loc.function == Func::vkCmdBlitImage2KHR;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    skip |= ValidateCmd(cb_state, loc);

    const char *vuid;

    // src image
    const VkFormat src_format = src_image_state->create_info.format;
    const VkImageType src_type = src_image_state->create_info.imageType;
    {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00233" : "VUID-vkCmdBlitImage-srcImage-00233";
        skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00220" : "VUID-vkCmdBlitImage-srcImage-00220";
        skip |= ValidateMemoryIsBoundToImage(src_objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00219" : "VUID-vkCmdBlitImage-srcImage-00219";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-01999" : "VUID-vkCmdBlitImage-srcImage-01999";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_BLIT_SRC_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01834" : "VUID-vkCmdBlitImage-commandBuffer-01834";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(vuid, src_objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (VK_FILTER_LINEAR == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02001" : "VUID-vkCmdBlitImage-filter-02001";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT, src_image_loc, vuid);
        } else if (VK_FILTER_CUBIC_IMG == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02002" : "VUID-vkCmdBlitImage-filter-02002";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT, src_image_loc, vuid);
        }

        if (FormatRequiresYcbcrConversionExplicitly(src_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-06421" : "VUID-vkCmdBlitImage-srcImage-06421";
            skip |= LogError(vuid, src_objlist, src_image_loc,
                             "format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(src_format));
        }

        if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_2D != src_type)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-00237" : "VUID-vkCmdBlitImage-filter-00237";
            skip |= LogError(vuid, src_objlist, loc.dot(Field::filter),
                             "is VK_FILTER_CUBIC_IMG but srcImage was created with %s (not VK_IMAGE_TYPE_2D).",
                             string_VkImageType(src_type));
        }

        // Validate filter for Depth/Stencil formats
        if (vkuFormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00232" : "VUID-vkCmdBlitImage-srcImage-00232";
            skip |= LogError(vuid, src_objlist, src_image_loc, "has depth-stencil format %s but filter is %s.",
                             string_VkFormat(src_format), string_VkFilter(filter));
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-01398" : "VUID-vkCmdBlitImage-srcImageLayout-01398";
            skip |= LogError(vuid, src_objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }
    }

    // dst image
    const VkFormat dst_format = dst_image_state->create_info.format;
    const VkImageType dst_type = dst_image_state->create_info.imageType;
    {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00234" : "VUID-vkCmdBlitImage-dstImage-00234";
        skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00225" : "VUID-vkCmdBlitImage-dstImage-00225";
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00224" : "VUID-vkCmdBlitImage-dstImage-00224";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02000" : "VUID-vkCmdBlitImage-dstImage-02000";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_BLIT_DST_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01835" : "VUID-vkCmdBlitImage-commandBuffer-01835";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01836" : "VUID-vkCmdBlitImage-commandBuffer-01836";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (FormatRequiresYcbcrConversionExplicitly(dst_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-06422" : "VUID-vkCmdBlitImage-dstImage-06422";
            skip |= LogError(vuid, dst_objlist, dst_image_loc,
                             "format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(dst_format));
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-01399" : "VUID-vkCmdBlitImage-dstImageLayout-01399";
            skip |= LogError(vuid, dst_objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }
    }

    if (vkuFormatIsUINT(src_format) != vkuFormatIsUINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00230" : "VUID-vkCmdBlitImage-srcImage-00230";
        skip |= LogError(vuid, all_objlist, loc,
                         "srcImage format %s is different than dstImage format %s (if one is UINT, both must be UINT).",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    if (vkuFormatIsSINT(src_format) != vkuFormatIsSINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00229" : "VUID-vkCmdBlitImage-srcImage-00229";
        skip |= LogError(vuid, all_objlist, loc,
                         "srcImage format %s is different than dstImage format %s (if one is SINT, both must be SINT).",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    if (vkuFormatIsDepthOrStencil(src_format) || vkuFormatIsDepthOrStencil(dst_format)) {
        if (src_format != dst_format) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00231" : "VUID-vkCmdBlitImage-srcImage-00231";
            skip |= LogError(vuid, all_objlist, loc,
                             "srcImage format %s is different than dstImage format %s (if one is DepthStencil, both must be exact "
                             "same DepthStencil).",
                             string_VkFormat(src_format), string_VkFormat(dst_format));
        }
    }

    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];

        // When performing blit from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        // Will resolve VK_REMAINING_ARRAY_LAYERS to actual value (some VUs just want the value)
        const uint32_t normalized_src_layer_count = src_image_state->NormalizeLayerCount(src_subresource);
        const uint32_t normalized_dst_layer_count = dst_image_state->NormalizeLayerCount(dst_subresource);

        const bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                       RangesIntersect(src_subresource.baseArrayLayer, normalized_src_layer_count,
                                                       dst_subresource.baseArrayLayer, normalized_dst_layer_count));
        if (same_subresource) {
            if (!IsValueIn(srcImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL}) ||
                !IsValueIn(dstImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL})) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-09459" : "VUID-vkCmdBlitImage-srcImage-09459";
                skip |= LogError(vuid, src_objlist, loc,
                                 "blitting to same VkImage at miplevel = %" PRIu32
                                 "\n"
                                 "srcSubresource baseArrayLayer = %" PRIu32
                                 ", layerCount = %s\n"
                                 "dstSubresource baseArrayLayer = %" PRIu32
                                 ", layerCount = %s\n"
                                 "but srcImageLayout is %s and is dstImageLayout is %s",
                                 src_subresource.mipLevel, src_subresource.baseArrayLayer,
                                 string_LayerCount(src_image_state->create_info, src_subresource).c_str(),
                                 dst_subresource.baseArrayLayer,
                                 string_LayerCount(src_image_state->create_info, dst_subresource).c_str(),
                                 string_VkImageLayout(srcImageLayout), string_VkImageLayout(dstImageLayout));
            }
        }

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-00221" : "VUID-vkCmdBlitImage-srcImageLayout-00221";
        const int32_t src_depth_offset = (int32_t)std::min(region.srcOffsets[0].z, region.srcOffsets[1].z);
        const uint32_t src_depth_extent = (uint32_t)std::abs(region.srcOffsets[1].z - region.srcOffsets[0].z);
        skip |= ValidateSubresourceImageLayout(cb_state, *src_image_state, src_subresource, src_depth_offset, src_depth_extent,
                                               srcImageLayout, src_image_loc, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-00226" : "VUID-vkCmdBlitImage-dstImageLayout-00226";
        const int32_t dst_depth_offset = (int32_t)std::min(region.dstOffsets[0].z, region.dstOffsets[1].z);
        const uint32_t dst_depth_extent = (uint32_t)std::abs(region.dstOffsets[1].z - region.dstOffsets[0].z);
        skip |= ValidateSubresourceImageLayout(cb_state, *dst_image_state, dst_subresource, dst_depth_offset, dst_depth_extent,
                                               dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateImageSubresourceLayers(commandBuffer, *src_image_state, src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, *dst_image_state, dst_subresource, dst_subresource_loc);

        if (src_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS || dst_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
            if (src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (src_subresource.layerCount != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08801" : "VUID-VkImageBlit-layerCount-08801";
                    skip |= LogError(
                        vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        src_subresource.layerCount, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer);
                }
            } else if (dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (dst_subresource.layerCount != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08801" : "VUID-VkImageBlit-layerCount-08801";
                    skip |= LogError(
                        vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        dst_subresource.layerCount, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer);
                }
            }
        } else if (src_subresource.layerCount != dst_subresource.layerCount) {
            vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08800" : "VUID-VkImageBlit-layerCount-08800";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", src_subresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), dst_subresource.layerCount);
        }

        const VkImageAspectFlags src_aspect = src_subresource.aspectMask;
        const VkImageAspectFlags dst_aspect = dst_subresource.aspectMask;
        if (src_aspect != dst_aspect) {
            vuid = is_2 ? "VUID-VkImageBlit2-aspectMask-00238" : "VUID-VkImageBlit-aspectMask-00238";
            skip |=
                LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                         string_VkImageAspectFlags(src_aspect).c_str(), dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                         string_VkImageAspectFlags(dst_aspect).c_str());
        }

        // pre-maintenance8 both src/dst had to both match, after we only validate them independently
        if (!enabled_features.maintenance8 && (src_type == VK_IMAGE_TYPE_3D || dst_type == VK_IMAGE_TYPE_3D)) {
            if ((src_subresource.baseArrayLayer != 0) || (normalized_src_layer_count != 1) ||
                (dst_subresource.baseArrayLayer != 0) || (normalized_dst_layer_count != 1)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00240" : "VUID-vkCmdBlitImage-srcImage-00240";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "Using VK_IMAGE_TYPE_3D image so only the first layer may be used\n"
                                 "srcImage %s\n"
                                 "dstImage %s\n"
                                 "srcSubresource (baseArrayLayer = %" PRIu32
                                 ", layerCount = %s)\n"
                                 "dstSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %s)\n",
                                 string_VkImageType(src_type), string_VkImageType(dst_type), src_subresource.baseArrayLayer,
                                 string_LayerCount(src_image_state->create_info, src_subresource).c_str(),
                                 dst_subresource.baseArrayLayer,
                                 string_LayerCount(dst_image_state->create_info, dst_subresource).c_str());
            }
        } else if (enabled_features.maintenance8) {
            if (src_type == VK_IMAGE_TYPE_3D) {
                if (src_subresource.baseArrayLayer != 0 || normalized_src_layer_count != 1 || normalized_dst_layer_count != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10207" : "VUID-vkCmdBlitImage-maintenance8-10207";
                    skip |= LogError(vuid, all_objlist, region_loc,
                                     "the srcImage is VK_IMAGE_TYPE_3D so only can use its first layer\n"
                                     "srcSubresource (baseArrayLayer = %" PRIu32
                                     ", layerCount = %s)\n"
                                     "dstSubresource.layerCount = %s\n",
                                     src_subresource.baseArrayLayer,
                                     string_LayerCount(src_image_state->create_info, src_subresource).c_str(),
                                     string_LayerCount(dst_image_state->create_info, dst_subresource).c_str());
                }
            } else if (dst_type == VK_IMAGE_TYPE_3D) {
                const uint32_t abs_diff = static_cast<uint32_t>(abs(region.dstOffsets[0].z - region.dstOffsets[1].z));
                if (abs_diff != normalized_src_layer_count) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10579" : "VUID-vkCmdBlitImage-maintenance8-10579";
                    skip |= LogError(vuid, all_objlist, region_loc,
                                     "has the absolute difference of %" PRIu32 " between dstOffsets[0].z (%" PRIu32
                                     ") and dstOffsets[1].z (%" PRIu32 "), which is not equal to srcSubresource.layerCount (%s)",
                                     abs_diff, region.dstOffsets[0].z, region.dstOffsets[1].z,
                                     string_LayerCount(src_image_state->create_info, src_subresource).c_str());
                }
            }

            if (dst_type == VK_IMAGE_TYPE_3D) {
                if (dst_subresource.baseArrayLayer != 0 || normalized_dst_layer_count != 1 || normalized_src_layer_count != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10208" : "VUID-vkCmdBlitImage-maintenance8-10208";
                    skip |= LogError(vuid, all_objlist, region_loc,
                                     "the dstImage is VK_IMAGE_TYPE_3D so only can use its first layer\n"
                                     "srcSubresource.layerCount = %s\n"
                                     "dstSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %s)\n",
                                     string_LayerCount(src_image_state->create_info, src_subresource).c_str(),
                                     dst_subresource.baseArrayLayer,
                                     string_LayerCount(dst_image_state->create_info, dst_subresource).c_str());
                }
            } else if (src_type == VK_IMAGE_TYPE_3D) {
                const uint32_t abs_diff = static_cast<uint32_t>(abs(region.srcOffsets[0].z - region.srcOffsets[1].z));
                if (abs_diff != normalized_dst_layer_count) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10580" : "VUID-vkCmdBlitImage-maintenance8-10580";
                    skip |= LogError(vuid, all_objlist, region_loc,
                                     "has the absolute difference of %" PRIu32 " between srcOffsets[0].z (%" PRIu32
                                     ") and srcOffsets[1].z (%" PRIu32 "), which is not equal to dstSubresource.layerCount (%s)",
                                     abs_diff, region.srcOffsets[0].z, region.srcOffsets[1].z,
                                     string_LayerCount(dst_image_state->create_info, dst_subresource).c_str());
                }
            }
        }

        // src
        {
            if (!IsValidAspectMaskForFormat(src_aspect, src_format)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00241" : "VUID-vkCmdBlitImage-aspectMask-00241";
                skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for source image format %s. (%s)", string_VkImageAspectFlags(src_aspect).c_str(),
                                 string_VkFormat(src_format), DescribeValidAspectMaskForFormat(src_format).c_str());
            }

            if (src_type == VK_IMAGE_TYPE_1D) {
                if (region.srcOffsets[0].y != 0 || region.srcOffsets[1].y != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00245" : "VUID-vkCmdBlitImage-srcImage-00245";
                    skip |= LogError(vuid, src_objlist, region_loc,
                                     "srcOffsets[0].y is %" PRId32 " and srcOffsets[1].y is %" PRId32
                                     " but srcImage is VK_IMAGE_TYPE_1D.",
                                     region.srcOffsets[0].y, region.srcOffsets[1].y);
                }
            }
            if (src_type == VK_IMAGE_TYPE_1D || src_type == VK_IMAGE_TYPE_2D) {
                if (region.srcOffsets[0].z != 0 || region.srcOffsets[1].z != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00247" : "VUID-vkCmdBlitImage-srcImage-00247";
                    skip |= LogError(vuid, src_objlist, region_loc,
                                     "srcOffsets[0].z is %" PRId32 " and srcOffsets[1].z is %" PRId32 " but srcImage is %s.",
                                     region.srcOffsets[0].z, region.srcOffsets[1].z, string_VkImageType(src_type));
                }
            }

            VkExtent3D src_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
            if (region.srcOffsets[0].x < 0 || region.srcOffsets[1].x < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::x),
                                 "is %" PRId32 " and srcOffsets[1].x is %" PRId32 " but both offsets must be greater than zero.",
                                 region.srcOffsets[0].x, region.srcOffsets[1].x);
            } else if (region.srcOffsets[0].x > static_cast<int32_t>(src_extent.width) ||
                       region.srcOffsets[1].x > static_cast<int32_t>(src_extent.width)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::x),
                                 "is %" PRId32 " and srcOffsets[1].x is %" PRId32
                                 " which exceed srcSubresource width extent (%" PRIu32 ")\n%s.",
                                 region.srcOffsets[0].x, region.srcOffsets[1].x, src_extent.width,
                                 src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
            } else if (region.srcOffsets[0].y < 0 || region.srcOffsets[1].y < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::y),
                                 "is %" PRId32 " and srcOffsets[1].y is %" PRId32 " but both offsets must be greater than zero.",
                                 region.srcOffsets[0].y, region.srcOffsets[1].y);
            } else if (region.srcOffsets[0].y > static_cast<int32_t>(src_extent.height) ||
                       region.srcOffsets[1].y > static_cast<int32_t>(src_extent.height)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::y),
                                 "is %" PRId32 " and srcOffsets[1].y is %" PRId32
                                 " which exceed srcSubresource height extent (%" PRIu32 ").\n%s",
                                 region.srcOffsets[0].y, region.srcOffsets[1].y, src_extent.height,
                                 src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
            } else if (region.srcOffsets[0].z < 0 || region.srcOffsets[1].z < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::z),
                                 "is %" PRId32 " and srcOffsets[1].z is %" PRId32 " but both offsets must be greater than zero.",
                                 region.srcOffsets[0].z, region.srcOffsets[1].z);
            } else if (region.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth) ||
                       region.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
                skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffsets, 0).dot(Field::z),
                                 "is %" PRId32 " and srcOffsets[1].z is %" PRId32
                                 " which exceed srcSubresource depth extent (%" PRIu32 ").\n%s",
                                 region.srcOffsets[0].z, region.srcOffsets[1].z, src_extent.depth,
                                 src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
            }
        }

        // dst
        {
            if (!IsValidAspectMaskForFormat(dst_aspect, dst_format)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00242" : "VUID-vkCmdBlitImage-aspectMask-00242";
                skip |=
                    LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                             "(%s) is invalid for destination image format %s. (%s)", string_VkImageAspectFlags(src_aspect).c_str(),
                             string_VkFormat(src_format), DescribeValidAspectMaskForFormat(dst_format).c_str());
            }

            if (dst_type == VK_IMAGE_TYPE_1D) {
                if (region.dstOffsets[0].y != 0 || region.dstOffsets[1].y != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00250" : "VUID-vkCmdBlitImage-dstImage-00250";
                    skip |= LogError(vuid, dst_objlist, region_loc,
                                     "dstOffsets[0].y is %" PRId32 " and dstOffsets[1].y is %" PRId32
                                     " but dstImage is VK_IMAGE_TYPE_1D.",
                                     region.dstOffsets[0].y, region.dstOffsets[1].y);
                }
            }

            if (dst_type == VK_IMAGE_TYPE_1D || dst_type == VK_IMAGE_TYPE_2D) {
                if (region.dstOffsets[0].z != 0 || region.dstOffsets[1].z != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00252" : "VUID-vkCmdBlitImage-dstImage-00252";
                    skip |= LogError(vuid, dst_objlist, region_loc,
                                     "dstOffsets[0].z is %" PRId32 " and dstOffsets[1].z is %" PRId32 " but dstImage is %s.",
                                     region.dstOffsets[0].z, region.dstOffsets[1].z, string_VkImageType(dst_type));
                }
            }

            VkExtent3D dst_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
            if (region.dstOffsets[0].x < 0 || region.dstOffsets[1].x < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::x),
                                 "is %" PRId32 " and dstOffsets[1].x is %" PRId32 " but both offsets must be greater than zero.",
                                 region.dstOffsets[0].x, region.dstOffsets[1].x);
            } else if (region.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width) ||
                       region.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::x),
                                 "is %" PRId32 " and dstOffsets[1].x is %" PRId32
                                 " which exceed dstSubresource width extent (%" PRIu32 ").\n%s",
                                 region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.width,
                                 dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
            } else if (region.dstOffsets[0].y < 0 || region.dstOffsets[1].y < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::y),
                                 "is %" PRId32 " and dstOffsets[1].y is %" PRId32 " but both offsets must be greater than zero.",
                                 region.dstOffsets[0].x, region.dstOffsets[1].x);
            } else if (region.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height) ||
                       region.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::y),
                                 "is %" PRId32 " and dstOffsets[1].y is %" PRId32
                                 " which exceed dstSubresource height extent (%" PRIu32 ").\n%s",
                                 region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.height,
                                 dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
            } else if (region.dstOffsets[0].z < 0 || region.dstOffsets[1].z < 0) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::z),
                                 "is %" PRId32 " and dstOffsets[1].z is %" PRId32 " but both offsets must be greater than zero.",
                                 region.dstOffsets[0].z, region.dstOffsets[1].z);
            } else if (region.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth) ||
                       region.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
                skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffsets, 0).dot(Field::z),
                                 "is %" PRId32 " and dstOffsets[1].z is %" PRId32
                                 " which exceed dstSubresource depth extent (%" PRIu32 ").\n%s",
                                 region.dstOffsets[0].z, region.dstOffsets[1].z, dst_extent.depth,
                                 dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (srcImage == dstImage) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersectsBlit(&region, &pRegions[j], src_image_state->create_info.imageType,
                                         vkuFormatIsMultiplane(src_format))) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00217" : "VUID-vkCmdBlitImage-pRegions-00217";
                    skip |=
                        LogError(vuid, all_objlist, loc, "pRegion[%" PRIu32 "] src overlaps with pRegions[%" PRIu32 "] dst.", i, j);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                error_obj.location);
}

bool CoreChecks::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                 const ErrorObject &error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, error_obj.location.dot(Field::pBlitImageInfo));
}

template <typename RegionType>
bool CoreChecks::ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state);
    ASSERT_AND_RETURN_SKIP(dst_image_state);

    const bool is_2 = loc.function == Func::vkCmdResolveImage2 || loc.function == Func::vkCmdResolveImage2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    skip |= ValidateCmd(cb_state, loc);

    // src image
    {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00256" : "VUID-vkCmdResolveImage-srcImage-00256";
        skip |= ValidateMemoryIsBoundToImage(src_objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01837" : "VUID-vkCmdResolveImage-commandBuffer-01837";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06762" : "VUID-vkCmdResolveImage-srcImage-06762";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06763" : "VUID-vkCmdResolveImage-srcImage-06763";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, src_image_loc,
                                                vuid);

        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(vuid, src_objlist, src_image_loc,
                             "must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-01400" : "VUID-vkCmdResolveImage-srcImageLayout-01400";
            const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle());
            skip |= LogError(vuid, objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }

        if (src_image_state->create_info.samples == VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00257" : "VUID-vkCmdResolveImage-srcImage-00257";
            skip |= LogError(vuid, src_objlist, src_image_loc, "was created with sample count VK_SAMPLE_COUNT_1_BIT.");
        }
    }

    // dst image
    {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00258" : "VUID-vkCmdResolveImage-dstImage-00258";
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image_state, dst_image_loc, vuid);

        if (!enabled_features.maintenance10) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02003" : "VUID-vkCmdResolveImage-dstImage-02003";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT,
                                                    dst_image_loc, vuid);
        }
        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01838" : "VUID-vkCmdResolveImage-commandBuffer-01838";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01839" : "VUID-vkCmdResolveImage-commandBuffer-01839";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06764" : "VUID-vkCmdResolveImage-dstImage-06764";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06765" : "VUID-vkCmdResolveImage-dstImage-06765";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, dst_image_loc,
                                                vuid);

        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(vuid, dst_objlist, dst_image_loc,
                             "must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-01401" : "VUID-vkCmdResolveImage-dstImageLayout-01401";
            const LogObjectList objlist(cb_state.Handle(), dst_image_state->Handle());
            skip |= LogError(vuid, objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }

        if (dst_image_state->create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00259" : "VUID-vkCmdResolveImage-dstImage-00259";
            skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with %s (not VK_SAMPLE_COUNT_1_BIT).",
                             string_VkSampleCountFlagBits(dst_image_state->create_info.samples));
        }

        if (enabled_features.maintenance10) {
            if (!(dst_image_state->format_features &
                  (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))) {
                skip |= LogError("VUID-vkCmdResolveImage-maintenance10-11799", dst_objlist, dst_image_loc,
                                 "was created with %s which only has the following format features:\n%s\n",
                                 string_VkFormat(dst_image_state->create_info.format),
                                 string_VkFormatFeatureFlags2(dst_image_state->format_features).c_str());
            }
        }
    }

    if (src_image_state->create_info.format != dst_image_state->create_info.format) {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-01386" : "VUID-vkCmdResolveImage-srcImage-01386";
        skip |=
            LogError(vuid, all_objlist, src_image_loc, "was created with format %s but dstImage format is %s.",
                     string_VkFormat(src_image_state->create_info.format), string_VkFormat(dst_image_state->create_info.format));
    }

    // For each region, the number of layers in the image subresource should not be zero
    // For each region, src and dest image aspect must be color only
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        skip |= ValidateImageSubresourceLayers(commandBuffer, *src_image_state, src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, *dst_image_state, dst_subresource, dst_subresource_loc);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-00260" : "VUID-vkCmdResolveImage-srcImageLayout-00260";
        skip |= ValidateSubresourceImageLayout(cb_state, *src_image_state, src_subresource, region.srcOffset.z, region.extent.depth,
                                               srcImageLayout, src_image_loc, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-00262" : "VUID-vkCmdResolveImage-dstImageLayout-00262";
        skip |= ValidateSubresourceImageLayout(cb_state, *dst_image_state, dst_subresource, region.dstOffset.z, region.extent.depth,
                                               dstImageLayout, dst_image_loc, vuid);

        if (src_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS || dst_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
            if (src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (src_subresource.layerCount != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08804" : "VUID-VkImageResolve-layerCount-08804";
                    skip |= LogError(
                        vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        src_subresource.layerCount, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer);
                }
            } else if (dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (dst_subresource.layerCount != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08804" : "VUID-VkImageResolve-layerCount-08804";
                    skip |= LogError(
                        vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        dst_subresource.layerCount, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer);
                }
            }
        } else if (src_subresource.layerCount != dst_subresource.layerCount) {
            vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08803" : "VUID-VkImageResolve-layerCount-08803";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.srcSubresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.dstSubresource.layerCount);
        }
        const VkImageType src_image_type = src_image_state->create_info.imageType;
        const VkImageType dst_image_type = dst_image_state->create_info.imageType;

        if (!IsValidAspectMaskForFormat(src_subresource.aspectMask, src_image_state->create_info.format)) {
            skip |=
                LogError("VUID-vkCmdResolveImage-srcSubresource-11800", src_objlist, src_subresource_loc.dot(Field::aspectMask),
                         "(%s) is invalid for image format %s. (%s)", string_VkImageAspectFlags(src_subresource.aspectMask).c_str(),
                         string_VkFormat(src_image_state->create_info.format),
                         DescribeValidAspectMaskForFormat(src_image_state->create_info.format).c_str());
        }
        if (!IsValidAspectMaskForFormat(dst_subresource.aspectMask, dst_image_state->create_info.format)) {
            skip |=
                LogError("VUID-vkCmdResolveImage-dstSubresource-11801", dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                         "(%s) is invalid for image format %s. (%s)", string_VkImageAspectFlags(dst_subresource.aspectMask).c_str(),
                         string_VkFormat(dst_image_state->create_info.format),
                         DescribeValidAspectMaskForFormat(dst_image_state->create_info.format).c_str());
        }

        if (dst_image_type == VK_IMAGE_TYPE_3D) {
            // Will resolve VK_REMAINING_ARRAY_LAYERS to actual value (some VUs just want the value)
            const uint32_t normalized_src_layer_count = src_image_state->NormalizeLayerCount(src_subresource);
            const uint32_t normalized_dst_layer_count = dst_image_state->NormalizeLayerCount(dst_subresource);

            if (normalized_src_layer_count != 1) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04446" : "VUID-vkCmdResolveImage-srcImage-04446";
                skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::layerCount),
                                 "is %s but dstImage is VK_IMAGE_TYPE_3D.",
                                 string_LayerCount(src_image_state->create_info, src_subresource).c_str());
            }
            if ((dst_subresource.baseArrayLayer != 0) || (normalized_dst_layer_count != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04447" : "VUID-vkCmdResolveImage-srcImage-04447";
                skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and layerCount is %s but dstImage is VK_IMAGE_TYPE_3D.",
                                 dst_subresource.baseArrayLayer,
                                 string_LayerCount(dst_image_state->create_info, dst_subresource).c_str());
            }
        }

        // src
        {
            if (src_image_type == VK_IMAGE_TYPE_1D) {
                if (region.srcOffset.y != 0 || region.extent.height != 1) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00271" : "VUID-vkCmdResolveImage-srcImage-00271";
                    skip |=
                        LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but srcImage (%s) is VK_IMAGE_TYPE_1D.",
                                 region.srcOffset.y, region.extent.height, FormatHandle(src_image_state->Handle()).c_str());
                }
            }
            if (src_image_type == VK_IMAGE_TYPE_1D || src_image_type == VK_IMAGE_TYPE_2D) {
                if (region.srcOffset.z != 0 || region.extent.depth != 1) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00273" : "VUID-vkCmdResolveImage-srcImage-00273";
                    skip |= LogError(vuid, src_objlist, region_loc,
                                     "srcOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but srcImage (%s) is %s.",
                                     region.srcOffset.z, region.extent.depth, FormatHandle(src_image_state->Handle()).c_str(),
                                     string_VkImageType(src_image_type));
                }
            }

            // Each srcImage dimension offset + extent limits must fall with image subresource extent
            VkExtent3D subresource_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
            // developer
            if (src_subresource.mipLevel < src_image_state->create_info.mipLevels) {
                if (region.srcOffset.x < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::x),
                                     "(%" PRId32 ") must be greater than zero.", region.srcOffset.x);
                } else if ((uint64_t)region.srcOffset.x + (uint64_t)region.extent.width > (uint64_t)subresource_extent.width) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::x),
                                     "(%" PRId32 ") + extent.width (%" PRIu32 ") exceeds srcSubresource width extent (%" PRIu32
                                     ").\n%s",
                                     region.srcOffset.x, region.extent.width, subresource_extent.width,
                                     src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
                } else if (region.srcOffset.y < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::y),
                                     "(%" PRId32 ") must be greater than zero.", region.srcOffset.y);
                } else if ((uint64_t)region.srcOffset.y + (uint64_t)region.extent.height > (uint64_t)subresource_extent.height) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::y),
                                     "(%" PRId32 ") + extent.height (%" PRIu32 ") exceeds srcSubresource height extent (%" PRIu32
                                     ").\n%s",
                                     region.srcOffset.y, region.extent.height, subresource_extent.height,
                                     src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
                } else if (region.srcOffset.z < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::z),
                                     "(%" PRId32 ") must be greater than zero.", region.srcOffset.z);
                } else if ((uint64_t)region.srcOffset.z + (uint64_t)region.extent.depth > (uint64_t)subresource_extent.depth) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                    skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset).dot(Field::z),
                                     "(%" PRId32 ") + extent.depth (%" PRIu32 ") exceeds srcSubresource depth extent (%" PRIu32
                                     ").\n%s",
                                     region.srcOffset.z, region.extent.depth, subresource_extent.depth,
                                     src_image_state->DescribeSubresourceLayers(src_subresource).c_str());
                }
            }
        }

        // dst
        {
            if (dst_image_type == VK_IMAGE_TYPE_1D) {
                if (region.dstOffset.y != 0 || region.extent.height != 1) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00276" : "VUID-vkCmdResolveImage-dstImage-00276";
                    skip |= LogError(vuid, dst_objlist, region_loc,
                                     "dstOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but dstImage (%s) is 1D.",
                                     region.dstOffset.y, region.extent.height, FormatHandle(dst_image_state->Handle()).c_str());
                }
            }
            if (dst_image_type == VK_IMAGE_TYPE_1D || dst_image_type == VK_IMAGE_TYPE_2D) {
                if (region.dstOffset.z != 0 || region.extent.depth != 1) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00278" : "VUID-vkCmdResolveImage-dstImage-00278";
                    skip |= LogError(vuid, dst_objlist, region_loc,
                                     "dstOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but dstImage (%s) is 2D.",
                                     region.dstOffset.z, region.extent.depth, FormatHandle(dst_image_state->Handle()).c_str());
                }
            }

            // Each dstImage dimension offset + extent limits must fall with image subresource extent
            VkExtent3D subresource_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing
            if (dst_subresource.mipLevel < dst_image_state->create_info.mipLevels) {
                if (region.dstOffset.x < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::x),
                                     "(%" PRId32 ") must be greater than zero.", region.dstOffset.x);
                } else if ((uint64_t)region.dstOffset.x + (uint64_t)region.extent.width > (uint64_t)subresource_extent.width) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::x),
                                     "(%" PRId32 ") + extent.width (%" PRIu32 ") exceeds dstSubresource width extent (%" PRIu32
                                     ").\n%s",
                                     region.dstOffset.x, region.extent.width, subresource_extent.width,
                                     dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
                } else if (region.dstOffset.y < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::y),
                                     "(%" PRId32 ") must be greater than zero.", region.dstOffset.y);
                } else if ((uint64_t)region.dstOffset.y + (uint64_t)region.extent.height > (uint64_t)subresource_extent.height) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::y),
                                     "(%" PRId32 ") + extent.height (%" PRIu32 ") exceeds dstSubresource height extent (%" PRIu32
                                     ").\n%s",
                                     region.dstOffset.y, region.extent.height, subresource_extent.height,
                                     dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
                } else if (region.dstOffset.z < 0) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::z),
                                     "(%" PRId32 ") must be greater than zero.", region.dstOffset.z);
                } else if ((uint64_t)region.dstOffset.z + (uint64_t)region.extent.depth > (uint64_t)subresource_extent.depth) {
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                    skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::z),
                                     "(%" PRId32 ") + extent.depth (%" PRIu32 ") exceeds dstSubresource depth extent (%" PRIu32
                                     ").\n%s",
                                     region.dstOffset.z, region.extent.depth, subresource_extent.depth,
                                     dst_image_state->DescribeSubresourceLayers(dst_subresource).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                   error_obj.location);
}

bool CoreChecks::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

bool CoreChecks::ValidateResolveImageModeInfo(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                              const ErrorObject &error_obj) const {
    bool skip = false;

    const Location resolve_info_loc = error_obj.location.dot(Field::pResolveImageInfo);
    const Location src_image_loc = resolve_info_loc.dot(Field::srcImage);
    const LogObjectList src_objlist(commandBuffer, pResolveImageInfo->srcImage);

    const auto *resolve_mode_info = vku::FindStructInPNextChain<VkResolveImageModeInfoKHR>(pResolveImageInfo->pNext);
    if (!resolve_mode_info) {
        auto src_image_state = Get<vvl::Image>(pResolveImageInfo->srcImage);
        if (vkuFormatIsDepthOrStencil(src_image_state->create_info.format)) {
            skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10986", src_objlist, src_image_loc,
                             "has format %s but there is no VkResolveImageModeInfoKHR included in the pNext chain.\n%s",
                             string_VkFormat(src_image_state->create_info.format),
                             PrintPNextChain(Struct::VkResolveImageInfo2, pResolveImageInfo->pNext).c_str());
        }
        return skip;
    }

    const LogObjectList dst_objlist(commandBuffer, pResolveImageInfo->dstImage);
    const LogObjectList all_objlist(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->dstImage);

    auto src_image_state = Get<vvl::Image>(pResolveImageInfo->srcImage);
    auto dst_image_state = Get<vvl::Image>(pResolveImageInfo->dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state);
    ASSERT_AND_RETURN_SKIP(dst_image_state);

    if (resolve_mode_info->flags &
        (VK_RESOLVE_IMAGE_SKIP_TRANSFER_FUNCTION_BIT_KHR | VK_RESOLVE_IMAGE_ENABLE_TRANSFER_FUNCTION_BIT_KHR)) {
        if (!vkuFormatIsSRGB(src_image_state->create_info.format)) {
            skip |= LogError("VUID-VkResolveImageInfo2-pNext-10982", all_objlist,
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::flags),
                             "is %s but pResolveImageInfo->srcImage (%s) does not use sRGB encoding.",
                             string_VkResolveImageFlagsKHR(resolve_mode_info->flags).c_str(),
                             string_VkFormat(src_image_state->create_info.format));
        }
        if (!vkuFormatIsSRGB(dst_image_state->create_info.format)) {
            skip |= LogError("VUID-VkResolveImageInfo2-pNext-10982", all_objlist,
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::flags),
                             "is %s but pResolveImageInfo->dstImage (%s) does not use sRGB encoding.",
                             string_VkResolveImageFlagsKHR(resolve_mode_info->flags).c_str(),
                             string_VkFormat(dst_image_state->create_info.format));
        }
    }

    if (vkuFormatIsColor(src_image_state->create_info.format) && resolve_mode_info->resolveMode == VK_RESOLVE_MODE_NONE) {
        skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10983", src_objlist, src_image_loc, "has format %s but %s is %s.",
                         string_VkFormat(src_image_state->create_info.format),
                         resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                         string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
    }

    if (!vkuFormatIsSampledInt(src_image_state->create_info.format) &&
        resolve_mode_info->resolveMode != VK_RESOLVE_MODE_AVERAGE_BIT) {
        skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10984", src_objlist, src_image_loc, "has format %s but %s is %s.",
                         string_VkFormat(src_image_state->create_info.format),
                         resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                         string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
    }

    if (vkuFormatIsSampledInt(src_image_state->create_info.format) &&
        resolve_mode_info->resolveMode != VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) {
        skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10985", src_objlist, src_image_loc, "has format %s but %s is %s.",
                         string_VkFormat(src_image_state->create_info.format),
                         resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                         string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
    }

    if (vkuFormatIsDepthOrStencil(src_image_state->create_info.format)) {
        uint32_t first_region_with_depth_aspect = vvl::kNoIndex32;
        uint32_t first_region_with_stencil_aspect = vvl::kNoIndex32;
        uint32_t first_region_without_both_depth_and_stencil_aspects = vvl::kNoIndex32;
        for (uint32_t i = 0; i < pResolveImageInfo->regionCount; i++) {
            const VkImageResolve2 &region = pResolveImageInfo->pRegions[i];

            if (first_region_with_depth_aspect == vvl::kNoIndex32 &&
                (region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT ||
                 region.dstSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)) {
                first_region_with_depth_aspect = i;
            }

            if (first_region_with_stencil_aspect == vvl::kNoIndex32 &&
                (region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT ||
                 region.dstSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                first_region_with_stencil_aspect = i;
            }

            if (first_region_without_both_depth_and_stencil_aspects != vvl::kNoIndex32 &&
                (!(region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
                 !(region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) ||

                 !(region.dstSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
                 !(region.dstSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT))) {
                first_region_without_both_depth_and_stencil_aspects = i;
            }
        }

        if (first_region_with_depth_aspect != vvl::kNoIndex32 && resolve_mode_info->resolveMode == VK_RESOLVE_MODE_NONE) {
            skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10987", src_objlist,
                             resolve_info_loc.dot(Field::pRegions, first_region_with_depth_aspect).dot(Field::aspectMask),
                             "has VK_IMAGE_ASPECT_DEPTH_BIT but %s is %s.",
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                             string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
        }

        if (first_region_with_stencil_aspect != vvl::kNoIndex32 && resolve_mode_info->resolveMode == VK_RESOLVE_MODE_NONE) {
            skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10988", src_objlist,
                             resolve_info_loc.dot(Field::pRegions, first_region_with_stencil_aspect).dot(Field::aspectMask),
                             "has VK_IMAGE_ASPECT_STENCIL_BIT but %s is %s.",
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                             string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
        }

        if (first_region_with_depth_aspect != vvl::kNoIndex32 &&
            ((resolve_mode_info->resolveMode & phys_dev_props_core12.supportedDepthResolveModes) == 0)) {
            skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10989", src_objlist,
                             resolve_info_loc.dot(Field::pRegions, first_region_with_depth_aspect).dot(Field::aspectMask),
                             "has VK_IMAGE_ASPECT_DEPTH_BIT but "
                             "VkPhysicalDeviceDepthStencilResolveProperties::supportedDepthResolveModes is %s and %s is %s.",
                             string_VkResolveModeFlags(phys_dev_props_core12.supportedDepthResolveModes).c_str(),
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                             string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
        }

        if (first_region_with_stencil_aspect != vvl::kNoIndex32 &&
            ((resolve_mode_info->stencilResolveMode & phys_dev_props_core12.supportedStencilResolveModes) == 0)) {
            skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10990", src_objlist,
                             resolve_info_loc.dot(Field::pRegions, first_region_with_stencil_aspect).dot(Field::aspectMask),
                             "has VK_IMAGE_ASPECT_STENCIL_BIT but "
                             "VkPhysicalDeviceDepthStencilResolveProperties::supportedStencilResolveModes is %s and %s is %s.",
                             string_VkResolveModeFlags(phys_dev_props_core12.supportedStencilResolveModes).c_str(),
                             resolve_info_loc.pNext(Struct::VkResolveImageModeInfoKHR, Field::resolveMode).Fields().c_str(),
                             string_VkResolveModeFlagBits(resolve_mode_info->resolveMode));
        }

        if (first_region_with_depth_aspect != vvl::kNoIndex32 && first_region_with_stencil_aspect != vvl::kNoIndex32 &&
            !phys_dev_props_core12.independentResolve) {
            if (resolve_mode_info->resolveMode != resolve_mode_info->stencilResolveMode) {
                skip |= LogError("VUID-VkResolveImageInfo2-srcImage-10991", src_objlist, resolve_info_loc.dot(Field::resolveMode),
                                 "is %s but %s is %s.", string_VkResolveModeFlagBits(resolve_mode_info->resolveMode),
                                 resolve_info_loc.dot(Field::stencilResolveMode).Fields().c_str(),
                                 string_VkResolveModeFlagBits(resolve_mode_info->stencilResolveMode));
            }
        }

        if (vkuFormatIsDepthAndStencil(src_image_state->create_info.format) && !phys_dev_props_core12.independentResolveNone &&
            first_region_without_both_depth_and_stencil_aspects != vvl::kNoIndex32) {
            skip |=
                LogError("VUID-VkResolveImageInfo2-srcImage-10992", src_objlist, src_image_loc,
                         "has format %s, VkPhysicalDeviceDepthStencilResolveProperties::indepdendentResolveNone is VK_FALSE but "
                         "pResolveImageInfo->pRegions[%" PRIu32 "] does not contain both depth and stencil aspects.",
                         string_VkFormat(src_image_state->create_info.format), first_region_without_both_depth_and_stencil_aspects);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                    pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                    pResolveImageInfo->pRegions, error_obj.location.dot(Field::pResolveImageInfo));
    skip |= ValidateResolveImageModeInfo(commandBuffer, pResolveImageInfo, error_obj);
    return skip;
}

bool CoreChecks::ValidateStridedDeviceAddressRange(VkCommandBuffer command_buffer,
                                                   const VkStridedDeviceAddressRangeKHR &strided_range,
                                                   const Location &strided_range_loc) const {
    bool skip = false;
    if (strided_range.stride > strided_range.size) {
        skip |= LogError("VUID-VkStridedDeviceAddressRangeKHR-stride-10957", command_buffer, strided_range_loc.dot(Field::stride),
                         "(%" PRIu64 ") must be less than size (%" PRIu64 ")", strided_range.stride, strided_range.size);
    }

    if (strided_range.size != 0 && strided_range.address == 0) {
        skip |= LogError("VUID-VkStridedDeviceAddressRangeKHR-size-11411", command_buffer, strided_range_loc.dot(Field::address),
                         "is zero, but size is non-zero (%" PRIu64 ")", strided_range.size);
    }

    BufferAddressValidation<1> buffer_address_validator = {
        {{{"VUID-VkStridedDeviceAddressRangeKHR-address-11365",
           [&strided_range](const vvl::Buffer &buffer_state) {
               const VkDeviceSize end = buffer_state.create_info.size - (strided_range.address - buffer_state.deviceAddress);
               return strided_range.size > end;
           },
           [&strided_range]() {
               return "The VkStridedDeviceAddressRangeKHR::size (" + std::to_string(strided_range.size) +
                      ") bytes does not fit in any buffer";
           },
           kEmptyErrorMsgBuffer}}}};

    skip |= buffer_address_validator.ValidateDeviceAddress(
        *this, strided_range_loc.dot(Field::address), LogObjectList(command_buffer), strided_range.address, strided_range.size);

    return skip;
}

bool CoreChecks::ValidateCopyMemoryIndirectInfo(VkCommandBuffer command_buffer,
                                                const VkCopyMemoryIndirectInfoKHR &memory_indirect_info,
                                                const Location &info_loc) const {
    bool skip = false;

    if (memory_indirect_info.srcCopyFlags & VK_ADDRESS_COPY_PROTECTED_BIT_KHR) {
        skip |= LogError(
            "VUID-VkCopyMemoryIndirectInfoKHR-srcCopyFlags-10940", command_buffer, info_loc.dot(Field::srcCopyFlags),
            "(%s) must not contain VK_ADDRESS_COPY_PROTECTED_BIT_KHR (flag was added for vkCmdCopyMemoryToImageIndirectKHR)",
            string_VkAddressCopyFlagsKHR(memory_indirect_info.srcCopyFlags).c_str());
    }
    if (memory_indirect_info.dstCopyFlags & VK_ADDRESS_COPY_PROTECTED_BIT_KHR) {
        skip |= LogError(
            "VUID-VkCopyMemoryIndirectInfoKHR-dstCopyFlags-10941", command_buffer, info_loc.dot(Field::dstCopyFlags),
            "(%s) must not contain VK_ADDRESS_COPY_PROTECTED_BIT_KHR (flag was added for vkCmdCopyMemoryToImageIndirectKHR)",
            string_VkAddressCopyFlagsKHR(memory_indirect_info.dstCopyFlags).c_str());
    }

    const Location copy_range_loc = info_loc.dot(Field::copyAddressRange);
    skip |= ValidateStridedDeviceAddressRange(command_buffer, memory_indirect_info.copyAddressRange, copy_range_loc);

    const VkDeviceAddress address = memory_indirect_info.copyAddressRange.address;
    if (address % 4 != 0) {
        skip |=
            LogError("VUID-VkCopyMemoryIndirectInfoKHR-copyAddressRange-10942", command_buffer, copy_range_loc.dot(Field::address),
                     "is 0x%" PRIx64 " but it must be 4 byte aligned", memory_indirect_info.copyAddressRange.address);
    }

    const VkDeviceSize stride = memory_indirect_info.copyAddressRange.stride;
    if (stride % 4 != 0 || stride < sizeof(VkCopyMemoryIndirectCommandKHR)) {
        skip |= LogError(
            "VUID-VkCopyMemoryIndirectInfoKHR-copyAddressRange-10943", command_buffer, copy_range_loc.dot(Field::stride),
            "is %" PRIu64
            " but it must be a multiple of 4 and must be greater than or equal to sizeof(VkCopyMemoryIndirectCommandKHR) (%zu).",
            stride, sizeof(VkCopyMemoryIndirectCommandKHR));
    }

    const VkDeviceSize size = memory_indirect_info.copyAddressRange.size;
    if (memory_indirect_info.copyCount > 0 && stride > 0 && memory_indirect_info.copyCount > (size / stride)) {
        skip |= LogError("VUID-VkCopyMemoryIndirectInfoKHR-copyCount-10944", command_buffer, info_loc.dot(Field::copyCount),
                         "%" PRIu32 " must be less than or equal to size/stride (%" PRIu64 ")\nCalculated from size (%" PRIu64
                         ") / stride (%" PRIu64 ")",
                         memory_indirect_info.copyCount, size / stride, size, stride);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMemoryIndirectKHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyMemoryIndirectInfoKHR *pCopyMemoryIndirectInfo,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);

    if (!enabled_features.indirectMemoryCopy) {
        skip |= LogError("VUID-vkCmdCopyMemoryIndirectKHR-indirectMemoryCopy-10935", commandBuffer, error_obj.location,
                         "The indirectMemoryCopy feature must be enabled.");
    }

    if (!(phys_dev_ext_props.copy_memory_indirect_props.supportedQueues & cb_state->command_pool->queue_flags)) {
        skip |= LogError("VUID-vkCmdCopyMemoryIndirectKHR-commandBuffer-10936", commandBuffer, error_obj.location,
                         "was allocated from a VkCommandPool with queue flags %s\nNone are supported by "
                         "VkPhysicalDeviceCopyMemoryIndirectPropertiesKHR::supportedQueues %s",
                         string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str(),
                         string_VkQueueFlags(phys_dev_ext_props.copy_memory_indirect_props.supportedQueues).c_str());
    }

    if (!cb_state->unprotected) {
        skip |= LogError("VUID-vkCmdCopyMemoryIndirectKHR-commandBuffer-10937", commandBuffer, error_obj.location,
                         "command can't be used in protected command buffers.");
    }

    skip |= ValidateCopyMemoryIndirectInfo(commandBuffer, *pCopyMemoryIndirectInfo,
                                           error_obj.location.dot(Field::pCopyMemoryIndirectInfo));

    return skip;
}

bool CoreChecks::ValidateCopyMemoryToImageIndirectInfo(const vvl::CommandBuffer &cb_state,
                                                       const VkCopyMemoryToImageIndirectInfoKHR &indirect_info,
                                                       const Location &info_loc) const {
    bool skip = false;

    const VkStridedDeviceAddressRangeKHR &copy_range = indirect_info.copyAddressRange;
    const Location copy_range_loc = info_loc.dot(Field::copyAddressRange);
    skip |= ValidateStridedDeviceAddressRange(cb_state.VkHandle(), copy_range, copy_range_loc);

    if (copy_range.address % 4 != 0) {
        skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-copyAddressRange-10952", cb_state.Handle(),
                         copy_range_loc.dot(Field::address), "is 0x%" PRIx64 " but it must be 4 byte aligned", copy_range.address);
    }

    if (copy_range.stride % 4 != 0 || copy_range.stride < sizeof(VkCopyMemoryToImageIndirectCommandKHR)) {
        skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-copyAddressRange-10953", cb_state.Handle(),
                         copy_range_loc.dot(Field::stride),
                         "is %" PRIu64
                         " but it must be a multiple of 4 and must be greater than or equal to "
                         "sizeof(VkCopyMemoryToImageIndirectCommandKHR) (%zu).",
                         copy_range.stride, sizeof(VkCopyMemoryToImageIndirectCommandKHR));
    }

    if (indirect_info.copyCount > 0 && copy_range.stride > 0 && indirect_info.copyCount > (copy_range.size / copy_range.stride)) {
        skip |=
            LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-copyCount-10951", cb_state.Handle(), info_loc.dot(Field::copyCount),
                     "%" PRIu32 " must be less than or equal to size/stride (%" PRIu64 ")\nCalculated from size (%" PRIu64
                     ") / stride (%" PRIu64 ")",
                     indirect_info.copyCount, copy_range.size / copy_range.stride, copy_range.size, copy_range.stride);
    }

    auto dst_image = Get<vvl::Image>(indirect_info.dstImage);
    ASSERT_AND_RETURN_SKIP(dst_image);
    const LogObjectList dst_objlist(cb_state.Handle(), indirect_info.dstImage);
    const Location dst_image_loc = info_loc.dot(Field::dstImage);

    // dst image
    {
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image, dst_image_loc,
                                             "VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-07665");

        if (!dst_image->unprotected) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-07661", dst_objlist, dst_image_loc,
                             "must not be a protected image.");
        }

        if (!(dst_image->create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-07664", dst_objlist, dst_image_loc,
                             "was created with usage (%s) which is missing VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
                             string_VkImageUsageFlags(dst_image->create_info.usage).c_str());
        }

        if (dst_image->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-07673", dst_objlist, dst_image_loc,
                             "was created with %s (which contains VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT).",
                             string_VkImageCreateFlags(dst_image->create_info.flags).c_str());
        }

        if (dst_image->create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-07973", dst_objlist, dst_image_loc,
                             "was created with %s (must be VK_SAMPLE_COUNT_1_BIT).",
                             string_VkSampleCountFlagBits(dst_image->create_info.samples));
        }

        skip |= ValidateImageFormatFeatureFlags(cb_state.VkHandle(), *dst_image, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, dst_image_loc,
                                                "VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-10974");
        skip |=
            ValidateImageFormatFeatureFlags(cb_state.VkHandle(), *dst_image, VK_FORMAT_FEATURE_2_COPY_IMAGE_INDIRECT_DST_BIT_KHR,
                                            dst_image_loc, "VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImage-10955");

        // TODO - how do the whole maintenance9 depth transition work here?
        skip |= ValidateSubresourceImageLayout(cb_state, *dst_image, *indirect_info.pImageSubresources, 0,
                                               dst_image->create_info.extent.depth, indirect_info.dstImageLayout, info_loc,
                                               "VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImageLayout-07667");
    }

    for (uint32_t i = 0; i < indirect_info.copyCount; ++i) {
        const VkImageSubresourceLayers &subresource_layers = indirect_info.pImageSubresources[i];
        const Location subresource_loc = info_loc.dot(Field::pImageSubresources, i);

        const VkImageAspectFlags aspect_mask = subresource_layers.aspectMask;

        if (!IsSingleBitSet(aspect_mask)) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-aspectMask-07662", dst_objlist,
                             subresource_loc.dot(Field::aspectMask),
                             "(%s) must have only a single bit set. (You can only copy one part of the image at a time)",
                             string_VkImageAspectFlags(aspect_mask).c_str());
        }

        if (!(cb_state.command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) &&
            (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
            skip |= LogError(
                "VUID-VkCopyMemoryToImageIndirectInfoKHR-commandBuffer-07674", dst_objlist, subresource_loc.dot(Field::aspectMask),
                "(%s) must not be VK_IMAGE_ASPECT_DEPTH_BIT "
                "or VK_IMAGE_ASPECT_STENCIL_BIT when the queue family does not support VK_QUEUE_GRAPHICS_BIT\n VkCommandPool queue "
                "flags: %s.",
                string_VkImageAspectFlags(aspect_mask).c_str(), string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
        }

        const uint32_t mip_level = subresource_layers.mipLevel;
        if (mip_level >= dst_image->create_info.mipLevels) {
            skip |= LogError(
                "VUID-VkCopyMemoryToImageIndirectInfoKHR-mipLevel-07670", dst_objlist, subresource_loc.dot(Field::mipLevel),
                "(%" PRIu32 ") must be less than the VkImageCreateInfo::mipLevels (%" PRIu32 ") when dstImage was created.",
                mip_level, dst_image->create_info.mipLevels);
        }

        if (subresource_layers.layerCount != VK_REMAINING_ARRAY_LAYERS &&
            subresource_layers.baseArrayLayer + subresource_layers.layerCount > dst_image->create_info.arrayLayers) {
            skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-layerCount-08764", dst_objlist,
                             subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") + baseArrayLayer (%" PRIu32
                             ") must be less than or equal to "
                             "the VkImageCreateInfo::arrayLayers (%" PRIu32 ") when dstImage was created.",
                             subresource_layers.layerCount, subresource_layers.baseArrayLayer, dst_image->create_info.arrayLayers);
        }
    }

    if (!IsValueIn(indirect_info.dstImageLayout,
                   {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL})) {
        skip |= LogError("VUID-VkCopyMemoryToImageIndirectInfoKHR-dstImageLayout-07669", dst_objlist,
                         info_loc.dot(Field::dstImageLayout),
                         "is %s but must be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, "
                         "or VK_IMAGE_LAYOUT_GENERAL.",
                         string_VkImageLayout(indirect_info.dstImageLayout));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMemoryToImageIndirectKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToImageIndirectInfoKHR *pCopyMemoryToImageIndirectInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN_SKIP(cb_state);

    if (!enabled_features.indirectMemoryToImageCopy) {
        skip |= LogError("VUID-vkCmdCopyMemoryToImageIndirectKHR-indirectMemoryToImageCopy-10947", commandBuffer,
                         error_obj.location, "The indirectMemoryToImageCopy feature must be enabled.");
    }

    if (!(phys_dev_ext_props.copy_memory_indirect_props.supportedQueues & cb_state->command_pool->queue_flags)) {
        skip |= LogError("VUID-vkCmdCopyMemoryToImageIndirectKHR-commandBuffer-10948", commandBuffer, error_obj.location,
                         "was allocated from a VkCommandPool with queue flags %s\nNone are supported by "
                         "VkPhysicalDeviceCopyMemoryIndirectPropertiesKHR::supportedQueues %s",
                         string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str(),
                         string_VkQueueFlags(phys_dev_ext_props.copy_memory_indirect_props.supportedQueues).c_str());
    }

    if (!cb_state->unprotected) {
        skip |= LogError("VUID-vkCmdCopyMemoryToImageIndirectKHR-commandBuffer-10949", commandBuffer, error_obj.location,
                         "command can't be used in protected command buffers.");
    }

    skip |= ValidateCopyMemoryToImageIndirectInfo(*cb_state, *pCopyMemoryToImageIndirectInfo,
                                                  error_obj.location.dot(Field::pCopyMemoryToImageIndirectInfo));

    return skip;
}
