/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include <string>
#include <sstream>
#include <vector>

#include "containers/range_vector.h"
#include "core_validation.h"
#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"

// Returns the intersection of the ranges [x, x + x_size) and [y, y + y_size)
static sparse_container::range<int64_t> GetRangeIntersection(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    int64_t intersection_min = std::max(x, y);
    int64_t intersection_max = std::min(x + static_cast<int64_t>(x_size), y + static_cast<int64_t>(y_size));

    return {intersection_min, intersection_max};
}

// Returns true if [x, x + x_size) and [y, y + y_size) overlap
static bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    auto intersection = GetRangeIntersection(x, x_size, y, y_size);
    return intersection.non_empty();
}

struct ImageRegionIntersection {
    VkImageSubresourceLayers subresource = {};
    VkOffset3D offset = {0, 0, 0};
    VkExtent3D extent = {1, 1, 1};
    bool has_instersection = false;
    std::string String() const noexcept {
        std::stringstream ss;
        ss << "{ subresource { aspectMask: " << string_VkImageAspectFlags(subresource.aspectMask)
           << ", mipLevel: " << subresource.mipLevel << ", baseArrayLayer: " << subresource.baseArrayLayer
           << ", layerCount: " << subresource.layerCount << " }, offset {" << offset.x << ", " << offset.y << ", " << offset.z
           << "}, extent {" << extent.width << ", " << extent.height << ", " << extent.depth << "} }";
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
        result.has_instersection = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.z, region0.extent.depth, region1.dstOffset.z, region1.extent.depth);
                if (intersection.non_empty()) {
                    result.offset.z = static_cast<int32_t>(intersection.begin);
                    result.extent.depth = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_instersection = false;
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
                    result.has_instersection = false;
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
                    result.has_instersection = false;
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

// Test if the extent argument has all dimensions set to 0.
static inline bool IsExtentAllZeroes(const VkExtent3D &extent) {
    return ((extent.width == 0) && (extent.height == 0) && (extent.depth == 0));
}

// Returns the image transfer granularity for a specific image scaled by compressed block size if necessary.
VkExtent3D CoreChecks::GetScaledItg(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state) const {
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    VkExtent3D granularity = {0, 0, 0};
    const VkFormat image_format = image_state.createInfo.format;
    const auto pool = cb_state.command_pool;
    if (pool) {
        granularity = physical_device_state->queue_family_properties[pool->queueFamilyIndex].minImageTransferGranularity;
        if (vkuFormatIsBlockedImage(image_format)) {
            auto block_size = vkuFormatTexelBlockExtent(image_format);
            granularity.width *= block_size.width;
            granularity.height *= block_size.height;
        }
    }
    return granularity;
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

// Check elements of a VkOffset3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::CheckItgOffset(const LogObjectList &objlist, const VkOffset3D &offset, const VkExtent3D &granularity,
                                const Location &offset_loc, const char *vuid) const {
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset.x));
    offset_extent.height = static_cast<uint32_t>(abs(offset.y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset.z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (IsExtentAllZeroes(offset_extent) == false) {
            skip |= LogError(vuid, objlist, offset_loc,
                             "(x=%" PRId32 ", y=%" PRId32 ", z=%" PRId32
                             ") must be (x=0, y=0, z=0) when the command buffer's queue family "
                             "image transfer granularity is (w=0, h=0, d=0).",
                             offset.x, offset.y, offset.z);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        if (IsExtentAligned(offset_extent, granularity) == false) {
            skip |= LogError(vuid, objlist, offset_loc,
                             "(x=%" PRId32 ", y=%" PRId32 ", z=%" PRId32
                             ") dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32 ").",
                             offset.x, offset.y, offset.z, granularity.width, granularity.height, granularity.depth);
        }
    }
    return skip;
}

// Test if two VkExtent3D structs are equivalent
static inline bool IsExtentEqual(const VkExtent3D &extent, const VkExtent3D &other_extent) {
    bool result = true;
    if ((extent.width != other_extent.width) || (extent.height != other_extent.height) || (extent.depth != other_extent.depth)) {
        result = false;
    }
    return result;
}

// Check elements of a VkExtent3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::CheckItgExtent(const LogObjectList &objlist, const VkExtent3D &extent, const VkOffset3D &offset,
                                const VkExtent3D &granularity, const VkExtent3D &subresource_extent, const VkImageType image_type,
                                const Location &extent_loc, const char *vuid) const {
    bool skip = false;
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (IsExtentEqual(extent, subresource_extent) == false) {
            skip |= LogError(vuid, objlist, extent_loc,
                             "(w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32 ") must match the image subresource extents (w=%" PRIu32
                             ", h=%" PRIu32 ", d=%" PRIu32
                             ") "
                             "when the command buffer's queue family image transfer granularity is (w=0, h=0, d=0).",
                             extent.width, extent.height, extent.depth, subresource_extent.width, subresource_extent.height,
                             subresource_extent.depth);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the extent dimensions must always be even
        // integer multiples of the image transfer granularity or the offset + extent dimensions must always match the image
        // subresource extent dimensions.
        VkExtent3D offset_extent_sum = {};
        offset_extent_sum.width = static_cast<uint32_t>(abs(offset.x)) + extent.width;
        offset_extent_sum.height = static_cast<uint32_t>(abs(offset.y)) + extent.height;
        offset_extent_sum.depth = static_cast<uint32_t>(abs(offset.z)) + extent.depth;
        bool x_ok = true;
        bool y_ok = true;
        bool z_ok = true;
        switch (image_type) {
            case VK_IMAGE_TYPE_3D:
                z_ok =
                    ((0 == SafeModulo(extent.depth, granularity.depth)) || (subresource_extent.depth == offset_extent_sum.depth));
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                y_ok = ((0 == SafeModulo(extent.height, granularity.height)) ||
                        (subresource_extent.height == offset_extent_sum.height));
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                x_ok =
                    ((0 == SafeModulo(extent.width, granularity.width)) || (subresource_extent.width == offset_extent_sum.width));
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
        if (!(x_ok && y_ok && z_ok)) {
            skip |= LogError(vuid, objlist, extent_loc,
                             "(w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32
                             ") dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32
                             ") or offset (x=%" PRId32 ", y=%" PRId32 ", z=%" PRId32
                             ") + "
                             "extent (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32
                             ") must match the image subresource extents (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32 ").",
                             extent.width, extent.height, extent.depth, granularity.width, granularity.height, granularity.depth,
                             offset.x, offset.y, offset.z, extent.width, extent.height, extent.depth, subresource_extent.width,
                             subresource_extent.height, subresource_extent.depth);
        }
    }
    return skip;
}
template <typename HandleT>
bool CoreChecks::ValidateImageMipLevel(const HandleT handle, const IMAGE_STATE &image_state, uint32_t mip_level,
                                       const Location &mip_loc, const char *vuid) const {
    bool skip = false;
    if (mip_level >= image_state.createInfo.mipLevels) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(vuid, objlist, mip_loc, "is %" PRIu32 ", but provided %s has %" PRIu32 " mip levels.", mip_level,
                         FormatHandle(image_state).c_str(), image_state.createInfo.mipLevels);
    }
    return skip;
}
template <typename HandleT>
bool CoreChecks::ValidateImageArrayLayerRange(const HandleT handle, const IMAGE_STATE &img, const uint32_t base_layer,
                                              const uint32_t layer_count, const Location &subresource_loc, const char *vuid) const {
    bool skip = false;
    if (base_layer >= img.createInfo.arrayLayers || layer_count > img.createInfo.arrayLayers ||
        (base_layer + layer_count) > img.createInfo.arrayLayers) {
        if (layer_count != VK_REMAINING_ARRAY_LAYERS) {
            const LogObjectList objlist(handle, img.Handle());
            skip |= LogError(vuid, objlist, subresource_loc.dot(Field::baseArrayLayer),
                             "is %" PRIu32 " and layerCount is %" PRIu32 ", but provided %s has %" PRIu32 " array layers.",
                             base_layer, layer_count, FormatHandle(img).c_str(), img.createInfo.arrayLayers);
        }
    }
    return skip;
}

// All VUID from copy_bufferimage_to_imagebuffer_common.txt with more as a result of host_image_copy
static const char *GetBufferMemoryImageCopyCommandVUID(const std::string &id, bool from_image, bool copy2, bool is_memory = false) {
    // clang-format off
    static const std::map<std::string, std::array<const char *, 6>> copy_imagebuffermemory_vuid = {
        {"06659", {
            "VUID-VkBufferImageCopy-imageExtent-06659",      // !copy2  & !from_image
            "VUID-VkBufferImageCopy-imageExtent-06659",      // !copy2  &  from_image
            "VUID-VkBufferImageCopy2-imageExtent-06659",     //  copy2  & !from_image
            "VUID-VkBufferImageCopy2-imageExtent-06659",     //  copy2  &  from_image
            "VUID-VkMemoryToImageCopyEXT-imageExtent-06659", //  memory & !from_image
            "VUID-VkImageToMemoryCopyEXT-imageExtent-06659", //  memory &  from_image
        }},
        {"06660", {
            "VUID-VkBufferImageCopy-imageExtent-06660",
            "VUID-VkBufferImageCopy-imageExtent-06660",
            "VUID-VkBufferImageCopy2-imageExtent-06660",
            "VUID-VkBufferImageCopy2-imageExtent-06660",
            "VUID-VkMemoryToImageCopyEXT-imageExtent-06660",
            "VUID-VkImageToMemoryCopyEXT-imageExtent-06660",
        }},
        {"06661", {
            "VUID-VkBufferImageCopy-imageExtent-06661",
            "VUID-VkBufferImageCopy-imageExtent-06661",
            "VUID-VkBufferImageCopy2-imageExtent-06661",
            "VUID-VkBufferImageCopy2-imageExtent-06661",
            "VUID-VkMemoryToImageCopyEXT-imageExtent-06661",
            "VUID-VkImageToMemoryCopyEXT-imageExtent-06661",
        }},
        {"09101", {
            "VUID-VkBufferImageCopy-bufferRowLength-09101",
            "VUID-VkBufferImageCopy-bufferRowLength-09101",
            "VUID-VkBufferImageCopy2-bufferRowLength-09101",
            "VUID-VkBufferImageCopy2-bufferRowLength-09101",
            "VUID-VkMemoryToImageCopyEXT-memoryRowLength-09101",
            "VUID-VkImageToMemoryCopyEXT-memoryRowLength-09101",
        }},
        {"00196", {
            "VUID-VkBufferImageCopy-bufferImageHeight-09102",
            "VUID-VkBufferImageCopy-bufferImageHeight-09102",
            "VUID-VkBufferImageCopy2-bufferImageHeight-09102",
            "VUID-VkBufferImageCopy2-bufferImageHeight-09102",
            "VUID-VkMemoryToImageCopyEXT-memoryImageHeight-09102",
            "VUID-VkImageToMemoryCopyEXT-memoryImageHeight-09102",
        }},
        {"09103", {
            "VUID-VkBufferImageCopy-aspectMask-09103",
            "VUID-VkBufferImageCopy-aspectMask-09103",
            "VUID-VkBufferImageCopy2-aspectMask-09103",
            "VUID-VkBufferImageCopy2-aspectMask-09103",
            "VUID-VkMemoryToImageCopyEXT-aspectMask-09103",
            "VUID-VkImageToMemoryCopyEXT-aspectMask-09103",
        }},
        {"07975", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07975",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07975",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07975",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07975",
            kVUIDUndefined,
            kVUIDUndefined,
        }},
        {"07976", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07976",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07976",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07976",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07976",
            kVUIDUndefined,
            kVUIDUndefined,
        }},
        {"00197", {
            "VUID-vkCmdCopyBufferToImage-imageSubresource-07971",
            "VUID-vkCmdCopyImageToBuffer-imageSubresource-07971",
            "VUID-VkCopyBufferToImageInfo2-pRegions-06223",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-00197",
            "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07971",
            "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07971",
        }},
        {"00198", {
            "VUID-vkCmdCopyBufferToImage-imageSubresource-07972",
            "VUID-vkCmdCopyImageToBuffer-imageSubresource-07972",
            "VUID-VkCopyBufferToImageInfo2-pRegions-06224",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-00198",
            "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07972",
            "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07972",
        }},
        {"07979", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07979",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07979",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07979",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07979",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07979",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07979",
        }},
        {"09104", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-09104",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-09104",
            "VUID-VkCopyBufferToImageInfo2-imageOffset-09104",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-09104",
            "VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09104",
            "VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09104",
        }},
        {"07980", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07980",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07980",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07980",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07980",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07980",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07980",
        }},
        {"09106", {
            "VUID-vkCmdCopyBufferToImage-bufferRowLength-09106",
            "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09106",
            "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09106",
            "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09106",
            "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09106",
            "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09106",
        }},
        {"09107", {
            "VUID-vkCmdCopyBufferToImage-bufferImageHeight-09107",
            "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-09107",
            "VUID-VkCopyBufferToImageInfo2-bufferImageHeight-09107",
            "VUID-VkCopyImageToBufferInfo2-bufferImageHeight-09107",
            "VUID-VkCopyMemoryToImageInfoEXT-memoryImageHeight-09107",
            "VUID-VkCopyImageToMemoryInfoEXT-memoryImageHeight-09107",
        }},
        {"07274", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07274",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07274",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07274",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07274",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07274",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07274",
        }},
        {"07275", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07275",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07275",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07275",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07275",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07275",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07275",
        }},
        {"07276", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07276",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07276",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07276",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07276",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07276",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07276",
        }},
        {"09108", {
            "VUID-vkCmdCopyBufferToImage-bufferRowLength-09108",
            "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09108",
            "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09108",
            "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09108",
            "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09108",
            "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09108",
        }},
        {"00207", {
            "VUID-vkCmdCopyBufferToImage-dstImage-00207",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00207",
            "VUID-VkCopyBufferToImageInfo2-dstImage-00207",
            "VUID-VkCopyImageToBufferInfo2-srcImage-00207",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00207",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00207",
        }},
        {"00208", {
            "VUID-vkCmdCopyBufferToImage-dstImage-00208",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00208",
            "VUID-VkCopyBufferToImageInfo2-dstImage-00208",
            "VUID-VkCopyImageToBufferInfo2-srcImage-00208",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00208",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00208",
        }},
        {"00209", {
            "VUID-vkCmdCopyBufferToImage-dstImage-00209",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00209",
            "VUID-VkCopyBufferToImageInfo2-dstImage-00209",
            "VUID-VkCopyImageToBufferInfo2-srcImage-00209",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00209",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00209",
        }},
        {"09105", {
            "VUID-vkCmdCopyBufferToImage-imageSubresource-09105",
            "VUID-vkCmdCopyImageToBuffer-imageSubresource-09105",
            "VUID-VkCopyBufferToImageInfo2-imageSubresource-09105",
            "VUID-VkCopyImageToBufferInfo2-imageSubresource-09105",
            "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-09105",
            "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-09105",
        }},
        {"07981", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07981",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07981",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07981",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07981",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07981",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07981",
        }},
        {"07983", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07983",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07983",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07983",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07983",
            "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07983",
            "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07983",
        }},
        {"04052", {
            // was split up in 1.3.236 spec (internal MR 5371)
            "VUID-vkCmdCopyBufferToImage-commandBuffer-07737",
            "VUID-vkCmdCopyImageToBuffer-commandBuffer-07746",
            "VUID-vkCmdCopyBufferToImage2-commandBuffer-07737",
            "VUID-vkCmdCopyImageToBuffer2-commandBuffer-07746",
            kVUIDUndefined,
            kVUIDUndefined,
        }},
        {"07978", {
            "VUID-vkCmdCopyBufferToImage-dstImage-07978",
            "VUID-vkCmdCopyImageToBuffer-srcImage-07978",
            "VUID-VkCopyBufferToImageInfo2-dstImage-07978",
            "VUID-VkCopyImageToBufferInfo2-srcImage-07978",
            kVUIDUndefined,
            kVUIDUndefined,
        }}
    };
    // clang-format on

    uint8_t index = 0;
    if (is_memory) {
        index = 4 + (from_image ? 1 : 0);
    } else {
        index |= uint8_t((from_image) ? 0x1 : 0);
        index |= uint8_t((copy2) ? 0x2 : 0);
    }
    return copy_imagebuffermemory_vuid.at(id).at(index);
}

