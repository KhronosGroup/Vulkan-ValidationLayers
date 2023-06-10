// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See format_utils_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
 ****************************************************************************/

#include "vk_format_utils.h"
#include "utils/vk_layer_utils.h"
#include <vector>


enum class COMPONENT_TYPE {
    NONE,
    R,
    G,
    B,
    A,
    D,
    S
};

// Compressed formats don't have a defined component size
const uint32_t COMPRESSED_COMPONENT = 0xFFFFFFFF;

struct COMPONENT_INFO {
    COMPONENT_TYPE type;
    uint32_t size; // bits

    COMPONENT_INFO() : type(COMPONENT_TYPE::NONE), size(0) {};
    COMPONENT_INFO(COMPONENT_TYPE type, uint32_t size) : type(type), size(size) {};
};

// Generic information for all formats
struct FORMAT_INFO {
    FORMAT_COMPATIBILITY_CLASS compatibility;
    uint32_t block_size; // bytes
    uint32_t texel_per_block;
    VkExtent3D block_extent;
    uint32_t component_count;
    COMPONENT_INFO components[FORMAT_MAX_COMPONENTS];
};

namespace std {
template <>
struct hash<VkFormat> {
    size_t operator()(VkFormat fmt) const noexcept {
        return hash<uint32_t>()(static_cast<uint32_t>(fmt));
    }
};
}

// clang-format off
static const vvl::unordered_map<VkFormat, FORMAT_INFO> kVkFormatTable = {
    {VK_FORMAT_UNDEFINED, {FORMAT_COMPATIBILITY_CLASS::NONE, 0, 0, {0, 0, 0}, 0, {}}}};
// clang-format on

struct PER_PLANE_COMPATIBILITY {
    uint32_t width_divisor;
    uint32_t height_divisor;
    VkFormat compatible_format;

    // Need default otherwise if app tries to grab a plane that doesn't exist it will crash
    // if returned the value of 0 in IMAGE_STATE::GetEffectiveSubresourceExtent()
    // This is ok, because there are VUs later that will catch the bad app behaviour
    PER_PLANE_COMPATIBILITY() : width_divisor(1), height_divisor(1), compatible_format(VK_FORMAT_UNDEFINED) {}
    PER_PLANE_COMPATIBILITY(uint32_t width_divisor, uint32_t height_divisor, VkFormat compatible_format) :
        width_divisor(width_divisor), height_divisor(height_divisor), compatible_format(compatible_format) {}
};

// Information for multiplanar formats
struct MULTIPLANE_COMPATIBILITY {
    PER_PLANE_COMPATIBILITY per_plane[FORMAT_MAX_PLANES];
};

// Source: Vulkan spec Table 47. Plane Format Compatibility Table
// clang-format off
static const vvl::unordered_map<VkFormat, MULTIPLANE_COMPATIBILITY> kVkMultiplaneCompatibilityMap {
};
// clang-format on



