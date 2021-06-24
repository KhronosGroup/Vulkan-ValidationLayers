/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
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
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#pragma once
#include <stdbool.h>
#include <vector>
#include "vulkan/vulkan.h"

#if !defined(VK_LAYER_EXPORT)
#if defined(__GNUC__) && __GNUC__ >= 4
#define VK_LAYER_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define VK_LAYER_EXPORT __attribute__((visibility("default")))
#else
#define VK_LAYER_EXPORT
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define VK_MULTIPLANE_FORMAT_MAX_PLANES 3

typedef enum VkFormatCompatibilityClass {
    VK_FORMAT_COMPATIBILITY_CLASS_NONE_BIT = 0,
    VK_FORMAT_COMPATIBILITY_CLASS_8_BIT = 1,
    VK_FORMAT_COMPATIBILITY_CLASS_16_BIT = 2,
    VK_FORMAT_COMPATIBILITY_CLASS_24_BIT = 3,
    VK_FORMAT_COMPATIBILITY_CLASS_32_BIT = 4,
    VK_FORMAT_COMPATIBILITY_CLASS_48_BIT = 5,
    VK_FORMAT_COMPATIBILITY_CLASS_64_BIT = 6,
    VK_FORMAT_COMPATIBILITY_CLASS_96_BIT = 7,
    VK_FORMAT_COMPATIBILITY_CLASS_128_BIT = 8,
    VK_FORMAT_COMPATIBILITY_CLASS_192_BIT = 9,
    VK_FORMAT_COMPATIBILITY_CLASS_256_BIT = 10,
    VK_FORMAT_COMPATIBILITY_CLASS_BC1_RGB_BIT = 11,
    VK_FORMAT_COMPATIBILITY_CLASS_BC1_RGBA_BIT = 12,
    VK_FORMAT_COMPATIBILITY_CLASS_BC2_BIT = 13,
    VK_FORMAT_COMPATIBILITY_CLASS_BC3_BIT = 14,
    VK_FORMAT_COMPATIBILITY_CLASS_BC4_BIT = 15,
    VK_FORMAT_COMPATIBILITY_CLASS_BC5_BIT = 16,
    VK_FORMAT_COMPATIBILITY_CLASS_BC6H_BIT = 17,
    VK_FORMAT_COMPATIBILITY_CLASS_BC7_BIT = 18,
    VK_FORMAT_COMPATIBILITY_CLASS_ETC2_RGB_BIT = 19,
    VK_FORMAT_COMPATIBILITY_CLASS_ETC2_RGBA_BIT = 20,
    VK_FORMAT_COMPATIBILITY_CLASS_ETC2_EAC_RGBA_BIT = 21,
    VK_FORMAT_COMPATIBILITY_CLASS_EAC_R_BIT = 22,
    VK_FORMAT_COMPATIBILITY_CLASS_EAC_RG_BIT = 23,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_4X4_BIT = 24,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_5X4_BIT = 25,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_5X5_BIT = 26,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_6X5_BIT = 27,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_6X6_BIT = 28,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_8X5_BIT = 29,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_8X6_BIT = 20,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_8X8_BIT = 31,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_10X5_BIT = 32,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_10X6_BIT = 33,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_10X8_BIT = 34,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_10X10_BIT = 35,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_12X10_BIT = 36,
    VK_FORMAT_COMPATIBILITY_CLASS_ASTC_12X12_BIT = 37,
    VK_FORMAT_COMPATIBILITY_CLASS_D16_BIT = 38,
    VK_FORMAT_COMPATIBILITY_CLASS_D24_BIT = 39,
    VK_FORMAT_COMPATIBILITY_CLASS_D32_BIT = 30,
    VK_FORMAT_COMPATIBILITY_CLASS_S8_BIT = 41,
    VK_FORMAT_COMPATIBILITY_CLASS_D16S8_BIT = 42,
    VK_FORMAT_COMPATIBILITY_CLASS_D24S8_BIT = 43,
    VK_FORMAT_COMPATIBILITY_CLASS_D32S8_BIT = 44,
    VK_FORMAT_COMPATIBILITY_CLASS_PVRTC1_2BPP_BIT = 45,
    VK_FORMAT_COMPATIBILITY_CLASS_PVRTC1_4BPP_BIT = 46,
    VK_FORMAT_COMPATIBILITY_CLASS_PVRTC2_2BPP_BIT = 47,
    VK_FORMAT_COMPATIBILITY_CLASS_PVRTC2_4BPP_BIT = 48,
    /* KHR_sampler_YCbCr_conversion */
    VK_FORMAT_COMPATIBILITY_CLASS_32BIT_G8B8G8R8 = 49,
    VK_FORMAT_COMPATIBILITY_CLASS_32BIT_B8G8R8G8 = 50,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_R10G10B10A10 = 51,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_G10B10G10R10 = 52,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_B10G10R10G10 = 53,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_R12G12B12A12 = 54,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_G12B12G12R12 = 55,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_B12G12R12G12 = 56,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_G16B16G16R16 = 57,
    VK_FORMAT_COMPATIBILITY_CLASS_64BIT_B16G16R16G16 = 58,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_420 = 59,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_420 = 60,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_422 = 61,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_422 = 62,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_444 = 63,
    VK_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_444 = 64,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_420 = 65,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_420 = 66,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_422 = 67,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_422 = 68,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_444 = 69,
    VK_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_444 = 70,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_420 = 71,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_420 = 72,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_422 = 73,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_422 = 74,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_444 = 75,
    VK_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_444 = 76,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_420 = 77,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_420 = 78,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_422 = 79,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_422 = 80,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_444 = 81,
    VK_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_444 = 82,
    VK_FORMAT_COMPATIBILITY_CLASS_MAX_ENUM = 83
} VkFormatCompatibilityClass;

