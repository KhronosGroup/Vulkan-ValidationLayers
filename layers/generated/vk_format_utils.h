// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See format_utils_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Spencer Fricke <s.fricke@samsung.com>
 *
 ****************************************************************************/

#pragma once
#include <vulkan/vk_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

static constexpr uint32_t FORMAT_MAX_PLANES = 3;
static constexpr uint32_t FORMAT_MAX_COMPONENTS = 4;

enum class FORMAT_NUMERICAL_TYPE {
    NONE = 0,
    SFLOAT,
    SINT,
    SNORM,
    SRGB,
    SSCALED,
    UFLOAT,
    UINT,
    UNORM,
    USCALED
};

enum class FORMAT_COMPATIBILITY_CLASS {
    NONE = 0,
    _10BIT_2PLANE_420,
    _10BIT_2PLANE_422,
    _10BIT_2PLANE_444,
    _10BIT_3PLANE_420,
    _10BIT_3PLANE_422,
    _10BIT_3PLANE_444,
    _12BIT_2PLANE_420,
    _12BIT_2PLANE_422,
    _12BIT_2PLANE_444,
    _12BIT_3PLANE_420,
    _12BIT_3PLANE_422,
    _12BIT_3PLANE_444,
    _128BIT,
    _16BIT,
    _16BIT_2PLANE_420,
    _16BIT_2PLANE_422,
    _16BIT_2PLANE_444,
    _16BIT_3PLANE_420,
    _16BIT_3PLANE_422,
    _16BIT_3PLANE_444,
    _192BIT,
    _24BIT,
    _256BIT,
    _32BIT,
    _32BIT_B8G8R8G8,
    _32BIT_G8B8G8R8,
    _48BIT,
    _64BIT,
    _64BIT_B10G10R10G10,
    _64BIT_B12G12R12G12,
    _64BIT_B16G16R16G16,
    _64BIT_G10B10G10R10,
    _64BIT_G12B12G12R12,
    _64BIT_G16B16G16R16,
    _64BIT_R10G10B10A10,
    _64BIT_R12G12B12A12,
    _8BIT,
    _8BIT_2PLANE_420,
    _8BIT_2PLANE_422,
    _8BIT_2PLANE_444,
    _8BIT_3PLANE_420,
    _8BIT_3PLANE_422,
    _8BIT_3PLANE_444,
    _96BIT,
    ASTC_10X10,
    ASTC_10X5,
    ASTC_10X6,
    ASTC_10X8,
    ASTC_12X10,
    ASTC_12X12,
    ASTC_4X4,
    ASTC_5X4,
    ASTC_5X5,
    ASTC_6X5,
    ASTC_6X6,
    ASTC_8X5,
    ASTC_8X6,
    ASTC_8X8,
    BC1_RGB,
    BC1_RGBA,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,
    D16,
    D16S8,
    D24,
    D24S8,
    D32,
    D32S8,
    EAC_R,
    EAC_RG,
    ETC2_EAC_RGBA,
    ETC2_RGB,
    ETC2_RGBA,
    PVRTC1_2BPP,
    PVRTC1_4BPP,
    PVRTC2_2BPP,
    PVRTC2_4BPP,
    S8
};

// Numeric
// Formats with more then one numeric type (VK_FORMAT_D16_UNORM_S8_UINT) will return false
VK_LAYER_EXPORT bool FormatIsUNORM(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSNORM(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUSCALED(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSSCALED(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUINT(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSINT(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSRGB(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSFLOAT(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUFLOAT(VkFormat format);

// Types from "Interpretation of Numeric Format" table (OpTypeFloat vs OpTypeInt)
static inline bool FormatIsSampledInt(VkFormat format) { return (FormatIsSINT(format) || FormatIsUINT(format)); }
static inline bool FormatIsSampledFloat(VkFormat format) {
    return (FormatIsUNORM(format)   || FormatIsSNORM(format)   ||
            FormatIsUSCALED(format) || FormatIsSSCALED(format) ||
            FormatIsUFLOAT(format)  || FormatIsSFLOAT(format)  ||
            FormatIsSRGB(format));
}

// Compressed
VK_LAYER_EXPORT bool FormatIsCompressed_ASTC_HDR(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ASTC_LDR(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_BC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_EAC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ETC2(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_PVRTC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed(VkFormat format);

// Depth/Stencil
VK_LAYER_EXPORT bool FormatIsDepthOrStencil(VkFormat format);
VK_LAYER_EXPORT bool FormatIsDepthAndStencil(VkFormat format);
VK_LAYER_EXPORT bool FormatIsDepthOnly(VkFormat format);
VK_LAYER_EXPORT bool FormatIsStencilOnly(VkFormat format);
static inline bool FormatHasDepth(VkFormat format) { return (FormatIsDepthOnly(format) || FormatIsDepthAndStencil(format)); }
static inline bool FormatHasStencil(VkFormat format) { return (FormatIsStencilOnly(format) || FormatIsDepthAndStencil(format)); }
VK_LAYER_EXPORT uint32_t FormatDepthSize(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatStencilSize(VkFormat format);
VK_LAYER_EXPORT FORMAT_NUMERICAL_TYPE FormatDepthNumericalType(VkFormat format);
VK_LAYER_EXPORT FORMAT_NUMERICAL_TYPE FormatStencilNumericalType(VkFormat format);

// Packed
VK_LAYER_EXPORT bool FormatIsPacked(VkFormat format);

// YCbCr
VK_LAYER_EXPORT bool FormatRequiresYcbcrConversion(VkFormat format);
VK_LAYER_EXPORT bool FormatIsXChromaSubsampled(VkFormat format);
VK_LAYER_EXPORT bool FormatIsYChromaSubsampled(VkFormat format);

// Multiplane
VK_LAYER_EXPORT bool FormatIsSinglePlane_422(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatPlaneCount(VkFormat format);
static inline bool FormatIsMultiplane(VkFormat format) { return ((FormatPlaneCount(format)) > 1u); }
VK_LAYER_EXPORT VkFormat FindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);
VK_LAYER_EXPORT VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);

// Size
VK_LAYER_EXPORT uint32_t FormatComponentCount(VkFormat format);
VK_LAYER_EXPORT VkExtent3D FormatTexelBlockExtent(VkFormat format);
VK_LAYER_EXPORT FORMAT_COMPATIBILITY_CLASS FormatCompatibilityClass(VkFormat format);
VK_LAYER_EXPORT bool FormatElementIsTexel(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
VK_LAYER_EXPORT double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

// Components
VK_LAYER_EXPORT bool FormatHasRed(VkFormat format);
VK_LAYER_EXPORT bool FormatHasGreen(VkFormat format);
VK_LAYER_EXPORT bool FormatHasBlue(VkFormat format);
VK_LAYER_EXPORT bool FormatHasAlpha(VkFormat format);


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
