/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_format_utils.h"

class PositiveFormatUtils : public VkPositiveLayerTest {};

// These test check utils in the layer without needing to create a full Vulkan instance

TEST_F(PositiveFormatUtils, FormatsSameComponentBits) {
    ASSERT_TRUE(FormatsSameComponentBits(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM));
    ASSERT_TRUE(FormatsSameComponentBits(VK_FORMAT_ASTC_10x10_UNORM_BLOCK, VK_FORMAT_ASTC_12x10_SRGB_BLOCK));
    // Order doesn't matter
    ASSERT_TRUE(FormatsSameComponentBits(VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_B8G8R8_USCALED));
    ASSERT_TRUE(FormatsSameComponentBits(VK_FORMAT_A4R4G4B4_UNORM_PACK16, VK_FORMAT_R4G4B4A4_UNORM_PACK16));
    ASSERT_TRUE(FormatsSameComponentBits(VK_FORMAT_B5G5R5A1_UNORM_PACK16, VK_FORMAT_A1R5G5B5_UNORM_PACK16));
    // Components mismatch
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_R8_UINT, VK_FORMAT_S8_UINT));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, VK_FORMAT_G8B8G8R8_422_UNORM));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_R12X4_UNORM_PACK16, VK_FORMAT_R12X4G12X4_UNORM_2PACK16));
    // Bits mismatch
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_R32_UINT, VK_FORMAT_R16_UINT));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_B8G8R8_UINT));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT));
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16));
    // UNDEFINED is always bad
    ASSERT_FALSE(FormatsSameComponentBits(VK_FORMAT_UNDEFINED, VK_FORMAT_B8G8R8_UINT));
}

TEST_F(PositiveFormatUtils, FormatIs64bit) {
    ASSERT_TRUE(FormatIs64bit(VK_FORMAT_R64_SFLOAT));
    ASSERT_TRUE(FormatIs64bit(VK_FORMAT_R64G64B64A64_SINT));
    ASSERT_FALSE(FormatIs64bit(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM));
}