bool VerifyAspectsPresent(VkImageAspectFlags aspect_mask, VkFormat format) {
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

template <typename T>
uint32_t GetRowLength(T data) {
    return data.bufferRowLength;
}
template <>
uint32_t GetRowLength<VkMemoryToImageCopyEXT>(VkMemoryToImageCopyEXT data) {
    return data.memoryRowLength;
}
template <>
uint32_t GetRowLength<VkImageToMemoryCopyEXT>(VkImageToMemoryCopyEXT data) {
    return data.memoryRowLength;
}
template <typename T>
uint32_t GetImageHeight(T data) {
    return data.bufferImageHeight;
}
template <>
uint32_t GetImageHeight<VkMemoryToImageCopyEXT>(VkMemoryToImageCopyEXT data) {
    return data.memoryImageHeight;
}
template <>
uint32_t GetImageHeight<VkImageToMemoryCopyEXT>(VkImageToMemoryCopyEXT data) {
    return data.memoryImageHeight;
}
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateHeterogeneousCopyData(const HandleT handle, uint32_t regionCount, const RegionType *pRegions,
                                               const IMAGE_STATE &image_state, const Location &loc) const {
    bool skip = false;
    const bool is_2 = IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR});
    const bool from_image = IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer, Func::vkCmdCopyImageToBuffer2,
                                                     Func::vkCmdCopyImageToBuffer2KHR, Func::vkCopyImageToMemoryEXT});
    const bool is_memory = IsValueIn(loc.function, {Func::vkCopyMemoryToImageEXT, Func::vkCopyImageToMemoryEXT});

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((region.imageOffset.y != 0) || (region.imageExtent.height != 1)) {
                const LogObjectList objlist(handle, image_state.image());
                skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07979", from_image, is_2, is_memory), objlist, region_loc,
                                 "imageOffset.y is %" PRId32 " and imageExtent.height is %" PRIu32
                                 ". For 1D images these must be 0 "
                                 "and 1, respectively.",
                                 region.imageOffset.y, region.imageExtent.height);
            }
        }

        if ((image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) || (image_state.createInfo.imageType == VK_IMAGE_TYPE_2D)) {
            if ((region.imageOffset.z != 0) || (region.imageExtent.depth != 1)) {
                const LogObjectList objlist(handle, image_state.image());
                skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07980", from_image, is_2, is_memory), objlist, region_loc,
                                 "imageOffset.z is %" PRId32 " and imageExtent.depth is %" PRIu32
                                 ". For 1D and 2D images these "
                                 "must be 0 and 1, respectively.",
                                 region.imageOffset.z, region.imageExtent.depth);
            }
        }

        if (image_state.createInfo.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != region.imageSubresource.baseArrayLayer) || (1 != region.imageSubresource.layerCount)) {
                const LogObjectList objlist(handle, image_state.image());
                skip |=
                    LogError(GetBufferMemoryImageCopyCommandVUID("07983", from_image, is_2, is_memory), objlist, subresource_loc,
                             "baseArrayLayer is %" PRIu32 " and layerCount is %" PRIu32
                             ". "
                             "For 3D images these must be 0 and 1, respectively.",
                             region.imageSubresource.baseArrayLayer, region.imageSubresource.layerCount);
            }
        }

        // Make sure not a empty region
        if (region.imageExtent.width == 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("06659", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::width), "is zero (empty copies are not allowed).");
        }
        if (region.imageExtent.height == 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("06660", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::height), "is zero (empty copies are not allowed).");
        }
        if (region.imageExtent.depth == 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("06661", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::depth), "is zero (empty copies are not allowed).");
        }

        //  BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
        uint32_t row_length = GetRowLength(region);
        if ((row_length != 0) && (row_length < region.imageExtent.width)) {
            const LogObjectList objlist(handle, image_state.image());
            Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
            skip |=
                LogError(GetBufferMemoryImageCopyCommandVUID("09101", from_image, is_2, is_memory), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") must be zero or greater-than-or-equal-to imageExtent.width (%" PRIu32 ").", row_length,
                         region.imageExtent.width);
        }

         //  BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
        uint32_t image_height = GetImageHeight(region);
        if ((image_height != 0) && (image_height < region.imageExtent.height)) {
            const LogObjectList objlist(handle, image_state.image());
            Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
            skip |=
                LogError(GetBufferMemoryImageCopyCommandVUID("00196", from_image, is_2, is_memory), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") must be zero or greater-than-or-equal-to imageExtent.height (%" PRIu32 ").", image_height,
                         region.imageExtent.height);
        }

        // subresource aspectMask must have exactly 1 bit set
        if (GetBitSetCount(region_aspect_mask) != 1) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("09103", from_image, is_2, is_memory), objlist,
                             subresource_loc.dot(Field::aspectMask), "is %s (only one bit allowed).",
                             string_VkImageAspectFlags(region_aspect_mask).c_str());
        }

        // Calculate adjusted image extent, accounting for multiplane image factors
        VkExtent3D adjusted_image_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
        // imageOffset.x and (imageExtent.width + imageOffset.x) must both be >= 0 and <= image subresource width
        if ((region.imageOffset.x < 0) || (region.imageOffset.x > static_cast<int32_t>(adjusted_image_extent.width)) ||
            ((region.imageOffset.x + static_cast<int32_t>(region.imageExtent.width)) >
             static_cast<int32_t>(adjusted_image_extent.width))) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("00197", from_image, is_2, is_memory), objlist, region_loc,
                             "imageOffset.x (%" PRId32 ") and (imageExtent.width + imageOffset.x) (%" PRIu32
                             ") must be >= "
                             "zero or <= image subresource width (%" PRIu32 ").",
                             region.imageOffset.x, (region.imageOffset.x + region.imageExtent.width), adjusted_image_extent.width);
        }

        // imageOffset.y and (imageExtent.height + imageOffset.y) must both be >= 0 and <= image subresource height
        if ((region.imageOffset.y < 0) || (region.imageOffset.y > static_cast<int32_t>(adjusted_image_extent.height)) ||
            ((region.imageOffset.y + static_cast<int32_t>(region.imageExtent.height)) >
             static_cast<int32_t>(adjusted_image_extent.height))) {
            const LogObjectList objlist(handle, image_state.image());
            skip |=
                LogError(GetBufferMemoryImageCopyCommandVUID("00198", from_image, is_2, is_memory), objlist, region_loc,
                         "imageOffset.y (%" PRId32 ") and (imageExtent.height + imageOffset.y) (%" PRIu32
                         ") must be >= "
                         "zero or <= image subresource height (%" PRIu32 ").",
                         region.imageOffset.y, (region.imageOffset.y + region.imageExtent.height), adjusted_image_extent.height);
        }

        // imageOffset.z and (imageExtent.depth + imageOffset.z) must both be >= 0 and <= image subresource depth
        if ((region.imageOffset.z < 0) || (region.imageOffset.z > static_cast<int32_t>(adjusted_image_extent.depth)) ||
            ((region.imageOffset.z + static_cast<int32_t>(region.imageExtent.depth)) >
             static_cast<int32_t>(adjusted_image_extent.depth))) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("09104", from_image, is_2, is_memory), objlist, region_loc,
                             "imageOffset.z (%" PRId32 ") and (imageExtent.depth + imageOffset.z) (%" PRIu32
                             ") must be >= "
                             "zero or <= image subresource depth (%" PRIu32 ").",
                             region.imageOffset.z, (region.imageOffset.z + region.imageExtent.depth), adjusted_image_extent.depth);
        }

        const VkFormat image_format = image_state.createInfo.format;
        // image subresource aspect bit must match format
        if (!VerifyAspectsPresent(region_aspect_mask, image_format)) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("09105", from_image, is_2, is_memory), objlist,
                             subresource_loc.dot(Field::aspectMask), "%s invalid for image format %s.",
                             string_VkImageAspectFlags(region_aspect_mask).c_str(), string_VkFormat(image_format));
        }

        auto block_size = vkuFormatTexelBlockExtent(image_format);
        //  BufferRowLength must be a multiple of block width
        if (SafeModulo(row_length, block_size.width) != 0) {
            const LogObjectList objlist(handle, image_state.image());
            Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
            skip |= LogError(
                GetBufferMemoryImageCopyCommandVUID("09106", from_image, is_2, is_memory), objlist, region_loc.dot(field),
                "(%" PRIu32 ") must be a multiple of the blocked image's texel width (%" PRIu32 ").", row_length, block_size.width);
        }

         //  BufferRowHeight must be a multiple of block height
        if (SafeModulo(image_height, block_size.height) != 0) {
            const LogObjectList objlist(handle, image_state.image());
            Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
            skip |=
                LogError(GetBufferMemoryImageCopyCommandVUID("09107", from_image, is_2, is_memory), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") must be a multiple of the blocked image's texel height (%" PRIu32 ").", image_height,
                         block_size.height);
        }

        //  image offsets x must be multiple of block width
        if (SafeModulo(region.imageOffset.x, block_size.width) != 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07274", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::x),
                             "(%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "width (%" PRIu32 ").",
                             region.imageOffset.x, block_size.width);
        }

        //  image offsets y must be multiple of block height
        if (SafeModulo(region.imageOffset.y, block_size.height) != 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07275", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::y),
                             "(%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "height (%" PRIu32 ").",
                             region.imageOffset.y, block_size.height);
        }

        //  image offsets z must be multiple of block depth
        if (SafeModulo(region.imageOffset.z, block_size.depth) != 0) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07276", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z),
                             "(%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "depth (%" PRIu32 ").",
                             region.imageOffset.z, block_size.depth);
        }

        // imageExtent width must be a multiple of block width, or extent+offset width must equal subresource width
        VkExtent3D mip_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
        if ((SafeModulo(region.imageExtent.width, block_size.width) != 0) &&
            (region.imageExtent.width + region.imageOffset.x != mip_extent.width)) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("00207", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::width),
                             "(%" PRIu32
                             ") must be a multiple of the blocked texture block width "
                             "(%" PRIu32 "), or when added to imageOffset.x (%" PRId32
                             ") must equal the image subresource width (%" PRIu32 ").",
                             region.imageExtent.width, block_size.width, region.imageOffset.x, mip_extent.width);
        }

        // imageExtent height must be a multiple of block height, or extent+offset height must equal subresource height
        if ((SafeModulo(region.imageExtent.height, block_size.height) != 0) &&
            (region.imageExtent.height + region.imageOffset.y != mip_extent.height)) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("00208", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::height),
                             "(%" PRIu32
                             ") must be a multiple of the blocked texture block height "
                             "(%" PRIu32 "), or when added to imageOffset.y (%" PRId32
                             ") must equal the image subresource height (%" PRIu32 ").",
                             region.imageExtent.height, block_size.height, region.imageOffset.y, mip_extent.height);
        }

        // imageExtent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
        if ((SafeModulo(region.imageExtent.depth, block_size.depth) != 0) &&
            (region.imageExtent.depth + region.imageOffset.z != mip_extent.depth)) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("00209", from_image, is_2, is_memory), objlist,
                             region_loc.dot(Field::imageExtent).dot(Field::depth),
                             "(%" PRIu32
                             ") must be a multiple of the blocked texture block depth "
                             "(%" PRIu32 "), or when added to imageOffset.z (%" PRId32
                             ") must equal the image subresource depth (%" PRIu32 ").",
                             region.imageExtent.depth, block_size.depth, region.imageOffset.z, mip_extent.depth);
        }

        // *RowLength divided by the texel block extent width and then multiplied by the texel block size of the image must be
        // less than or equal to 2^31-1
        const uint32_t element_size =
            vkuFormatIsDepthOrStencil(image_format) ? 0 : vkuFormatElementSizeWithAspect(image_format, static_cast<VkImageAspectFlagBits>(region_aspect_mask));
        double test_value = row_length / block_size.width;
        test_value = test_value * element_size;
        const auto two_to_31_minus_1 = static_cast<double>((1u << 31) - 1);
        if (test_value > two_to_31_minus_1) {
            const LogObjectList objlist(handle, image_state.image());
            Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
            skip |=
                LogError(GetBufferMemoryImageCopyCommandVUID("09108", from_image, is_2, is_memory), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") divided by the texel block extent width (%" PRIu32
                         ") then multiplied by the "
                         "texel block size of image (%" PRIu32 ") is (%" PRIu64 ") which is greater than 2^31 - 1",
                         row_length, block_size.width, element_size, static_cast<uint64_t>(test_value));
        }

        // Checks that apply only to multi-planar format images
        if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, region_aspect_mask)) {
            const LogObjectList objlist(handle, image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07981", from_image, is_2, is_memory), objlist,
                             subresource_loc.dot(Field::aspectMask), "(%s) is invalid for multi-planar format %s.",
                             string_VkImageAspectFlags(region_aspect_mask).c_str(), string_VkFormat(image_format));
        }
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferImageCopyData(const CMD_BUFFER_STATE &cb_state, uint32_t regionCount, const RegionType *pRegions,
                                             const IMAGE_STATE &image_state, const Location &loc) const {
    bool skip = false;

    const bool is_2 = IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR});
    const bool image_to_buffer =
        IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer, Func::vkCmdCopyImageToBuffer2, Func::vkCmdCopyImageToBuffer2KHR});

    const VkFormat image_format = image_state.createInfo.format;

    skip |= ValidateHeterogeneousCopyData(cb_state.commandBuffer(), regionCount, pRegions, image_state, loc);

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;

        // If the the calling command's VkImage parameter's format is not a depth/stencil format,
        // then bufferOffset must be a multiple of the calling command's VkImage parameter's element size
        const uint32_t element_size =
            vkuFormatIsDepthOrStencil(image_format) ? 0 : vkuFormatElementSizeWithAspect(image_format, static_cast<VkImageAspectFlagBits>(region_aspect_mask));
        const VkDeviceSize bufferOffset = region.bufferOffset;

        if (vkuFormatIsDepthOrStencil(image_format)) {
            if (SafeModulo(bufferOffset, 4) != 0) {
                const LogObjectList objlist(cb_state.commandBuffer(), image_state.image());
                skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07978", image_to_buffer, is_2), objlist,
                                 region_loc.dot(Field::bufferOffset),
                                 "(%" PRIu64 ") must be a multiple 4 if using a depth/stencil format (%s).", bufferOffset,
                                 string_VkFormat(image_format));
            }
        } else {
            // If not depth/stencil and not multi-plane
            if (!vkuFormatIsMultiplane(image_format) && (SafeModulo(bufferOffset, element_size) != 0)) {
                const LogObjectList objlist(cb_state.commandBuffer(), image_state.image());
                skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07975", image_to_buffer, is_2), objlist,
                                 region_loc.dot(Field::bufferOffset),
                                 "(%" PRIu64 ") must be a multiple of %s texel size (%" PRIu32 ").", bufferOffset,
                                 string_VkFormat(image_format), element_size);
            }
        }

        // Checks that apply only to multi-planar format images
        if (vkuFormatIsMultiplane(image_format)) {

            // image subresource aspectMask must be VK_IMAGE_ASPECT_PLANE_*_BIT
            if (0 !=
                (region_aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
                // Know aspect mask is valid
                const VkFormat compatible_format = vkuFindMultiplaneCompatibleFormat(image_format, static_cast<VkImageAspectFlagBits>(region_aspect_mask));
                const uint32_t compatible_size = vkuFormatElementSize(compatible_format);
                if (SafeModulo(bufferOffset, compatible_size) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), image_state.image());
                    skip |= LogError(GetBufferMemoryImageCopyCommandVUID("07976", image_to_buffer, is_2), objlist,
                                     region_loc.dot(Field::bufferOffset),
                                     "(%" PRIu64 ") is not a multiple of %s texel size (%" PRIu32 ") for plane %" PRIu32 " (%s).",
                                     bufferOffset, string_VkFormat(image_format), element_size, vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(region_aspect_mask)),
                                     string_VkFormat(compatible_format));
                }
            }
        }

        // TODO - Don't use ValidateCmdQueueFlags due to currently not having way to add more descriptive message
        const COMMAND_POOL_STATE *command_pool = cb_state.command_pool;
        assert(command_pool != nullptr);
        const uint32_t queue_family_index = command_pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        if (((queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == 0) && (SafeModulo(bufferOffset, 4) != 0)) {
            const LogObjectList objlist(cb_state.commandBuffer(), command_pool->commandPool(), image_state.image());
            skip |= LogError(GetBufferMemoryImageCopyCommandVUID("04052", image_to_buffer, is_2), objlist,
                             region_loc.dot(Field::bufferOffset),
                             "(%" PRIu64
                             ") is not a multiple of 4 because, but the command buffer %s was allocated from the command pool %s "
                             "which was created with queueFamilyIndex %" PRIu32 " of type %s.",
                             bufferOffset, FormatHandle(cb_state).c_str(), FormatHandle(command_pool->commandPool()).c_str(),
                             queue_family_index, string_VkQueueFlags(queue_flags).c_str());
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferBounds(VkCommandBuffer cb, const BUFFER_STATE &src_buffer_state,
                                             const BUFFER_STATE &dst_buffer_state, uint32_t regionCount, const RegionType *pRegions,
                                             const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid;

    VkDeviceSize src_buffer_size = src_buffer_state.createInfo.size;
    VkDeviceSize dst_buffer_size = dst_buffer_state.createInfo.size;
    const bool are_buffers_sparse = src_buffer_state.sparse || dst_buffer_state.sparse;

    const LogObjectList src_objlist(cb, dst_buffer_state.Handle());
    const LogObjectList dst_objlist(cb, dst_buffer_state.Handle());
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];

        // The srcOffset member of each element of pRegions must be less than the size of srcBuffer
        if (region.srcOffset >= src_buffer_size) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcOffset-00113" : "VUID-vkCmdCopyBuffer-srcOffset-00113";
            skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset),
                             "(%" PRIuLEAST64 ") is greater than size of srcBuffer (%" PRIuLEAST64 ").", region.srcOffset,
                             src_buffer_size);
        }

        // The dstOffset member of each element of pRegions must be less than the size of dstBuffer
        if (region.dstOffset >= dst_buffer_size) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstOffset-00114" : "VUID-vkCmdCopyBuffer-dstOffset-00114";
            skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset),
                             "(%" PRIuLEAST64 ") is greater than size of dstBuffer (%" PRIuLEAST64 ").", region.dstOffset,
                             dst_buffer_size);
        }

        // The size member of each element of pRegions must be less than or equal to the size of srcBuffer minus srcOffset
        if (region.size > (src_buffer_size - region.srcOffset)) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00115" : "VUID-vkCmdCopyBuffer-size-00115";
            skip |= LogError(vuid, src_objlist, region_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") is greater than the source buffer size (%" PRIuLEAST64
                             ") minus srcOffset (%" PRIuLEAST64 ").",
                             region.size, src_buffer_size, region.srcOffset);
        }

        // The size member of each element of pRegions must be less than or equal to the size of dstBuffer minus dstOffset
        if (region.size > (dst_buffer_size - region.dstOffset)) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00116" : "VUID-vkCmdCopyBuffer-size-00116";
            skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") is greater than the destination buffer size (%" PRIuLEAST64
                             ") minus dstOffset (%" PRIuLEAST64 ").",
                             region.size, dst_buffer_size, region.dstOffset);
        }

        // The union of the source regions, and the union of the destination regions, must not overlap in memory
        if (!skip && !are_buffers_sparse) {
            auto src_region = sparse_container::range<VkDeviceSize>{region.srcOffset, region.srcOffset + region.size};
            for (uint32_t j = 0; j < regionCount; j++) {
                auto dst_region =
                    sparse_container::range<VkDeviceSize>{pRegions[j].dstOffset, pRegions[j].dstOffset + pRegions[j].size};
                if (src_buffer_state.DoesResourceMemoryOverlap(src_region, &dst_buffer_state, dst_region)) {
                    const LogObjectList objlist(cb, src_buffer_state.Handle(), dst_buffer_state.Handle());
                    vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";
                    skip |= LogError(vuid, objlist, region_loc, "Detected overlap between source and dest regions in memory.");
                }
            }
        }
    }

    return skip;
}
template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                       const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !src_buffer_state || !dst_buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid;
    const Location src_buffer_loc = loc.dot(Field::srcBuffer);
    const Location dst_buffer_loc = loc.dot(Field::dstBuffer);

    vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00119" : "VUID-vkCmdCopyBuffer-srcBuffer-00119";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00121" : "VUID-vkCmdCopyBuffer-dstBuffer-00121";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

    // Validate that SRC & DST buffers have correct usage flags set
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00118" : "VUID-vkCmdCopyBuffer-srcBuffer-00118";
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, srcBuffer), *src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     true, vuid, src_buffer_loc);
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00120" : "VUID-vkCmdCopyBuffer-dstBuffer-00120";
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, dstBuffer), *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                     true, vuid, dst_buffer_loc);

    skip |= ValidateCmd(cb_state, loc);
    skip |= ValidateCmdCopyBufferBounds(commandBuffer, *src_buffer_state, *dst_buffer_state, regionCount, pRegions, loc);

    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01822" : "VUID-vkCmdCopyBuffer-commandBuffer-01822";
    skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01823" : "VUID-vkCmdCopyBuffer-commandBuffer-01823";
    skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01824" : "VUID-vkCmdCopyBuffer-commandBuffer-01824";
    skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);

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
bool CoreChecks::ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE &cb_state,
                                                                        const IMAGE_STATE &image_state, const RegionType *region,
                                                                        const Location &region_loc, const char *vuid) const {
    bool skip = false;
    const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
    VkExtent3D granularity = GetScaledItg(cb_state, image_state);
    skip |= CheckItgOffset(objlist, region->imageOffset, granularity, region_loc.dot(Field::imageOffset), vuid);
    VkExtent3D subresource_extent = image_state.GetEffectiveSubresourceExtent(region->imageSubresource);
    skip |= CheckItgExtent(objlist, region->imageExtent, region->imageOffset, granularity, subresource_extent,
                           image_state.createInfo.imageType, region_loc.dot(Field::imageExtent), vuid);
    return skip;
}