typedef enum VkFormatNumericalType {
    VK_FORMAT_NUMERICAL_TYPE_NONE,
    VK_FORMAT_NUMERICAL_TYPE_UINT,
    VK_FORMAT_NUMERICAL_TYPE_SINT,
    VK_FORMAT_NUMERICAL_TYPE_UNORM,
    VK_FORMAT_NUMERICAL_TYPE_SNORM,
    VK_FORMAT_NUMERICAL_TYPE_USCALED,
    VK_FORMAT_NUMERICAL_TYPE_SSCALED,
    VK_FORMAT_NUMERICAL_TYPE_UFLOAT,
    VK_FORMAT_NUMERICAL_TYPE_SFLOAT,
    VK_FORMAT_NUMERICAL_TYPE_SRGB
} VkFormatNumericalType;

VK_LAYER_EXPORT bool FormatIsDepthOrStencil(VkFormat format);
VK_LAYER_EXPORT bool FormatIsDepthAndStencil(VkFormat format);
VK_LAYER_EXPORT bool FormatIsDepthOnly(VkFormat format);
VK_LAYER_EXPORT bool FormatIsStencilOnly(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ETC2_EAC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ASTC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ASTC_LDR(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_ASTC_HDR(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_BC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed_PVRTC(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSinglePlane_422(VkFormat format);
VK_LAYER_EXPORT bool FormatIsNorm(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUNorm(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSNorm(VkFormat format);
VK_LAYER_EXPORT bool FormatIsInt(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSInt(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUInt(VkFormat format);
VK_LAYER_EXPORT bool FormatIsFloat(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSRGB(VkFormat format);
VK_LAYER_EXPORT bool FormatIsUScaled(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSScaled(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSampledInt(VkFormat format);
VK_LAYER_EXPORT bool FormatIsSampledFloat(VkFormat format);
VK_LAYER_EXPORT bool FormatIsCompressed(VkFormat format);
VK_LAYER_EXPORT bool FormatIsPacked(VkFormat format);
VK_LAYER_EXPORT bool FormatElementIsTexel(VkFormat format);
VK_LAYER_EXPORT bool FormatSizesAreEqual(VkFormat srcFormat, VkFormat dstFormat, uint32_t region_count, const VkImageCopy *regions);
VK_LAYER_EXPORT bool FormatRequiresYcbcrConversion(VkFormat format);
VK_LAYER_EXPORT bool FormatIsXChromaSubsampled(VkFormat format);
VK_LAYER_EXPORT bool FormatIsYChromaSubsampled(VkFormat format);
VK_LAYER_EXPORT VkDeviceSize GetIndexAlignment(VkIndexType indexType);

VK_LAYER_EXPORT uint32_t FormatDepthSize(VkFormat format);
VK_LAYER_EXPORT VkFormatNumericalType FormatDepthNumericalType(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatStencilSize(VkFormat format);
VK_LAYER_EXPORT VkFormatNumericalType FormatStencilNumericalType(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatPlaneCount(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatChannelCount(VkFormat format);
VK_LAYER_EXPORT VkExtent3D FormatTexelBlockExtent(VkFormat format);
VK_LAYER_EXPORT uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
VK_LAYER_EXPORT double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
VK_LAYER_EXPORT VkFormatCompatibilityClass FormatCompatibilityClass(VkFormat format);
VK_LAYER_EXPORT VkDeviceSize SafeModulo(VkDeviceSize dividend, VkDeviceSize divisor);
VK_LAYER_EXPORT VkDeviceSize SafeDivision(VkDeviceSize dividend, VkDeviceSize divisor);
VK_LAYER_EXPORT uint32_t GetPlaneIndex(VkImageAspectFlags aspect);
VK_LAYER_EXPORT VkFormat FindMultiplaneCompatibleFormat(VkFormat fmt, VkImageAspectFlags plane_aspect);
VK_LAYER_EXPORT VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);

static inline bool FormatIsUndef(VkFormat format) { return (format == VK_FORMAT_UNDEFINED); }
static inline bool FormatHasDepth(VkFormat format) { return (FormatIsDepthOnly(format) || FormatIsDepthAndStencil(format)); }
static inline bool FormatHasStencil(VkFormat format) { return (FormatIsStencilOnly(format) || FormatIsDepthAndStencil(format)); }
static inline bool FormatIsMultiplane(VkFormat format) { return ((FormatPlaneCount(format)) > 1u); }
static inline bool FormatIsColor(VkFormat format) {
    return !(FormatIsUndef(format) || FormatIsDepthOrStencil(format) || FormatIsMultiplane(format));
}

#ifdef __cplusplus
}
#endif
