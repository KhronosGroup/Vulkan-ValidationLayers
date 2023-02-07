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
#include <vector>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

// Returns true if [x, xoffset] and [y, yoffset] overlap
static bool RangesIntersect(int32_t start, uint32_t start_offset, int32_t end, uint32_t end_offset) {
    bool result = false;
    uint32_t intersection_min = std::max(static_cast<uint32_t>(start), static_cast<uint32_t>(end));
    uint32_t intersection_max = std::min(static_cast<uint32_t>(start) + start_offset, static_cast<uint32_t>(end) + end_offset);

    if (intersection_max > intersection_min) {
        result = true;
    }
    return result;
}

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static bool RegionIntersects(const RegionType *region0, const RegionType *region1, VkImageType type, bool is_multiplane) {
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
                result &= RangesIntersect(region0->srcOffset.z, region0->extent.depth, region1->dstOffset.z, region1->extent.depth);
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                result &=
                    RangesIntersect(region0->srcOffset.y, region0->extent.height, region1->dstOffset.y, region1->extent.height);
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                result &= RangesIntersect(region0->srcOffset.x, region0->extent.width, region1->dstOffset.x, region1->extent.width);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
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
VkExtent3D CoreChecks::GetScaledItg(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE *img) const {
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    VkExtent3D granularity = {0, 0, 0};
    const auto pool = cb_state.command_pool;
    if (pool) {
        granularity = physical_device_state->queue_family_properties[pool->queueFamilyIndex].minImageTransferGranularity;
        if (FormatIsBlockedImage(img->createInfo.format)) {
            auto block_size = FormatTexelBlockExtent(img->createInfo.format);
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
                                const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset.x));
    offset_extent.height = static_cast<uint32_t>(abs(offset.y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset.z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (IsExtentAllZeroes(offset_extent) == false) {
            skip |= LogError(objlist, vuid,
                             "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) must be (x=0, y=0, z=0) when the command buffer's queue family "
                             "image transfer granularity is (w=0, h=0, d=0).",
                             function, i, member, offset.x, offset.y, offset.z);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        if (IsExtentAligned(offset_extent, granularity) == false) {
            skip |= LogError(objlist, vuid,
                             "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d).",
                             function, i, member, offset.x, offset.y, offset.z, granularity.width, granularity.height,
                             granularity.depth);
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
                                const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (IsExtentEqual(extent, subresource_extent) == false) {
            skip |= LogError(objlist, vuid,
                             "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d) "
                             "when the command buffer's queue family image transfer granularity is (w=0, h=0, d=0).",
                             function, i, member, extent.width, extent.height, extent.depth, subresource_extent.width,
                             subresource_extent.height, subresource_extent.depth);
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
            skip |= LogError(objlist, vuid,
                             "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d) or offset (x=%d, y=%d, z=%d) + "
                             "extent (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d).",
                             function, i, member, extent.width, extent.height, extent.depth, granularity.width, granularity.height,
                             granularity.depth, offset.x, offset.y, offset.z, extent.width, extent.height, extent.depth,
                             subresource_extent.width, subresource_extent.height, subresource_extent.depth);
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageMipLevel(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &img, uint32_t mip_level,
                                       const uint32_t i, const char *function, const char *member, const char *vuid) const {
    bool skip = false;
    if (mip_level >= img.createInfo.mipLevels) {
        LogObjectList objlist(cb_state.Handle(), img.Handle());
        skip |= LogError(objlist, vuid,
                         "In %s, pRegions[%" PRIu32 "].%s.mipLevel is %" PRIu32 ", but provided %s has %" PRIu32 " mip levels.",
                         function, i, member, mip_level, report_data->FormatHandle(img.image()).c_str(), img.createInfo.mipLevels);
    }
    return skip;
}

bool CoreChecks::ValidateImageArrayLayerRange(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &img, const uint32_t base_layer,
                                              const uint32_t layer_count, const uint32_t i, const char *function,
                                              const char *member, const char *vuid) const {
    bool skip = false;
    if (base_layer >= img.createInfo.arrayLayers || layer_count > img.createInfo.arrayLayers ||
        (base_layer + layer_count) > img.createInfo.arrayLayers) {
        if (layer_count == VK_REMAINING_ARRAY_LAYERS) {
            LogObjectList objlist(cb_state.Handle(), img.Handle());
            skip |= LogError(objlist, vuid,
                             "In %s, pRegions[%" PRIu32
                             "].%s.layerCount is VK_REMAINING_ARRAY_LAYERS, "
                             "but this special value is not supported here.",
                             function, i, member);
        } else {
            LogObjectList objlist(cb_state.Handle(), img.Handle());
            skip |= LogError(objlist, vuid,
                             "In %s, pRegions[%" PRIu32 "].%s.baseArrayLayer is %" PRIu32
                             " and .layerCount is "
                             "%" PRIu32 ", but provided %s has %" PRIu32 " array layers.",
                             function, i, member, base_layer, layer_count, report_data->FormatHandle(img.image()).c_str(),
                             img.createInfo.arrayLayers);
        }
    }
    return skip;
}

// All VUID from copy_bufferimage_to_imagebuffer_common.txt
static const char *GetBufferImageCopyCommandVUID(const std::string &id, bool image_to_buffer, bool copy2) {
    // clang-format off
    static const std::map<std::string, std::array<const char *, 4>> copy_imagebuffer_vuid = {
        {"00193", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-00193",      // !copy2 & !image_to_buffer
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-00193",      // !copy2 &  image_to_buffer
            "VUID-VkCopyBufferToImageInfo2-bufferOffset-00193", //  copy2 & !image_to_buffer
            "VUID-VkCopyImageToBufferInfo2-bufferOffset-00193", //  copy2 &  image_to_buffer
        }},
        {"01558", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-01558",
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-01558",
            "VUID-VkCopyBufferToImageInfo2-bufferOffset-01558",
            "VUID-VkCopyImageToBufferInfo2-bufferOffset-01558",
        }},
        {"01559", {
            "VUID-vkCmdCopyBufferToImage-bufferOffset-01559",
            "VUID-vkCmdCopyImageToBuffer-bufferOffset-01559",
            "VUID-VkCopyBufferToImageInfo2-bufferOffset-01559",
            "VUID-VkCopyImageToBufferInfo2-bufferOffset-01559",
        }},
        {"00197", {
            "VUID-vkCmdCopyBufferToImage-pRegions-06218",
            "VUID-vkCmdCopyImageToBuffer-pRegions-06221",
            "VUID-VkCopyBufferToImageInfo2-pRegions-06223",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-00197",
        }},
        {"00198", {
            "VUID-vkCmdCopyBufferToImage-pRegions-06219",
            "VUID-vkCmdCopyImageToBuffer-pRegions-06222",
            "VUID-VkCopyBufferToImageInfo2-pRegions-06224",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-00198",
        }},
        {"00199", {
            "VUID-vkCmdCopyBufferToImage-srcImage-00199",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00199",
            "VUID-VkCopyBufferToImageInfo2-srcImage-00199",
            "VUID-VkCopyImageToBufferInfo2-srcImage-00199",
        }},
        {"00200", {
            "VUID-vkCmdCopyBufferToImage-imageOffset-00200",
            "VUID-vkCmdCopyImageToBuffer-imageOffset-00200",
            "VUID-VkCopyBufferToImageInfo2-imageOffset-00200",
            "VUID-VkCopyImageToBufferInfo2-imageOffset-00200",
        }},
        {"00201", {
            "VUID-vkCmdCopyBufferToImage-srcImage-00201",
            "VUID-vkCmdCopyImageToBuffer-srcImage-00201",
            "VUID-VkCopyBufferToImageInfo2-srcImage-00201",
            "VUID-VkCopyImageToBufferInfo2-srcImage-00201",
        }},
        {"00203", {
            "VUID-vkCmdCopyBufferToImage-bufferRowLength-00203",
            "VUID-vkCmdCopyImageToBuffer-bufferRowLength-00203",
            "VUID-VkCopyBufferToImageInfo2-bufferRowLength-00203",
            "VUID-VkCopyImageToBufferInfo2-bufferRowLength-00203",
        }},
        {"00204", {
            "VUID-vkCmdCopyBufferToImage-bufferImageHeight-00204",
            "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-00204",
            "VUID-VkCopyBufferToImageInfo2-bufferImageHeight-00204",
            "VUID-VkCopyImageToBufferInfo2-bufferImageHeight-00204",
        }},
        {"07273", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07273",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07273",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07273",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07273",
        }},
        {"07274", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07274",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07274",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07274",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07274",
        }},
        {"07275", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07275",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07275",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07275",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07275",
        }},
        {"07276", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07276",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07276",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07276",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07276",
        }},
        {"00207", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00207",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00207",
            "VUID-VkCopyBufferToImageInfo2-imageExtent-00207",
            "VUID-VkCopyImageToBufferInfo2-imageExtent-00207",
        }},
        {"00208", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00208",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00208",
            "VUID-VkCopyBufferToImageInfo2-imageExtent-00208",
            "VUID-VkCopyImageToBufferInfo2-imageExtent-00208",
        }},
        {"00209", {
            "VUID-vkCmdCopyBufferToImage-imageExtent-00209",
            "VUID-vkCmdCopyImageToBuffer-imageExtent-00209",
            "VUID-VkCopyBufferToImageInfo2-imageExtent-00209",
            "VUID-VkCopyImageToBufferInfo2-imageExtent-00209",
        }},
        {"00211", {
            "VUID-vkCmdCopyBufferToImage-aspectMask-00211",
            "VUID-vkCmdCopyImageToBuffer-aspectMask-00211",
            "VUID-VkCopyBufferToImageInfo2-aspectMask-00211",
            "VUID-VkCopyImageToBufferInfo2-aspectMask-00211",
        }},
        {"07740", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07740",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07740",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07740",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07740",
        }},
        {"07741", {
            "VUID-vkCmdCopyBufferToImage-pRegions-07741",
            "VUID-vkCmdCopyImageToBuffer-pRegions-07741",
            "VUID-VkCopyBufferToImageInfo2-pRegions-07741",
            "VUID-VkCopyImageToBufferInfo2-pRegions-07741",
        }},
        {"00213", {
            "VUID-vkCmdCopyBufferToImage-baseArrayLayer-00213",
            "VUID-vkCmdCopyImageToBuffer-baseArrayLayer-00213",
            "VUID-VkCopyBufferToImageInfo2-baseArrayLayer-00213",
            "VUID-VkCopyImageToBufferInfo2-baseArrayLayer-00213",
        }},
        {"04052", {
            // was split up in 1.3.236 spec (internal MR 5371)
            "VUID-vkCmdCopyBufferToImage-commandBuffer-07737",
            "VUID-vkCmdCopyImageToBuffer-commandBuffer-07746",
            "VUID-vkCmdCopyBufferToImage2-commandBuffer-07737",
            "VUID-vkCmdCopyImageToBuffer2-commandBuffer-07746",
        }},
        {"04053", {
            "VUID-vkCmdCopyBufferToImage-srcImage-04053",
            "VUID-vkCmdCopyImageToBuffer-srcImage-04053",
            "VUID-VkCopyBufferToImageInfo2-srcImage-04053",
            "VUID-VkCopyImageToBufferInfo2-srcImage-04053",
        }}
    };
    // clang-format on

    uint8_t index = 0;
    index |= uint8_t((image_to_buffer) ? 0x1 : 0);
    index |= uint8_t((copy2) ? 0x2 : 0);
    return copy_imagebuffer_vuid.at(id).at(index);
}