template <typename HandleT>
bool CoreChecks::ValidateImageSubresourceLayers(HandleT handle, const VkImageSubresourceLayers *subresource_layers,
                                                const Location &subresource_loc) const {
    bool skip = false;
    const VkImageAspectFlags aspect_mask = subresource_layers->aspectMask;
    if (subresource_layers->layerCount == VK_REMAINING_ARRAY_LAYERS) {
        if (!enabled_features.maintenance5) {
            skip |= LogError("VUID-VkImageSubresourceLayers-layerCount-09243", handle, subresource_loc.dot(Field::layerCount),
                             "is VK_REMAINING_ARRAY_LAYERS.");
        }
    } else if (subresource_layers->layerCount == 0) {
        skip |=
            LogError("VUID-VkImageSubresourceLayers-layerCount-01700", handle, subresource_loc.dot(Field::layerCount), "is zero.");
    }
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
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy/VkImageCopy2KHR structure
template <typename RegionType>
bool CoreChecks::ValidateCopyImageTransferGranularityRequirements(const CMD_BUFFER_STATE &cb_state,
                                                                  const IMAGE_STATE &src_image_state,
                                                                  const IMAGE_STATE &dst_image_state, const RegionType *region,
                                                                  const Location &region_loc) const {
    bool skip = false;
    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const char *vuid;

    const VkExtent3D extent = region->extent;
    {
        // Source image checks
        const LogObjectList objlist(cb_state.Handle(), src_image_state.Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, src_image_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= CheckItgOffset(objlist, region->srcOffset, granularity, region_loc.dot(Field::srcOffset), vuid);
        const VkExtent3D subresource_extent = src_image_state.GetEffectiveSubresourceExtent(region->srcSubresource);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= CheckItgExtent(objlist, extent, region->srcOffset, granularity, subresource_extent,
                               src_image_state.createInfo.imageType, region_loc.dot(Field::extent), vuid);
    }

    {
        // Destination image checks
        const LogObjectList objlist(cb_state.Handle(), dst_image_state.Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, dst_image_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= CheckItgOffset(objlist, region->dstOffset, granularity, region_loc.dot(Field::dstOffset), vuid);
        // Adjust dest extent, if necessary
        const VkExtent3D dest_effective_extent =
            GetAdjustedDestImageExtent(src_image_state.createInfo.format, dst_image_state.createInfo.format, extent);
        const VkExtent3D subresource_extent = dst_image_state.GetEffectiveSubresourceExtent(region->dstSubresource);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= CheckItgExtent(objlist, dest_effective_extent, region->dstOffset, granularity, subresource_extent,
                               dst_image_state.createInfo.imageType, region_loc.dot(Field::extent), vuid);
    }
    return skip;
}

static const char *GetImageCopyVUID(const std::string &id, bool copy2, bool host_version) {
    // clang-format off
    static const std::map<std::string, std::array<const char *, 3>> copy_image_vuid = {
        {"00146", {
            "VUID-vkCmdCopyImage-srcImage-00146",            // !copy2 & !host_version
            "VUID-VkCopyImageInfo2-srcImage-00146",          // copy2 && !host_version
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07979", // host_version
        }},
        {"01785", {
            "VUID-vkCmdCopyImage-srcImage-01785",
            "VUID-VkCopyImageInfo2-srcImage-01785",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07980",
        }},
        {"07278", {
            "VUID-vkCmdCopyImage-pRegions-07278",
            "VUID-VkCopyImageInfo2-pRegions-07278",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07274",
        }},
        {"07279", {
            "VUID-vkCmdCopyImage-pRegions-07279",
            "VUID-VkCopyImageInfo2-pRegions-07279",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07275",
        }},
        {"07280", {
            "VUID-vkCmdCopyImage-pRegions-07280",
            "VUID-VkCopyImageInfo2-pRegions-07280",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07276",
        }},
        {"01728", {
            "VUID-vkCmdCopyImage-srcImage-01728",
            "VUID-VkCopyImageInfo2-srcImage-01728",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-00207",
        }},
        {"01729", {
            "VUID-vkCmdCopyImage-srcImage-01729",
            "VUID-VkCopyImageInfo2-srcImage-01729",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-00208",
        }},
        {"01730", {
            "VUID-vkCmdCopyImage-srcImage-01730",
            "VUID-VkCopyImageInfo2-srcImage-01730",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-00209",
        }},
        {"00152", {
            "VUID-vkCmdCopyImage-dstImage-00152",
            "VUID-VkCopyImageInfo2-dstImage-00152",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07979",
        }},
        {"01786", {
            "VUID-vkCmdCopyImage-dstImage-01786",
            "VUID-VkCopyImageInfo2-dstImage-01786",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07980",
        }},
        {"04443", {
            "VUID-vkCmdCopyImage-srcImage-04443",
            "VUID-VkCopyImageInfo2-srcImage-04443",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07983",
       }},
       {"04444", {
            "VUID-vkCmdCopyImage-dstImage-04444",
            "VUID-VkCopyImageInfo2-dstImage-04444",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07983",
       }},
       {"07281", {
            "VUID-vkCmdCopyImage-pRegions-07281",
            "VUID-VkCopyImageInfo2-pRegions-07281",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07274",
       }},
       {"07282", {
            "VUID-vkCmdCopyImage-pRegions-07282",
            "VUID-VkCopyImageInfo2-pRegions-07282",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07275",
       }},
       {"07283", {
            "VUID-vkCmdCopyImage-pRegions-07283",
            "VUID-VkCopyImageInfo2-pRegions-07283",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07276",
       }},
       {"01732", {
            "VUID-vkCmdCopyImage-dstImage-01732",
            "VUID-VkCopyImageInfo2-dstImage-01732",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-00207",
       }},
       {"01733", {
            "VUID-vkCmdCopyImage-dstImage-01733",
            "VUID-VkCopyImageInfo2-dstImage-01733",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-00208",
       }},
       {"01734", {
            "VUID-vkCmdCopyImage-dstImage-01734",
            "VUID-VkCopyImageInfo2-dstImage-01734",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-00209",
       }},
       {"07967src", {
            "VUID-vkCmdCopyImage-srcSubresource-07967",
            "VUID-VkCopyImageInfo2-srcSubresource-07967",
            "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07967",
       }},
       {"07967dst", {
            "VUID-vkCmdCopyImage-dstSubresource-07967",
            "VUID-VkCopyImageInfo2-dstSubresource-07967",
            "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07967",
       }},
       {"07968src", {
            "VUID-vkCmdCopyImage-srcSubresource-07968",
            "VUID-VkCopyImageInfo2-srcSubresource-07968",
            "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07968",
       }},
       {"07968dst", {
            "VUID-vkCmdCopyImage-dstSubresource-07968",
            "VUID-VkCopyImageInfo2-dstSubresource-07968",
            "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07968",
       }},
       {"00142", {
            "VUID-vkCmdCopyImage-aspectMask-00142",
            "VUID-VkCopyImageInfo2-aspectMask-00142",
            "VUID-VkCopyImageToImageInfoEXT-srcSubresource-09105",
       }},
       {"00143", {
            "VUID-vkCmdCopyImage-aspectMask-00143",
            "VUID-VkCopyImageInfo2-aspectMask-00143",
            "VUID-VkCopyImageToImageInfoEXT-dstSubresource-09105",
       }},
       {"00144", {
            "VUID-vkCmdCopyImage-srcOffset-00144",
            "VUID-VkCopyImageInfo2-srcOffset-00144",
            "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07971",
       }},
       {"00145", {
            "VUID-vkCmdCopyImage-srcOffset-00145",
            "VUID-VkCopyImageInfo2-srcOffset-00145",
            "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07972",
       }},
       {"00147", {
            "VUID-vkCmdCopyImage-srcOffset-00147",
            "VUID-VkCopyImageInfo2-srcOffset-00147",
            "VUID-VkCopyImageToImageInfoEXT-srcOffset-09104",
       }},
       {"00150", {
            "VUID-vkCmdCopyImage-dstOffset-00150",
            "VUID-VkCopyImageInfo2-dstOffset-00150",
            "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07971",
       }},
       {"00151", {
            "VUID-vkCmdCopyImage-dstOffset-00151",
            "VUID-VkCopyImageInfo2-dstOffset-00151",
            "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07972",
       }},
       {"00153", {
            "VUID-vkCmdCopyImage-dstOffset-00153",
            "VUID-VkCopyImageInfo2-dstOffset-00153",
            "VUID-VkCopyImageToImageInfoEXT-dstOffset-09104",
       }},
       {"07966src", {
            "VUID-vkCmdCopyImage-srcImage-07966",
            "VUID-VkCopyImageInfo2-srcImage-07966",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07966",
       }},
       {"07966dst", {
            "VUID-vkCmdCopyImage-dstImage-07966",
            "VUID-VkCopyImageInfo2-dstImage-07966",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07966",
       }},
       {"07969src", {
            "VUID-vkCmdCopyImage-srcImage-07969",
            "VUID-VkCopyImageInfo2-srcImage-07969",
            "VUID-VkCopyImageToImageInfoEXT-srcImage-07969",
       }},
       {"07969dst", {
            "VUID-vkCmdCopyImage-dstImage-07969",
            "VUID-VkCopyImageInfo2-dstImage-07969",
            "VUID-VkCopyImageToImageInfoEXT-dstImage-07969",
       }},
    };
    // clang-format on
    uint8_t index = 0;

    if (host_version) {
        index = 2;
    } else if (copy2) {
        index = 1;
    }
    return copy_image_vuid.at(id).at(index);
}

// Validate contents of a VkImageCopy or VkImageCopy2KHR struct
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateImageCopyData(const HandleT handle, const uint32_t regionCount, const RegionType *pRegions,
                                       const IMAGE_STATE &src_image_state, const IMAGE_STATE &dst_image_state, bool is_host,
                                       const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdCopyImage2 || loc.function == Func::vkCmdCopyImage2KHR;
    const char *vuid;

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];

        // For comp<->uncomp copies, the copy extent for the dest image must be adjusted
        const VkExtent3D src_copy_extent = region.extent;
        const VkExtent3D dst_copy_extent =
            GetAdjustedDestImageExtent(src_image_state.createInfo.format, dst_image_state.createInfo.format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_image_state.createInfo.imageType) &&
            (VK_IMAGE_TYPE_3D != dst_image_state.createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_image_state.createInfo.imageType) &&
                   (VK_IMAGE_TYPE_3D != src_image_state.createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        // Do all checks on source image
        if (src_image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.srcOffset.y) || (1 != src_copy_extent.height)) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("00146", is_2, is_host), objlist, region_loc,
                                 "srcOffset.y is %" PRId32 " and extent.height is %" PRIu32
                                 ". For 1D images these must "
                                 "be 0 and 1, respectively.",
                                 region.srcOffset.y, src_copy_extent.height);
            }
        }

        if (((src_image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) ||
            ((src_image_state.createInfo.imageType == VK_IMAGE_TYPE_2D) && is_host)) &&
            ((0 != region.srcOffset.z) || (1 != src_copy_extent.depth))) {
            const LogObjectList objlist(handle, src_image_state.image());
            const char *image_type = is_host ? "1D or 2D" : "1D";
            skip |= LogError(GetImageCopyVUID("01785", is_2, is_host), objlist, region_loc,
                             "srcOffset.z is %" PRId32 " and extent.depth is %" PRIu32
                             ". For %s images "
                             "these must be 0 and 1, respectively.",
                             region.srcOffset.z, src_copy_extent.depth, image_type);
        }

        if ((src_image_state.createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.srcOffset.z) && (!is_host)) {
            const LogObjectList objlist(handle, src_image_state.image());
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01787" : "VUID-vkCmdCopyImage-srcImage-01787";
            skip |= LogError(vuid, objlist, region_loc, "srcOffset.z is %" PRId32 ". For 2D images the z-offset must be 0.",
                             region.srcOffset.z);
        }

        {  // Used to be compressed checks, now apply to all
            const VkExtent3D block_size = vkuFormatTexelBlockExtent(src_image_state.createInfo.format);
            if (SafeModulo(region.srcOffset.x, block_size.width) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07278", is_2, is_host), objlist, region_loc,
                                 "srcOffset.x (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "width (%" PRIu32 ").",
                                 region.srcOffset.x, block_size.width);
            }

            //  image offsets y must be multiple of block height
            if (SafeModulo(region.srcOffset.y, block_size.height) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07279", is_2, is_host), objlist, region_loc,
                                 "srcOffset.y (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "height (%" PRIu32 ").",
                                 region.srcOffset.y, block_size.height);
            }

            //  image offsets z must be multiple of block depth
            if (SafeModulo(region.srcOffset.z, block_size.depth) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07280", is_2, is_host), objlist, region_loc,
                                 "srcOffset.z (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "depth (%" PRIu32 ").",
                                 region.srcOffset.z, block_size.depth);
            }

            const VkExtent3D mip_extent = src_image_state.GetEffectiveSubresourceExtent(region.srcSubresource);
            if ((SafeModulo(src_copy_extent.width, block_size.width) != 0) &&
                (src_copy_extent.width + region.srcOffset.x != mip_extent.width)) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("01728", is_2, is_host), objlist, region_loc,
                                 "extent width (%" PRIu32
                                 ") must be a multiple of the blocked texture block "
                                 "width (%" PRIu32 "), or when added to srcOffset.x (%" PRId32
                                 ") must equal the image subresource width (%" PRIu32 ").",
                                 src_copy_extent.width, block_size.width, region.srcOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(src_copy_extent.height, block_size.height) != 0) &&
                (src_copy_extent.height + region.srcOffset.y != mip_extent.height)) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("01729", is_2, is_host), objlist, region_loc,
                                 "extent height (%" PRIu32
                                 ") must be a multiple of the compressed texture block "
                                 "height (%" PRIu32 "), or when added to srcOffset.y (%" PRId32
                                 ") must equal the image subresource height (%" PRIu32 ").",
                                 src_copy_extent.height, block_size.height, region.srcOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : src_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.srcOffset.z != mip_extent.depth)) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("01730", is_2, is_host), objlist, region_loc,
                                 "extent width (%" PRIu32
                                 ") must be a multiple of the compressed texture block "
                                 "depth (%" PRIu32 "), or when added to srcOffset.z (%" PRId32
                                 ") must equal the image subresource depth (%" PRIu32 ").",
                                 src_copy_extent.depth, block_size.depth, region.srcOffset.z, mip_extent.depth);
            }
        }

        // Do all checks on dest image
        if (dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.dstOffset.y) || (1 != dst_copy_extent.height)) {
                const LogObjectList objlist(handle, dst_image_state.image());
                skip |= LogError(GetImageCopyVUID("00152", is_2, is_host), objlist, region_loc,
                                 "dstOffset.y is %" PRId32 " and dst_copy_extent.height is %" PRIu32
                                 ". For 1D images "
                                 "these must be 0 and 1, respectively.",
                                 region.dstOffset.y, dst_copy_extent.height);
            }
        }

        if (((dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_1D) ||
             ((dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_2D) && is_host)) &&
            ((0 != region.dstOffset.z) || (1 != dst_copy_extent.depth))) {
            const LogObjectList objlist(handle, dst_image_state.image());
            const char *image_type = is_host ? "1D or 2D" : "1D";
            skip |= LogError(GetImageCopyVUID("01786", is_2, is_host), objlist, region_loc,
                             "dstOffset.z is %" PRId32 " and extent.depth is %" PRIu32
                             ". For %s images these must be 0 "
                             "and 1, respectively.",
                             region.dstOffset.z, dst_copy_extent.depth, image_type);
        }

        if ((dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.dstOffset.z) && !(is_host)) {
            const LogObjectList objlist(handle, dst_image_state.image());
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01788" : "VUID-vkCmdCopyImage-dstImage-01788";
            skip |= LogError(vuid, objlist, region_loc, "dstOffset.z is %" PRId32 ". For 2D images the z-offset must be 0.",
                             region.dstOffset.z);
        }

        // Handle difference between Maintenance 1
        if (IsExtEnabled(device_extensions.vk_khr_maintenance1) || is_host) {
            if (src_image_state.createInfo.imageType == VK_IMAGE_TYPE_3D) {
                const LogObjectList objlist(handle, src_image_state.image());
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    skip |= LogError(GetImageCopyVUID("04443", is_2, is_host), objlist, region_loc,
                                     "srcSubresource.baseArrayLayer is %" PRIu32
                                     " and srcSubresource.layerCount "
                                     "is %" PRIu32 ". For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
            }
            if (dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_3D) {
                const LogObjectList objlist(handle, dst_image_state.image());
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    skip |= LogError(GetImageCopyVUID("04444", is_2, is_host), objlist, region_loc,
                                     "dstSubresource.baseArrayLayer is %" PRIu32
                                     " and dstSubresource.layerCount "
                                     "is %" PRIu32 ". For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        } else {  // Pre maint 1
            if (src_image_state.createInfo.imageType == VK_IMAGE_TYPE_3D ||
                dst_image_state.createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    const LogObjectList objlist(handle, src_image_state.image());
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                    skip |= LogError(vuid, objlist, region_loc,
                                     "srcSubresource.baseArrayLayer is %" PRIu32
                                     " and "
                                     "srcSubresource.layerCount is %" PRIu32
                                     ". For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    const LogObjectList objlist(handle, dst_image_state.image());
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                    skip |= LogError(vuid, objlist, region_loc,
                                     "dstSubresource.baseArrayLayer is %" PRIu32
                                     " and "
                                     "dstSubresource.layerCount is %" PRIu32
                                     ". For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        }

        {
            const VkExtent3D block_size = vkuFormatTexelBlockExtent(dst_image_state.createInfo.format);
            //  image offsets x must be multiple of block width
            if (SafeModulo(region.dstOffset.x, block_size.width) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07281", is_2, is_host), objlist, region_loc,
                                 "srcOffset.x (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "width (%" PRIu32 ").",
                                 region.dstOffset.x, block_size.width);
            }

            //  image offsets y must be multiple of block height
            if (SafeModulo(region.dstOffset.y, block_size.height) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07282", is_2, is_host), objlist, region_loc,
                                 "srcOffset.y (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "height (%" PRIu32 ").",
                                 region.dstOffset.y, block_size.height);
            }

            //  image offsets z must be multiple of block depth
            if (SafeModulo(region.dstOffset.z, block_size.depth) != 0) {
                const LogObjectList objlist(handle, src_image_state.image());
                skip |= LogError(GetImageCopyVUID("07283", is_2, is_host), objlist, region_loc,
                                 "srcOffset.z (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "depth (%" PRIu32 ").",
                                 region.dstOffset.z, block_size.depth);
            }

            const VkExtent3D mip_extent = dst_image_state.GetEffectiveSubresourceExtent(region.dstSubresource);
            if ((SafeModulo(dst_copy_extent.width, block_size.width) != 0) &&
                (dst_copy_extent.width + region.dstOffset.x != mip_extent.width)) {
                const LogObjectList objlist(handle, dst_image_state.image());
                skip |= LogError(GetImageCopyVUID("01732", is_2, is_host), objlist, region_loc,
                                 "dst_copy_extent width (%" PRIu32
                                 ") must be a multiple of the blocked texture "
                                 "block width (%" PRIu32 "), or when added to dstOffset.x (%" PRId32
                                 ") must equal the image subresource width (%" PRIu32 ").",
                                 dst_copy_extent.width, block_size.width, region.dstOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or dst_copy_extent+offset height must equal subresource height
            if ((SafeModulo(dst_copy_extent.height, block_size.height) != 0) &&
                (dst_copy_extent.height + region.dstOffset.y != mip_extent.height)) {
                const LogObjectList objlist(handle, dst_image_state.image());
                skip |= LogError(GetImageCopyVUID("01733", is_2, is_host), objlist, region_loc,
                                 "dst_copy_extent height (%" PRIu32
                                 ") must be a multiple of the compressed "
                                 "texture block height (%" PRIu32 "), or when added to dstOffset.y (%" PRId32
                                 ") must equal the image subresource "
                                 "height (%" PRIu32 ").",
                                 dst_copy_extent.height, block_size.height, region.dstOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or dst_copy_extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : dst_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.dstOffset.z != mip_extent.depth)) {
                const LogObjectList objlist(handle, dst_image_state.image());
                skip |= LogError(GetImageCopyVUID("01734", is_2, is_host), objlist, region_loc,
                                 "dst_copy_extent width (%" PRIu32
                                 ") must be a multiple of the compressed texture "
                                 "block depth (%" PRIu32 "), or when added to dstOffset.z (%" PRId32
                                 ") must equal the image subresource depth (%" PRIu32 ").",
                                 dst_copy_extent.depth, block_size.depth, region.dstOffset.z, mip_extent.depth);
            }
        }
    }
    return skip;
}

// Returns non-zero if offset and extent exceed image extents
static constexpr uint32_t kXBit = 1;
static constexpr uint32_t kYBit = 2;
static constexpr uint32_t kZBit = 4;
static uint32_t ExceedsBounds(const VkOffset3D *offset, const VkExtent3D *extent, const VkExtent3D *image_extent) {
    uint32_t result = 0;
    // Extents/depths cannot be negative but checks left in for clarity
    if ((offset->z + extent->depth > image_extent->depth) || (offset->z < 0) ||
        ((offset->z + static_cast<int32_t>(extent->depth)) < 0)) {
        result |= kZBit;
    }
    if ((offset->y + extent->height > image_extent->height) || (offset->y < 0) ||
        ((offset->y + static_cast<int32_t>(extent->height)) < 0)) {
        result |= kYBit;
    }
    if ((offset->x + extent->width > image_extent->width) || (offset->x < 0) ||
        ((offset->x + static_cast<int32_t>(extent->width)) < 0)) {
        result |= kXBit;
    }
    return result;
}
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateCopyImageCommon(HandleT handle, const IMAGE_STATE &src_image_state, const IMAGE_STATE &dst_image_state,
                                         uint32_t regionCount, const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdCopyImage2 || loc.function == Func::vkCmdCopyImage2KHR;
    const bool is_host = loc.function == Func::vkCopyImageToImageEXT;

    const VkFormat src_format = src_image_state.createInfo.format;
    const VkFormat dst_format = dst_image_state.createInfo.format;
    auto src_image = src_image_state.image();
    auto dst_image = dst_image_state.image();
    const bool src_is_3d = (VK_IMAGE_TYPE_3D == src_image_state.createInfo.imageType);
    const bool dst_is_3d = (VK_IMAGE_TYPE_3D == dst_image_state.createInfo.imageType);

    const LogObjectList src_objlist(handle, src_image);
    const LogObjectList dst_objlist(handle, dst_image);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];

        // For comp/uncomp copies, the copy extent for the dest image must be adjusted
        VkExtent3D src_copy_extent = region.extent;
        VkExtent3D dst_copy_extent = GetAdjustedDestImageExtent(src_format, dst_format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if (src_is_3d && !dst_is_3d) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if (dst_is_3d && !src_is_3d) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        skip |= ValidateImageSubresourceLayers(handle, &region.srcSubresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(handle, &region.dstSubresource, dst_subresource_loc);

        skip |= ValidateImageMipLevel(handle, src_image_state, region.srcSubresource.mipLevel,
                                      src_subresource_loc.dot(Field::mipLevel), GetImageCopyVUID("07967src", is_2, is_host));
        skip |= ValidateImageMipLevel(handle, dst_image_state, region.dstSubresource.mipLevel,
                                      dst_subresource_loc.dot(Field::mipLevel), GetImageCopyVUID("07967dst", is_2, is_host));
        skip |= ValidateImageArrayLayerRange(handle, src_image_state, region.srcSubresource.baseArrayLayer,
                                             region.srcSubresource.layerCount, src_subresource_loc,
                                             GetImageCopyVUID("07968src", is_2, is_host));
        skip |= ValidateImageArrayLayerRange(handle, dst_image_state, region.dstSubresource.baseArrayLayer,
                                             region.dstSubresource.layerCount, dst_subresource_loc,
                                             GetImageCopyVUID("07968dst", is_2, is_host));

        if (api_version < VK_API_VERSION_1_1) {
            if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
                // For each region the layerCount member of srcSubresource and dstSubresource must match
                if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                    const LogObjectList objlist(handle, src_image, dst_image);
                    const char *vuid =
                        (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07941" : "VUID-VkImageCopy-apiVersion-07941";
                    skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::layerCount),
                                     "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.srcSubresource.layerCount,
                                     dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.dstSubresource.layerCount);
                }
            }
            if (!IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                // For each region the aspectMask member of srcSubresource and dstSubresource must match
                if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                    const LogObjectList objlist(handle, src_image, dst_image);
                    const char *vuid =
                        (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07940" : "VUID-VkImageCopy-apiVersion-07940";
                    skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                                     string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(),
                                     dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                                     string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
                }
            }
        }

        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!VerifyAspectsPresent(region.srcSubresource.aspectMask, src_format)) {
            skip |= LogError(GetImageCopyVUID("00142", is_2, is_host), src_objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) cannot specify aspects not present in source image (%s).",
                             string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(), string_VkFormat(src_format));
        }
        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!VerifyAspectsPresent(region.dstSubresource.aspectMask, dst_format)) {
            skip |= LogError(GetImageCopyVUID("00143", is_2, is_host), dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                             "(%s) cannot specify aspects not present in destination image (%s).",
                             string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str(), string_VkFormat(dst_format));
        }

        // Make sure not a empty region
        if (src_copy_extent.width == 0) {
            const LogObjectList objlist(handle, src_image);
            const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06668" : "VUID-VkImageCopy-extent-06668";
            skip |= LogError(vuid, objlist, region_loc.dot(Field::extent).dot(Field::width),
                             "is zero. (empty copies are not allowed).");
        }
        if (src_copy_extent.height == 0) {
            const LogObjectList objlist(handle, src_image);
            const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06669" : "VUID-VkImageCopy-extent-06669";
            skip |= LogError(vuid, objlist, region_loc.dot(Field::extent).dot(Field::height),
                             "is zero. (empty copies are not allowed).");
        }
        if (src_copy_extent.depth == 0) {
            const LogObjectList objlist(handle, src_image);
            const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06670" : "VUID-VkImageCopy-extent-06670";
            skip |= LogError(vuid, objlist, region_loc.dot(Field::extent).dot(Field::depth),
                             "is zero. (empty copies are not allowed).");
        }

        // Each dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = src_image_state.GetEffectiveSubresourceExtent(region.srcSubresource);
        if (slice_override) src_copy_extent.depth = depth_slices;
        uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &src_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            skip |= LogError(GetImageCopyVUID("00144", is_2, is_host), src_objlist, region_loc.dot(Field::srcOffset).dot(Field::x),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource width (%" PRIu32 ").", region.srcOffset.x,
                             src_copy_extent.width, subresource_extent.width);
        }

        if (extent_check & kYBit) {
            skip |= LogError(GetImageCopyVUID("00145", is_2, is_host), src_objlist, region_loc.dot(Field::srcOffset).dot(Field::y),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource height (%" PRIu32 ").", region.srcOffset.y,
                             src_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            skip |= LogError(GetImageCopyVUID("00147", is_2, is_host), src_objlist, region_loc.dot(Field::srcOffset).dot(Field::z),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource depth (%" PRIu32 ").", region.srcOffset.z,
                             src_copy_extent.depth, subresource_extent.depth);
        }

        // Adjust dest extent if necessary
        subresource_extent = dst_image_state.GetEffectiveSubresourceExtent(region.dstSubresource);
        if (slice_override) dst_copy_extent.depth = depth_slices;

        extent_check = ExceedsBounds(&(region.dstOffset), &dst_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            skip |= LogError(GetImageCopyVUID("00150", is_2, is_host), dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::x),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource width (%" PRIu32 ").", region.dstOffset.x,
                             dst_copy_extent.width, subresource_extent.width);
        }
        if (extent_check & kYBit) {
            skip |= LogError(GetImageCopyVUID("00151", is_2, is_host), dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::y),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource height (%" PRIu32 ").", region.dstOffset.y,
                             dst_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            skip |= LogError(GetImageCopyVUID("00153", is_2, is_host), dst_objlist, region_loc.dot(Field::dstOffset).dot(Field::z),
                             "(%" PRId32 ") + extent (%" PRIu32 ") exceeds subResource depth (%" PRIu32 ").", region.dstOffset.z,
                             dst_copy_extent.depth, subresource_extent.depth);
        }
    }

    skip |= ValidateMemoryIsBoundToImage(src_objlist, src_image_state, loc.dot(Field::srcImage),
                                         GetImageCopyVUID("07966src", is_2, is_host));
    skip |= ValidateMemoryIsBoundToImage(dst_objlist, dst_image_state, loc.dot(Field::dstImage),
                                         GetImageCopyVUID("07966dst", is_2, is_host));

    if (src_image_state.createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        skip |= LogError(GetImageCopyVUID("07969src", is_2, is_host), src_objlist, loc.dot(Field::srcImage),
                         "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }
    if (dst_image_state.createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        skip |= LogError(GetImageCopyVUID("07969dst", is_2, is_host), dst_objlist, loc.dot(Field::dstImage),
                         "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (!cb_state_ptr || !src_image_state || !dst_image_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    const VkFormat src_format = src_image_state->createInfo.format;
    const VkFormat dst_format = dst_image_state->createInfo.format;
    const VkImageType src_image_type = src_image_state->createInfo.imageType;
    const VkImageType dst_image_type = dst_image_state->createInfo.imageType;
    const bool src_is_2d = (VK_IMAGE_TYPE_2D == src_image_type);
    const bool src_is_3d = (VK_IMAGE_TYPE_3D == src_image_type);
    const bool dst_is_2d = (VK_IMAGE_TYPE_2D == dst_image_type);
    const bool dst_is_3d = (VK_IMAGE_TYPE_3D == dst_image_type);
    const bool is_2 = loc.function == Func::vkCmdCopyImage2 || loc.function == Func::vkCmdCopyImage2KHR;

    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    skip = ValidateImageCopyData(commandBuffer, regionCount, pRegions, *src_image_state, *dst_image_state, false, loc);
    skip = ValidateCopyImageCommon(commandBuffer, *src_image_state, *dst_image_state, regionCount, pRegions, loc);

    bool has_stencil_aspect = false;
    bool has_non_stencil_aspect = false;
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType &region = pRegions[i];

        // For comp/uncomp copies, the copy extent for the dest image must be adjusted
        VkExtent3D src_copy_extent = region.extent;
        VkExtent3D dst_copy_extent = GetAdjustedDestImageExtent(src_format, dst_format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if (src_is_3d && !dst_is_3d) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if (dst_is_3d && !src_is_3d) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            // No chance of mismatch if we're overriding depth slice count
            if (!slice_override) {
                // The number of depth slices in srcSubresource and dstSubresource must match
                // Depth comes from layerCount for 1D,2D resources, from extent.depth for 3D
                uint32_t src_slices = (src_is_3d ? src_copy_extent.depth : region.srcSubresource.layerCount);
                uint32_t dst_slices = (dst_is_3d ? dst_copy_extent.depth : region.dstSubresource.layerCount);
                if (src_slices != dst_slices && src_slices != VK_REMAINING_ARRAY_LAYERS &&
                    dst_slices != VK_REMAINING_ARRAY_LAYERS) {
                    const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08793" : "VUID-vkCmdCopyImage-srcImage-08793";
                    skip |= LogError(vuid, objlist, region_loc, "%s (%" PRIu32 ") is different from %s (%" PRIu32 ").",
                                     src_is_3d ? "extent.depth" : "srcSubresource.layerCount", src_slices,
                                     dst_is_3d ? "extent.depth" : "dstSubresource.layerCount", dst_slices);
                }
            }
            // Maintenance 1 requires both while prior only required one to be 2D
            if ((src_is_2d && dst_is_2d) && (src_copy_extent.depth != 1)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01790" : "VUID-vkCmdCopyImage-srcImage-01790";
                skip |= LogError(vuid, objlist, region_loc,
                                 "both srcImage and dstImage are 2D and extent.depth is %" PRIu32 " and has to be 1",
                                 src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                // if different, one must be 3D and the other 2D
                const bool valid = (src_is_2d && dst_is_3d) || (src_is_3d && dst_is_2d) || enabled_features.maintenance5;
                if (!valid) {
                    const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07743" : "VUID-vkCmdCopyImage-srcImage-07743";
                    skip |=
                        LogError(vuid, objlist, region_loc,
                                 "srcImage type (%s) must be equal to dstImage type (%s) or else one must be 2D and the other 3D",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
                }
            }

            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01995" : "VUID-vkCmdCopyImage-srcImage-01995";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    src_image_loc, vuid);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01996" : "VUID-vkCmdCopyImage-dstImage-01996";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    dst_image_loc, vuid);

            // Check if 2D with 3D and depth not equal to 2D layerCount
            if (src_is_2d && dst_is_3d && (src_copy_extent.depth != region.srcSubresource.layerCount)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01791" : "VUID-vkCmdCopyImage-srcImage-01791";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcImage is 2D, dstImage is 3D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "srcSubresource.layerCount (%" PRIu32 ")",
                                 src_copy_extent.depth, region.srcSubresource.layerCount);
            } else if (src_is_3d && dst_is_2d && (src_copy_extent.depth != region.dstSubresource.layerCount)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01792" : "VUID-vkCmdCopyImage-dstImage-01792";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcImage is 3D, dstImage is 2D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "dstSubresource.layerCount (%" PRIu32 ")",
                                 src_copy_extent.depth, region.dstSubresource.layerCount);
            }
        } else {  // !vk_khr_maintenance1
            if ((src_is_2d || dst_is_2d) && (src_copy_extent.depth != 1)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-08969" : "VUID-vkCmdCopyImage-apiVersion-08969";
                skip |= LogError(vuid, objlist, region_loc, "srcImage is %s is dstImage is %s but extent.depth is %" PRIu32 ".",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type), src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07933" : "VUID-vkCmdCopyImage-apiVersion-07933";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcImage (%s) must be equal to dstImage (%s) without VK_KHR_maintenance1 enabled",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
            }
        }

        if ((!vkuFormatIsMultiplane(src_format)) && (!vkuFormatIsMultiplane(dst_format))) {
            // If neither image is multi-plane the aspectMask member of src and dst must match
            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01551" : "VUID-vkCmdCopyImage-srcImage-01551";
                skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                                 string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(),
                                 dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                                 string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
            }
        } else {
            // Source image multiplane checks
            VkImageAspectFlags aspect = region.srcSubresource.aspectMask;
            if (vkuFormatIsMultiplane(src_format) && !IsOnlyOneValidPlaneAspect(src_format, aspect)) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08713" : "VUID-vkCmdCopyImage-srcImage-08713";
                skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(aspect).c_str(),
                                 string_VkFormat(src_format));
            }
            // Single-plane to multi-plane
            if ((!vkuFormatIsMultiplane(src_format)) && (vkuFormatIsMultiplane(dst_format)) && (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01557" : "VUID-vkCmdCopyImage-dstImage-01557";
                skip |=
                    LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                             string_VkImageAspectFlags(aspect).c_str(), string_VkFormat(src_format), string_VkFormat(dst_format));
            }

            // Dest image multiplane checks
            aspect = region.dstSubresource.aspectMask;
            if (vkuFormatIsMultiplane(dst_format) && !IsOnlyOneValidPlaneAspect(dst_format, aspect)) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-08714" : "VUID-vkCmdCopyImage-dstImage-08714";
                skip |= LogError(vuid, objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(aspect).c_str(),
                                 string_VkFormat(dst_format));
            }
            // Multi-plane to single-plane
            if ((vkuFormatIsMultiplane(src_format)) && (!vkuFormatIsMultiplane(dst_format)) && (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01556" : "VUID-vkCmdCopyImage-srcImage-01556";
                skip |=
                    LogError(vuid, objlist, dst_subresource_loc.dot(Field::aspectMask),
                             "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                             string_VkImageAspectFlags(aspect).c_str(), string_VkFormat(src_format), string_VkFormat(dst_format));
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        // Validation is only performed when source image is the same as destination image.
        // In the general case, the mapping between an image and its underlying memory is undefined,
        // so checking for memory overlaps is not possible.
        if (src_image_state->image() == dst_image_state->image()) {
            for (uint32_t j = 0; j < regionCount; j++) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                if (auto intersection = GetRegionIntersection(region, pRegions[j], src_image_type, vkuFormatIsMultiplane(src_format));
                    intersection.has_instersection) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-00124" : "VUID-vkCmdCopyImage-pRegions-00124";
                    skip |= LogError(vuid, objlist, loc,
                                     "pRegion[%" PRIu32 "] copy source overlaps with pRegions[%" PRIu32
                                     "] copy destination. Overlap info, with respect to image (%s): %s.",
                                     i, j, FormatHandle(srcImage).c_str(), intersection.String().c_str());
                }
            }
        }

        // Check for multi-plane format compatiblity
        if (vkuFormatIsMultiplane(src_format) || vkuFormatIsMultiplane(dst_format)) {
            const VkFormat src_plane_format = vkuFormatIsMultiplane(src_format)
                                                  ? vkuFindMultiplaneCompatibleFormat(src_format, static_cast<VkImageAspectFlagBits>(region.srcSubresource.aspectMask))
                                                  : src_format;
            const VkFormat dst_plane_format = vkuFormatIsMultiplane(dst_format)
                                                  ? vkuFindMultiplaneCompatibleFormat(dst_format, static_cast<VkImageAspectFlagBits>(region.dstSubresource.aspectMask))
                                                  : dst_format;
            const size_t src_format_size = vkuFormatElementSize(src_plane_format);
            const size_t dst_format_size = vkuFormatElementSize(dst_plane_format);

            // If size is still zero, then format is invalid and will be caught in another VU
            if ((src_format_size != dst_format_size) && (src_format_size != 0) && (dst_format_size != 0)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                vuid = is_2 ? "VUID-VkCopyImageInfo2-None-01549" : "VUID-vkCmdCopyImage-None-01549";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcImage format %s with aspectMask %s is not compatible with dstImage format %s aspectMask %s.",
                                 string_VkFormat(src_format), string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(),
                                 string_VkFormat(dst_format), string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
            }
        }

        // track aspect mask in loop through regions
        if ((region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            has_stencil_aspect = true;
        }
        if ((region.srcSubresource.aspectMask & (~VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            has_non_stencil_aspect = true;
        }
    }

    // The formats of non-multiplane src_image and dst_image must be compatible. Formats are considered compatible if their texel
    // size in bytes is the same between both formats. For example, VK_FORMAT_R8G8B8A8_UNORM is compatible with VK_FORMAT_R32_UINT
    // because because both texels are 4 bytes in size.
    if (!vkuFormatIsMultiplane(src_format) && !vkuFormatIsMultiplane(dst_format)) {
        const char *compatible_vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-01548";
        // Depth/stencil formats must match exactly.
        if (vkuFormatIsDepthOrStencil(src_format) || vkuFormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                skip |= LogError(compatible_vuid, objlist, loc, "srcImage format (%s) is different from dstImage format (%s).",
                                 string_VkFormat(src_format), string_VkFormat(dst_format));
            }
        } else {
            if (vkuFormatElementSize(src_format) != vkuFormatElementSize(dst_format)) {
                const LogObjectList objlist(commandBuffer, srcImage, dstImage);
                skip |= LogError(compatible_vuid, objlist, loc,
                                 "srcImage format %s has size of %" PRIu32 " and dstImage format %s has size of %" PRIu32 ".",
                                 string_VkFormat(src_format), vkuFormatElementSize(src_format), string_VkFormat(dst_format),
                                 vkuFormatElementSize(dst_format));
            }
        }
    }

    if (vkuFormatIsCompressed(src_format) && vkuFormatIsCompressed(dst_format)) {
        auto src_block_extent = vkuFormatTexelBlockExtent(src_format);
        auto dst_block_extent = vkuFormatTexelBlockExtent(dst_format);
        if (src_block_extent.width != dst_block_extent.width || src_block_extent.height != dst_block_extent.height ||
            src_block_extent.depth != dst_block_extent.depth) {
            const char *compatible_vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-09247" : "VUID-vkCmdCopyImage-srcImage-09247";
            const LogObjectList objlist(commandBuffer, srcImage, dstImage);
            skip |= LogError(compatible_vuid, objlist, loc,
                             "srcImage format %s has texel block extent (w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32
                             ") and dstImage format %s has texel block extent (w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32 ").",
                             string_VkFormat(src_format), src_block_extent.width, src_block_extent.height, src_block_extent.depth,
                             string_VkFormat(dst_format), dst_block_extent.width, dst_block_extent.height, dst_block_extent.depth);
        }
    }

    // Validate that SRC & DST images have correct usage flags set
    if (!IsExtEnabled(device_extensions.vk_ext_separate_stencil_usage)) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid, src_image_loc);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid, dst_image_loc);
    } else {
        auto src_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(src_image_state->createInfo.pNext);
        if (src_separate_stencil && has_stencil_aspect &&
            ((src_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0)) {
            const LogObjectList objlist(commandBuffer, srcImage);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06664" : "VUID-vkCmdCopyImage-aspect-06664";
            skip = LogError(vuid, objlist, src_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_SRC_BIT.",
                            FormatHandle(src_image_state->Handle()).c_str(),
                            string_VkImageUsageFlags(src_separate_stencil->stencilUsage).c_str());
        }
        if (!src_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
            skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid,
                                            src_image_loc);
        }

        auto dst_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(dst_image_state->createInfo.pNext);
        if (dst_separate_stencil && has_stencil_aspect &&
            ((dst_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)) {
            const LogObjectList objlist(commandBuffer, dstImage);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06665" : "VUID-vkCmdCopyImage-aspect-06665";
            skip = LogError(vuid, objlist, dst_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
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
    if (src_image_state->createInfo.samples != dst_image_state->createInfo.samples) {
        const LogObjectList objlist(commandBuffer, srcImage, dstImage);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00136" : "VUID-vkCmdCopyImage-srcImage-00136";
        skip |= LogError(vuid, objlist, src_image_loc, "was created with (%s) but the dstImage was created with (%s).",
                         string_VkSampleCountFlagBits(src_image_state->createInfo.samples),
                         string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
    }

    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01825" : "VUID-vkCmdCopyImage-commandBuffer-01825";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01826" : "VUID-vkCmdCopyImage-commandBuffer-01826";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01827" : "VUID-vkCmdCopyImage-commandBuffer-01827";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

    skip |= ValidateCmd(cb_state, loc);

    const char *invalid_src_layout_vuid =
        is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-01917" : "VUID-vkCmdCopyImage-srcImageLayout-01917";
    const char *invalid_dst_layout_vuid =
        is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-01395" : "VUID-vkCmdCopyImage-dstImageLayout-01395";
    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; ++i) {
        // When performing copy from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;
        bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                 (src_subresource.baseArrayLayer == dst_subresource.baseArrayLayer));
        VkImageLayout source_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        VkImageLayout destination_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-00128" : "VUID-vkCmdCopyImage-srcImageLayout-00128";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, region.srcSubresource, srcImageLayout, source_optimal,
                                             src_image_loc, invalid_src_layout_vuid, vuid);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-00133" : "VUID-vkCmdCopyImage-dstImageLayout-00133";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, region.dstSubresource, dstImageLayout, destination_optimal,
                                             dst_image_loc, invalid_dst_layout_vuid, vuid);
        skip |= ValidateCopyImageTransferGranularityRequirements(cb_state, *src_image_state, *dst_image_state, &region, region_loc);
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

void CoreChecks::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions);
    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
        }
    }
}

