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

TEST_F(VkPositiveLayerTest, ToolingExtension) {
    TEST_DESCRIPTION("Call Tooling Extension and verify layer results");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    auto fpGetPhysicalDeviceToolPropertiesEXT =
        (PFN_vkGetPhysicalDeviceToolPropertiesEXT)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceToolPropertiesEXT");

    uint32_t tool_count = 0;
    auto result = fpGetPhysicalDeviceToolPropertiesEXT(gpu(), &tool_count, nullptr);

    if (tool_count <= 0) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }

    std::vector<VkPhysicalDeviceToolPropertiesEXT> tool_properties(tool_count);
    for (uint32_t i = 0; i < tool_count; i++) {
        tool_properties[i] = LvlInitStruct<VkPhysicalDeviceToolPropertiesEXT>();
    }

    bool found_validation_layer = false;

    if (result == VK_SUCCESS) {
        result = fpGetPhysicalDeviceToolPropertiesEXT(gpu(), &tool_count, tool_properties.data());

        for (uint32_t i = 0; i < tool_count; i++) {
            if (strcmp(tool_properties[0].name, "Khronos Validation Layer") == 0) {
                found_validation_layer = true;
                break;
            }
        }
    }
    if (!found_validation_layer) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }
}
