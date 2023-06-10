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

#pragma once
#include <vulkan/vk_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

static constexpr uint32_t FORMAT_MAX_PLANES = 1;
static constexpr uint32_t FORMAT_MAX_COMPONENTS = 1;

enum class FORMAT_NUMERICAL_TYPE {
    NONE = 0,
};

enum class FORMAT_COMPATIBILITY_CLASS {
    NONE = 0,
};

// Numeric
// Formats with more then one numeric type (VK_FORMAT_D16_UNORM_S8_UINT) will return false

// Types from "Interpretation of Numeric Format" table (OpTypeFloat vs OpTypeInt)
static inline bool FormatIsSampledInt(VkFormat format) { return (FormatIsSINT(format) || FormatIsUINT(format)); }
static inline bool FormatIsSampledFloat(VkFormat format) {
    return (FormatIsUNORM(format)   || FormatIsSNORM(format)   ||
            FormatIsUSCALED(format) || FormatIsSSCALED(format) ||
            FormatIsUFLOAT(format)  || FormatIsSFLOAT(format)  ||
            FormatIsSRGB(format));
}

// Compressed
bool FormatIsCompressed(VkFormat format);

// Depth/Stencil
bool FormatIsDepthOrStencil(VkFormat format);
bool FormatIsDepthAndStencil(VkFormat format);
bool FormatIsDepthOnly(VkFormat format);
bool FormatIsStencilOnly(VkFormat format);
static inline bool FormatHasDepth(VkFormat format) { return (FormatIsDepthOnly(format) || FormatIsDepthAndStencil(format)); }
static inline bool FormatHasStencil(VkFormat format) { return (FormatIsStencilOnly(format) || FormatIsDepthAndStencil(format)); }
uint32_t FormatDepthSize(VkFormat format);
uint32_t FormatStencilSize(VkFormat format);
FORMAT_NUMERICAL_TYPE FormatDepthNumericalType(VkFormat format);
FORMAT_NUMERICAL_TYPE FormatStencilNumericalType(VkFormat format);

// Packed
bool FormatIsPacked(VkFormat format);

// YCbCr
bool FormatRequiresYcbcrConversion(VkFormat format);
bool FormatIsXChromaSubsampled(VkFormat format);
bool FormatIsYChromaSubsampled(VkFormat format);

// Multiplane
// Single-plane "_422" formats are treated as 2x1 compressed (for copies)

constexpr bool FormatIsSinglePlane_422(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Returns number of planes in format (which is 1 by default)
constexpr uint32_t FormatPlaneCount(VkFormat format) {
    switch (format) {
        default:
            return 1;
     }
}

constexpr bool FormatIsMultiplane(VkFormat format) { return ((FormatPlaneCount(format)) > 1u); }
VkFormat FindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);
VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);

// Size
uint32_t FormatComponentCount(VkFormat format);
VkExtent3D FormatTexelBlockExtent(VkFormat format);
FORMAT_COMPATIBILITY_CLASS FormatCompatibilityClass(VkFormat format);
bool FormatElementIsTexel(VkFormat format);
uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
// True if Format contains a 64-bit component

constexpr bool FormatIs64bit(VkFormat format) {
    bool found = false;
    switch (format) {
            found = true;
            break;
        default:
            break;
    }
    return found;
}

// Components
bool FormatHasComponentSize(VkFormat format, uint32_t size);
bool FormatHasRed(VkFormat format);
bool FormatHasGreen(VkFormat format);
bool FormatHasBlue(VkFormat format);
bool FormatHasAlpha(VkFormat format);
bool FormatsSameComponentBits(VkFormat format_a, VkFormat format_b);


// Utils/misc
static inline bool FormatIsUndef(VkFormat format) { return (format == VK_FORMAT_UNDEFINED); }
// "blocked image" are defined in the spec (vkspec.html#blocked-image)
static inline bool FormatIsBlockedImage(VkFormat format) {
    return (FormatIsCompressed(format) || FormatIsSinglePlane_422(format));
}
// No official spec definition of "color format"
// So anything that could NOT be a "color format" is a color format
static inline bool FormatIsColor(VkFormat format) {
    return !(FormatIsUndef(format) || FormatIsDepthOrStencil(format) || FormatIsMultiplane(format));
}

#ifdef __cplusplus
}
#endif