void CoreChecks::RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(pCopyImageInfo->srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(pCopyImageInfo->dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < pCopyImageInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pCopyImageInfo->pRegions[i].srcSubresource,
                                                pCopyImageInfo->srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pCopyImageInfo->pRegions[i].dstSubresource,
                                                pCopyImageInfo->dstImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) {
    StateTracker::PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

void CoreChecks::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    StateTracker::PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo);
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

template <typename RegionType>
void CoreChecks::RecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                     const RegionType *pRegions, const Location &loc) {
    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";

    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (src_buffer_state->sparse || dst_buffer_state->sparse) {
        auto cb_state_ptr = Get<CMD_BUFFER_STATE>(commandBuffer);

        std::vector<sparse_container::range<VkDeviceSize>> src_ranges;
        std::vector<sparse_container::range<VkDeviceSize>> dst_ranges;

        for (uint32_t i = 0; i < regionCount; ++i) {
            const RegionType &region = pRegions[i];
            src_ranges.emplace_back(sparse_container::range<VkDeviceSize>{region.srcOffset, region.srcOffset + region.size});
            dst_ranges.emplace_back(sparse_container::range<VkDeviceSize>{region.dstOffset, region.dstOffset + region.size});
        }

        auto queue_submit_validation = [this, commandBuffer, src_buffer_state, dst_buffer_state, regionCount, src_ranges,
                                        dst_ranges, loc,
                                        vuid](const ValidationStateTracker &device_data, const class QUEUE_STATE &queue_state,
                                              const CMD_BUFFER_STATE &cb_state) -> bool {
            bool skip = false;
            for (uint32_t i = 0; i < regionCount; ++i) {
                const auto &src = src_ranges[i];
                for (uint32_t j = 0; j < regionCount; ++j) {
                    const auto &dst = dst_ranges[j];
                    if (const auto [memory, overlap_range] =
                            src_buffer_state->GetResourceMemoryOverlap(src, dst_buffer_state.get(), dst);
                        memory != VK_NULL_HANDLE) {
                        const LogObjectList objlist(commandBuffer, src_buffer_state->buffer(), dst_buffer_state->buffer(), memory);
                        skip |= this->LogError(vuid, objlist, loc,
                                               "Memory (%s) has copy overlap on range %s. Source "
                                               "buffer range is pRegions[%" PRIu32
                                               "] (%s), destination buffer range is pRegions[%" PRIu32 "] (%s).",
                                               FormatHandle(memory).c_str(), string_range(overlap_range).c_str(), i,
                                               string_range(src).c_str(), j, string_range(dst).c_str());
                    }
                }
            }

            return skip;
        };

        cb_state_ptr->queue_submit_functions.emplace_back(queue_submit_validation);
    }
}

