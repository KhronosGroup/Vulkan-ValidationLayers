/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Rodrigo Locatti <rlocatti@nvidia.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

// Tests for NVIDIA-specific best practices
const char *kEnableNVIDIAValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA";

static constexpr float defaultQueuePriority = 0.0f;

TEST_F(VkNvidiaBestPracticesLayerTest, PageableDeviceLocalMemory) {
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    InitBestPracticesFramework(kEnableNVIDIAValidation);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageable = LvlInitStruct<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>();
    pageable.pageableDeviceLocalMemory = VK_TRUE;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory");
        VkDevice test_device = VK_NULL_HANDLE;
        VkResult err = vk::CreateDevice(gpu(), &device_ci, nullptr, &test_device);
        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyDevice(test_device, nullptr);
        }
    }

    // Now enable the expected features
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
    device_ci.pNext = &pageable;

    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory");
        vk_testing::Device test_device(gpu());
        test_device.init(device_ci);
        m_errorMonitor->Finish();
    }
}
