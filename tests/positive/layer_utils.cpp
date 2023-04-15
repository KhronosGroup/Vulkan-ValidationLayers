
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
#include "generated/vk_extension_helper.h"
#include "utils/vk_layer_utils.h"

// These test check utils in the layer without needing to create a full Vulkan instance

TEST_F(VkPositiveLayerTest, GetEffectiveExtent) {
    TEST_DESCRIPTION("Test unlikely GetEffectiveExtent edge cases");

    auto ci = LvlInitStruct<VkImageCreateInfo>();
    VkExtent3D extent = {};

    // Return zero extent if mip level doesn't exist
    {
        ci.mipLevels = 0;
        ci.extent = {1, 1, 1};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);

        ci.mipLevels = 1;
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);
    }

    // Check that 0 based extent is respected
    {
        ci.flags = 0;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {0, 0, 0};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);

        ci.extent = {16, 32, 0};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 8);
        ASSERT_TRUE(extent.height == 16);
        ASSERT_TRUE(extent.depth == 0);
    }

    // Corner sampled images
    {
        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {1, 1, 1};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        // The minimum level size is 2x2x2 for 3D corner sampled images.
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 2);
        ASSERT_TRUE(extent.depth == 2);

        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {4, 8, 16};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 4);
        ASSERT_TRUE(extent.depth == 8);

        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 3;
        ci.extent = {8, 16, 32};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 2);
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 4);
        ASSERT_TRUE(extent.depth == 8);
    }
}

TEST_F(VkPositiveLayerTest, IsOnlyOneValidPlaneAspect) {
    const VkFormat two_plane_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, 0));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_COLOR_BIT));
    ASSERT_TRUE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_2_BIT));
}

TEST_F(VkPositiveLayerTest, IsValidPlaneAspect) {
    const VkFormat two_plane_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, 0));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_COLOR_BIT));
    ASSERT_TRUE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT));
    ASSERT_TRUE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_2_BIT));
}