void CoreChecks::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                            uint32_t regionCount, const VkBufferCopy *pRegions) {
    const Location loc(Func::vkCmdCopyBuffer);
    RecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, loc);
}

void CoreChecks::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo) {
    const Location loc(Func::vkCmdCopyBuffer2KHR);
    RecordCmdCopyBuffer(commandBuffer, pCopyBufferInfo->srcBuffer, pCopyBufferInfo->dstBuffer, pCopyBufferInfo->regionCount,
                        pCopyBufferInfo->pRegions, loc);
}

void CoreChecks::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) {
    const Location loc(Func::vkCmdCopyBuffer2);
    RecordCmdCopyBuffer(commandBuffer, pCopyBufferInfo->srcBuffer, pCopyBufferInfo->dstBuffer, pCopyBufferInfo->regionCount,
                        pCopyBufferInfo->pRegions, loc);
}

template <typename T>
VkImageSubresourceLayers GetImageSubresource(T data, bool is_src) {
    return data.imageSubresource;
}
template <>
VkImageSubresourceLayers GetImageSubresource<VkImageCopy2>(VkImageCopy2 data, bool is_src) {
    return is_src ? data.srcSubresource : data.dstSubresource;
}
template <typename T>
VkOffset3D GetOffset(T data, bool is_src) {
    return data.imageOffset;
}
template <>
VkOffset3D GetOffset<VkImageCopy2>(VkImageCopy2 data, bool is_src) {
    return is_src ? data.srcOffset : data.dstOffset;
}
template <typename T>
VkExtent3D GetExtent(T data) {
    return data.imageExtent;
}
template <>
VkExtent3D GetExtent<VkImageCopy2>(VkImageCopy2 data) {
    return data.extent;
}
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateImageBounds(const HandleT handle, const IMAGE_STATE &image_state, const uint32_t regionCount,
                                     const RegionType *pRegions, const Location &loc, const char *vuid, bool is_src) const {
    bool skip = false;
    const VkImageCreateInfo *image_info = &(image_state.createInfo);

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];
        VkExtent3D extent = GetExtent(region);
        VkOffset3D offset = GetOffset(region, is_src);
        VkImageSubresourceLayers subresource_layout = GetImageSubresource(region, is_src);

        VkExtent3D image_extent = image_state.GetEffectiveSubresourceExtent(subresource_layout);

        // If we're using a blocked image format, valid extent is rounded up to multiple of block size (per
        // vkspec.html#_common_operation)
        if (vkuFormatIsBlockedImage(image_info->format)) {
            auto block_extent = vkuFormatTexelBlockExtent(image_info->format);
            if (image_extent.width % block_extent.width) {
                image_extent.width += (block_extent.width - (image_extent.width % block_extent.width));
            }
            if (image_extent.height % block_extent.height) {
                image_extent.height += (block_extent.height - (image_extent.height % block_extent.height));
            }
            if (image_extent.depth % block_extent.depth) {
                image_extent.depth += (block_extent.depth - (image_extent.depth % block_extent.depth));
            }
        }

        if (0 != ExceedsBounds(&offset, &extent, &image_extent)) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(vuid, objlist, region_loc,
                             "exceeds image bounds\n"
                             "region extent (w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32
                             ")\n"
                             "region offset (x = %" PRId32 ", y = %" PRId32 ", z = %" PRId32
                             ")\n"
                             "image extent (w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32 ")\n",
                             extent.width, extent.height, extent.depth, offset.x, offset.y, offset.z, image_extent.width,
                             image_extent.height, image_extent.depth);
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferBounds(VkCommandBuffer cb, const IMAGE_STATE &image_state, const BUFFER_STATE &buff_state,
                                      uint32_t regionCount, const RegionType *pRegions, const Location &loc,
                                      const char *vuid) const {
    bool skip = false;

    const VkDeviceSize buffer_size = buff_state.createInfo.size;

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType region = pRegions[i];
        const VkDeviceSize buffer_copy_size =
            GetBufferSizeFromCopyImage(region, image_state.createInfo.format, image_state.createInfo.arrayLayers);
        // This blocks against invalid VkBufferCopyImage that already have been caught elsewhere
        if (buffer_copy_size != 0) {
            const VkDeviceSize max_buffer_copy = buffer_copy_size + region.bufferOffset;
            if (buffer_size < max_buffer_copy) {
                const LogObjectList objlist(cb, buff_state.Handle());
                skip |= LogError(vuid, objlist, region_loc,
                                 "is trying to copy %" PRIu64 " bytes plus %" PRIu64
                                 " offset to/from the VkBuffer (%s) which exceeds the VkBuffer total size of %" PRIu64 " bytes.",
                                 buffer_copy_size, region.bufferOffset, FormatHandle(buff_state).c_str(), buffer_size);
            }
        }
    }

    return skip;
}

template <typename HandleT>
// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(const HandleT handle, const IMAGE_STATE &image_state, VkSampleCountFlagBits sample_count,
                                          const Location &loc, const std::string &vuid) const {
    bool skip = false;
    if (image_state.createInfo.samples != sample_count) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip = LogError(vuid, objlist, loc, "%s was created with a sample count of %s but must be %s.",
                        FormatHandle(image_state).c_str(), string_VkSampleCountFlagBits(image_state.createInfo.samples),
                        string_VkSampleCountFlagBits(sample_count));
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !src_image_state || !dst_buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyImageToBuffer2 || loc.function == Func::vkCmdCopyImageToBuffer2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_buffer_loc = loc.dot(Field::dstBuffer);

    skip |= ValidateBufferImageCopyData(cb_state, regionCount, pRegions, *src_image_state, loc);

    skip |= ValidateCmd(cb_state, loc);

    // Command pool must support graphics, compute, or transfer operations
    const auto pool = cb_state.command_pool;

    VkQueueFlags queue_flags = physical_device_state->queue_family_properties[pool->queueFamilyIndex].queueFlags;

    if (0 == (queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))) {
        const LogObjectList objlist(cb_state.createInfo.commandPool, commandBuffer, srcImage, dstBuffer);
        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-cmdpool" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-cmdpool";
        skip |= LogError(vuid, objlist, loc, "command buffer allocated from a pool with queue type %s.",
                         string_VkQueueFlags(queue_flags).c_str());
    }

    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-pRegions-04566" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-07970";
    skip |= ValidateImageBounds(commandBuffer, *src_image_state, regionCount, pRegions, loc, vuid, true);
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-pRegions-00183" : "VUID-vkCmdCopyImageToBuffer-pRegions-00183";
    skip |= ValidateBufferBounds(commandBuffer, *src_image_state, *dst_buffer_state, regionCount, pRegions, loc, vuid);

    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07973" : "VUID-vkCmdCopyImageToBuffer-srcImage-07973";
    skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);

    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer-srcImage-07966" : "VUID-vkCmdCopyImageToBuffer-srcImage-07966";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, srcImage), *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "vkCmdCopyImageToBuffer-dstBuffer2-00192" : "vkCmdCopyImageToBuffer dstBuffer-00192";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

    // Validate that SRC image & DST buffer have correct usage flags set
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00186" : "VUID-vkCmdCopyImageToBuffer-srcImage-00186";
    skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191";
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, dstBuffer), *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                     true, vuid, dst_buffer_loc);
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01831" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01832" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832";
    skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01833" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833";
    skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const LogObjectList objlist(commandBuffer, srcImage, dstBuffer);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07969" : "VUID-vkCmdCopyImageToBuffer-srcImage-07969";
        skip |= LogError(vuid, objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-01998" : "VUID-vkCmdCopyImageToBuffer-srcImage-01998";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                src_image_loc, vuid);
    }

    const char *src_invalid_layout_vuid =
        is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-01397" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397";

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];
        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-00189" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, region.imageSubresource, srcImageLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_image_loc, src_invalid_layout_vuid, vuid);
        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-imageOffset-07747" : "VUID-vkCmdCopyImageToBuffer-imageOffset-07747";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *src_image_state, &region, region_loc, vuid);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-imageSubresource-07967" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-07967";
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, region.imageSubresource.mipLevel,
                                      subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-imageSubresource-07968" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-07968";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc, vuid);
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

