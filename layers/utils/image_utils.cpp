/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "image_utils.h"
#include "containers/range.h"
#include "utils/math_utils.h"

#include <algorithm>
#include <sstream>
#include <array>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask, const uint32_t mip_level) {
    // Return zero extent if mip level doesn't exist
    if (mip_level >= ci.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    VkExtent3D extent = ci.extent;

    // If multi-plane, adjust per-plane extent
    const VkFormat format = ci.format;
    if (vkuFormatIsMultiplane(format)) {
        VkExtent2D divisors = vkuFindMultiplaneExtentDivisors(format, static_cast<VkImageAspectFlagBits>(aspect_mask));
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    // Mip Maps
    {
        const uint32_t corner = (ci.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) ? 1 : 0;
        const uint32_t min_size = 1 + corner;

        if (extent.width != 0) {
            extent.width >>= mip_level;
            extent.width = std::max({min_size, extent.width});
        }
        if (extent.height != 0) {
            extent.height >>= mip_level;
            extent.height = std::max({min_size, extent.height});
        }
        if (extent.depth != 0) {
            extent.depth >>= mip_level;
            extent.depth = std::max({min_size, extent.depth});
        }
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != ci.imageType) {
        extent.depth = ci.arrayLayers;
    }

    return extent;
}

// Returns true if [x, x + x_size) and [y, y + y_size) overlap
bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    auto intersection = GetRangeIntersection(x, x_size, y, y_size);
    return intersection.non_empty();
}

// Implements the vkspec.html#formats-size-compatibility section of the spec
bool AreFormatsSizeCompatible(VkFormat a, VkFormat b, VkImageAspectFlags aspect_mask) {
    const bool is_a_a8 = a == VK_FORMAT_A8_UNORM;
    const bool is_b_a8 = b == VK_FORMAT_A8_UNORM;
    if ((is_a_a8 && !is_b_a8) || (!is_a_a8 && is_b_a8)) {
        return false;
    }

    const bool is_a_depth_stencil = vkuFormatIsDepthOrStencil(a);
    const bool is_b_depth_stencil = vkuFormatIsDepthOrStencil(b);
    if (is_a_depth_stencil && !is_b_depth_stencil) {
        return vkuFormatIsDepthStencilWithColorSizeCompatible(b, a, aspect_mask);
    } else if (!is_a_depth_stencil && is_b_depth_stencil) {
        return vkuFormatIsDepthStencilWithColorSizeCompatible(a, b, aspect_mask);
    } else if (is_a_depth_stencil && is_b_depth_stencil) {
        return a == b;
    }

    // Color formats are considered compatible if their texel block size in bytes is the same
    return vkuFormatTexelBlockSize(a) == vkuFormatTexelBlockSize(b);
}

std::string DescribeFormatsSizeCompatible(VkFormat a, VkFormat b) {
    std::stringstream ss;
    const bool is_a_a8 = a == VK_FORMAT_A8_UNORM;
    const bool is_b_a8 = b == VK_FORMAT_A8_UNORM;
    if ((is_a_a8 && !is_b_a8) || (!is_a_a8 && is_b_a8)) {
        ss << string_VkFormat(a) << " and " << string_VkFormat(b)
           << " either both need to be VK_FORMAT_A8_UNORM or neither of them";
        return ss.str();
    }

    const bool is_a_depth_stencil = vkuFormatIsDepthOrStencil(a);
    const bool is_b_depth_stencil = vkuFormatIsDepthOrStencil(b);
    if (is_a_depth_stencil && is_b_depth_stencil) {
        ss << string_VkFormat(a) << " and " << string_VkFormat(b)
           << " are both depth/stencil, therefor they must be the exact same format";
    } else if (is_a_depth_stencil || is_b_depth_stencil) {
        if (is_a_depth_stencil && !is_b_depth_stencil) {
            ss << string_VkFormat(a) << " is a depth/stencil and " << string_VkFormat(b) << " is color";
        } else if (!is_a_depth_stencil && is_b_depth_stencil) {
            ss << string_VkFormat(a) << " is a color and " << string_VkFormat(b) << " is depth/stencil";
        }
        ss << " (this is only allowed with a certain set of formats during image copy with VK_KHR_maintenance8)";
    } else {
        ss << string_VkFormat(a) << " has a texel block size of " << vkuFormatTexelBlockSize(a) << " while " << string_VkFormat(b)
           << " has a texel block size of " << vkuFormatTexelBlockSize(b);
    }
    return ss.str();
}

uint32_t GetVertexInputFormatSize(VkFormat format) {
    // Vertex input attributes use VkFormat, but only to make use of how they define sizes, things such as
    // depth/multi-plane/compressed will never be used here because they would mean nothing. So we can ensure these are "standard"
    // color formats being used. This function is a wrapper to make it more clear of the intent.
    return vkuFormatTexelBlockSize(format);
}

uint32_t GetTexelBufferFormatSize(VkFormat format) {
    // The spec says "If format is a block-compressed format, then bufferFeatures must not support any features for the format"
    // For Texel Buffers, we can assume the texel blocks are a 1x1x1 extent
    // See https://gitlab.khronos.org/vulkan/vulkan/-/issues/4155 for more details
    return vkuFormatTexelBlockSize(format);
}

// Used to get the VkExternalFormatANDROID without having to use ifdef in logic
// Result of zero is same of not having pNext struct
uint64_t GetExternalFormat(const void *pNext) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (pNext) {
        const auto *external_format = vku::FindStructInPNextChain<VkExternalFormatANDROID>(pNext);
        if (external_format) {
            return external_format->externalFormat;
        }
    }
#endif
    (void)pNext;
    return 0;
}

// vkspec.html#formats-planes-image-aspect
bool IsValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const uint32_t planes = vkuFormatPlaneCount(format);
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;

    if (((aspect_mask & valid_planes) == aspect_mask) && (aspect_mask != 0)) {
        if ((planes == 3) || ((planes == 2) && ((aspect_mask & VK_IMAGE_ASPECT_PLANE_2_BIT) == 0))) {
            return true;
        }
    }
    return false;  // Expects calls to make sure it is a multi-planar format
}

bool IsOnlyOneValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const bool multiple_bits = aspect_mask != 0 && !IsPowerOfTwo(aspect_mask);
    return !multiple_bits && IsValidPlaneAspect(format, aspect_mask);
}

bool IsMultiplePlaneAspect(VkImageAspectFlags aspect_mask) {
    // If checking for multiple planes, there will already be another check if valid for plane count
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    const VkImageAspectFlags planes = aspect_mask & valid_planes;
    return planes != 0 && !IsPowerOfTwo(planes);
}

bool IsAnyPlaneAspect(VkImageAspectFlags aspect_mask) {
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    return (aspect_mask & valid_planes) != 0;
}

bool IsImageLayoutReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

bool IsImageLayoutDepthOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

bool IsImageLayoutDepthReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

bool IsImageLayoutStencilOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                               VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

bool IsImageLayoutStencilReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}