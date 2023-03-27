/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(VkPositiveLayerTest, GetMemoryFdHandle) {
    TEST_DESCRIPTION("Get POXIS handle for memory allocation");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    auto vkGetMemoryFdKHR = GetInstanceProcAddr<PFN_vkGetMemoryFdKHR>("vkGetMemoryFdKHR");

    auto export_info = LvlInitStruct<VkExportMemoryAllocateInfo>();
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_info);
    alloc_info.allocationSize = 1024;
    alloc_info.memoryTypeIndex = 0;

    vk_testing::DeviceMemory memory;
    memory.init(*m_device, alloc_info);
    auto get_handle_info = LvlInitStruct<VkMemoryGetFdInfoKHR>();
    get_handle_info.memory = memory;
    get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    int fd = -1;
    vkGetMemoryFdKHR(*m_device, &get_handle_info, &fd);
}