void CoreChecks::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    if (cb_state_ptr && src_image_state) {
        // Make sure that all image slices record referenced layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].imageSubresource, srcImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage);
    if (cb_state_ptr && src_image_state) {
        // Make sure that all image slices record referenced layout
        for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pCopyImageToBufferInfo->pRegions[i].imageSubresource,
                                                pCopyImageToBufferInfo->srcImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage);
    if (cb_state_ptr && src_image_state) {
        // Make sure that all image slices record referenced layout
        for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pCopyImageToBufferInfo->pRegions[i].imageSubresource,
                                                pCopyImageToBufferInfo->srcImageLayout);
        }
    }
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (!cb_state_ptr || !src_buffer_state || !dst_image_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBufferToImage2 || loc.function == Func::vkCmdCopyBufferToImage2KHR;
    const char *vuid;
    const Location src_buffer_loc = loc.dot(Field::srcBuffer);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    skip |= ValidateBufferImageCopyData(cb_state, regionCount, pRegions, *dst_image_state, loc);

    skip |= ValidateCmd(cb_state, loc);

    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-pRegions-04565" : "VUID-vkCmdCopyBufferToImage-imageSubresource-07970";
    skip |= ValidateImageBounds(commandBuffer, *dst_image_state, regionCount, pRegions, loc, vuid, false);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-pRegions-00171" : "VUID-vkCmdCopyBufferToImage-pRegions-00171";
    skip |= ValidateBufferBounds(commandBuffer, *dst_image_state, *src_buffer_state, regionCount, pRegions, loc, vuid);

    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07973" : "VUID-vkCmdCopyBufferToImage-dstImage-07973";
    skip |= ValidateImageSampleCount(cb_state.commandBuffer(), *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00176" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00176";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07966" : "VUID-vkCmdCopyBufferToImage-dstImage-07966";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, dstImage), *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00174";
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, srcBuffer), *src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     true, vuid, src_buffer_loc);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00177" : "VUID-vkCmdCopyBufferToImage-dstImage-00177";
    skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01828" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01828";
    skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01829" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01829";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage-commandBuffer-01830" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01830";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const LogObjectList objlist(commandBuffer, srcBuffer, dstImage);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07969" : "VUID-vkCmdCopyBufferToImage-dstImage-07969";
        skip |= LogError(vuid, objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-01997" : "VUID-vkCmdCopyBufferToImage-dstImage-01997";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                dst_image_loc, vuid);
    }

    const char *dst_invalid_layout_vuid =
        is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-01396" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396";

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];
        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-00180" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, region.imageSubresource, dstImageLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst_image_loc, dst_invalid_layout_vuid, vuid);
        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-imageOffset-07738" : "VUID-vkCmdCopyBufferToImage-imageOffset-07738";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *dst_image_state, &region, region_loc, vuid);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-imageSubresource-07967" : "VUID-vkCmdCopyBufferToImage-imageSubresource-07967";
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, region.imageSubresource.mipLevel,
                                      subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-imageSubresource-07968" : "VUID-vkCmdCopyBufferToImage-imageSubresource-07968";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc, vuid);

        // TODO - Don't use ValidateCmdQueueFlags due to currently not having way to add more descriptive message
        const COMMAND_POOL_STATE *command_pool = cb_state.command_pool;
        assert(command_pool != nullptr);
        const uint32_t queue_family_index = command_pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (((queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
            ((region_aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)) {
            const LogObjectList objlist(commandBuffer, command_pool->commandPool(), srcBuffer, dstImage);
            vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-07739" : "VUID-vkCmdCopyBufferToImage-commandBuffer-07739";
            skip |= LogError(vuid, objlist, subresource_loc.dot(Field::aspectMask),
                             "is %s but the command buffer (%s) was allocated from the command pool (%s) "
                             "which was created with queueFamilyIndex %" PRIu32 ", which has queue type %s.",
                             string_VkImageAspectFlags(region_aspect_mask).c_str(), FormatHandle(cb_state).c_str(),
                             FormatHandle(command_pool->commandPool()).c_str(), queue_family_index,
                             string_VkQueueFlags(queue_flags).c_str());
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

void CoreChecks::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                   VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (cb_state_ptr && dst_image_state) {
        // Make sure that all image slices are record referenced layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pRegions[i].imageSubresource, dstImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo2KHR);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_image_state = Get<IMAGE_STATE>(pCopyBufferToImageInfo2KHR->dstImage);
    if (cb_state_ptr && dst_image_state) {
        // Make sure that all image slices are record referenced layout
        for (uint32_t i = 0; i < pCopyBufferToImageInfo2KHR->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pCopyBufferToImageInfo2KHR->pRegions[i].imageSubresource,
                                                pCopyBufferToImageInfo2KHR->dstImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                    const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);

    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_image_state = Get<IMAGE_STATE>(pCopyBufferToImageInfo->dstImage);
    if (cb_state_ptr && dst_image_state) {
        // Make sure that all image slices are record referenced layout
        for (uint32_t i = 0; i < pCopyBufferToImageInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pCopyBufferToImageInfo->pRegions[i].imageSubresource,
                                                pCopyBufferToImageInfo->dstImageLayout);
        }
    }
}

bool CoreChecks::UsageHostTransferCheck(VkDevice device, const IMAGE_STATE &image_state, bool has_stencil, bool has_non_stencil,
                                        const char *vuid_09111, const char *vuid_09112, const char *vuid_09113,
                                        const Location &loc) const {
    bool skip = false;
    if (has_stencil) {
        const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state.createInfo.pNext);
        if (image_stencil_struct != nullptr) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0) {
                LogObjectList objlist(device, image_state.image());
                skip |= LogError(
                    vuid_09112, objlist, loc,
                    "An element of pRegions has an aspectMask that includes "
                    "VK_IMAGE_ASPECT_STENCIL_BIT "
                    "and the image was created with separate stencil usage, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT was not "
                    "included in VkImageStencilUsageCreateInfo::stencilUsage used to create image");
            }
        } else {
            if ((image_state.createInfo.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0) {
                LogObjectList objlist(device, image_state.image());
                skip |= LogError(
                    vuid_09111, objlist, loc,
                    "An element of pRegions has an aspectMask that includes "
                    "VK_IMAGE_ASPECT_STENCIL_BIT and the "
                    "image was not created with separate stencil usage, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT was not included "
                    "in VkImageCreateInfo::usage used to create image");
            }
        }
    }
    if (has_non_stencil) {
        if ((image_state.createInfo.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0) {
            LogObjectList objlist(device, image_state.image());
            skip |= LogError(
                vuid_09113, objlist, loc,
                "An element of pRegions has an aspectMask that includes "
                "aspects other than VK_IMAGE_ASPECT_STENCIL_BIT, but  VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT was not included "
                "in VkImageCreateInfo::usage used to create image");
        }
    }
    return skip;
}

template <typename T>
VkImageLayout GetImageLayout(T data) {
    return VK_IMAGE_LAYOUT_UNDEFINED;
}
template <>
VkImageLayout GetImageLayout<VkCopyMemoryToImageInfoEXT>(VkCopyMemoryToImageInfoEXT data) {
    return data.dstImageLayout;
}
template <>
VkImageLayout GetImageLayout<VkCopyImageToMemoryInfoEXT>(VkCopyImageToMemoryInfoEXT data) {
    return data.srcImageLayout;
}
template <typename T>
VkImage GetImage(T data) {
    return VK_NULL_HANDLE;
}
template <>
VkImage GetImage<VkCopyMemoryToImageInfoEXT>(VkCopyMemoryToImageInfoEXT data) {
    return data.dstImage;
}
template <>
VkImage GetImage<VkCopyImageToMemoryInfoEXT>(VkCopyImageToMemoryInfoEXT data) {
    return data.srcImage;
}
template <typename InfoPointer>
bool CoreChecks::ValidateMemoryImageCopyCommon(VkDevice device, InfoPointer info_ptr, const Location &loc) const {
    bool skip = false;
    VkImage image = GetImage(*info_ptr);
    auto image_state = Get<IMAGE_STATE>(image);
    auto image_layout = GetImageLayout(*info_ptr);
    auto regionCount = info_ptr->regionCount;
    const bool from_image = loc.function == Func::vkCopyImageToMemoryEXT;
    const Location image_loc = loc.dot(from_image ? Field::srcImage : Field::dstImage);
    const char *info_type = from_image ? "pCopyImageToMemoryInfo" : "pCopyMemoryToImageInfo";
    const char *source_or_destination = from_image ? "source" : "destination";
    const char *image_layout_vuid = from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImageLayout-09064"
                                               : "VUID-VkCopyMemoryToImageInfoEXT-dstImageLayout-09059";

    if (!(enabled_features.hostImageCopy)) {
        const char *vuid =
            from_image ? "VUID-vkCopyImageToMemoryEXT-hostImageCopy-09063" : "VUID-vkCopyMemoryToImageEXT-hostImageCopy-09058";
        skip |= LogError(vuid, device, loc, "the hostImageCopy feature was not enabled");
    }

    skip |= ValidateHeterogeneousCopyData(device, regionCount, info_ptr->pRegions, *image_state, loc);
    skip |= ValidateMemoryIsBoundToImage(
        LogObjectList(device, image), *image_state, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07966" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07966");

    if (image_state->sparse && (!image_state->HasFullRangeBound())) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09109" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09109";
        LogObjectList objlist(device, image_state->image());
        skip |= LogError(vuid, objlist, image_loc, "is a sparse image with no memory bound");
    }

    if (image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07969" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07969";
        const LogObjectList objlist(device, image);
        skip |= LogError(vuid, objlist, image_loc,
                         "must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }
    skip |= ValidateImageBounds(device, *image_state, regionCount, info_ptr->pRegions, loc,
                                from_image ? "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07970"
                                           : "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07970",
                                from_image);

    skip |= ValidateImageSampleCount(
        device, *image_state, VK_SAMPLE_COUNT_1_BIT, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07973" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07973");

    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY_EXT);
    bool has_stencil = false;
    bool has_non_stencil = false;
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const auto region = info_ptr->pRegions[i];

        skip |= ValidateImageMipLevel(device, *image_state, region.imageSubresource.mipLevel, subresource_loc.dot(Field::mipLevel),
                                      from_image ? "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07967"
                                                 : "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07967");
        skip |= ValidateImageArrayLayerRange(device, *image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc,
                                             from_image ? "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07968"
                                                        : "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07968");
        skip |= ValidateImageSubresourceLayers(device, &region.imageSubresource, subresource_loc);
        if (region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) has_stencil = true;
        if (region.imageSubresource.aspectMask & ~VK_IMAGE_ASPECT_STENCIL_BIT) has_non_stencil = true;

        if (check_memcpy) {
            if (region.imageOffset.x != 0 || region.imageOffset.y != 0 || region.imageOffset.z != 0) {
                const char *vuid = from_image ? "VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09114"
                                              : "VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09114";
                LogObjectList objlist(device);
                LogError(vuid, objlist, loc,
                         "%s->flags contains VK_HOST_IMAGE_COPY_MEMCPY_EXT which "
                         "means that pRegions[%" PRIu32 "].imageOffset x(%" PRIu32 "), y(%" PRIu32 ") and z(%" PRIu32
                         ") must all be zero",
                         info_type, i, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z);
            }
            const VkExtent3D subresource_extent = image_state->GetEffectiveSubresourceExtent(region.imageSubresource);
            if (!IsExtentEqual(region.imageExtent, subresource_extent)) {
                const char *vuid = from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09115"
                                              : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09115";
                LogObjectList objlist(device, image_state->image());
                skip |= LogError(vuid, objlist, loc,
                                 "pRegion[%" PRIu32 "].imageExtent (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32
                                 ") must match the image's subresource "
                                 "extents (w=%" PRIu32 ", h=%" PRIu32 ", d=%" PRIu32
                                 ") %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY_EXT",
                                 i, region.imageExtent.width, region.imageExtent.height, region.imageExtent.depth,
                                 subresource_extent.width, subresource_extent.height, subresource_extent.depth, info_type);
            }
            if ((region.memoryRowLength != 0) || (region.memoryImageHeight != 0)) {
                const char *vuid =
                    from_image ? "VUID-VkCopyImageToMemoryInfoEXT-flags-09394" : "VUID-VkCopyMemoryToImageInfoEXT-flags-09393";
                LogObjectList objlist(device, image_state->image());
                skip |= LogError(vuid, objlist, loc,
                                 "pRegion[%" PRIu32 "].memoryRowLength (%" PRIu32 "), and pRegion[%" PRIu32
                                 "].memoryImageHeight (%" PRIu32
                                 ") must both be zero if %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY_EXT",
                                 i, region.memoryRowLength, i, region.memoryImageHeight, info_type);
            }
        }

        Field field = from_image ? Field::srcImageLayout : Field::dstImageLayout;
        skip |= ValidateHostCopyCurrentLayout(device, image_layout, region.imageSubresource, i, *image_state, region_loc.dot(field),
                                              source_or_destination, image_layout_vuid);
    }

    const char *vuid_09111 =
        from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09111" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09111";
    const char *vuid_09112 =
        from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09112" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09112";
    const char *vuid_09113 =
        from_image ? "VUID-VkCopyImageToMemoryInfoEXT-srcImage-09113" : "VUID-VkCopyMemoryToImageInfoEXT-dstImage-09113";
    skip |= UsageHostTransferCheck(device, *image_state, has_stencil, has_non_stencil, vuid_09111, vuid_09112, vuid_09113, loc);

    const auto &memory_states = image_state->GetBoundMemoryStates();
    for (const auto &state : memory_states) {
        // Image and host memory can't overlap unless the image memory is mapped
        if (state->mapped_range.size != 0) {
            const uint64_t mapped_size = (state->mapped_range.size == VK_WHOLE_SIZE)
                                             ? state->alloc_info.allocationSize
                                             : (state->mapped_range.offset + state->mapped_range.size);
            const void *mapped_end = static_cast<char *>(state->p_driver_data) + mapped_size;
            for (uint32_t i = 0; i < regionCount; i++) {
                const auto region = info_ptr->pRegions[i];
                auto element_size = vkuFormatElementSize(image_state->createInfo.format);
                uint64_t copy_size;
                if (region.memoryRowLength != 0 && region.memoryImageHeight != 0) {
                    copy_size = ((region.memoryRowLength * region.memoryImageHeight) * element_size);
                } else {
                    copy_size = ((region.imageExtent.width * region.imageExtent.height * region.imageExtent.depth) * element_size);
                }
                const void *copy_end = static_cast<const char *>(region.pHostPointer) + copy_size;

                if ((region.pHostPointer >= state->p_driver_data && region.pHostPointer < mapped_end) ||
                    (copy_end >= state->p_driver_data && copy_end < mapped_end) ||
                    (region.pHostPointer <= state->p_driver_data && copy_end > mapped_end)) {
                    const char *vuid =
                        from_image ? "VUID-VkImageToMemoryCopyEXT-pRegions-09067" : "VUID-VkMemoryToImageCopyEXT-pRegions-09062";
                    LogObjectList objlist(device, image_state->image());
                    skip |= LogError(vuid, objlist, loc.dot(Field::pRegions, i).dot(Field::pHostPointer),
                                     "points to memory spanning %p through %p, which overlaps with image memory"
                                     "mapped %p through %p",
                                     region.pHostPointer, copy_end, state->p_driver_data, mapped_end);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateHostCopyImageCreateInfos(VkDevice device, const IMAGE_STATE &src_image_state,
                                                  const IMAGE_STATE &dst_image_state, const Location &loc) const {
    bool skip = false;
    std::stringstream mismatch_stream{};
    const VkImageCreateInfo src_info = src_image_state.createInfo;
    const VkImageCreateInfo dst_info = dst_image_state.createInfo;

    if (src_info.flags != dst_info.flags) {
        mismatch_stream << "srcImage flags = " << string_VkImageCreateFlags(src_info.flags)
                        << " and dstImage flags = " << string_VkImageCreateFlags(dst_info.flags) << "\n";
    }
    if (src_info.imageType != dst_info.imageType) {
        mismatch_stream << "srcImage imageType = " << string_VkImageType(src_info.imageType)
                        << " and dstImage imageType = " << string_VkImageType(dst_info.imageType) << "\n";
    }
    if (src_info.format != dst_info.format) {
        mismatch_stream << "srcImage format = " << string_VkFormat(src_info.format)
                        << " and dstImage format = " << string_VkFormat(dst_info.format) << "\n";
    }
    if ((src_info.extent.width != dst_info.extent.width) || (src_info.extent.height != dst_info.extent.height) ||
        (src_info.extent.depth != dst_info.extent.depth)) {
        mismatch_stream << "srcImage extent.width = " << src_info.extent.width << " extent.height = " << src_info.extent.height
                        << " extent.depth = " << src_info.extent.depth << " but dstImage extent.width = " << dst_info.extent.width
                        << " extent.height = " << dst_info.extent.height << " extent.depth = " << dst_info.extent.depth << "\n";
    }
    if (src_info.mipLevels != dst_info.mipLevels) {
        mismatch_stream << "srcImage mipLevels = " << src_info.mipLevels << "and dstImage mipLevels = " << dst_info.mipLevels
                        << "\n";
    }
    if (src_info.arrayLayers != dst_info.arrayLayers) {
        mismatch_stream << "srcImage arrayLayers = " << src_info.arrayLayers
                        << " and dstImage arrayLayers = " << dst_info.arrayLayers << "\n";
    }
    if (src_info.samples != dst_info.samples) {
        mismatch_stream << "srcImage samples = " << string_VkSampleCountFlagBits(src_info.samples)
                        << " and dstImage samples = " << string_VkSampleCountFlagBits(dst_info.samples) << "\n";
    }
    if (src_info.tiling != dst_info.tiling) {
        mismatch_stream << "srcImage tiling = " << string_VkImageTiling(src_info.tiling)
                        << " and dstImage tiling = " << string_VkImageTiling(dst_info.tiling) << "\n";
    }
    if (src_info.usage != dst_info.usage) {
        mismatch_stream << "srcImage usage = " << string_VkImageUsageFlags(src_info.usage)
                        << " and dstImage usage = " << string_VkImageUsageFlags(dst_info.usage) << "\n";
    }
    if (src_info.sharingMode != dst_info.sharingMode) {
        mismatch_stream << "srcImage sharingMode = " << string_VkSharingMode(src_info.sharingMode)
                        << " and dstImage sharingMode = " << string_VkSharingMode(dst_info.sharingMode) << "\n";
    }
    if (src_info.initialLayout != dst_info.initialLayout) {
        mismatch_stream << "srcImage initialLayout = " << string_VkImageLayout(src_info.initialLayout)
                        << " and dstImage initialLayout = " << string_VkImageLayout(dst_info.initialLayout) << "\n";
    }

    if (mismatch_stream.str().length() > 0) {
        std::stringstream ss;
        ss << "The creation parameters for srcImage and dstImage differ:\n" << mismatch_stream.str();
        LogObjectList objlist(device, src_image_state.image(), dst_image_state.image());
        skip |= LogError("VUID-VkCopyImageToImageInfoEXT-srcImage-09069", objlist, loc, "%s.", ss.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyImageLayout(const VkDevice device, const VkImage image, const uint32_t layout_count,
                                             const VkImageLayout *supported_image_layouts, const VkImageLayout image_layout,
                                             const Location &loc, const char *supported_name, const char *vuid) const {
    for (uint32_t i = 0; i < layout_count; ++i) {
        if (supported_image_layouts[i] == image_layout) {
            return false;
        }
    }

    LogObjectList objlist(device, image);
    bool skip = LogError(vuid, objlist, loc,
                         "is %s which is not one of the layouts returned in "
                         "VkPhysicalDeviceHostImageCopyPropertiesEXT::%s",
                         string_VkImageLayout(image_layout), supported_name);

    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT *pCopyMemoryToImageInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyMemoryToImageInfo);
    auto dst_image = pCopyMemoryToImageInfo->dstImage;
    auto image_state = Get<IMAGE_STATE>(dst_image);

    skip |= ValidateMemoryImageCopyCommon(device, pCopyMemoryToImageInfo, copy_loc);
    auto *props = &phys_dev_ext_props.host_image_copy_properties;
    skip |= ValidateHostCopyImageLayout(device, dst_image, props->copyDstLayoutCount, props->pCopyDstLayouts,
                                        pCopyMemoryToImageInfo->dstImageLayout, copy_loc.dot(Field::dstImageLayout),
                                        "pCopyDstLayouts", "VUID-VkCopyMemoryToImageInfoEXT-dstImageLayout-09060");
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT *pCopyImageToMemoryInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyImageToMemoryInfo);
    auto src_image = pCopyImageToMemoryInfo->srcImage;
    auto image_state = Get<IMAGE_STATE>(src_image);

    skip |= ValidateMemoryImageCopyCommon(device, pCopyImageToMemoryInfo, copy_loc);
    auto *props = &phys_dev_ext_props.host_image_copy_properties;
    skip |= ValidateHostCopyImageLayout(device, src_image, props->copySrcLayoutCount, props->pCopySrcLayouts,
                                        pCopyImageToMemoryInfo->srcImageLayout, copy_loc.dot(Field::srcImageLayout),
                                        "pCopySrcLayouts", "VUID-VkCopyImageToMemoryInfoEXT-srcImageLayout-09065");
    return skip;
}

bool CoreChecks::ValidateMemcpyExtents(VkDevice device, const VkImageCopy2 region, const IMAGE_STATE &image_state, bool is_src,
                                       const Location &region_loc) const {
    bool skip = false;
    if (region.srcOffset.x != 0 || region.srcOffset.y != 0 || region.srcOffset.z != 0) {
        const char *vuid =
            is_src ? "VUID-VkCopyImageToImageInfoEXT-srcOffset-09114" : "VUID-VkCopyImageToImageInfoEXT-dstOffset-09114";
        Field field = is_src ? Field::srcOffset : Field::dstOffset;
        const LogObjectList objlist(device);
        skip |= LogError(vuid, objlist, region_loc.dot(field),
                         "is (x = %" PRIu32 ", y = %" PRIu32 ", z = %" PRIu32 ") but flags contains VK_HOST_IMAGE_COPY_MEMCPY_EXT.",
                         region.srcOffset.x, region.srcOffset.y, region.srcOffset.z);
    }
    if (!IsExtentEqual(region.extent, image_state.createInfo.extent)) {
        const char *vuid =
            is_src ? "VUID-VkCopyImageToImageInfoEXT-srcImage-09115" : "VUID-VkCopyImageToImageInfoEXT-dstImage-09115";
        const LogObjectList objlist(device, image_state.image());
        skip |= LogError(vuid, objlist, region_loc.dot(Field::imageExtent),
                         "(w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32
                         ") must match the image's subresource "
                         "extents (w = %" PRIu32 ", h = %" PRIu32 ", d = %" PRIu32
                         ") when VkCopyImageToImageInfoEXT->flags contains VK_HOST_IMAGE_COPY_MEMCPY_EXT",
                         region.extent.width, region.extent.height, region.extent.depth, image_state.createInfo.extent.width,
                         image_state.createInfo.extent.height, image_state.createInfo.extent.depth);
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyMultiplane(VkDevice device, VkImageCopy2 region, const IMAGE_STATE &image_state, bool is_src,
                                            const Location &region_loc) const {
    bool skip = false;
    auto aspect_mask = is_src ? region.srcSubresource.aspectMask : region.dstSubresource.aspectMask;
    if (vkuFormatPlaneCount(image_state.createInfo.format) == 2 &&
        (aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
        const char *vuid =
            is_src ? "VUID-VkCopyImageToImageInfoEXT-srcImage-07981" : "VUID-VkCopyImageToImageInfoEXT-dstImage-07981";
        Field field = is_src ? Field::srcSubresource : Field::dstSubresource;
        LogObjectList objlist(device, image_state.image());
        skip |= LogError(vuid, objlist, region_loc.dot(field), "is %s but %s has 2-plane format (%s).",
                         string_VkImageAspectFlags(aspect_mask).c_str(), is_src ? "srcImage" : "dstImage",
                         string_VkFormat(image_state.createInfo.format));
    }
    if (vkuFormatPlaneCount(image_state.createInfo.format) == 3 &&
        (aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT &&
         aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
        const char *vuid =
            is_src ? "VUID-VkCopyImageToImageInfoEXT-srcImage-07981" : "VUID-VkCopyImageToImageInfoEXT-dstImage-07981";
        Field field = is_src ? Field::srcSubresource : Field::dstSubresource;
        LogObjectList objlist(device, image_state.image());
        skip |= LogError(vuid, objlist, region_loc.dot(field), "is %s but %s has 3-plane format (%s).",
                         string_VkImageAspectFlags(aspect_mask).c_str(), is_src ? "srcImage" : "dstImage",
                         string_VkFormat(image_state.createInfo.format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT *pCopyImageToImageInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    auto info_ptr = pCopyImageToImageInfo;
    const Location loc = error_obj.location.dot(Field::pCopyImageToImageInfo);
    auto src_image_state = Get<IMAGE_STATE>(info_ptr->srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(info_ptr->dstImage);
    // Formats are required to match, but check each image anyway
    auto src_plane_count = vkuFormatPlaneCount(src_image_state->createInfo.format);
    auto dst_plane_count = vkuFormatPlaneCount(dst_image_state->createInfo.format);
    bool check_multiplane = ((src_plane_count == 2 || src_plane_count == 3) || (dst_plane_count == 2 || dst_plane_count == 3));
    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY_EXT);
    auto regionCount = info_ptr->regionCount;
    auto pRegions = info_ptr->pRegions;

    if (!(enabled_features.hostImageCopy)) {
        skip |= LogError("VUID-vkCopyImageToImageEXT-hostImageCopy-09068", device, error_obj.location,
                         "the hostImageCopy feature was not enabled");
    }

    skip |= ValidateHostCopyImageCreateInfos(device, *src_image_state, *dst_image_state, error_obj.location);
    skip |= ValidateImageCopyData(device, regionCount, pRegions, *src_image_state, *dst_image_state, true, error_obj.location);
    skip |= ValidateCopyImageCommon(device, *src_image_state, *dst_image_state, regionCount, pRegions, error_obj.location);
    skip |= ValidateImageBounds(device, *src_image_state, regionCount, pRegions, loc,
                                "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07970", true);
    skip |= ValidateImageBounds(device, *dst_image_state, regionCount, pRegions, loc,
                                "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07970", false);
    auto *props = &phys_dev_ext_props.host_image_copy_properties;
    skip |= ValidateHostCopyImageLayout(device, info_ptr->srcImage, props->copySrcLayoutCount, props->pCopySrcLayouts,
                                        info_ptr->srcImageLayout, loc.dot(Field::srcImageLayout), "pCopySrcLayouts",
                                        "VUID-VkCopyImageToImageInfoEXT-srcImageLayout-09072");
    skip |= ValidateHostCopyImageLayout(device, info_ptr->dstImage, props->copyDstLayoutCount, props->pCopyDstLayouts,
                                        info_ptr->dstImageLayout, loc.dot(Field::dstImageLayout), "pCopyDstLayouts",
                                        "VUID-VkCopyImageToImageInfoEXT-dstImageLayout-09073");

    if (src_image_state->sparse && (!src_image_state->HasFullRangeBound())) {
        LogObjectList objlist(device, src_image_state->image());
        skip |= LogError("VUID-VkCopyImageToImageInfoEXT-srcImage-09109", objlist, loc.dot(Field::srcImage),
                         "is a sparse image with no memory bound");
    }
    if (dst_image_state->sparse && (!dst_image_state->HasFullRangeBound())) {
        LogObjectList objlist(device, dst_image_state->image());
        skip |= LogError("VUID-VkCopyImageToImageInfoEXT-dstImage-09109", objlist, loc.dot(Field::dstImage),
                         "is a sparse image with no memory bound");
    }

    bool has_stencil = false;
    bool has_non_stencil = false;
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const auto &region = info_ptr->pRegions[i];
        if (check_memcpy) {
            skip |= ValidateMemcpyExtents(device, region, *src_image_state, true, region_loc);
            skip |= ValidateMemcpyExtents(device, region, *dst_image_state, false, region_loc);
        }
        if (check_multiplane) {
            skip |= ValidateHostCopyMultiplane(device, region, *src_image_state, true, region_loc);
            skip |= ValidateHostCopyMultiplane(device, region, *dst_image_state, false, region_loc);
        }

        if ((region.srcSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            has_stencil = true;
        }
        if ((region.srcSubresource.aspectMask & (~VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            has_non_stencil = true;
        }

        skip |= ValidateHostCopyCurrentLayout(device, info_ptr->srcImageLayout, region.srcSubresource, i, *src_image_state,
                                              region_loc.dot(Field::srcImageLayout), "source",
                                              "VUID-VkCopyImageToImageInfoEXT-srcImageLayout-09070");
        skip |= ValidateHostCopyCurrentLayout(device, info_ptr->dstImageLayout, region.dstSubresource, i, *dst_image_state,
                                              region_loc.dot(Field::dstImageLayout), "destination",
                                              "VUID-VkCopyImageToImageInfoEXT-dstImageLayout-09071");
    }

    skip |= UsageHostTransferCheck(device, *src_image_state, has_stencil, has_non_stencil,
                                   "VUID-VkCopyImageToImageInfoEXT-srcImage-09111", "VUID-VkCopyImageToImageInfoEXT-srcImage-09112",
                                   "VUID-VkCopyImageToImageInfoEXT-srcImage-09113", error_obj.location);
    skip |= UsageHostTransferCheck(device, *dst_image_state, has_stencil, has_non_stencil,
                                   "VUID-VkCopyImageToImageInfoEXT-dstImage-09111", "VUID-VkCopyImageToImageInfoEXT-dstImage-09112",
                                   "VUID-VkCopyImageToImageInfoEXT-dstImage-09113", error_obj.location);
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (!cb_state_ptr || !src_image_state || !src_image_state) {
        return skip;
    }

    const bool is_2 = loc.function == Func::vkCmdBlitImage2 || loc.function == Func::vkCmdBlitImage2KHR;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    skip |= ValidateCmd(cb_state, loc);

    const char *vuid;
    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00233" : "VUID-vkCmdBlitImage-srcImage-00233";
    skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00234" : "VUID-vkCmdBlitImage-dstImage-00234";
    skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00220" : "VUID-vkCmdBlitImage-srcImage-00220";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(device, srcImage), *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00225" : "VUID-vkCmdBlitImage-dstImage-00225";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(device, dstImage), *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00219" : "VUID-vkCmdBlitImage-srcImage-00219";
    skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00224" : "VUID-vkCmdBlitImage-dstImage-00224";
    skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-01999" : "VUID-vkCmdBlitImage-srcImage-01999";
    skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_BLIT_SRC_BIT, src_image_loc, vuid);
    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02000" : "VUID-vkCmdBlitImage-dstImage-02000";
    skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_BLIT_DST_BIT, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01834" : "VUID-vkCmdBlitImage-commandBuffer-01834";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01835" : "VUID-vkCmdBlitImage-commandBuffer-01835";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01836" : "VUID-vkCmdBlitImage-commandBuffer-01836";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);
    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
        skip |= LogError(vuid, src_objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
    }
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
        skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
    }

    // TODO: Need to validate image layouts, which will include layout validation for shared presentable images

    VkFormat src_format = src_image_state->createInfo.format;
    VkFormat dst_format = dst_image_state->createInfo.format;
    VkImageType src_type = src_image_state->createInfo.imageType;
    VkImageType dst_type = dst_image_state->createInfo.imageType;

    if (VK_FILTER_LINEAR == filter) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02001" : "VUID-vkCmdBlitImage-filter-02001";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT, src_image_loc, vuid);
    } else if (VK_FILTER_CUBIC_IMG == filter) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02002" : "VUID-vkCmdBlitImage-filter-02002";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT,
                                                src_image_loc, vuid);
    }

    if (FormatRequiresYcbcrConversionExplicitly(src_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-06421" : "VUID-vkCmdBlitImage-srcImage-06421";
        skip |= LogError(vuid, src_objlist, src_image_loc,
                         "format (%s) must not be one of the formats requiring sampler YCBCR "
                         "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                         string_VkFormat(src_format));
    }

    if (FormatRequiresYcbcrConversionExplicitly(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-06422" : "VUID-vkCmdBlitImage-dstImage-06422";
        skip |= LogError(vuid, dst_objlist, dst_image_loc,
                         "format (%s) must not be one of the formats requiring sampler YCBCR "
                         "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                         string_VkFormat(dst_format));
    }

    if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_2D != src_type)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-00237" : "VUID-vkCmdBlitImage-filter-00237";
        skip |= LogError(vuid, src_objlist, loc.dot(Field::filter), "is VK_FILTER_CUBIC_IMG but srcImage was created with %s.",
                         string_VkImageType(src_type));
    }

    // Validate consistency for unsigned formats
    if (vkuFormatIsUINT(src_format) != vkuFormatIsUINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00230" : "VUID-vkCmdBlitImage-srcImage-00230";
        skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    // Validate consistency for signed formats
    if (vkuFormatIsSINT(src_format) != vkuFormatIsSINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00229" : "VUID-vkCmdBlitImage-srcImage-00229";
        skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    // Validate filter for Depth/Stencil formats
    if (vkuFormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00232" : "VUID-vkCmdBlitImage-srcImage-00232";
        skip |= LogError(vuid, src_objlist, src_image_loc, "has depth-stencil format %s but filter is %s.",
                         string_VkFormat(src_format), string_VkFilter(filter));
    }

    // Validate aspect bits and formats for depth/stencil images
    if (vkuFormatIsDepthOrStencil(src_format) || vkuFormatIsDepthOrStencil(dst_format)) {
        if (src_format != dst_format) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00231" : "VUID-vkCmdBlitImage-srcImage-00231";
            skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                             string_VkFormat(src_format), string_VkFormat(dst_format));
        }
    }

    // Do per-region checks
    const char *invalid_src_layout_vuid =
        is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-01398" : "VUID-vkCmdBlitImage-srcImageLayout-01398";
    const char *invalid_dst_layout_vuid =
        is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-01399" : "VUID-vkCmdBlitImage-dstImageLayout-01399";

    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];

        // When performing blit from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                 (src_subresource.baseArrayLayer == dst_subresource.baseArrayLayer));
        VkImageLayout source_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        VkImageLayout destination_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-00221" : "VUID-vkCmdBlitImage-srcImageLayout-00221";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, src_subresource, srcImageLayout, source_optimal,
                                             src_image_loc, invalid_src_layout_vuid, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-00226" : "VUID-vkCmdBlitImage-dstImageLayout-00226";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, dst_subresource, dstImageLayout, destination_optimal,
                                             dst_image_loc, invalid_dst_layout_vuid, vuid);
        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &dst_subresource, dst_subresource_loc);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcSubresource-01705" : "VUID-vkCmdBlitImage-srcSubresource-01705";
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, src_subresource.mipLevel,
                                      src_subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstSubresource-01706" : "VUID-vkCmdBlitImage-dstSubresource-01706";
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, dst_subresource.mipLevel,
                                      dst_subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcSubresource-01707" : "VUID-vkCmdBlitImage-srcSubresource-01707";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, src_subresource.baseArrayLayer,
                                             src_subresource.layerCount, src_subresource_loc, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstSubresource-01708" : "VUID-vkCmdBlitImage-dstSubresource-01708";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, dst_subresource.baseArrayLayer,
                                             dst_subresource.layerCount, dst_subresource_loc, vuid);
        // Check that src/dst layercounts match
        if (src_subresource.layerCount != dst_subresource.layerCount && src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS &&
            dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
            vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08800" : "VUID-VkImageBlit-layerCount-08800";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", src_subresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), dst_subresource.layerCount);
        }

        if (src_subresource.aspectMask != dst_subresource.aspectMask) {
            vuid = is_2 ? "VUID-VkImageBlit2-aspectMask-00238" : "VUID-VkImageBlit-aspectMask-00238";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                             string_VkImageAspectFlags(src_subresource.aspectMask).c_str(),
                             dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                             string_VkImageAspectFlags(dst_subresource.aspectMask).c_str());
        }

        if (!VerifyAspectsPresent(src_subresource.aspectMask, src_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00241" : "VUID-vkCmdBlitImage-aspectMask-00241";
            skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) cannot specify aspects not present in source image (%s).",
                             string_VkImageAspectFlags(src_subresource.aspectMask).c_str(), string_VkFormat(src_format));
        }

        if (!VerifyAspectsPresent(dst_subresource.aspectMask, dst_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00242" : "VUID-vkCmdBlitImage-aspectMask-00242";
            skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                             "(%s) cannot specify aspects not present in destination image (%s).",
                             string_VkImageAspectFlags(src_subresource.aspectMask).c_str(), string_VkFormat(src_format));
        }

        // Validate source image offsets
        VkExtent3D src_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
        if (VK_IMAGE_TYPE_1D == src_type) {
            if ((0 != region.srcOffsets[0].y) || (1 != region.srcOffsets[1].y)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00245" : "VUID-vkCmdBlitImage-srcImage-00245";
                skip |=
                    LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].y is %" PRId32 " and srcOffsets[1].y is %" PRId32 " but srcImage is VK_IMAGE_TYPE_1D.",
                             region.srcOffsets[0].y, region.srcOffsets[1].y);
            }
        }

        if ((VK_IMAGE_TYPE_1D == src_type) || (VK_IMAGE_TYPE_2D == src_type)) {
            if ((0 != region.srcOffsets[0].z) || (1 != region.srcOffsets[1].z)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00247" : "VUID-vkCmdBlitImage-srcImage-00247";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffsets[0].z is %" PRId32 " and srcOffsets[1].z is %" PRId32 " but srcImage is %s.",
                                 region.srcOffsets[0].z, region.srcOffsets[1].z, string_VkImageType(src_type));
            }
        }

        bool oob = false;
        if ((region.srcOffsets[0].x < 0) || (region.srcOffsets[0].x > static_cast<int32_t>(src_extent.width)) ||
            (region.srcOffsets[1].x < 0) || (region.srcOffsets[1].x > static_cast<int32_t>(src_extent.width))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].x is %" PRId32 " and srcOffsets[1].x is %" PRId32
                             " which exceed srcSubresource width extent (%" PRIu32 ").",
                             region.srcOffsets[0].x, region.srcOffsets[1].x, src_extent.width);
        }
        if ((region.srcOffsets[0].y < 0) || (region.srcOffsets[0].y > static_cast<int32_t>(src_extent.height)) ||
            (region.srcOffsets[1].y < 0) || (region.srcOffsets[1].y > static_cast<int32_t>(src_extent.height))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].y is %" PRId32 " and srcOffsets[1].y is %" PRId32
                             " which exceed srcSubresource height extent (%" PRIu32 ").",
                             region.srcOffsets[0].y, region.srcOffsets[1].y, src_extent.height);
        }
        if ((region.srcOffsets[0].z < 0) || (region.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth)) ||
            (region.srcOffsets[1].z < 0) || (region.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].z is %" PRId32 " and srcOffsets[1].z is %" PRId32
                             " which exceed srcSubresource depth extent (%" PRIu32 ").",
                             region.srcOffsets[0].z, region.srcOffsets[1].z, src_extent.depth);
        }
        if (oob) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00215" : "VUID-vkCmdBlitImage-pRegions-00215";
            skip |= LogError(vuid, src_objlist, region_loc, "source image blit region exceeds image dimensions.");
        }

        // Validate dest image offsets
        VkExtent3D dst_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
        if (VK_IMAGE_TYPE_1D == dst_type) {
            if ((0 != region.dstOffsets[0].y) || (1 != region.dstOffsets[1].y)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00250" : "VUID-vkCmdBlitImage-dstImage-00250";
                skip |=
                    LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].y is %" PRId32 " and dstOffsets[1].y is %" PRId32 " but dstImage is VK_IMAGE_TYPE_1D.",
                             region.dstOffsets[0].y, region.dstOffsets[1].y);
            }
        }

        if ((VK_IMAGE_TYPE_1D == dst_type) || (VK_IMAGE_TYPE_2D == dst_type)) {
            if ((0 != region.dstOffsets[0].z) || (1 != region.dstOffsets[1].z)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00252" : "VUID-vkCmdBlitImage-dstImage-00252";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffsets[0].z is %" PRId32 " and dstOffsets[1].z is %" PRId32 " but dstImage is %s.",
                                 region.dstOffsets[0].z, region.dstOffsets[1].z, string_VkImageType(dst_type));
            }
        }

        oob = false;
        if ((region.dstOffsets[0].x < 0) || (region.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width)) ||
            (region.dstOffsets[1].x < 0) || (region.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].x is %" PRId32 " and dstOffsets[1].x is %" PRId32
                             " which exceed dstSubresource width extent (%" PRIu32 ").",
                             region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.width);
        }
        if ((region.dstOffsets[0].y < 0) || (region.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height)) ||
            (region.dstOffsets[1].y < 0) || (region.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].y is %" PRId32 " and dstOffsets[1].y is %" PRId32
                             " which exceed dstSubresource height extent (%" PRIu32 ").",
                             region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.height);
        }
        if ((region.dstOffsets[0].z < 0) || (region.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth)) ||
            (region.dstOffsets[1].z < 0) || (region.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].z is %" PRId32 " and dstOffsets[1].z is %" PRId32
                             " which exceed dstSubresource depth extent (%" PRIu32 ").",
                             region.dstOffsets[0].z, region.dstOffsets[1].z, dst_extent.depth);
        }
        if (oob) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00216" : "VUID-vkCmdBlitImage-pRegions-00216";
            skip |= LogError(vuid, dst_objlist, region_loc, "destination image blit region exceeds image dimensions.");
        }

        if ((VK_IMAGE_TYPE_3D == src_type) || (VK_IMAGE_TYPE_3D == dst_type)) {
            if ((0 != src_subresource.baseArrayLayer) || (1 != src_subresource.layerCount) ||
                (0 != dst_subresource.baseArrayLayer) || (1 != dst_subresource.layerCount)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00240" : "VUID-vkCmdBlitImage-srcImage-00240";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage %s\n"
                                 "dstImage %s\n"
                                 "srcSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %" PRIu32
                                 ")\n"
                                 "dstSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %" PRIu32 ")\n",
                                 string_VkImageType(src_type), string_VkImageType(dst_type), src_subresource.baseArrayLayer,
                                 src_subresource.layerCount, dst_subresource.baseArrayLayer, dst_subresource.layerCount);
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (srcImage == dstImage) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersectsBlit(&region, &pRegions[j], src_image_state->createInfo.imageType,
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
void CoreChecks::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                    VkFilter filter) {
    auto cb_state_ptr = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
        }
    }
}

