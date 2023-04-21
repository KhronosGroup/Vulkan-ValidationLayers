/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(VkPositiveLayerTest, MapMemory2) {
    TEST_DESCRIPTION("Validate vkMapMemory2 and vkUnmapMemory2");

    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    /* Vulkan doesn't have any requirements on what allocationSize can be
     * other than that it must be non-zero.  Pick 64KB because that should
     * work out to an even number of pages on basically any GPU.
     */
    const VkDeviceSize allocation_size = 64 << 10;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->phy().set_memory_type(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    VkDeviceMemory memory;
    VkResult err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory);
    ASSERT_VK_SUCCESS(err);

    VkMemoryMapInfoKHR map_info = LvlInitStruct<VkMemoryMapInfoKHR>();
    map_info.memory = memory;
    map_info.offset = 0;
    map_info.size = memory_info.allocationSize;

    VkMemoryUnmapInfoKHR unmap_info = LvlInitStruct<VkMemoryUnmapInfoKHR>();
    unmap_info.memory = memory;

    uint32_t *pData = NULL;
    err = vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    ASSERT_TRUE(pData != NULL);

    err = vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    ASSERT_VK_SUCCESS(err);

    map_info.size = VK_WHOLE_SIZE;

    pData = NULL;
    err = vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    ASSERT_TRUE(pData != NULL);

    err = vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    ASSERT_VK_SUCCESS(err);

    vk::FreeMemory(m_device->device(), memory, NULL);
}
