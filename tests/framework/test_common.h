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

#include "generated/vk_enum_string_helper.h"

#ifdef _MSC_VER
#pragma warning(push)
/*
    warnings 4251 and 4275 have to do with potential dll-interface mismatch
    between library (gtest) and users. Since we build the gtest library
    as part of the test build we know that the dll-interface will match and
    can disable these warnings.
 */
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#endif

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "binding.h"

#define ASSERT_VK_SUCCESS(err)                                                \
    {                                                                         \
        const VkResult resolved_err = err;                                    \
        ASSERT_EQ(VK_SUCCESS, resolved_err) << string_VkResult(resolved_err); \
    }

static inline void test_error_callback(const char *expr, const char *file, unsigned int line, const char *function) {
    ADD_FAILURE_AT(file, line) << "Assertion: `" << expr << "'";
}

// Wrap FAIL:
//  * DRY for common messages
//  * for test stability reasons sometimes cleanup code is required *prior* to the return hidden in FAIL
//  * result_arg_ *can* (should) have side-effect, but is referenced exactly once
//  * label_ must be converitble to bool, and *should* *not* have side-effects
//  * clean_ *can* (should) have side-effects
//    * "{}" or ";" are valid clean_ values for noop
#define REQUIRE_SUCCESS(result_arg_, label_, clean_)                    \
    {                                                                   \
        const VkResult result_ = (result_arg_);                         \
        if (result_ != VK_SUCCESS) {                                    \
            { clean_; }                                                 \
            if (bool(label_)) {                                         \
                FAIL() << string_VkResult(result_) << ": " << (label_); \
            } else {                                                    \
                FAIL() << string_VkResult(result_);                     \
            }                                                           \
        }                                                               \
    }
