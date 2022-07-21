/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

//
// POSITIVE VALIDATION TESTS
//
// These tests do not expect to encounter ANY validation errors pass only if this is true

TEST_F(VkPositiveLayerTest, TwoInstances) {
    TEST_DESCRIPTION("Create two instances before destroy");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkInstance i1, i2, i3;

    VkInstanceCreateInfo ici = LvlInitStruct<VkInstanceCreateInfo>();
    ici.enabledLayerCount = instance_layers_.size();
    ici.ppEnabledLayerNames = instance_layers_.data();

    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &i1));

    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &i2));
    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i2, nullptr));

    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &i3));
    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i3, nullptr));

    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i1, nullptr));
}

TEST_F(VkPositiveLayerTest, NullFunctionPointer) {
    TEST_DESCRIPTION("On 1_0 instance , call GetDeviceProcAddr on promoted 1_1 device-level entrypoint");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, "VK_KHR_get_memory_requirements2")) {
        m_device_extension_names.push_back("VK_KHR_get_memory_requirements2");
    } else {
        printf("%s VK_KHR_get_memory_reqirements2 extension not supported, skipping NullFunctionPointer test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpGetBufferMemoryRequirements =
        (PFN_vkGetBufferMemoryRequirements2)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferMemoryRequirements2");
    if (fpGetBufferMemoryRequirements) {
        m_errorMonitor->SetError("Null was expected!");
    }
}

TEST_F(VkPositiveLayerTest, ValidationInstanceExtensions) {
    ASSERT_NO_FATAL_FAILURE(Init());

    std::string layer_name = "VK_LAYER_KHRONOS_validation";
    std::vector<std::string> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                           VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME};
    uint32_t property_count;
    vk::EnumerateInstanceExtensionProperties(layer_name.c_str(), &property_count, NULL);
    std::vector<VkExtensionProperties> properties(property_count);
    vk::EnumerateInstanceExtensionProperties(layer_name.c_str(), &property_count, properties.data());
    for (size_t i = 0; i < extensions.size(); i++) {
        bool found = false;
        for (auto props : properties) {
            if (!strcmp(props.extensionName, extensions[i].c_str())) {
                found = true;
                break;
            }
        }
        if (!found) {
            FAIL() << "Validation layer is missing extension " << extensions[i].c_str();
        }
    }
}