void CoreChecks::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageBlit *pRegions, VkFilter filter) {
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CoreChecks::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter);
}

void CoreChecks::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter);
}

template <typename RegionType>
bool CoreChecks::ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (!cb_state_ptr || !src_image_state || !dst_image_state) {
        return skip;
    }

    const bool is_2 = loc.function == Func::vkCmdResolveImage2 || loc.function == Func::vkCmdResolveImage2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00256" : "VUID-vkCmdResolveImage-srcImage-00256";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, srcImage), *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00258" : "VUID-vkCmdResolveImage-dstImage-00258";
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(commandBuffer, dstImage), *dst_image_state, dst_image_loc, vuid);
    skip |= ValidateCmd(cb_state, loc);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02003" : "VUID-vkCmdResolveImage-dstImage-02003";
    skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT,
                                            dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01837" : "VUID-vkCmdResolveImage-commandBuffer-01837";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01838" : "VUID-vkCmdResolveImage-commandBuffer-01838";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01839" : "VUID-vkCmdResolveImage-commandBuffer-01839";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06762" : "VUID-vkCmdResolveImage-srcImage-06762";
    skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06763" : "VUID-vkCmdResolveImage-srcImage-06763";
    skip |=
        ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, src_image_loc, vuid);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06764" : "VUID-vkCmdResolveImage-dstImage-06764";
    skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);
    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06765" : "VUID-vkCmdResolveImage-dstImage-06765";
    skip |=
        ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, dst_image_loc, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const LogObjectList objlist(commandBuffer, srcImage);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
        skip |= LogError(vuid, objlist, src_image_loc,
                         "must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const LogObjectList objlist(commandBuffer, dstImage);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
        skip |= LogError(vuid, objlist, dst_image_loc,
                         "must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }

    const char *invalid_src_layout_vuid =
        is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-01400" : "VUID-vkCmdResolveImage-srcImageLayout-01400";
    const char *invalid_dst_layout_vuid =
        is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-01401" : "VUID-vkCmdResolveImage-dstImageLayout-01401";
    // For each region, the number of layers in the image subresource should not be zero
    // For each region, src and dest image aspect must be color only
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(cb_state.commandBuffer(), &dst_subresource, dst_subresource_loc);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-00260" : "VUID-vkCmdResolveImage-srcImageLayout-00260";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, src_subresource, srcImageLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_image_loc, invalid_src_layout_vuid, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-00262" : "VUID-vkCmdResolveImage-dstImageLayout-00262";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, dst_subresource, dstImageLayout,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst_image_loc, invalid_dst_layout_vuid, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcSubresource-01709" : "VUID-vkCmdResolveImage-srcSubresource-01709";
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, src_subresource.mipLevel,
                                      src_subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstSubresource-01710" : "VUID-vkCmdResolveImage-dstSubresource-01710";
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, dst_subresource.mipLevel,
                                      dst_subresource_loc.dot(Field::mipLevel), vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcSubresource-01711" : "VUID-vkCmdResolveImage-srcSubresource-01711";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, src_subresource.baseArrayLayer,
                                             src_subresource.layerCount, src_subresource_loc, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstSubresource-01712" : "VUID-vkCmdResolveImage-dstSubresource-01712";
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, dst_subresource.baseArrayLayer,
                                             dst_subresource.layerCount, dst_subresource_loc, vuid);

        // layer counts must match
        if (src_subresource.layerCount != dst_subresource.layerCount && src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS &&
            dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
            const LogObjectList objlist(commandBuffer, srcImage, dstImage);
            vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08803" : "VUID-VkImageResolve-layerCount-08803";
            skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.srcSubresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.dstSubresource.layerCount);
        }
        // For each region, src and dest image aspect must be color only
        if ((src_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
            (dst_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT)) {
            const LogObjectList objlist(commandBuffer, srcImage, dstImage);
            vuid = is_2 ? "VUID-VkImageResolve2-aspectMask-00266" : "VUID-VkImageResolve-aspectMask-00266";
            skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) and dstSubresource.aspectMask (%s) must only be VK_IMAGE_ASPECT_COLOR_BIT.",
                             string_VkImageAspectFlags(src_subresource.aspectMask).c_str(),
                             string_VkImageAspectFlags(dst_subresource.aspectMask).c_str());
        }

        const VkImageType src_image_type = src_image_state->createInfo.imageType;
        const VkImageType dst_image_type = dst_image_state->createInfo.imageType;

        if (VK_IMAGE_TYPE_3D == dst_image_type) {
            if (src_subresource.layerCount != 1) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04446" : "VUID-vkCmdResolveImage-srcImage-04446";
                skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::layerCount), "is %" PRIu32 " but dstImage is 3D.",
                                 src_subresource.layerCount);
            }
            if ((dst_subresource.baseArrayLayer != 0) || (dst_subresource.layerCount != 1)) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04447" : "VUID-vkCmdResolveImage-srcImage-04447";
                skip |= LogError(vuid, objlist, dst_subresource_loc.dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and layerCount is %" PRIu32 " but dstImage 3D.", dst_subresource.baseArrayLayer,
                                 dst_subresource.layerCount);
            }
        }

        if (VK_IMAGE_TYPE_1D == src_image_type) {
            if ((region.srcOffset.y != 0) || (region.extent.height != 1)) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00271" : "VUID-vkCmdResolveImage-srcImage-00271";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but srcImage (%s) is 1D.",
                                 region.srcOffset.y, region.extent.height, FormatHandle(src_image_state->image()).c_str());
            }
        }
        if ((VK_IMAGE_TYPE_1D == src_image_type) || (VK_IMAGE_TYPE_2D == src_image_type)) {
            if ((region.srcOffset.z != 0) || (region.extent.depth != 1)) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00273" : "VUID-vkCmdResolveImage-srcImage-00273";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but srcImage (%s) is 2D.",
                                 region.srcOffset.z, region.extent.depth, FormatHandle(src_image_state->image()).c_str());
            }
        }

        if (VK_IMAGE_TYPE_1D == dst_image_type) {
            if ((region.dstOffset.y != 0) || (region.extent.height != 1)) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00276" : "VUID-vkCmdResolveImage-dstImage-00276";
                skip |= LogError(vuid, objlist, region_loc,
                                 "dstOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but dstImage (%s) is 1D.",
                                 region.dstOffset.y, region.extent.height, FormatHandle(dst_image_state->image()).c_str());
            }
        }
        if ((VK_IMAGE_TYPE_1D == dst_image_type) || (VK_IMAGE_TYPE_2D == dst_image_type)) {
            if ((region.dstOffset.z != 0) || (region.extent.depth != 1)) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00278" : "VUID-vkCmdResolveImage-dstImage-00278";
                skip |= LogError(vuid, objlist, region_loc,
                                 "dstOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but dstImage (%s) is 2D.",
                                 region.dstOffset.z, region.extent.depth, FormatHandle(dst_image_state->image()).c_str());
            }
        }

        // Each srcImage dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
        // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
        // developer
        if (src_subresource.mipLevel < src_image_state->createInfo.mipLevels) {
            uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &(region.extent), &subresource_extent);
            if ((extent_check & kXBit) != 0) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.width (%" PRIu32
                                 ") exceeds srcSubresource.extent.width (%" PRIu32 ").",
                                 region.srcOffset.x, region.extent.width, subresource_extent.width);
            }

            if ((extent_check & kYBit) != 0) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.height (%" PRIu32
                                 ") exceeds srcSubresource.extent.height (%" PRIu32 ").",
                                 region.srcOffset.y, region.extent.height, subresource_extent.height);
            }

            if ((extent_check & kZBit) != 0) {
                const LogObjectList objlist(commandBuffer, srcImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                skip |= LogError(vuid, objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.depth (%" PRIu32
                                 ") exceeds srcSubresource.extent.depth (%" PRIu32 ").",
                                 region.srcOffset.z, region.extent.depth, subresource_extent.depth);
            }
        }

        // Each dstImage dimension offset + extent limits must fall with image subresource extent
        subresource_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
        // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
        // developer
        if (dst_subresource.mipLevel < dst_image_state->createInfo.mipLevels) {
            uint32_t extent_check = ExceedsBounds(&(region.dstOffset), &(region.extent), &subresource_extent);
            if ((extent_check & kXBit) != 0) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                skip |= LogError(vuid, objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.width (%" PRIu32
                                 ") exceeds dstSubresource.extent.width (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.width, subresource_extent.width);
            }

            if ((extent_check & kYBit) != 0) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                skip |= LogError(vuid, objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.height (%" PRIu32
                                 ") exceeds dstSubresource.extent.height (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.height, subresource_extent.height);
            }

            if ((extent_check & kZBit) != 0) {
                const LogObjectList objlist(commandBuffer, dstImage);
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                skip |= LogError(vuid, objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.depth (%" PRIu32
                                 ") exceeds dstSubresource.extent.depth (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.depth, subresource_extent.depth);
            }
        }
    }

    if (src_image_state->createInfo.format != dst_image_state->createInfo.format) {
        const LogObjectList objlist(commandBuffer, srcImage, dstImage);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-01386" : "VUID-vkCmdResolveImage-srcImage-01386";
        skip |= LogError(vuid, objlist, src_image_loc, "was created with format %s but dstImage format is %s.",
                         string_VkFormat(src_image_state->createInfo.format), string_VkFormat(dst_image_state->createInfo.format));
    }
    if (src_image_state->createInfo.samples == VK_SAMPLE_COUNT_1_BIT) {
        const LogObjectList objlist(commandBuffer, srcImage);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00257" : "VUID-vkCmdResolveImage-srcImage-00257";
        skip |= LogError(vuid, objlist, src_image_loc, "was created with sample count VK_SAMPLE_COUNT_1_BIT.");
    }
    if (dst_image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
        const LogObjectList objlist(commandBuffer, dstImage);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00259" : "VUID-vkCmdResolveImage-dstImage-00259";
        skip |= LogError(vuid, objlist, dst_image_loc, "was created with sample count (%s) (not VK_SAMPLE_COUNT_1_BIT).",
                         string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
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

bool CoreChecks::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                   pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                   pResolveImageInfo->pRegions, error_obj.location.dot(Field::pResolveImageInfo));
}
