/* Copyright (c) 2018-2021 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
 * Copyright (C) 2018-2021 Google Inc.
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

// Overview of Android NDK headers:
//
// <android/hardware_buffer.h>
//    - All enums referenced by VK_ANDROID_external_memory_android_hardware_buffer
// <android/api-level.h>
//     - __ANDROID_API_
//     - The version of android compiling **for**
//     - Currently there is no need for the VVL to build for anything greater than android-26
// <android/ndk-version.h>
//     - __NDK_MAJOR__
//     - Is the version of NDK compiling **with**
//     - r16 has a subset of needed hardware_buffer.h enums value
//     - r18 proper ANDROID_API checking added
//     - r20 YCbCr was added
//     - r23 (current) no differences from r20 being used

#ifndef ANDROID_NDK_TYPES_H_
#define ANDROID_NDK_TYPES_H_

// Everyone should be able to include this file and ignore it if not building for Android
#ifdef VK_USE_PLATFORM_ANDROID_KHR

#ifdef __ANDROID__  // Compiling for Android
#include <android/hardware_buffer.h>

// Building Vulkan validation with NDK header files prior to platform-26 is supported, but will remove
// all validation checks for Android Hardware Buffers.
// This is due to AHB not being introduced until Android 26 (Android Oreo)
//
// NOTE: VK_USE_PLATFORM_ANDROID_KHR != Android Hardware Buffer
//       AHB is all things not found in the Vulkan Spec
#if __ANDROID_API__ < 24
#error "Vulkan not supported on Android 23 and below"
#elif __ANDROID_API__ < 26
#pragma message("Building for Android without Android Hardward Buffer support")
#else
// This is used to allow building for Android without AHB support
#define AHB_VALIDATION_SUPPORT
#endif  // __ANDROID_API__

// Require at least NDK 20 to build Validation Layers. Makes everything simpler to just have people building the layers to use a
// recent (over 2 years old) version of the NDK.
#if __NDK_MAJOR__ < 20
#error "Validation Layers require at least NDK r20 or greater to build"
#endif  // __NDK_MAJOR__

// Not in public NDK headers, only AOSP headers, but NDK will allow it for usage of camera apps and we use for AHB tests
constexpr uint32_t AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED = 0x22;
constexpr uint64_t AHARDWAREBUFFER_USAGE_CAMERA_WRITE = 0x20000;
constexpr uint64_t AHARDWAREBUFFER_USAGE_CAMERA_READ = 0x40000;

#else  // Not __ANDROID__, but VK_USE_PLATFORM_ANDROID_KHR
// This combination should not be seen in the wild
// There use to be a mock set of enums and functions to use, but it was not maintained and seems no one was in need of it anyways
#endif  // __ANDROID__

#endif  // VK_USE_PLATFORM_ANDROID_KHR

#endif  // ANDROID_NDK_TYPES_H_
