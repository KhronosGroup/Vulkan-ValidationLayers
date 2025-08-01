/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include <cctype>
#include <cstring>
#include <string>

#include <vulkan/vulkan_core.h>

uint32_t GetEffectiveLevelCount(const VkImageSubresourceRange &subresource_range, uint32_t total_level_count);
uint32_t GetEffectiveLayerCount(const VkImageSubresourceRange &subresource_range, uint32_t total_layer_count);
VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask, const uint32_t mip_level);

// When dealing with a compressed format, we could have a miplevel that is less than a single texel block
// In that case, we still view (from the API) that you need a full extent for 1 texel block
// if block extent width is 4,
//     then {1, 2, 3, 4} texel is 1 texel block
//     then {5, 6, 7, 8} texel is 2 texel block
//     .. etc
static inline VkExtent3D GetTexelBlocks(VkExtent3D texels, VkExtent3D block_extent) {
    return {
        ((texels.width - 1) / block_extent.width) + 1,
        ((texels.height - 1) / block_extent.height) + 1,
        ((texels.depth - 1) / block_extent.depth) + 1,
    };
};

// if block extent width is 4,
//     then {1, 2, 3, 4} texels is 4 texels
//     then {5, 6, 7, 8} texels is 8 texels
//     .. etc
static inline VkExtent3D RoundUpToFullTexelBlocks(VkExtent3D texels, VkExtent3D block_extent) {
    return {
        ((texels.width + block_extent.width - 1) / block_extent.width) * block_extent.width,
        ((texels.height + block_extent.height - 1) / block_extent.height) * block_extent.height,
        ((texels.depth + block_extent.depth - 1) / block_extent.depth) * block_extent.depth,
    };
};

bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size);

bool AreFormatsSizeCompatible(VkFormat a, VkFormat b,
                              VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);

std::string DescribeFormatsSizeCompatible(VkFormat a, VkFormat b);

uint64_t GetExternalFormat(const void *pNext);

uint32_t GetVertexInputFormatSize(VkFormat format);
uint32_t GetTexelBufferFormatSize(VkFormat format);

bool IsValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask);
bool IsOnlyOneValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask);
bool IsMultiplePlaneAspect(VkImageAspectFlags aspect_mask);
bool IsAnyPlaneAspect(VkImageAspectFlags aspect_mask);
VkImageAspectFlags NormalizeAspectMask(VkImageAspectFlags aspect_mask, VkFormat format);

bool IsImageLayoutReadOnly(VkImageLayout layout);
bool IsImageLayoutDepthOnly(VkImageLayout layout);
bool IsImageLayoutDepthReadOnly(VkImageLayout layout);
bool IsImageLayoutStencilOnly(VkImageLayout layout);
bool IsImageLayoutStencilReadOnly(VkImageLayout layout);

static inline bool IsIdentitySwizzle(VkComponentMapping components) {
    // clang-format off
    return (
        ((components.r == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.r == VK_COMPONENT_SWIZZLE_R)) &&
        ((components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.g == VK_COMPONENT_SWIZZLE_G)) &&
        ((components.b == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_B)) &&
        ((components.a == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.a == VK_COMPONENT_SWIZZLE_A))
    );
    // clang-format on
}

static inline uint32_t SampleCountSize(VkSampleCountFlagBits sample_count) {
    uint32_t size = 0;
    switch (sample_count) {
        case VK_SAMPLE_COUNT_1_BIT:
            size = 1;
            break;
        case VK_SAMPLE_COUNT_2_BIT:
            size = 2;
            break;
        case VK_SAMPLE_COUNT_4_BIT:
            size = 4;
            break;
        case VK_SAMPLE_COUNT_8_BIT:
            size = 8;
            break;
        case VK_SAMPLE_COUNT_16_BIT:
            size = 16;
            break;
        case VK_SAMPLE_COUNT_32_BIT:
            size = 32;
            break;
        case VK_SAMPLE_COUNT_64_BIT:
            size = 64;
            break;
        default:
            size = 0;
    }
    return size;
}