static bool VerifyAspectsPresent(VkImageAspectFlags aspect_mask, VkFormat format) {
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        if (!(FormatIsColor(format) || FormatIsMultiplane(format))) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        if (!FormatHasDepth(format)) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        if (!FormatHasStencil(format)) return false;
    }
    if (0 != (aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
        if (FormatPlaneCount(format) == 1) return false;
    }
    return true;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferImageCopyData(const CMD_BUFFER_STATE &cb_state, uint32_t regionCount, const RegionType *pRegions,
                                             const IMAGE_STATE *image_state, const char *function, CMD_TYPE cmd_type,
                                             bool image_to_buffer) const {
    bool skip = false;
    const bool is_2 = (cmd_type == CMD_COPYBUFFERTOIMAGE2KHR || cmd_type == CMD_COPYBUFFERTOIMAGE2);
    const char *vuid;

    assert(image_state != nullptr);
    const VkFormat image_format = image_state->createInfo.format;

    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((region.imageOffset.y != 0) || (region.imageExtent.height != 1)) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00199", image_to_buffer, is_2),
                                 "%s: pRegion[%d] imageOffset.y is %d and imageExtent.height is %d. For 1D images these must be 0 "
                                 "and 1, respectively.",
                                 function, i, region.imageOffset.y, region.imageExtent.height);
            }
        }

        if ((image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) || (image_state->createInfo.imageType == VK_IMAGE_TYPE_2D)) {
            if ((region.imageOffset.z != 0) || (region.imageExtent.depth != 1)) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00201", image_to_buffer, is_2),
                                 "%s: pRegion[%d] imageOffset.z is %d and imageExtent.depth is %d. For 1D and 2D images these "
                                 "must be 0 and 1, respectively.",
                                 function, i, region.imageOffset.z, region.imageExtent.depth);
            }
        }

        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != region.imageSubresource.baseArrayLayer) || (1 != region.imageSubresource.layerCount)) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00213", image_to_buffer, is_2),
                                 "%s: pRegion[%d] imageSubresource.baseArrayLayer is %d and imageSubresource.layerCount is %d. "
                                 "For 3D images these must be 0 and 1, respectively.",
                                 function, i, region.imageSubresource.baseArrayLayer, region.imageSubresource.layerCount);
            }
        }

        // If the the calling command's VkImage parameter's format is not a depth/stencil format,
        // then bufferOffset must be a multiple of the calling command's VkImage parameter's element size
        const uint32_t element_size =
            FormatIsDepthOrStencil(image_format) ? 0 : FormatElementSize(image_format, region_aspect_mask);
        const VkDeviceSize bufferOffset = region.bufferOffset;

        if (FormatIsDepthOrStencil(image_format)) {
            if (SafeModulo(bufferOffset, 4) != 0) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("04053", image_to_buffer, is_2),
                                 "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                                 " must be a multiple 4 if using a depth/stencil format (%s).",
                                 function, i, bufferOffset, string_VkFormat(image_format));
            }
        } else {
            // If not depth/stencil and not multi-plane
            if (!FormatIsMultiplane(image_format) && (SafeModulo(bufferOffset, element_size) != 0)) {
                vuid = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)
                           ? GetBufferImageCopyCommandVUID("01558", image_to_buffer, is_2)
                           : GetBufferImageCopyCommandVUID("00193", image_to_buffer, is_2);
                skip |= LogError(image_state->image(), vuid,
                                 "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                                 " must be a multiple of this format's texel size (%" PRIu32 ").",
                                 function, i, bufferOffset, element_size);
            }
        }

        // Make sure not a empty region
        if (region.imageExtent.width == 0) {
            vuid = is_2 ? "VUID-VkBufferImageCopy2-imageExtent-06659" : "VUID-VkBufferImageCopy-imageExtent-06659";
            const LogObjectList objlist(cb_state.commandBuffer(), image_state->image());
            skip |=
                LogError(objlist, vuid, "%s: pRegion[%" PRIu32 "] extent.width must not be zero as empty copies are not allowed.",
                         function, i);
        }
        if (region.imageExtent.height == 0) {
            vuid = is_2 ? "VUID-VkBufferImageCopy2-imageExtent-06660" : "VUID-VkBufferImageCopy-imageExtent-06660";
            const LogObjectList objlist(cb_state.commandBuffer(), image_state->image());
            skip |=
                LogError(objlist, vuid, "%s: pRegion[%" PRIu32 "] extent.height must not be zero as empty copies are not allowed.",
                         function, i);
        }
        if (region.imageExtent.depth == 0) {
            vuid = is_2 ? "VUID-VkBufferImageCopy2-imageExtent-06661" : "VUID-VkBufferImageCopy-imageExtent-06661";
            const LogObjectList objlist(cb_state.commandBuffer(), image_state->image());
            skip |=
                LogError(objlist, vuid, "%s: pRegion[%" PRIu32 "] extent.depth must not be zero as empty copies are not allowed.",
                         function, i);
        }

        //  BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
        if ((region.bufferRowLength != 0) && (region.bufferRowLength < region.imageExtent.width)) {
            vuid = (is_2) ? "VUID-VkBufferImageCopy2-bufferRowLength-00195" : "VUID-VkBufferImageCopy-bufferRowLength-00195";
            skip |=
                LogError(image_state->image(), vuid,
                         "%s: pRegion[%d] bufferRowLength (%d) must be zero or greater-than-or-equal-to imageExtent.width (%d).",
                         function, i, region.bufferRowLength, region.imageExtent.width);
        }

        //  BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
        if ((region.bufferImageHeight != 0) && (region.bufferImageHeight < region.imageExtent.height)) {
            vuid = (is_2) ? "VUID-VkBufferImageCopy2-bufferImageHeight-00196" : "VUID-VkBufferImageCopy-bufferImageHeight-00196";
            skip |=
                LogError(image_state->image(), vuid,
                         "%s: pRegion[%d] bufferImageHeight (%d) must be zero or greater-than-or-equal-to imageExtent.height (%d).",
                         function, i, region.bufferImageHeight, region.imageExtent.height);
        }

        // Calculate adjusted image extent, accounting for multiplane image factors
        VkExtent3D adjusted_image_extent = image_state->GetSubresourceExtent(region.imageSubresource);
        // imageOffset.x and (imageExtent.width + imageOffset.x) must both be >= 0 and <= image subresource width
        if ((region.imageOffset.x < 0) || (region.imageOffset.x > static_cast<int32_t>(adjusted_image_extent.width)) ||
            ((region.imageOffset.x + static_cast<int32_t>(region.imageExtent.width)) >
             static_cast<int32_t>(adjusted_image_extent.width))) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00197", image_to_buffer, is_2),
                             "%s: Both pRegion[%d] imageoffset.x (%d) and (imageExtent.width + imageOffset.x) (%d) must be >= "
                             "zero or <= image subresource width (%d).",
                             function, i, region.imageOffset.x, (region.imageOffset.x + region.imageExtent.width),
                             adjusted_image_extent.width);
        }

        // imageOffset.y and (imageExtent.height + imageOffset.y) must both be >= 0 and <= image subresource height
        if ((region.imageOffset.y < 0) || (region.imageOffset.y > static_cast<int32_t>(adjusted_image_extent.height)) ||
            ((region.imageOffset.y + static_cast<int32_t>(region.imageExtent.height)) >
             static_cast<int32_t>(adjusted_image_extent.height))) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00198", image_to_buffer, is_2),
                             "%s: Both pRegion[%d] imageoffset.y (%d) and (imageExtent.height + imageOffset.y) (%d) must be >= "
                             "zero or <= image subresource height (%d).",
                             function, i, region.imageOffset.y, (region.imageOffset.y + region.imageExtent.height),
                             adjusted_image_extent.height);
        }

        // imageOffset.z and (imageExtent.depth + imageOffset.z) must both be >= 0 and <= image subresource depth
        if ((region.imageOffset.z < 0) || (region.imageOffset.z > static_cast<int32_t>(adjusted_image_extent.depth)) ||
            ((region.imageOffset.z + static_cast<int32_t>(region.imageExtent.depth)) >
             static_cast<int32_t>(adjusted_image_extent.depth))) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00200", image_to_buffer, is_2),
                             "%s: Both pRegion[%d] imageoffset.z (%d) and (imageExtent.depth + imageOffset.z) (%d) must be >= "
                             "zero or <= image subresource depth (%d).",
                             function, i, region.imageOffset.z, (region.imageOffset.z + region.imageExtent.depth),
                             adjusted_image_extent.depth);
        }

        // subresource aspectMask must have exactly 1 bit set
        if (GetBitSetCount(region_aspect_mask) != 1) {
            vuid = (is_2) ? "VUID-VkBufferImageCopy2-aspectMask-00212" : "VUID-VkBufferImageCopy-aspectMask-00212";
            skip |= LogError(image_state->image(), vuid,
                             "%s: aspectMasks for imageSubresource in pRegion[%d] must have only a single bit set (currently %s).",
                             function, i, string_VkImageAspectFlags(region_aspect_mask).c_str());
        }

        // image subresource aspect bit must match format
        if (!VerifyAspectsPresent(region_aspect_mask, image_format)) {
            skip |=
                LogError(image_state->image(), GetBufferImageCopyCommandVUID("00211", image_to_buffer, is_2),
                         "%s: pRegion[%d] subresource aspectMask 0x%x specifies aspects that are not present in image format 0x%x.",
                         function, i, region_aspect_mask, image_format);
        }

        auto block_size = FormatTexelBlockExtent(image_format);
        //  BufferRowLength must be a multiple of block width
        if (SafeModulo(region.bufferRowLength, block_size.width) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00203", image_to_buffer, is_2),
                             "%s: pRegion[%d] bufferRowLength (%d) must be a multiple of the blocked image's texel width (%d).",
                             function, i, region.bufferRowLength, block_size.width);
        }

        //  BufferRowHeight must be a multiple of block height
        if (SafeModulo(region.bufferImageHeight, block_size.height) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00204", image_to_buffer, is_2),
                             "%s: pRegion[%d] bufferImageHeight (%d) must be a multiple of the blocked image's texel height (%d).",
                             function, i, region.bufferImageHeight, block_size.height);
        }

        //  image offsets x must be multiple of block width
        if (SafeModulo(region.imageOffset.x, block_size.width) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07274", image_to_buffer, is_2),
                             "%s: pRegion[%d] imageOffset.x (%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "width (%" PRIu32 ").",
                             function, i, region.imageOffset.x, block_size.width);
        }

        //  image offsets y must be multiple of block height
        if (SafeModulo(region.imageOffset.y, block_size.height) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07275", image_to_buffer, is_2),
                             "%s: pRegion[%d] imageOffset.y (%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "height (%" PRIu32 ").",
                             function, i, region.imageOffset.y, block_size.height);
        }

        //  image offsets z must be multiple of block depth
        if (SafeModulo(region.imageOffset.z, block_size.depth) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07276", image_to_buffer, is_2),
                             "%s: pRegion[%d] imageOffset.z (%" PRId32
                             ") must be a multiple of the blocked image's texel "
                             "depth (%" PRIu32 ").",
                             function, i, region.imageOffset.z, block_size.depth);
        }

        // bufferOffset must be a multiple of block size (linear bytes)
        if (SafeModulo(bufferOffset, element_size) != 0) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07273", image_to_buffer, is_2),
                             "%s: pRegion[%d] bufferOffset (0x%" PRIxLEAST64
                             ") must be a multiple of the blocked image's texel block size (%" PRIu32 ").",
                             function, i, bufferOffset, element_size);
        }

        // imageExtent width must be a multiple of block width, or extent+offset width must equal subresource width
        VkExtent3D mip_extent = image_state->GetSubresourceExtent(region.imageSubresource);
        if ((SafeModulo(region.imageExtent.width, block_size.width) != 0) &&
            (region.imageExtent.width + region.imageOffset.x != mip_extent.width)) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00207", image_to_buffer, is_2),
                             "%s: pRegion[%d] extent width (%d) must be a multiple of the blocked texture block width "
                             "(%d), or when added to offset.x (%d) must equal the image subresource width (%d).",
                             function, i, region.imageExtent.width, block_size.width, region.imageOffset.x, mip_extent.width);
        }

        // imageExtent height must be a multiple of block height, or extent+offset height must equal subresource height
        if ((SafeModulo(region.imageExtent.height, block_size.height) != 0) &&
            (region.imageExtent.height + region.imageOffset.y != mip_extent.height)) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00208", image_to_buffer, is_2),
                             "%s: pRegion[%d] extent height (%d) must be a multiple of the blocked texture block height "
                             "(%d), or when added to offset.y (%d) must equal the image subresource height (%d).",
                             function, i, region.imageExtent.height, block_size.height, region.imageOffset.y, mip_extent.height);
        }

        // imageExtent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
        if ((SafeModulo(region.imageExtent.depth, block_size.depth) != 0) &&
            (region.imageExtent.depth + region.imageOffset.z != mip_extent.depth)) {
            skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("00209", image_to_buffer, is_2),
                             "%s: pRegion[%d] extent width (%d) must be a multiple of the blocked texture block depth "
                             "(%d), or when added to offset.z (%d) must equal the image subresource depth (%d).",
                             function, i, region.imageExtent.depth, block_size.depth, region.imageOffset.z, mip_extent.depth);
        }

        // Checks that apply only to multi-planar format images
        if (FormatIsMultiplane(image_format)) {
            // VK_IMAGE_ASPECT_PLANE_2_BIT valid only for image formats with three planes
            if ((FormatPlaneCount(image_format) < 3) && (region_aspect_mask == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07740", image_to_buffer, is_2),
                                 "%s: pRegion[%d] subresource aspectMask cannot be VK_IMAGE_ASPECT_PLANE_2_BIT unless image "
                                 "format has three planes.",
                                 function, i);
            }

            // image subresource aspectMask must be VK_IMAGE_ASPECT_PLANE_*_BIT
            if (0 ==
                (region_aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
                skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("07741", image_to_buffer, is_2),
                                 "%s: pRegion[%d] subresource aspectMask for multi-plane image formats must have a "
                                 "VK_IMAGE_ASPECT_PLANE_*_BIT when copying to or from.",
                                 function, i);
            } else {
                // Know aspect mask is valid
                const VkFormat compatible_format = FindMultiplaneCompatibleFormat(image_format, region_aspect_mask);
                const uint32_t compatible_size = FormatElementSize(compatible_format);
                if (SafeModulo(bufferOffset, compatible_size) != 0) {
                    skip |= LogError(image_state->image(), GetBufferImageCopyCommandVUID("01559", image_to_buffer, is_2),
                                     "%s: pRegion[%d]->bufferOffset is 0x%" PRIxLEAST64
                                     " but must be a multiple of the multi-plane compatible format's texel size (%" PRIu32
                                     ") for plane %" PRIu32 " (%s).",
                                     function, i, bufferOffset, element_size, GetPlaneIndex(region_aspect_mask),
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
            const LogObjectList objlist(cb_state.commandBuffer(), command_pool->commandPool(), image_state->image());
            skip |= LogError(objlist, GetBufferImageCopyCommandVUID("04052", image_to_buffer, is_2),
                             "%s: pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                             " must be a multiple 4 because the command buffer %s was allocated from the command pool %s "
                             "which was created with queueFamilyIndex %" PRIu32
                             ", which doesn't contain the VK_QUEUE_GRAPHICS_BIT or "
                             "VK_QUEUE_COMPUTE_BIT flag.",
                             function, i, bufferOffset, report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                             report_data->FormatHandle(command_pool->commandPool()).c_str(), queue_family_index);
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferBounds(VkCommandBuffer cb, const BUFFER_STATE &src_buffer_state,
                                             const BUFFER_STATE &dst_buffer_state, uint32_t regionCount, const RegionType *pRegions,
                                             CMD_TYPE cmd_type) const {
    bool skip = false;
    const bool is_2 = (cmd_type == CMD_COPYBUFFER2KHR || cmd_type == CMD_COPYBUFFER2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    VkDeviceSize src_buffer_size = src_buffer_state.createInfo.size;
    VkDeviceSize dst_buffer_size = dst_buffer_state.createInfo.size;
    const bool are_buffers_sparse = src_buffer_state.sparse || dst_buffer_state.sparse;

    const LogObjectList src_objlist(cb, dst_buffer_state.Handle());
    const LogObjectList dst_objlist(cb, dst_buffer_state.Handle());
    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];

        // The srcOffset member of each element of pRegions must be less than the size of srcBuffer
        if (region.srcOffset >= src_buffer_size) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcOffset-00113" : "VUID-vkCmdCopyBuffer-srcOffset-00113";
            skip |= LogError(src_objlist, vuid,
                             "%s: pRegions[%" PRIu32 "].srcOffset (%" PRIuLEAST64
                             ") is greater than size of srcBuffer (%" PRIuLEAST64 ").",
                             func_name, i, region.srcOffset, src_buffer_size);
        }

        // The dstOffset member of each element of pRegions must be less than the size of dstBuffer
        if (region.dstOffset >= dst_buffer_size) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstOffset-00114" : "VUID-vkCmdCopyBuffer-dstOffset-00114";
            skip |= LogError(dst_objlist, vuid,
                             "%s: pRegions[%" PRIu32 "].dstOffset (%" PRIuLEAST64
                             ") is greater than size of dstBuffer (%" PRIuLEAST64 ").",
                             func_name, i, region.dstOffset, dst_buffer_size);
        }

        // The size member of each element of pRegions must be less than or equal to the size of srcBuffer minus srcOffset
        if (region.size > (src_buffer_size - region.srcOffset)) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00115" : "VUID-vkCmdCopyBuffer-size-00115";
            skip |= LogError(src_objlist, vuid,
                             "%s: pRegions[%d].size (%" PRIuLEAST64 ") is greater than the source buffer size (%" PRIuLEAST64
                             ") minus pRegions[%d].srcOffset (%" PRIuLEAST64 ").",
                             func_name, i, region.size, src_buffer_size, i, region.srcOffset);
        }

        // The size member of each element of pRegions must be less than or equal to the size of dstBuffer minus dstOffset
        if (region.size > (dst_buffer_size - region.dstOffset)) {
            vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00116" : "VUID-vkCmdCopyBuffer-size-00116";
            skip |= LogError(dst_objlist, vuid,
                             "%s: pRegions[%d].size (%" PRIuLEAST64 ") is greater than the destination buffer size (%" PRIuLEAST64
                             ") minus pRegions[%d].dstOffset (%" PRIuLEAST64 ").",
                             func_name, i, region.size, dst_buffer_size, i, region.dstOffset);
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
                    skip |= LogError(objlist, vuid, "%s: Detected overlap between source and dest regions in memory.", func_name);
                }
            }
        }
    }

    return skip;
}
template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                       const RegionType *pRegions, CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !src_buffer_state || !dst_buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = (cmd_type == CMD_COPYBUFFER2KHR || cmd_type == CMD_COPYBUFFER2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00119" : "VUID-vkCmdCopyBuffer-srcBuffer-00119";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00121" : "VUID-vkCmdCopyBuffer-dstBuffer-00121";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, func_name, vuid);

    // Validate that SRC & DST buffers have correct usage flags set
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00118" : "VUID-vkCmdCopyBuffer-srcBuffer-00118";
    skip |= ValidateBufferUsageFlags(commandBuffer, *src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00120" : "VUID-vkCmdCopyBuffer-dstBuffer-00120";
    skip |= ValidateBufferUsageFlags(commandBuffer, *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");

    skip |= ValidateCmd(cb_state, cmd_type);
    skip |= ValidateCmdCopyBufferBounds(commandBuffer, *src_buffer_state, *dst_buffer_state, regionCount, pRegions, cmd_type);

    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01822" : "VUID-vkCmdCopyBuffer-commandBuffer-01822";
    skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01823" : "VUID-vkCmdCopyBuffer-commandBuffer-01823";
    skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01824" : "VUID-vkCmdCopyBuffer-commandBuffer-01824";
    skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, func_name, vuid);

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy *pRegions) const {
    return ValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, CMD_COPYBUFFER);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                  const VkCopyBufferInfo2KHR *pCopyBufferInfos) const {
    return ValidateCmdCopyBuffer(commandBuffer, pCopyBufferInfos->srcBuffer, pCopyBufferInfos->dstBuffer,
                                 pCopyBufferInfos->regionCount, pCopyBufferInfos->pRegions, CMD_COPYBUFFER2KHR);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) const {
    return ValidateCmdCopyBuffer(commandBuffer, pCopyBufferInfos->srcBuffer, pCopyBufferInfos->dstBuffer,
                                 pCopyBufferInfos->regionCount, pCopyBufferInfos->pRegions, CMD_COPYBUFFER2);
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkBufferImageCopy/VkBufferImageCopy2 structure
template <typename RegionType>
bool CoreChecks::ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &img,
                                                                        const RegionType *region, const uint32_t i,
                                                                        const char *function, const char *vuid) const {
    bool skip = false;
    const LogObjectList objlist(cb_state.Handle(), img.Handle());
    VkExtent3D granularity = GetScaledItg(cb_state, &img);
    skip |= CheckItgOffset(objlist, region->imageOffset, granularity, i, function, "imageOffset", vuid);
    VkExtent3D subresource_extent = img.GetSubresourceExtent(region->imageSubresource);
    skip |= CheckItgExtent(objlist, region->imageExtent, region->imageOffset, granularity, subresource_extent,
                           img.createInfo.imageType, i, function, "imageExtent", vuid);
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy/VkImageCopy2KHR structure
template <typename RegionType>
bool CoreChecks::ValidateCopyImageTransferGranularityRequirements(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE *src_img,
                                                                  const IMAGE_STATE *dst_img, const RegionType *region,
                                                                  const uint32_t i, const char *function, CMD_TYPE cmd_type) const {
    bool skip = false;
    const bool is_2 = (cmd_type == CMD_COPYIMAGE2KHR || cmd_type == CMD_COPYIMAGE2);
    const char *vuid;

    const VkExtent3D extent = region->extent;
    {
        // Source image checks
        const LogObjectList objlist(cb_state.Handle(), src_img->Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, src_img);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= CheckItgOffset(objlist, region->srcOffset, granularity, i, function, "srcOffset", vuid);
        const VkExtent3D subresource_extent = src_img->GetSubresourceExtent(region->srcSubresource);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= CheckItgExtent(objlist, extent, region->srcOffset, granularity, subresource_extent, src_img->createInfo.imageType,
                               i, function, "extent", vuid);
    }

    {
        // Destination image checks
        const LogObjectList objlist(cb_state.Handle(), dst_img->Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, dst_img);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= CheckItgOffset(objlist, region->dstOffset, granularity, i, function, "dstOffset", vuid);
        // Adjust dest extent, if necessary
        const VkExtent3D dest_effective_extent =
            GetAdjustedDestImageExtent(src_img->createInfo.format, dst_img->createInfo.format, extent);
        const VkExtent3D subresource_extent = dst_img->GetSubresourceExtent(region->dstSubresource);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= CheckItgExtent(objlist, dest_effective_extent, region->dstOffset, granularity, subresource_extent,
                               dst_img->createInfo.imageType, i, function, "extent", vuid);
    }
    return skip;
}

// Validate contents of a VkImageCopy or VkImageCopy2KHR struct
template <typename RegionType>
bool CoreChecks::ValidateImageCopyData(const uint32_t regionCount, const RegionType *pRegions, const IMAGE_STATE *src_state,
                                       const IMAGE_STATE *dst_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const bool is_2 = (cmd_type == CMD_COPYIMAGE2KHR || cmd_type == CMD_COPYIMAGE2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];

        // For comp<->uncomp copies, the copy extent for the dest image must be adjusted
        const VkExtent3D src_copy_extent = region.extent;
        const VkExtent3D dst_copy_extent =
            GetAdjustedDestImageExtent(src_state->createInfo.format, dst_state->createInfo.format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != dst_state->createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != src_state->createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        // Do all checks on source image
        if (src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.srcOffset.y) || (1 != src_copy_extent.height)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00146" : "VUID-vkCmdCopyImage-srcImage-00146";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.y is %d and extent.height is %d. For 1D images these must "
                                 "be 0 and 1, respectively.",
                                 func_name, i, region.srcOffset.y, src_copy_extent.height);
            }
        }

        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.srcOffset.z) || (1 != src_copy_extent.depth))) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01785" : "VUID-vkCmdCopyImage-srcImage-01785";
            skip |= LogError(src_state->image(), vuid,
                             "%s: pRegion[%d] srcOffset.z is %d and extent.depth is %d. For 1D images "
                             "these must be 0 and 1, respectively.",
                             func_name, i, region.srcOffset.z, src_copy_extent.depth);
        }

        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.srcOffset.z)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01787" : "VUID-vkCmdCopyImage-srcImage-01787";
            skip |= LogError(src_state->image(), vuid, "%s: pRegion[%d] srcOffset.z is %d. For 2D images the z-offset must be 0.",
                             func_name, i, region.srcOffset.z);
        }

        {  // Used to be compressed checks, now apply to all
            const VkExtent3D block_size = FormatTexelBlockExtent(src_state->createInfo.format);
            if (SafeModulo(region.srcOffset.x, block_size.width) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07278" : "VUID-vkCmdCopyImage-pRegions-07278";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.x (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "width (%" PRIu32 ").",
                                 func_name, i, region.srcOffset.x, block_size.width);
            }

            //  image offsets y must be multiple of block height
            if (SafeModulo(region.srcOffset.y, block_size.height) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07279" : "VUID-vkCmdCopyImage-pRegions-07279";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.y (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "height (%" PRIu32 ").",
                                 func_name, i, region.srcOffset.y, block_size.height);
            }

            //  image offsets z must be multiple of block depth
            if (SafeModulo(region.srcOffset.z, block_size.depth) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07280" : "VUID-vkCmdCopyImage-pRegions-07280";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.z (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "depth (%" PRIu32 ").",
                                 func_name, i, region.srcOffset.z, block_size.depth);
            }

            const VkExtent3D mip_extent = src_state->GetSubresourceExtent(region.srcSubresource);
            if ((SafeModulo(src_copy_extent.width, block_size.width) != 0) &&
                (src_copy_extent.width + region.srcOffset.x != mip_extent.width)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01728" : "VUID-vkCmdCopyImage-srcImage-01728";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the blocked texture block "
                                 "width (%d), or when added to srcOffset.x (%d) must equal the image subresource width (%d).",
                                 func_name, i, src_copy_extent.width, block_size.width, region.srcOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(src_copy_extent.height, block_size.height) != 0) &&
                (src_copy_extent.height + region.srcOffset.y != mip_extent.height)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01729" : "VUID-vkCmdCopyImage-srcImage-01729";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] extent height (%d) must be a multiple of the compressed texture block "
                                 "height (%d), or when added to srcOffset.y (%d) must equal the image subresource height (%d).",
                                 func_name, i, src_copy_extent.height, block_size.height, region.srcOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : src_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.srcOffset.z != mip_extent.depth)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01730" : "VUID-vkCmdCopyImage-srcImage-01730";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] extent width (%d) must be a multiple of the compressed texture block "
                                 "depth (%d), or when added to srcOffset.z (%d) must equal the image subresource depth (%d).",
                                 func_name, i, src_copy_extent.depth, block_size.depth, region.srcOffset.z, mip_extent.depth);
            }
        }

        // Do all checks on dest image
        if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.dstOffset.y) || (1 != dst_copy_extent.height)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-00152" : "VUID-vkCmdCopyImage-dstImage-00152";
                skip |= LogError(dst_state->image(), vuid,
                                 "%s: pRegion[%d] dstOffset.y is %d and dst_copy_extent.height is %d. For 1D images "
                                 "these must be 0 and 1, respectively.",
                                 func_name, i, region.dstOffset.y, dst_copy_extent.height);
            }
        }

        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.dstOffset.z) || (1 != dst_copy_extent.depth))) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01786" : "VUID-vkCmdCopyImage-dstImage-01786";
            skip |= LogError(dst_state->image(), vuid,
                             "%s: pRegion[%d] dstOffset.z is %d and extent.depth is %d. For 1D images these must be 0 "
                             "and 1, respectively.",
                             func_name, i, region.dstOffset.z, dst_copy_extent.depth);
        }

        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.dstOffset.z)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01788" : "VUID-vkCmdCopyImage-dstImage-01788";
            skip |= LogError(dst_state->image(), vuid, "%s: pRegion[%d] dstOffset.z is %d. For 2D images the z-offset must be 0.",
                             func_name, i, region.dstOffset.z);
        }

        // Handle difference between Maintenance 1
        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-04443" : "VUID-vkCmdCopyImage-srcImage-04443";
                    skip |= LogError(src_state->image(), vuid,
                                     "%s: pRegion[%d] srcSubresource.baseArrayLayer is %d and srcSubresource.layerCount "
                                     "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     func_name, i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
            }
            if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-04444" : "VUID-vkCmdCopyImage-dstImage-04444";
                    skip |= LogError(dst_state->image(), vuid,
                                     "%s: pRegion[%d] dstSubresource.baseArrayLayer is %d and dstSubresource.layerCount "
                                     "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                     func_name, i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        } else {  // Pre maint 1
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D || dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00139" : "VUID-vkCmdCopyImage-srcImage-00139";
                    skip |= LogError(src_state->image(), vuid,
                                     "%s: pRegion[%d] srcSubresource.baseArrayLayer is %d and "
                                     "srcSubresource.layerCount is %d. For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     func_name, i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00139" : "VUID-vkCmdCopyImage-srcImage-00139";
                    skip |= LogError(dst_state->image(), vuid,
                                     "%s: pRegion[%d] dstSubresource.baseArrayLayer is %d and "
                                     "dstSubresource.layerCount is %d. For copies with either source or dest of type "
                                     "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                     func_name, i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        }

        {
            const VkExtent3D block_size = FormatTexelBlockExtent(dst_state->createInfo.format);
            //  image offsets x must be multiple of block width
            if (SafeModulo(region.dstOffset.x, block_size.width) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07281" : "VUID-vkCmdCopyImage-pRegions-07281";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.x (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "width (%" PRIu32 ").",
                                 func_name, i, region.dstOffset.x, block_size.width);
            }

            //  image offsets y must be multiple of block height
            if (SafeModulo(region.dstOffset.y, block_size.height) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07282" : "VUID-vkCmdCopyImage-pRegions-07282";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.y (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "height (%" PRIu32 ").",
                                 func_name, i, region.dstOffset.y, block_size.height);
            }

            //  image offsets z must be multiple of block depth
            if (SafeModulo(region.dstOffset.z, block_size.depth) != 0) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-07283" : "VUID-vkCmdCopyImage-pRegions-07283";
                skip |= LogError(src_state->image(), vuid,
                                 "%s: pRegion[%d] srcOffset.z (%" PRId32
                                 ") must be a multiple of the blocked image's texel "
                                 "depth (%" PRIu32 ").",
                                 func_name, i, region.dstOffset.z, block_size.depth);
            }

            const VkExtent3D mip_extent = dst_state->GetSubresourceExtent(region.dstSubresource);
            if ((SafeModulo(dst_copy_extent.width, block_size.width) != 0) &&
                (dst_copy_extent.width + region.dstOffset.x != mip_extent.width)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01732" : "VUID-vkCmdCopyImage-dstImage-01732";
                skip |= LogError(dst_state->image(), vuid,
                                 "%s: pRegion[%d] dst_copy_extent width (%d) must be a multiple of the blocked texture "
                                 "block width (%d), or when added to dstOffset.x (%d) must equal the image subresource width (%d).",
                                 func_name, i, dst_copy_extent.width, block_size.width, region.dstOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or dst_copy_extent+offset height must equal subresource height
            if ((SafeModulo(dst_copy_extent.height, block_size.height) != 0) &&
                (dst_copy_extent.height + region.dstOffset.y != mip_extent.height)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
                skip |= LogError(dst_state->image(), vuid,
                                 "%s: pRegion[%d] dst_copy_extent height (%d) must be a multiple of the compressed "
                                 "texture block height (%d), or when added to dstOffset.y (%d) must equal the image subresource "
                                 "height (%d).",
                                 func_name, i, dst_copy_extent.height, block_size.height, region.dstOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or dst_copy_extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : dst_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.dstOffset.z != mip_extent.depth)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01734" : "VUID-vkCmdCopyImage-dstImage-01734";
                skip |= LogError(dst_state->image(), vuid,
                                 "%s: pRegion[%d] dst_copy_extent width (%d) must be a multiple of the compressed texture "
                                 "block depth (%d), or when added to dstOffset.z (%d) must equal the image subresource depth (%d).",
                                 func_name, i, dst_copy_extent.depth, block_size.depth, region.dstOffset.z, mip_extent.depth);
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

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, CMD_TYPE cmd_type) const {
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
    const bool is_2 = (cmd_type == CMD_COPYIMAGE2KHR || cmd_type == CMD_COPYIMAGE2);

    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;
    skip = ValidateImageCopyData(regionCount, pRegions, src_image_state.get(), dst_image_state.get(), cmd_type);

    VkCommandBuffer command_buffer = cb_state.commandBuffer();

    bool has_stencil_aspect = false;
    bool has_non_stencil_aspect = false;

    for (uint32_t i = 0; i < regionCount; i++) {
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

        skip |= ValidateImageSubresourceLayers(cb_state, &region.srcSubresource, func_name, "srcSubresource", i);
        skip |= ValidateImageSubresourceLayers(cb_state, &region.dstSubresource, func_name, "dstSubresource", i);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-01696" : "VUID-vkCmdCopyImage-srcSubresource-01696";
        skip |=
            ValidateImageMipLevel(cb_state, *src_image_state, region.srcSubresource.mipLevel, i, func_name, "srcSubresource", vuid);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstSubresource-01697" : "VUID-vkCmdCopyImage-dstSubresource-01697";
        skip |=
            ValidateImageMipLevel(cb_state, *dst_image_state, region.dstSubresource.mipLevel, i, func_name, "dstSubresource", vuid);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-01698" : "VUID-vkCmdCopyImage-srcSubresource-01698";
        skip |= ValidateImageArrayLayerRange(cb_state, *src_image_state, region.srcSubresource.baseArrayLayer,
                                             region.srcSubresource.layerCount, i, func_name, "srcSubresource", vuid);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstSubresource-01699" : "VUID-vkCmdCopyImage-dstSubresource-01699";
        skip |= ValidateImageArrayLayerRange(cb_state, *dst_image_state, region.dstSubresource.baseArrayLayer,
                                             region.dstSubresource.layerCount, i, func_name, "dstSubresource", vuid);

        if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
            // No chance of mismatch if we're overriding depth slice count
            if (!slice_override) {
                // The number of depth slices in srcSubresource and dstSubresource must match
                // Depth comes from layerCount for 1D,2D resources, from extent.depth for 3D
                uint32_t src_slices = (src_is_3d ? src_copy_extent.depth : region.srcSubresource.layerCount);
                uint32_t dst_slices = (dst_is_3d ? dst_copy_extent.depth : region.dstSubresource.layerCount);
                if (src_slices != dst_slices) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07744" : "VUID-vkCmdCopyImage-srcImage-07744";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: number of depth slices in source (%" PRIu32 ") and destination (%" PRIu32
                                     ") subresources for pRegions[%" PRIu32
                                     "] "
                                     "do not match.",
                                     func_name, src_slices, dst_slices, i);
                }
            }

            // Maintenance 1 requires both while prior only required one to be 2D
            if ((src_is_2d && dst_is_2d) && (src_copy_extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01790" : "VUID-vkCmdCopyImage-srcImage-01790";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%" PRIu32 "] both srcImage and dstImage are 2D and extent.depth is %" PRIu32
                                 " and has to be 1",
                                 func_name, i, src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                // if different, one must be 3D and the other 2D
                bool valid = (src_is_2d && dst_is_3d) || (src_is_3d && dst_is_2d);
                if (!valid) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07743" : "VUID-vkCmdCopyImage-srcImage-07743";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegion[%" PRIu32
                                     "] srcImage (%s) must be equal to dstImage (%s) or else one must be 2D and the other 3D",
                                     func_name, i, string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
                }
            }

            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01995" : "VUID-vkCmdCopyImage-srcImage-01995";
            skip |= ValidateImageFormatFeatureFlags(command_buffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    func_name, vuid);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01996" : "VUID-vkCmdCopyImage-dstImage-01996";
            skip |= ValidateImageFormatFeatureFlags(command_buffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    func_name, vuid);

            // Check if 2D with 3D and depth not equal to 2D layerCount
            if (src_is_2d && dst_is_3d && (src_copy_extent.depth != region.srcSubresource.layerCount)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01791" : "VUID-vkCmdCopyImage-srcImage-01791";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%" PRIu32 "] srcImage is 2D, dstImage is 3D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "srcSubresource.layerCount (%" PRIu32 ")",
                                 func_name, i, src_copy_extent.depth, region.srcSubresource.layerCount);
            } else if (src_is_3d && dst_is_2d && (src_copy_extent.depth != region.dstSubresource.layerCount)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01792" : "VUID-vkCmdCopyImage-dstImage-01792";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%" PRIu32 "] srcImage is 3D, dstImage is 2D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "dstSubresource.layerCount (%" PRIu32 ")",
                                 func_name, i, src_copy_extent.depth, region.dstSubresource.layerCount);
            }
        } else {  // !vk_khr_maintenance1
            // For each region the layerCount member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                vuid = is_2 ? "VUID-VkImageCopy2-layerCount-00138" : "VUID-VkImageCopy-layerCount-00138";
                skip |= LogError(command_buffer, vuid,
                                 "%s: number of layers in source (%" PRIu32 ") and destination (%" PRIu32
                                 ") subresources for pRegions[%" PRIu32 "] do not match",
                                 func_name, region.srcSubresource.layerCount, region.dstSubresource.layerCount, i);
            }

            if ((src_is_2d || dst_is_2d) && (src_copy_extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01789" : "VUID-vkCmdCopyImage-srcImage-01789";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%" PRIu32 "] either srcImage or dstImage is 2D and extent.depth is %" PRIu32
                                 " and has to be 1",
                                 func_name, i, src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07742" : "VUID-vkCmdCopyImage-srcImage-07742";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegion[%" PRIu32
                                 "] srcImage (%s) must be equal to dstImage (%s) without VK_KHR_maintenance1 enabled",
                                 func_name, i, string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
            }
        }

        // Do multiplane-specific checks, if extension enabled
        if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
            if ((!FormatIsMultiplane(src_format)) && (!FormatIsMultiplane(dst_format))) {
                // If neither image is multi-plane the aspectMask member of src and dst must match
                if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01551" : "VUID-vkCmdCopyImage-srcImage-01551";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: Copy between non-multiplane images with differing aspectMasks in pRegions[%" PRIu32
                                     "] with "
                                     "source (0x%x) destination (0x%x).",
                                     func_name, i, region.srcSubresource.aspectMask, region.dstSubresource.aspectMask);
                }
            } else {
                // Source image multiplane checks
                uint32_t planes = FormatPlaneCount(src_format);
                VkImageAspectFlags aspect = region.srcSubresource.aspectMask;
                if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01552" : "VUID-vkCmdCopyImage-srcImage-01552";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].srcSubresource.aspectMask (0x%x) is invalid for 2-plane format.",
                                     func_name, i, aspect);
                }
                if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                    (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01553" : "VUID-vkCmdCopyImage-srcImage-01553";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].srcSubresource.aspectMask (0x%x) is invalid for 3-plane format.",
                                     func_name, i, aspect);
                }
                // Single-plane to multi-plane
                if ((!FormatIsMultiplane(src_format)) && (FormatIsMultiplane(dst_format)) &&
                    (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01557" : "VUID-vkCmdCopyImage-dstImage-01557";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].srcSubresource.aspectMask (0x%x) is not VK_IMAGE_ASPECT_COLOR_BIT.",
                                     func_name, i, aspect);
                }

                // Dest image multiplane checks
                planes = FormatPlaneCount(dst_format);
                aspect = region.dstSubresource.aspectMask;
                if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01554" : "VUID-vkCmdCopyImage-dstImage-01554";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].dstSubresource.aspectMask (0x%x) is invalid for 2-plane format.",
                                     func_name, i, aspect);
                }
                if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                    (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01555" : "VUID-vkCmdCopyImage-dstImage-01555";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].dstSubresource.aspectMask (0x%x) is invalid for 3-plane format.",
                                     func_name, i, aspect);
                }
                // Multi-plane to single-plane
                if ((FormatIsMultiplane(src_format)) && (!FormatIsMultiplane(dst_format)) &&
                    (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01556" : "VUID-vkCmdCopyImage-srcImage-01556";
                    skip |= LogError(command_buffer, vuid,
                                     "%s: pRegions[%" PRIu32 "].dstSubresource.aspectMask (0x%x) is not VK_IMAGE_ASPECT_COLOR_BIT.",
                                     func_name, i, aspect);
                }
            }
        } else {
            // !vk_khr_sampler_ycbcr_conversion
            // not multi-plane, the aspectMask member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                vuid = is_2 ? "VUID-VkImageCopy2-aspectMask-00137" : "VUID-VkImageCopy-aspectMask-00137";
                skip |= LogError(command_buffer, vuid,
                                 "%s: Copy between images with differing aspectMasks in pRegions[%" PRIu32
                                 "] with source (0x%x) destination (0x%x).",
                                 func_name, i, region.srcSubresource.aspectMask, region.dstSubresource.aspectMask);
            }
        }

        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!VerifyAspectsPresent(region.srcSubresource.aspectMask, src_format)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspectMask-00142" : "VUID-vkCmdCopyImage-aspectMask-00142";
            skip |= LogError(command_buffer, vuid,
                             "%s: pRegions[%" PRIu32
                             "].srcSubresource.aspectMask (0x%x) cannot specify aspects not present in source image.",
                             func_name, i, region.srcSubresource.aspectMask);
        }

        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!VerifyAspectsPresent(region.dstSubresource.aspectMask, dst_format)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspectMask-00143" : "VUID-vkCmdCopyImage-aspectMask-00143";
            skip |= LogError(command_buffer, vuid,
                             "%s: pRegions[%" PRIu32
                             "].dstSubresource.aspectMask (0x%x) cannot specify aspects not present in destination image.",
                             func_name, i, region.dstSubresource.aspectMask);
        }

        // Make sure not a empty region
        if (src_copy_extent.width == 0) {
            vuid = is_2 ? "VUID-VkImageCopy2-extent-06668" : "VUID-VkImageCopy-extent-06668";
            skip |=
                LogError(command_buffer, vuid,
                         "%s: pRegion[%" PRIu32 "] extent.width must not be zero as empty copies are not allowed.", func_name, i);
        }
        if (src_copy_extent.height == 0) {
            vuid = is_2 ? "VUID-VkImageCopy2-extent-06669" : "VUID-VkImageCopy-extent-06669";
            skip |=
                LogError(command_buffer, vuid,
                         "%s: pRegion[%" PRIu32 "] extent.height must not be zero as empty copies are not allowed.", func_name, i);
        }
        if (src_copy_extent.depth == 0) {
            vuid = is_2 ? "VUID-VkImageCopy2-extent-06670" : "VUID-VkImageCopy-extent-06670";
            skip |=
                LogError(command_buffer, vuid,
                         "%s: pRegion[%" PRIu32 "] extent.depth must not be zero as empty copies are not allowed.", func_name, i);
        }

        // Each dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = src_image_state->GetSubresourceExtent(region.srcSubresource);
        if (slice_override) src_copy_extent.depth = depth_slices;
        uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &src_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-00144" : "VUID-vkCmdCopyImage-srcOffset-00144";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%" PRIu32
                             "] x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "width [%1d].",
                             func_name, i, region.srcOffset.x, src_copy_extent.width, subresource_extent.width);
        }

        if (extent_check & kYBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-00145" : "VUID-vkCmdCopyImage-srcOffset-00145";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%" PRIu32
                             "] y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "height [%1d].",
                             func_name, i, region.srcOffset.y, src_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-00147" : "VUID-vkCmdCopyImage-srcOffset-00147";
            skip |= LogError(command_buffer, vuid,
                             "%s: Source image pRegion[%" PRIu32
                             "] z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "depth [%1d].",
                             func_name, i, region.srcOffset.z, src_copy_extent.depth, subresource_extent.depth);
        }

        // Adjust dest extent if necessary
        subresource_extent = dst_image_state->GetSubresourceExtent(region.dstSubresource);
        if (slice_override) dst_copy_extent.depth = depth_slices;

        extent_check = ExceedsBounds(&(region.dstOffset), &dst_copy_extent, &subresource_extent);
        if (extent_check & kXBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-00150" : "VUID-vkCmdCopyImage-dstOffset-00150";
            skip |= LogError(command_buffer, vuid,
                             "%s: Dest image pRegion[%" PRIu32
                             "] x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "width [%1d].",
                             func_name, i, region.dstOffset.x, dst_copy_extent.width, subresource_extent.width);
        }
        if (extent_check & kYBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-00151" : "VUID-vkCmdCopyImage-dstOffset-00151";
            skip |= LogError(command_buffer, vuid,
                             "%s): Dest image pRegion[%" PRIu32
                             "] y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "height [%1d].",
                             func_name, i, region.dstOffset.y, dst_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & kZBit) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-00153" : "VUID-vkCmdCopyImage-dstOffset-00153";
            skip |= LogError(command_buffer, vuid,
                             "%s: Dest image pRegion[%" PRIu32
                             "] z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                             "depth [%1d].",
                             func_name, i, region.dstOffset.z, dst_copy_extent.depth, subresource_extent.depth);
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (src_image_state->image() == dst_image_state->image()) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersects(&region, &pRegions[j], src_image_type, FormatIsMultiplane(src_format))) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-00124" : "VUID-vkCmdCopyImage-pRegions-00124";
                    skip |= LogError(command_buffer, vuid, "%s: pRegion[%" PRIu32 "] src overlaps with pRegions[%" PRIu32 "].",
                                     func_name, i, j);
                }
            }
        }

        // Check for multi-plane format compatiblity
        if (FormatIsMultiplane(src_format) || FormatIsMultiplane(dst_format)) {
            const VkFormat src_plane_format = FormatIsMultiplane(src_format)
                                                  ? FindMultiplaneCompatibleFormat(src_format, region.srcSubresource.aspectMask)
                                                  : src_format;
            const VkFormat dst_plane_format = FormatIsMultiplane(dst_format)
                                                  ? FindMultiplaneCompatibleFormat(dst_format, region.dstSubresource.aspectMask)
                                                  : dst_format;
            const size_t src_format_size = FormatElementSize(src_plane_format);
            const size_t dst_format_size = FormatElementSize(dst_plane_format);

            // If size is still zero, then format is invalid and will be caught in another VU
            if ((src_format_size != dst_format_size) && (src_format_size != 0) && (dst_format_size != 0)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-None-01549" : "VUID-vkCmdCopyImage-None-01549";
                skip |= LogError(command_buffer, vuid,
                                 "%s: pRegions[%" PRIu32
                                 "] called with non-compatible image formats. "
                                 "The src format %s with aspectMask %s is not compatible with dst format %s aspectMask %s.",
                                 func_name, i, string_VkFormat(src_format),
                                 string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(), string_VkFormat(dst_format),
                                 string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
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
    if (!FormatIsMultiplane(src_format) && !FormatIsMultiplane(dst_format)) {
        const char *compatible_vuid = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)
                                          ? (is_2 ? "VUID-VkCopyImageInfo2-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-01548")
                                          : (is_2 ? "VUID-VkCopyImageInfo2-srcImage-00135" : "VUID-vkCmdCopyImage-srcImage-00135");
        // Depth/stencil formats must match exactly.
        if (FormatIsDepthOrStencil(src_format) || FormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                skip |= LogError(command_buffer, compatible_vuid,
                                 "%s: Depth/stencil formats must match exactly for src (%s) and dst (%s).", func_name,
                                 string_VkFormat(src_format), string_VkFormat(dst_format));
            }
        } else {
            if (FormatElementSize(src_format) != FormatElementSize(dst_format)) {
                skip |= LogError(command_buffer, compatible_vuid,
                                 "%s: Unmatched image format sizes. "
                                 "The src format %s has size of %" PRIu32 " and dst format %s has size of %" PRIu32 ".",
                                 func_name, string_VkFormat(src_format), FormatElementSize(src_format), string_VkFormat(dst_format),
                                 FormatElementSize(dst_format));
            }
        }
    }

    // Source and dest image sample counts must match
    if (src_image_state->createInfo.samples != dst_image_state->createInfo.samples) {
        std::stringstream ss;
        ss << func_name << " called on image pair with non-identical sample counts.";
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00136" : "VUID-vkCmdCopyImage-srcImage-00136";
        skip |=
            LogError(command_buffer, vuid, "%s: The src image sample count (%s) dose not match the dst image sample count (%s).",
                     func_name, string_VkSampleCountFlagBits(src_image_state->createInfo.samples),
                     string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
    }

    vuid = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)
               ? (is_2 ? "VUID-VkCopyImageInfo2-srcImage-01546" : "VUID-vkCmdCopyImage-srcImage-01546")
               : (is_2 ? "VUID-VkCopyImageInfo2-srcImage-00127" : "VUID-vkCmdCopyImage-srcImage-00127");
    skip |= ValidateMemoryIsBoundToImage(commandBuffer, *src_image_state, func_name, vuid);
    vuid = IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)
               ? (is_2 ? "VUID-VkCopyImageInfo2-dstImage-01547" : "VUID-vkCmdCopyImage-dstImage-01547")
               : (is_2 ? "VUID-VkCopyImageInfo2-dstImage-00132" : "VUID-vkCmdCopyImage-dstImage-00132");
    skip |= ValidateMemoryIsBoundToImage(commandBuffer, *dst_image_state, func_name, vuid);

    // Validate that SRC & DST images have correct usage flags set
    if (!IsExtEnabled(device_extensions.vk_ext_separate_stencil_usage)) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00126" : "VUID-vkCmdCopyImage-srcImage-00126";
        skip |= ValidateImageUsageFlags(command_buffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid, func_name);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-00131" : "VUID-vkCmdCopyImage-dstImage-00131";
        skip |= ValidateImageUsageFlags(command_buffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid, func_name);
    } else {
        auto src_separate_stencil = LvlFindInChain<VkImageStencilUsageCreateInfo>(src_image_state->createInfo.pNext);
        if (src_separate_stencil && has_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06664" : "VUID-vkCmdCopyImage-aspect-06664";
            skip |= ValidateUsageFlags(src_separate_stencil->stencilUsage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false,
                                       src_image_state->image(), src_image_state->Handle(), vuid, func_name,
                                       "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
        }
        if (!src_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
            skip |=
                ValidateImageUsageFlags(command_buffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid, func_name);
        }

        auto dst_separate_stencil = LvlFindInChain<VkImageStencilUsageCreateInfo>(dst_image_state->createInfo.pNext);
        if (dst_separate_stencil && has_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06665" : "VUID-vkCmdCopyImage-aspect-06665";
            skip |= ValidateUsageFlags(dst_separate_stencil->stencilUsage, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false,
                                       dst_image_state->image(), dst_image_state->Handle(), vuid, func_name,
                                       "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
        }
        if (!dst_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-vkCmdCopyImage-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
            skip |=
                ValidateImageUsageFlags(command_buffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid, func_name);
        }
    }

    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01825" : "VUID-vkCmdCopyImage-commandBuffer-01825";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01826" : "VUID-vkCmdCopyImage-commandBuffer-01826";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01827" : "VUID-vkCmdCopyImage-commandBuffer-01827";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-02542" : "VUID-vkCmdCopyImage-dstImage-02542";
        skip |=
            LogError(command_buffer, vuid,
                     "%s: srcImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT", func_name);
    }
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-02542" : "VUID-vkCmdCopyImage-dstImage-02542";
        skip |=
            LogError(command_buffer, vuid,
                     "%s: dstImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT", func_name);
    }

    skip |= ValidateCmd(cb_state, cmd_type);
    bool hit_error = false;

    const char *invalid_src_layout_vuid =
        (src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
            ? (is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-01917" : "VUID-vkCmdCopyImage-srcImageLayout-01917")
            : (is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-00129" : "VUID-vkCmdCopyImage-srcImageLayout-00129");
    const char *invalid_dst_layout_vuid =
        (dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
            ? (is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-01395" : "VUID-vkCmdCopyImage-dstImageLayout-01395")
            : (is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-00134" : "VUID-vkCmdCopyImage-dstImageLayout-00134");

    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; ++i) {
        // When performing copy from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const RegionType region = pRegions[i];
        const auto &src_sub = region.srcSubresource;
        const auto &dst_sub = region.dstSubresource;
        bool same_subresource =
            (same_image && (src_sub.mipLevel == dst_sub.mipLevel) && (src_sub.baseArrayLayer == dst_sub.baseArrayLayer));
        VkImageLayout source_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        VkImageLayout destination_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-00128" : "VUID-vkCmdCopyImage-srcImageLayout-00128";
        skip |= VerifyImageLayout(cb_state, *src_image_state, region.srcSubresource, srcImageLayout, source_optimal, func_name,
                                  invalid_src_layout_vuid, vuid, &hit_error);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-00133" : "VUID-vkCmdCopyImage-dstImageLayout-00133";
        skip |= VerifyImageLayout(cb_state, *dst_image_state, region.dstSubresource, dstImageLayout, destination_optimal, func_name,
                                  invalid_dst_layout_vuid, vuid, &hit_error);
        skip |= ValidateCopyImageTransferGranularityRequirements(cb_state, src_image_state.get(), dst_image_state.get(), &region, i,
                                                                 func_name, cmd_type);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy *pRegions) const {
    return ValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                CMD_COPYIMAGE);
}

bool CoreChecks::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) const {
    return ValidateCmdCopyImage(commandBuffer, pCopyImageInfo->srcImage, pCopyImageInfo->srcImageLayout, pCopyImageInfo->dstImage,
                                pCopyImageInfo->dstImageLayout, pCopyImageInfo->regionCount, pCopyImageInfo->pRegions,
                                CMD_COPYIMAGE2KHR);
}

bool CoreChecks::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) const {
    return ValidateCmdCopyImage(commandBuffer, pCopyImageInfo->srcImage, pCopyImageInfo->srcImageLayout, pCopyImageInfo->dstImage,
                                pCopyImageInfo->dstImageLayout, pCopyImageInfo->regionCount, pCopyImageInfo->pRegions,
                                CMD_COPYIMAGE2);
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
                                     const RegionType *pRegions, CMD_TYPE cmd_type) {
    const bool is_2 = (cmd_type == CMD_COPYBUFFER2KHR || cmd_type == CMD_COPYBUFFER2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";

    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (src_buffer_state->sparse || dst_buffer_state->sparse) {
        auto cb_state_ptr = Get<CMD_BUFFER_STATE>(commandBuffer);

        std::vector<sparse_container::range<VkDeviceSize>> src_ranges;
        std::vector<sparse_container::range<VkDeviceSize>> dst_ranges;

        for (uint32_t i = 0u; i < regionCount; ++i) {
            const RegionType &region = pRegions[i];
            src_ranges.emplace_back(sparse_container::range<VkDeviceSize>{region.srcOffset, region.srcOffset + region.size});
            dst_ranges.emplace_back(sparse_container::range<VkDeviceSize>{region.dstOffset, region.dstOffset + region.size});
        }

        auto queue_submit_validation = [this, src_buffer_state, dst_buffer_state, src_ranges, dst_ranges, vuid, func_name](
                                           const ValidationStateTracker &device_data, const class QUEUE_STATE &queue_state,
                                           const CMD_BUFFER_STATE &cb_state) -> bool {
            bool skip = false;
            for (const auto &src : src_ranges) {
                for (const auto &dst : dst_ranges) {
                    if (src_buffer_state->DoesResourceMemoryOverlap(src, dst_buffer_state.get(), dst)) {
                        skip |= this->LogError(src_buffer_state->buffer(), vuid,
                                               "%s: Detected overlap between source and dest regions in memory.", func_name);
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
    RecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, CMD_COPYBUFFER);
}

void CoreChecks::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) {
    RecordCmdCopyBuffer(commandBuffer, pCopyBufferInfos->srcBuffer, pCopyBufferInfos->dstBuffer, pCopyBufferInfos->regionCount,
                        pCopyBufferInfos->pRegions, CMD_COPYBUFFER2KHR);
}

void CoreChecks::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) {
    RecordCmdCopyBuffer(commandBuffer, pCopyBufferInfos->srcBuffer, pCopyBufferInfos->dstBuffer, pCopyBufferInfos->regionCount,
                        pCopyBufferInfos->pRegions, CMD_COPYBUFFER2);
}

template <typename RegionType>
bool CoreChecks::ValidateImageBounds(VkCommandBuffer cb, const IMAGE_STATE &image_state, const uint32_t regionCount,
                                     const RegionType *pRegions, const char *func_name, const char *msg_code) const {
    bool skip = false;
    const VkImageCreateInfo *image_info = &(image_state.createInfo);

    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];
        VkExtent3D extent = region.imageExtent;
        VkOffset3D offset = region.imageOffset;

        VkExtent3D image_extent = image_state.GetSubresourceExtent(region.imageSubresource);

        // If we're using a blocked image format, valid extent is rounded up to multiple of block size (per
        // vkspec.html#_common_operation)
        if (FormatIsBlockedImage(image_info->format)) {
            auto block_extent = FormatTexelBlockExtent(image_info->format);
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
            const LogObjectList objlist(cb, image_state.Handle());
            skip |= LogError(objlist, msg_code, "%s: pRegion[%d] exceeds image bounds.", func_name, i);
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferBounds(VkCommandBuffer cb, const IMAGE_STATE &image_state, const BUFFER_STATE &buff_state,
                                      uint32_t regionCount, const RegionType *pRegions, const char *func_name,
                                      const char *msg_code) const {
    bool skip = false;

    const VkDeviceSize buffer_size = buff_state.createInfo.size;

    for (uint32_t i = 0; i < regionCount; i++) {
        const RegionType region = pRegions[i];
        const VkDeviceSize buffer_copy_size = GetBufferSizeFromCopyImage(region, image_state.createInfo.format);
        // This blocks against invalid VkBufferCopyImage that already have been caught elsewhere
        if (buffer_copy_size != 0) {
            const VkDeviceSize max_buffer_copy = buffer_copy_size + region.bufferOffset;
            if (buffer_size < max_buffer_copy) {
                const LogObjectList objlist(cb, buff_state.Handle());
                skip |= LogError(objlist, msg_code,
                                 "%s: pRegion[%" PRIu32 "] is trying to copy  %" PRIu64 " bytes plus %" PRIu64
                                 " offset to/from the VkBuffer (%s) which exceeds the VkBuffer total size of %" PRIu64 " bytes.",
                                 func_name, i, buffer_copy_size, region.bufferOffset,
                                 report_data->FormatHandle(buff_state.Handle()).c_str(), buffer_size);
            }
        }
    }

    return skip;
}

// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(VkCommandBuffer cb, const IMAGE_STATE &image_state, VkSampleCountFlagBits sample_count,
                                          const char *location, const std::string &msgCode) const {
    bool skip = false;
    if (image_state.createInfo.samples != sample_count) {
        LogObjectList objlist(cb, image_state.Handle());
        skip = LogError(objlist, msgCode, "%s for %s was created with a sample count of %s but must be %s.", location,
                        report_data->FormatHandle(image_state.Handle()).c_str(),
                        string_VkSampleCountFlagBits(image_state.createInfo.samples), string_VkSampleCountFlagBits(sample_count));
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,

                                              CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !src_image_state || !dst_buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = (cmd_type == CMD_COPYIMAGETOBUFFER2KHR || cmd_type == CMD_COPYIMAGETOBUFFER2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    skip |= ValidateBufferImageCopyData(cb_state, regionCount, pRegions, src_image_state.get(), func_name, cmd_type, true);

    // Validate command buffer state
    skip |= ValidateCmd(cb_state, cmd_type);

    // Command pool must support graphics, compute, or transfer operations
    const auto pool = cb_state.command_pool;

    VkQueueFlags queue_flags = physical_device_state->queue_family_properties[pool->queueFamilyIndex].queueFlags;

    if (0 == (queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))) {
        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-cmdpool" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-cmdpool";
        skip |= LogError(cb_state.createInfo.commandPool, vuid,
                         "Cannot call %s on a command buffer allocated from a pool without graphics, compute, "
                         "or transfer capabilities.",
                         func_name);
    }

    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-pRegions-00182" : "VUID-vkCmdCopyImageToBuffer-pRegions-06220";
    skip |= ValidateImageBounds(commandBuffer, *src_image_state, regionCount, pRegions, func_name, vuid);
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-pRegions-00183" : "VUID-vkCmdCopyImageToBuffer-pRegions-00183";
    skip |= ValidateBufferBounds(commandBuffer, *src_image_state, *dst_buffer_state, regionCount, pRegions, func_name, vuid);

    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00188" : "VUID-vkCmdCopyImageToBuffer-srcImage-00188";
    std::string location = func_name;
    location.append("() : srcImage");
    skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, location.c_str(), vuid);

    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00187" : "VUID-vkCmdCopyImageToBuffer-srcImage-00187";
    skip |= ValidateMemoryIsBoundToImage(commandBuffer, *src_image_state, func_name, vuid);
    vuid = is_2 ? "vkCmdCopyImageToBuffer-dstBuffer2-00192" : "vkCmdCopyImageToBuffer dstBuffer-00192";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, func_name, vuid);

    // Validate that SRC image & DST buffer have correct usage flags set
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00186" : "VUID-vkCmdCopyImageToBuffer-srcImage-00186";
    skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name);
    vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191";
    skip |= ValidateBufferUsageFlags(commandBuffer, *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01831" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01832" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832";
    skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01833" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833";
    skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle());
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-02544" : "VUID-vkCmdCopyImageToBuffer-srcImage-02544";
        skip |= LogError(objlist, vuid,
                         "%s: srcImage must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                         func_name);
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-01998" : "VUID-vkCmdCopyImageToBuffer-srcImage-01998";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT, func_name, vuid);
    }
    bool hit_error = false;

    const char *src_invalid_layout_vuid =
        (src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
            ? (vuid =
                   is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-01397" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397")
            : (vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-00190"
                           : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00190");

    for (uint32_t i = 0; i < regionCount; ++i) {
        const RegionType region = pRegions[i];
        skip |= ValidateImageSubresourceLayers(cb_state, &region.imageSubresource, func_name, "imageSubresource", i);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-00189" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189";
        skip |= VerifyImageLayout(cb_state, *src_image_state, region.imageSubresource, srcImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, src_invalid_layout_vuid, vuid, &hit_error);
        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-imageOffset-07747" : "VUID-vkCmdCopyImageToBuffer-imageOffset-07747";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *src_image_state, &region, i, func_name, vuid);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-imageSubresource-01703" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-01703";
        skip |= ValidateImageMipLevel(cb_state, *src_image_state, region.imageSubresource.mipLevel, i, func_name,
                                      "imageSubresource", vuid);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-imageSubresource-01704" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-01704";
        skip |= ValidateImageArrayLayerRange(cb_state, *src_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, i, func_name, "imageSubresource", vuid);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        CMD_COPYIMAGETOBUFFER);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2KHR);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2);
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
                                              CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_buffer_state = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    if (!cb_state_ptr || !src_buffer_state || !dst_image_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    const bool is_2 = (cmd_type == CMD_COPYBUFFERTOIMAGE2KHR || cmd_type == CMD_COPYBUFFERTOIMAGE2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    skip |= ValidateBufferImageCopyData(cb_state, regionCount, pRegions, dst_image_state.get(), func_name, cmd_type, false);

    // Validate command buffer state
    skip |= ValidateCmd(cb_state, cmd_type);

    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-pRegions-00172" : "VUID-vkCmdCopyBufferToImage-pRegions-06217";
    skip |= ValidateImageBounds(commandBuffer, *dst_image_state, regionCount, pRegions, func_name, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-pRegions-00171" : "VUID-vkCmdCopyBufferToImage-pRegions-00171";
    skip |= ValidateBufferBounds(commandBuffer, *dst_image_state, *src_buffer_state, regionCount, pRegions, func_name, vuid);

    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00179" : "VUID-vkCmdCopyBufferToImage-dstImage-00179";
    std::string location = func_name;
    location.append("() : dstImage");
    skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, location.c_str(), vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00176" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00176";
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00178" : "VUID-vkCmdCopyBufferToImage-dstImage-00178";
    skip |= ValidateMemoryIsBoundToImage(commandBuffer, *dst_image_state, func_name, vuid);
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00174";
    skip |= ValidateBufferUsageFlags(commandBuffer, *src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name,
                                     "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00177" : "VUID-vkCmdCopyBufferToImage-dstImage-00177";
    skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01828" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01828";
    skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01829" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01829";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, func_name, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyBufferToImage-commandBuffer-01830" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01830";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, func_name, vuid);

    // Validation for VK_EXT_fragment_density_map
    if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        LogObjectList objlist(cb_state.Handle(), dst_image_state->Handle());
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-02543" : "VUID-vkCmdCopyBufferToImage-dstImage-02543";
        skip |= LogError(objlist, vuid,
                         "%s: dstImage must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                         func_name);
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-01997" : "VUID-vkCmdCopyBufferToImage-dstImage-01997";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT, func_name, vuid);
    }
    bool hit_error = false;

    const char *dst_invalid_layout_vuid =
        (dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
            ? (is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-01396" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396")
            : (is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-00181" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00181");

    for (uint32_t i = 0; i < regionCount; ++i) {
        const RegionType region = pRegions[i];
        skip |= ValidateImageSubresourceLayers(cb_state, &region.imageSubresource, func_name, "imageSubresource", i);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-00180" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180";
        skip |= VerifyImageLayout(cb_state, *dst_image_state, region.imageSubresource, dstImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, dst_invalid_layout_vuid, vuid, &hit_error);
        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-imageOffset-07738" : "VUID-vkCmdCopyBufferToImage-imageOffset-07738";
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *dst_image_state, &region, i, func_name, vuid);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-imageSubresource-01701" : "VUID-vkCmdCopyBufferToImage-imageSubresource-01701";
        skip |= ValidateImageMipLevel(cb_state, *dst_image_state, region.imageSubresource.mipLevel, i, func_name,
                                      "imageSubresource", vuid);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-imageSubresource-01702" : "VUID-vkCmdCopyBufferToImage-imageSubresource-01702";
        skip |= ValidateImageArrayLayerRange(cb_state, *dst_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, i, func_name, "imageSubresource", vuid);

        // TODO - Don't use ValidateCmdQueueFlags due to currently not having way to add more descriptive message
        const COMMAND_POOL_STATE *command_pool = cb_state.command_pool;
        assert(command_pool != nullptr);
        const uint32_t queue_family_index = command_pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (((queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
            ((region_aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)) {
            const LogObjectList objlist(cb_state.commandBuffer(), command_pool->commandPool(), dst_image_state->image());
            vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-07739" : "VUID-vkCmdCopyBufferToImage-commandBuffer-07739";
            skip |= LogError(objlist, vuid,
                             "%s(): pRegion[%d] subresource aspectMask 0x%x specifies VK_IMAGE_ASPECT_DEPTH_BIT or "
                             "VK_IMAGE_ASPECT_STENCIL_BIT but the command buffer %s was allocated from the command pool %s "
                             "which was created with queueFamilyIndex %" PRIu32
                             ", which doesn't contain the VK_QUEUE_GRAPHICS_BIT flag.",
                             func_name, i, region_aspect_mask, report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                             report_data->FormatHandle(command_pool->commandPool()).c_str(), queue_family_index);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        CMD_COPYBUFFERTOIMAGE);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2KHR);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2);
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

template <typename RegionType>
bool CoreChecks::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, VkFilter filter, CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    const bool is_2 = (cmd_type == CMD_BLITIMAGE2KHR || cmd_type == CMD_BLITIMAGE2);
    const char *func_name = CommandTypeString(cmd_type);

    if (cb_state_ptr && src_image_state && dst_image_state) {
        const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
        skip |= ValidateCmd(cb_state, cmd_type);

        const char *vuid;
        std::string loc_head = std::string(func_name);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00233" : "VUID-vkCmdBlitImage-srcImage-00233";
        const char *location1 = is_2 ? loc_head.append("(): pBlitImageInfo->srcImage").c_str() : "vkCmdBlitImage(): srcImage";
        skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, location1, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00234" : "VUID-vkCmdBlitImage-dstImage-00234";
        loc_head = std::string(func_name);
        const char *location2 = is_2 ? loc_head.append("(): pBlitImageInfo->dstImage").c_str() : "vkCmdBlitImage(): dstImage";
        skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, location2, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00220" : "VUID-vkCmdBlitImage-srcImage-00220";
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *src_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00225" : "VUID-vkCmdBlitImage-dstImage-00225";
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *dst_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00219" : "VUID-vkCmdBlitImage-srcImage-00219";
        skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00224" : "VUID-vkCmdBlitImage-dstImage-00224";
        skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name);
        skip |= ValidateCmd(cb_state, cmd_type);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-01999" : "VUID-vkCmdBlitImage-srcImage-01999";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_BLIT_SRC_BIT, func_name, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02000" : "VUID-vkCmdBlitImage-dstImage-02000";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_BLIT_DST_BIT, func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01834" : "VUID-vkCmdBlitImage-commandBuffer-01834";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01835" : "VUID-vkCmdBlitImage-commandBuffer-01835";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01836" : "VUID-vkCmdBlitImage-commandBuffer-01836";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, func_name, vuid);

        const LogObjectList src_objlist(cb_state.Handle(), src_image_state->Handle());
        const LogObjectList dst_objlist(cb_state.Handle(), dst_image_state->Handle());
        const LogObjectList all_objlist(cb_state.Handle(), src_image_state->Handle(), dst_image_state->Handle());
        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(src_objlist, vuid,
                             "%s: srcImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }
        if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(dst_objlist, vuid,
                             "%s: dstImage must not have been created with flags containing VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }

        // TODO: Need to validate image layouts, which will include layout validation for shared presentable images

        VkFormat src_format = src_image_state->createInfo.format;
        VkFormat dst_format = dst_image_state->createInfo.format;
        VkImageType src_type = src_image_state->createInfo.imageType;
        VkImageType dst_type = dst_image_state->createInfo.imageType;

        if (VK_FILTER_LINEAR == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02001" : "VUID-vkCmdBlitImage-filter-02001";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT, func_name, vuid);
        } else if (VK_FILTER_CUBIC_IMG == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02002" : "VUID-vkCmdBlitImage-filter-02002";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT, func_name, vuid);
        }

        if (FormatRequiresYcbcrConversionExplicitly(src_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-06421" : "VUID-vkCmdBlitImage-srcImage-06421";
            skip |= LogError(src_objlist, vuid,
                             "%s: srcImage format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             func_name, string_VkFormat(src_format));
        }

        if (FormatRequiresYcbcrConversionExplicitly(dst_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-06422" : "VUID-vkCmdBlitImage-dstImage-06422";
            skip |= LogError(dst_objlist, vuid,
                             "%s: dstImage format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             func_name, string_VkFormat(dst_format));
        }

        if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_2D != src_type)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-00237" : "VUID-vkCmdBlitImage-filter-00237";
            skip |= LogError(src_objlist, vuid, "%s: source image type must be VK_IMAGE_TYPE_2D when cubic filtering is specified.",
                             func_name);
        }

        // Validate consistency for unsigned formats
        if (FormatIsUINT(src_format) != FormatIsUINT(dst_format)) {
            std::stringstream ss;
            ss << func_name << ": If one of srcImage and dstImage images has unsigned integer format, "
               << "the other one must also have unsigned integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00230" : "VUID-vkCmdBlitImage-srcImage-00230";
            skip |= LogError(all_objlist, vuid, "%s.", ss.str().c_str());
        }

        // Validate consistency for signed formats
        if (FormatIsSINT(src_format) != FormatIsSINT(dst_format)) {
            std::stringstream ss;
            ss << func_name << ": If one of srcImage and dstImage images has signed integer format, "
               << "the other one must also have signed integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00229" : "VUID-vkCmdBlitImage-srcImage-00229";
            skip |= LogError(all_objlist, vuid, "%s.", ss.str().c_str());
        }

        // Validate filter for Depth/Stencil formats
        if (FormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
            std::stringstream ss;
            ss << func_name << ": If the format of srcImage is a depth, stencil, or depth stencil "
               << "then filter must be VK_FILTER_NEAREST.";
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00232" : "VUID-vkCmdBlitImage-srcImage-00232";
            skip |= LogError(src_objlist, vuid, "%s.", ss.str().c_str());
        }

        // Validate aspect bits and formats for depth/stencil images
        if (FormatIsDepthOrStencil(src_format) || FormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                std::stringstream ss;
                ss << func_name << ": If one of srcImage and dstImage images has a format of depth, stencil or depth "
                   << "stencil, the other one must have exactly the same format.  "
                   << "Source format is " << string_VkFormat(src_format) << " Destination format is "
                   << string_VkFormat(dst_format);
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00231" : "VUID-vkCmdBlitImage-srcImage-00231";
                skip |= LogError(all_objlist, vuid, "%s.", ss.str().c_str());
            }
        }  // Depth or Stencil

        // Do per-region checks
        const char *invalid_src_layout_vuid =
            is_2 ? ((src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-VkBlitImageInfo2-srcImageLayout-01398"
                        : "VUID-VkBlitImageInfo2-srcImageLayout-00222")
                 : ((src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-vkCmdBlitImage-srcImageLayout-01398"
                        : "VUID-vkCmdBlitImage-srcImageLayout-00222");
        const char *invalid_dst_layout_vuid =
            is_2 ? ((dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-VkBlitImageInfo2-dstImageLayout-01399"
                        : "VUID-VkBlitImageInfo2-dstImageLayout-00227")
                 : ((dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-vkCmdBlitImage-dstImageLayout-01399"
                        : "VUID-vkCmdBlitImage-dstImageLayout-00227");

        const bool same_image = (src_image_state == dst_image_state);
        for (uint32_t i = 0; i < regionCount; i++) {
            const RegionType region = pRegions[i];
            bool hit_error = false;

            // When performing blit from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
            const auto &src_sub = region.srcSubresource;
            const auto &dst_sub = region.dstSubresource;
            bool same_subresource =
                (same_image && (src_sub.mipLevel == dst_sub.mipLevel) && (src_sub.baseArrayLayer == dst_sub.baseArrayLayer));
            VkImageLayout source_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            VkImageLayout destination_optimal = (same_subresource ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-00221" : "VUID-vkCmdBlitImage-srcImageLayout-00221";
            skip |= VerifyImageLayout(cb_state, *src_image_state, region.srcSubresource, srcImageLayout, source_optimal, func_name,
                                      invalid_src_layout_vuid, vuid, &hit_error);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-00226" : "VUID-vkCmdBlitImage-dstImageLayout-00226";
            skip |= VerifyImageLayout(cb_state, *dst_image_state, region.dstSubresource, dstImageLayout, destination_optimal,
                                      func_name, invalid_dst_layout_vuid, vuid, &hit_error);
            skip |= ValidateImageSubresourceLayers(cb_state, &region.srcSubresource, func_name, "srcSubresource", i);
            skip |= ValidateImageSubresourceLayers(cb_state, &region.dstSubresource, func_name, "dstSubresource", i);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcSubresource-01705" : "VUID-vkCmdBlitImage-srcSubresource-01705";
            skip |= ValidateImageMipLevel(cb_state, *src_image_state, region.srcSubresource.mipLevel, i, func_name,
                                          "srcSubresource", vuid);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstSubresource-01706" : "VUID-vkCmdBlitImage-dstSubresource-01706";
            skip |= ValidateImageMipLevel(cb_state, *dst_image_state, region.dstSubresource.mipLevel, i, func_name,
                                          "dstSubresource", vuid);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcSubresource-01707" : "VUID-vkCmdBlitImage-srcSubresource-01707";
            skip |= ValidateImageArrayLayerRange(cb_state, *src_image_state, region.srcSubresource.baseArrayLayer,
                                                 region.srcSubresource.layerCount, i, func_name, "srcSubresource", vuid);
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstSubresource-01708" : "VUID-vkCmdBlitImage-dstSubresource-01708";
            skip |= ValidateImageArrayLayerRange(cb_state, *dst_image_state, region.dstSubresource.baseArrayLayer,
                                                 region.dstSubresource.layerCount, i, func_name, "dstSubresource", vuid);
            // Check that src/dst layercounts match
            if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                vuid = is_2 ? "VUID-VkImageBlit2-layerCount-00239" : "VUID-VkImageBlit-layerCount-00239";
                skip |=
                    LogError(cb_state.commandBuffer(), vuid,
                             "%s: layerCount in source and destination subresource of pRegions[%d] does not match.", func_name, i);
            }

            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                vuid = is_2 ? "VUID-VkImageBlit2-aspectMask-00238" : "VUID-VkImageBlit-aspectMask-00238";
                skip |=
                    LogError(cb_state.commandBuffer(), vuid, "%s: aspectMask members for pRegion[%d] do not match.", func_name, i);
            }

            if (!VerifyAspectsPresent(region.srcSubresource.aspectMask, src_format)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00241" : "VUID-vkCmdBlitImage-aspectMask-00241";
                skip |= LogError(src_objlist, vuid,
                                 "%s: region [%d] source aspectMask (0x%x) specifies aspects not present in source "
                                 "image format %s.",
                                 func_name, i, region.srcSubresource.aspectMask, string_VkFormat(src_format));
            }

            if (!VerifyAspectsPresent(region.dstSubresource.aspectMask, dst_format)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00242" : "VUID-vkCmdBlitImage-aspectMask-00242";
                skip |= LogError(dst_objlist, vuid,
                                 "%s: region [%d] dest aspectMask (0x%x) specifies aspects not present in dest image format %s.",
                                 func_name, i, region.dstSubresource.aspectMask, string_VkFormat(dst_format));
            }

            // Validate source image offsets
            VkExtent3D src_extent = src_image_state->GetSubresourceExtent(region.srcSubresource);
            if (VK_IMAGE_TYPE_1D == src_type) {
                if ((0 != region.srcOffsets[0].y) || (1 != region.srcOffsets[1].y)) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00245" : "VUID-vkCmdBlitImage-srcImage-00245";
                    skip |= LogError(src_objlist, vuid,
                                     "%s: region [%d], source image of type VK_IMAGE_TYPE_1D with srcOffset[].y values "
                                     "of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, region.srcOffsets[0].y, region.srcOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == src_type) || (VK_IMAGE_TYPE_2D == src_type)) {
                if ((0 != region.srcOffsets[0].z) || (1 != region.srcOffsets[1].z)) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00247" : "VUID-vkCmdBlitImage-srcImage-00247";
                    skip |= LogError(all_objlist, vuid,
                                     "%s: region [%d], source image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                     "srcOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, region.srcOffsets[0].z, region.srcOffsets[1].z);
                }
            }

            bool oob = false;
            if ((region.srcOffsets[0].x < 0) || (region.srcOffsets[0].x > static_cast<int32_t>(src_extent.width)) ||
                (region.srcOffsets[1].x < 0) || (region.srcOffsets[1].x > static_cast<int32_t>(src_extent.width))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
                skip |= LogError(src_objlist, vuid,
                                 "%s: region [%d] srcOffset[].x values (%1d, %1d) exceed srcSubresource width extent (%1d).",
                                 func_name, i, region.srcOffsets[0].x, region.srcOffsets[1].x, src_extent.width);
            }
            if ((region.srcOffsets[0].y < 0) || (region.srcOffsets[0].y > static_cast<int32_t>(src_extent.height)) ||
                (region.srcOffsets[1].y < 0) || (region.srcOffsets[1].y > static_cast<int32_t>(src_extent.height))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
                skip |= LogError(src_objlist, vuid,
                                 "%s: region [%d] srcOffset[].y values (%1d, %1d) exceed srcSubresource height extent (%1d).",
                                 func_name, i, region.srcOffsets[0].y, region.srcOffsets[1].y, src_extent.height);
            }
            if ((region.srcOffsets[0].z < 0) || (region.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth)) ||
                (region.srcOffsets[1].z < 0) || (region.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
                skip |= LogError(src_objlist, vuid,
                                 "%s: region [%d] srcOffset[].z values (%1d, %1d) exceed srcSubresource depth extent (%1d).",
                                 func_name, i, region.srcOffsets[0].z, region.srcOffsets[1].z, src_extent.depth);
            }
            if (oob) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00215" : "VUID-vkCmdBlitImage-pRegions-00215";
                skip |=
                    LogError(src_objlist, vuid, "%s: region [%d] source image blit region exceeds image dimensions.", func_name, i);
            }

            // Validate dest image offsets
            VkExtent3D dst_extent = dst_image_state->GetSubresourceExtent(region.dstSubresource);
            if (VK_IMAGE_TYPE_1D == dst_type) {
                if ((0 != region.dstOffsets[0].y) || (1 != region.dstOffsets[1].y)) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00250" : "VUID-vkCmdBlitImage-dstImage-00250";
                    skip |= LogError(dst_objlist, vuid,
                                     "%s: region [%d], dest image of type VK_IMAGE_TYPE_1D with dstOffset[].y values of "
                                     "(%1d, %1d). These must be (0, 1).",
                                     func_name, i, region.dstOffsets[0].y, region.dstOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == dst_type) || (VK_IMAGE_TYPE_2D == dst_type)) {
                if ((0 != region.dstOffsets[0].z) || (1 != region.dstOffsets[1].z)) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00252" : "VUID-vkCmdBlitImage-dstImage-00252";
                    skip |= LogError(dst_objlist, vuid,
                                     "%s: region [%d], dest image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                     "dstOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                     func_name, i, region.dstOffsets[0].z, region.dstOffsets[1].z);
                }
            }

            oob = false;
            if ((region.dstOffsets[0].x < 0) || (region.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width)) ||
                (region.dstOffsets[1].x < 0) || (region.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
                skip |= LogError(dst_objlist, vuid,
                                 "%s: region [%d] dstOffset[].x values (%1d, %1d) exceed dstSubresource width extent (%1d).",
                                 func_name, i, region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.width);
            }
            if ((region.dstOffsets[0].y < 0) || (region.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height)) ||
                (region.dstOffsets[1].y < 0) || (region.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
                skip |= LogError(dst_objlist, vuid,
                                 "%s: region [%d] dstOffset[].y values (%1d, %1d) exceed dstSubresource height extent (%1d).",
                                 func_name, i, region.dstOffsets[0].y, region.dstOffsets[1].y, dst_extent.height);
            }
            if ((region.dstOffsets[0].z < 0) || (region.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth)) ||
                (region.dstOffsets[1].z < 0) || (region.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth))) {
                oob = true;
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
                skip |= LogError(dst_objlist, vuid,
                                 "%s: region [%d] dstOffset[].z values (%1d, %1d) exceed dstSubresource depth extent (%1d).",
                                 func_name, i, region.dstOffsets[0].z, region.dstOffsets[1].z, dst_extent.depth);
            }
            if (oob) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00216" : "VUID-vkCmdBlitImage-pRegions-00216";
                skip |= LogError(dst_objlist, vuid, "%s: region [%d] destination image blit region exceeds image dimensions.",
                                 func_name, i);
            }

            if ((VK_IMAGE_TYPE_3D == src_type) || (VK_IMAGE_TYPE_3D == dst_type)) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount) ||
                    (0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00240" : "VUID-vkCmdBlitImage-srcImage-00240";
                    skip |= LogError(all_objlist, vuid,
                                     "%s: region [%d] blit to/from a 3D image type with a non-zero baseArrayLayer, or a "
                                     "layerCount other than 1.",
                                     func_name, i);
                }
            }

            // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
            // must not overlap in memory
            if (srcImage == dstImage) {
                for (uint32_t j = 0; j < regionCount; j++) {
                    if (RegionIntersectsBlit(&region, &pRegions[j], src_image_state->createInfo.imageType,
                                             FormatIsMultiplane(src_format))) {
                        vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00217" : "VUID-vkCmdBlitImage-pRegions-00217";
                        skip |= LogError(src_objlist, vuid, "%s: pRegion[%" PRIu32 "] src overlaps with pRegions[%" PRIu32 "] dst.",
                                         func_name, i, j);
                    }
                }
            }
        }  // per-region checks
    } else {
        assert(0);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit *pRegions, VkFilter filter) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                CMD_BLITIMAGE);
}

bool CoreChecks::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, CMD_BLITIMAGE2KHR);
}

bool CoreChecks::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, CMD_BLITIMAGE2);
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
                                         const RegionType *pRegions, CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto src_image_state = Get<IMAGE_STATE>(srcImage);
    auto dst_image_state = Get<IMAGE_STATE>(dstImage);
    const bool is_2 = (cmd_type == CMD_RESOLVEIMAGE2KHR || cmd_type == CMD_RESOLVEIMAGE2);
    const char *func_name = CommandTypeString(cmd_type);
    const char *vuid;

    if (cb_state_ptr && src_image_state && dst_image_state) {
        const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00256" : "VUID-vkCmdResolveImage-srcImage-00256";
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *src_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00258" : "VUID-vkCmdResolveImage-dstImage-00258";
        skip |= ValidateMemoryIsBoundToImage(commandBuffer, *dst_image_state, func_name, vuid);
        skip |= ValidateCmd(cb_state, cmd_type);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02003" : "VUID-vkCmdResolveImage-dstImage-02003";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT,
                                                func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01837" : "VUID-vkCmdResolveImage-commandBuffer-01837";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01838" : "VUID-vkCmdResolveImage-commandBuffer-01838";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01839" : "VUID-vkCmdResolveImage-commandBuffer-01839";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, func_name, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06762" : "VUID-vkCmdResolveImage-srcImage-06762";
        skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, func_name);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06763" : "VUID-vkCmdResolveImage-srcImage-06763";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, func_name, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06764" : "VUID-vkCmdResolveImage-dstImage-06764";
        skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, func_name);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06765" : "VUID-vkCmdResolveImage-dstImage-06765";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, func_name, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle());
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(objlist, vuid,
                             "%s: srcImage must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }
        if (dst_image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            const LogObjectList objlist(cb_state.Handle(), dst_image_state->Handle());
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(objlist, vuid,
                             "%s: dstImage must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT",
                             func_name);
        }

        bool hit_error = false;
        const char *invalid_src_layout_vuid =
            is_2 ? ((src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-VkResolveImageInfo2-srcImageLayout-01400"
                        : "VUID-VkResolveImageInfo2-srcImageLayout-00261")
                 : ((src_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-vkCmdResolveImage-srcImageLayout-01400"
                        : "VUID-vkCmdResolveImage-srcImageLayout-00261");
        const char *invalid_dst_layout_vuid =
            is_2 ? ((dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-VkResolveImageInfo2-dstImageLayout-01401"
                        : "VUID-VkResolveImageInfo2-dstImageLayout-00263")
                 : ((dst_image_state->shared_presentable && IsExtEnabled(device_extensions.vk_khr_shared_presentable_image))
                        ? "VUID-vkCmdResolveImage-dstImageLayout-01401"
                        : "VUID-vkCmdResolveImage-dstImageLayout-00263");
        // For each region, the number of layers in the image subresource should not be zero
        // For each region, src and dest image aspect must be color only
        for (uint32_t i = 0; i < regionCount; i++) {
            const RegionType region = pRegions[i];
            const VkImageSubresourceLayers src_subresource = region.srcSubresource;
            const VkImageSubresourceLayers dst_subresource = region.dstSubresource;

            skip |= ValidateImageSubresourceLayers(cb_state, &src_subresource, func_name, "srcSubresource", i);
            skip |= ValidateImageSubresourceLayers(cb_state, &dst_subresource, func_name, "dstSubresource", i);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-00260" : "VUID-vkCmdResolveImage-srcImageLayout-00260";
            skip |= VerifyImageLayout(cb_state, *src_image_state, src_subresource, srcImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, func_name, invalid_src_layout_vuid, vuid, &hit_error);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-00262" : "VUID-vkCmdResolveImage-dstImageLayout-00262";
            skip |= VerifyImageLayout(cb_state, *dst_image_state, dst_subresource, dstImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, func_name, invalid_dst_layout_vuid, vuid, &hit_error);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcSubresource-01709" : "VUID-vkCmdResolveImage-srcSubresource-01709";
            skip |=
                ValidateImageMipLevel(cb_state, *src_image_state, src_subresource.mipLevel, i, func_name, "srcSubresource", vuid);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstSubresource-01710" : "VUID-vkCmdResolveImage-dstSubresource-01710";
            skip |=
                ValidateImageMipLevel(cb_state, *dst_image_state, dst_subresource.mipLevel, i, func_name, "dstSubresource", vuid);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcSubresource-01711" : "VUID-vkCmdResolveImage-srcSubresource-01711";
            skip |= ValidateImageArrayLayerRange(cb_state, *src_image_state, src_subresource.baseArrayLayer,
                                                 src_subresource.layerCount, i, func_name, "srcSubresource", vuid);
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstSubresource-01712" : "VUID-vkCmdResolveImage-dstSubresource-01712";
            skip |= ValidateImageArrayLayerRange(cb_state, *dst_image_state, dst_subresource.baseArrayLayer,
                                                 dst_subresource.layerCount, i, func_name, "srcSubresource", vuid);

            // layer counts must match
            if (src_subresource.layerCount != dst_subresource.layerCount) {
                const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle(), dst_image_state->Handle());
                vuid = is_2 ? "VUID-VkImageResolve2-layerCount-00267" : "VUID-VkImageResolve-layerCount-00267";
                skip |= LogError(objlist, vuid,
                                 "%s: layerCount in source and destination subresource of pRegions[%" PRIu32 "] does not match.",
                                 func_name, i);
            }
            // For each region, src and dest image aspect must be color only
            if ((src_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
                (dst_subresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT)) {
                const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle(), dst_image_state->Handle());
                vuid = is_2 ? "VUID-VkImageResolve2-aspectMask-00266" : "VUID-VkImageResolve-aspectMask-00266";
                skip |=
                    LogError(objlist, vuid,
                             "%s: src and dest aspectMasks for pRegions[%" PRIu32 "] must specify only VK_IMAGE_ASPECT_COLOR_BIT.",
                             func_name, i);
            }

            const VkImageType src_image_type = src_image_state->createInfo.imageType;
            const VkImageType dst_image_type = dst_image_state->createInfo.imageType;

            if ((VK_IMAGE_TYPE_3D == src_image_type) || (VK_IMAGE_TYPE_3D == dst_image_type)) {
                if ((0 != src_subresource.baseArrayLayer) || (1 != src_subresource.layerCount)) {
                    const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04446" : "VUID-vkCmdResolveImage-srcImage-04446";
                    skip |= LogError(objlist, vuid,
                                     "%s: pRegions[%" PRIu32
                                     "] baseArrayLayer must be 0 and layerCount must be 1 for all "
                                     "subresources if the src or dst image is 3D.",
                                     func_name, i);
                }
                if ((0 != dst_subresource.baseArrayLayer) || (1 != dst_subresource.layerCount)) {
                    const LogObjectList objlist(cb_state.Handle(), dst_image_state->Handle());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04447" : "VUID-vkCmdResolveImage-srcImage-04447";
                    skip |= LogError(objlist, vuid,
                                     "%s: pRegions[%" PRIu32
                                     "] baseArrayLayer must be 0 and layerCount must be 1 for all "
                                     "subresources if the src or dst image is 3D.",
                                     func_name, i);
                }
            }

            if (VK_IMAGE_TYPE_1D == src_image_type) {
                if ((region.srcOffset.y != 0) || (region.extent.height != 1)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), src_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00271" : "VUID-vkCmdResolveImage-srcImage-00271";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) is 1D but pRegions[%" PRIu32
                                     "] srcOffset.y (%d) is not 0 or "
                                     "extent.height (%" PRIu32 ") is not 1.",
                                     func_name, report_data->FormatHandle(src_image_state->image()).c_str(), i, region.srcOffset.y,
                                     region.extent.height);
                }
            }
            if ((VK_IMAGE_TYPE_1D == src_image_type) || (VK_IMAGE_TYPE_2D == src_image_type)) {
                if ((region.srcOffset.z != 0) || (region.extent.depth != 1)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), src_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00273" : "VUID-vkCmdResolveImage-srcImage-00273";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) is 2D but pRegions[%" PRIu32
                                     "] srcOffset.z (%d) is not 0 or "
                                     "extent.depth (%" PRIu32 ") is not 1.",
                                     func_name, report_data->FormatHandle(src_image_state->image()).c_str(), i, region.srcOffset.z,
                                     region.extent.depth);
                }
            }

            if (VK_IMAGE_TYPE_1D == dst_image_type) {
                if ((region.dstOffset.y != 0) || (region.extent.height != 1)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), dst_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00276" : "VUID-vkCmdResolveImage-dstImage-00276";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) is 1D but pRegions[%" PRIu32
                                     "] dstOffset.y (%d) is not 0 or "
                                     "extent.height (%" PRIu32 ") is not 1.",
                                     func_name, report_data->FormatHandle(dst_image_state->image()).c_str(), i, region.dstOffset.y,
                                     region.extent.height);
                }
            }
            if ((VK_IMAGE_TYPE_1D == dst_image_type) || (VK_IMAGE_TYPE_2D == dst_image_type)) {
                if ((region.dstOffset.z != 0) || (region.extent.depth != 1)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), dst_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00278" : "VUID-vkCmdResolveImage-dstImage-00278";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) is 2D but pRegions[%" PRIu32
                                     "] dstOffset.z (%d) is not 0 or "
                                     "extent.depth (%" PRIu32 ") is not 1.",
                                     func_name, report_data->FormatHandle(dst_image_state->image()).c_str(), i, region.dstOffset.z,
                                     region.extent.depth);
                }
            }

            // Each srcImage dimension offset + extent limits must fall with image subresource extent
            VkExtent3D subresource_extent = src_image_state->GetSubresourceExtent(src_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
            // developer
            if (src_subresource.mipLevel < src_image_state->createInfo.mipLevels) {
                uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &(region.extent), &subresource_extent);
                if ((extent_check & kXBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), src_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%" PRIu32 "] x-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource width [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(src_image_state->image()).c_str(), i, region.srcOffset.x,
                                     region.extent.width, subresource_extent.width);
                }

                if ((extent_check & kYBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), src_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%" PRIu32 "] y-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource height [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(src_image_state->image()).c_str(), i, region.srcOffset.y,
                                     region.extent.height, subresource_extent.height);
                }

                if ((extent_check & kZBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), src_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                    skip |= LogError(objlist, vuid,
                                     "%s: srcImage (%s) pRegions[%" PRIu32 "] z-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource depth [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(src_image_state->image()).c_str(), i, region.srcOffset.z,
                                     region.extent.depth, subresource_extent.depth);
                }
            }

            // Each dstImage dimension offset + extent limits must fall with image subresource extent
            subresource_extent = dst_image_state->GetSubresourceExtent(dst_subresource);
            // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
            // developer
            if (dst_subresource.mipLevel < dst_image_state->createInfo.mipLevels) {
                uint32_t extent_check = ExceedsBounds(&(region.dstOffset), &(region.extent), &subresource_extent);
                if ((extent_check & kXBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), dst_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%" PRIu32 "] x-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource width [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(dst_image_state->image()).c_str(), i, region.srcOffset.x,
                                     region.extent.width, subresource_extent.width);
                }

                if ((extent_check & kYBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), dst_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%" PRIu32 "] y-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource height [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(dst_image_state->image()).c_str(), i, region.srcOffset.y,
                                     region.extent.height, subresource_extent.height);
                }

                if ((extent_check & kZBit) != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), dst_image_state->image());
                    vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                    skip |= LogError(objlist, vuid,
                                     "%s: dstImage (%s) pRegions[%" PRIu32 "] z-dimension offset [%1d] + extent [%" PRIu32
                                     "] "
                                     "exceeds subResource depth [%" PRIu32 "].",
                                     func_name, report_data->FormatHandle(dst_image_state->image()).c_str(), i, region.srcOffset.z,
                                     region.extent.depth, subresource_extent.depth);
                }
            }
        }

        if (src_image_state->createInfo.format != dst_image_state->createInfo.format) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-01386" : "VUID-vkCmdResolveImage-srcImage-01386";
            skip |= LogError(cb_state.commandBuffer(), vuid, "%s: srcImage format (%s) and dstImage format (%s) are not the same.",
                             func_name, string_VkFormat(src_image_state->createInfo.format),
                             string_VkFormat(dst_image_state->createInfo.format));
        }
        if (src_image_state->createInfo.samples == VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00257" : "VUID-vkCmdResolveImage-srcImage-00257";
            skip |= LogError(cb_state.commandBuffer(), vuid, "%s: srcImage sample count is VK_SAMPLE_COUNT_1_BIT.", func_name);
        }
        if (dst_image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00259" : "VUID-vkCmdResolveImage-dstImage-00259";
            skip |= LogError(cb_state.commandBuffer(), vuid, "%s: dstImage sample count (%s) is not VK_SAMPLE_COUNT_1_BIT.",
                             func_name, string_VkSampleCountFlagBits(dst_image_state->createInfo.samples));
        }
    } else {
        assert(0);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve *pRegions) const {
    return ValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                   CMD_RESOLVEIMAGE);
}

bool CoreChecks::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkResolveImageInfo2KHR *pResolveImageInfo) const {
    return ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                   pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                   pResolveImageInfo->pRegions, CMD_RESOLVEIMAGE2KHR);
}

bool CoreChecks::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                 const VkResolveImageInfo2 *pResolveImageInfo) const {
    return ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                   pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                   pResolveImageInfo->pRegions, CMD_RESOLVEIMAGE2);
}
