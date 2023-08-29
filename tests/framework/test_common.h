/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "error_message/logging.h"

#include <vulkan/vulkan.h>

#include <vulkan/vk_enum_string_helper.h>

// GTest and Xlib collide due to redefinitions of "None" and "Bool"
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma push_macro("None")
#pragma push_macro("Bool")
#undef None
#undef Bool
#endif

#include <gtest/gtest.h>

// Redefine Xlib definitions
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma pop_macro("Bool")
#pragma pop_macro("None")
#endif

#include "binding.h"

#define ASSERT_VK_SUCCESS(err)                                                \
    {                                                                         \
        const VkResult resolved_err = err;                                    \
        ASSERT_EQ(VK_SUCCESS, resolved_err) << string_VkResult(resolved_err); \
    }