// Return true if the format is any compressed image format
bool FormatIsCompressed(VkFormat format) {
    return (
}


// Return true if format is a depth OR stencil format
bool FormatIsDepthOrStencil(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Return true if format is a depth AND stencil format
bool FormatIsDepthAndStencil(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Return true if format is a depth ONLY format
bool FormatIsDepthOnly(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Return true if format is a stencil ONLY format
bool FormatIsStencilOnly(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Returns size of depth component in bits
// Returns zero if no depth component
uint32_t FormatDepthSize(VkFormat format) {
    switch (format) {
        default:
            return 0;
     }
}

// Returns size of stencil component in bits
// Returns zero if no stencil component
uint32_t FormatStencilSize(VkFormat format) {
    switch (format) {
        default:
            return 0;
     }
}

// Returns NONE if no depth component
FORMAT_NUMERICAL_TYPE FormatDepthNumericalType(VkFormat format) {
    switch (format) {
        default:
            return FORMAT_NUMERICAL_TYPE::NONE;
     }
}

// Returns NONE if no stencil component
FORMAT_NUMERICAL_TYPE FormatStencilNumericalType(VkFormat format) {
    switch (format) {
        default:
            return FORMAT_NUMERICAL_TYPE::NONE;
     }
}


// Return true if format is a packed format
bool FormatIsPacked(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}


// Return true if format requires sampler YCBCR conversion
// for VK_IMAGE_ASPECT_COLOR_BIT image views
// Table found in spec
bool FormatRequiresYcbcrConversion(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

bool FormatIsXChromaSubsampled(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

bool FormatIsYChromaSubsampled(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}


// Will return VK_FORMAT_UNDEFINED if given a plane aspect that doesn't exist for the format
VkFormat FindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlags plane_aspect) {
    const uint32_t plane_idx = GetPlaneIndex(plane_aspect);
    auto it = kVkMultiplaneCompatibilityMap.find(mp_fmt);
    if ((it == kVkMultiplaneCompatibilityMap.end()) || (plane_idx >= FORMAT_MAX_PLANES)) {
        return VK_FORMAT_UNDEFINED;
    }

    return it->second.per_plane[plane_idx].compatible_format;
}

// Will return {1, 1} if given a plane aspect that doesn't exist for the format
VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect) {
    VkExtent2D divisors = {1, 1};
    const uint32_t plane_idx = GetPlaneIndex(plane_aspect);
    auto it = kVkMultiplaneCompatibilityMap.find(mp_fmt);
    if ((it == kVkMultiplaneCompatibilityMap.end()) || (plane_idx >= FORMAT_MAX_PLANES)) {
        return divisors;
    }

    divisors.width = it->second.per_plane[plane_idx].width_divisor;
    divisors.height = it->second.per_plane[plane_idx].height_divisor;
    return divisors;
}


uint32_t FormatComponentCount(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.component_count;
    }
    return 0;
}

VkExtent3D FormatTexelBlockExtent(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.block_extent;
    }
    return {1, 1, 1};
}

FORMAT_COMPATIBILITY_CLASS FormatCompatibilityClass(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.compatibility;
    }
    return FORMAT_COMPATIBILITY_CLASS::NONE;
}

// Return true if format is 'normal', with one texel per format element
bool FormatElementIsTexel(VkFormat format) {
    if (FormatIsPacked(format) || FormatIsCompressed(format) || FormatIsSinglePlane_422(format) || FormatIsMultiplane(format)) {
        return false;
    } else {
        return true;
    }
}

// Return size, in bytes, of one element of the specified format
// For uncompressed this is one texel, for compressed it is one block
uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask) {
    // Depth/Stencil aspect have separate helper functions
    if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
        return FormatStencilSize(format) / 8;
    } else if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
        return FormatDepthSize(format) / 8;
    } else if (FormatIsMultiplane(format)) {
        // Element of entire multiplane format is not useful,
        // Want to get just a single plane as the lookup format
        format = FindMultiplaneCompatibleFormat(format, aspectMask);
    }

    auto item = kVkFormatTable.find(format);
    if (item != kVkFormatTable.end()) {
        return item->second.block_size;
    }
    return 0;
}

// Return the size in bytes of one texel of given foramt
// For compressed or multi-plane, this may be a fractional number
double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask) {
    double texel_size = static_cast<double>(FormatElementSize(format, aspectMask));
    VkExtent3D block_extent = FormatTexelBlockExtent(format);
    uint32_t texels_per_block = block_extent.width * block_extent.height * block_extent.depth;
    if (1 < texels_per_block) {
        texel_size /= static_cast<double>(texels_per_block);
    }
    return texel_size;
}

bool FormatHasComponentSize(VkFormat format, uint32_t size) {
    auto item = kVkFormatTable.find(format);
    if (item == kVkFormatTable.end()) {
        return false;
    }
    const COMPONENT_INFO* begin = item->second.components;
    const COMPONENT_INFO* end = item->second.components + FORMAT_MAX_COMPONENTS;
    return std::find_if(begin, end, [size](const COMPONENT_INFO& info) { return info.size == size; }) != end;
}

static bool FormatHasComponentType(VkFormat format, COMPONENT_TYPE component) {
    auto item = kVkFormatTable.find(format);
    if (item == kVkFormatTable.end()) {
        return false;
    }
    const COMPONENT_INFO* begin = item->second.components;
    const COMPONENT_INFO* end = item->second.components + FORMAT_MAX_COMPONENTS;
    return std::find_if(begin, end, [component](const COMPONENT_INFO& info) { return info.type == component; }) != end;
}

bool FormatHasRed(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::R);
}

bool FormatHasGreen(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::G);
}

bool FormatHasBlue(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::B);
}

bool FormatHasAlpha(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::A);
}

bool FormatsSameComponentBits(VkFormat format_a, VkFormat format_b) {
    const auto item_a = kVkFormatTable.find(format_a);
    const auto item_b = kVkFormatTable.find(format_b);
    if (item_a == kVkFormatTable.end() || item_b == kVkFormatTable.end()) {
        return false;
    } else if (item_a->second.component_count != item_b->second.component_count) {
        return false;
    }
    // Need to loop match each component type is found in both formats
    // formats are maxed at 4 components, so the double loop is not going to scale
    for (uint32_t i = 0; i < item_a->second.component_count; i++) {
        const auto& component_a = item_a->second.components[i];
        bool component_match = false;
        for (uint32_t j = 0; j < item_b->second.component_count; j++) {
            const auto& component_b = item_b->second.components[j];
            if ((component_a.type == component_b.type) && (component_a.size == component_b.size)) {
                component_match = true;
                break;
            }
        }
        if (!component_match) {
            return false;
        }
    }
    return true;
}

