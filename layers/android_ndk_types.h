/* Copyright (c) 2018-2019 The Khronos Group Inc.
 * Copyright (c) 2018-2019 Valve Corporation
 * Copyright (c) 2018-2019 LunarG, Inc.
 * Copyright (C) 2018-2019 Google Inc.
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
 *
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#ifndef ANDROID_NDK_TYPES_H_
#define ANDROID_NDK_TYPES_H_

#ifdef VK_USE_PLATFORM_ANDROID_KHR

// All eums referenced by VK_ANDROID_external_memory_android_hardware_buffer are present in
// the platform-28 (Android P) versions of the header files.  A partial set exists in the
// platform-26 (O) headers, where hardware_buffer.h first appears in the NDK.
//
// Building Vulkan validation with NDK header files prior to platform-26 is not supported.
//
// Decoder ring for Android compile symbols found here: https://github.com/android-ndk/ndk/issues/407

#ifdef __ANDROID__  // Compiling for Android
#include <android/api-level.h>
#include <android/hardware_buffer.h>  // First appearance in Android O (platform-26)

// If NDK is O (platform-26 or -27), supplement the missing enums with pre-processor defined literals
// If Android P or later, then all required enums are already defined
#if defined(__ANDROID_API_O__) && !defined(__ANDROID_API_P__)
// Formats
#define AHARDWAREBUFFER_FORMAT_D16_UNORM 0x30
#define AHARDWAREBUFFER_FORMAT_D24_UNORM 0x31
#define AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT 0x32
#define AHARDWAREBUFFER_FORMAT_D32_FLOAT 0x33
#define AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT 0x34
#define AHARDWAREBUFFER_FORMAT_S8_UINT 0x35
// Usage bits
#define AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP 0x2000000
#define AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE 0x4000000
#endif  // __ANDROID_API_O__ && !_P__

#else  // Not __ANDROID__, but VK_USE_PLATFORM_ANDROID_KHR
// This combination should not be seen in the wild, but can be used to allow testing
// of the AHB extension validation on other platforms using MockICD
//
// Define the minimal set of NDK enums and structs needed to compile
// VK_ANDROID_external_memory_android_hardware_buffer validation without an NDK present
struct AHardwareBuffer {};

// Enumerations of format and usage flags for Android opaque external memory blobs
typedef enum AHardwareBufferFormat {
    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM = 1,
    AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM = 2,
    AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM = 3,
    AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM = 4,
    AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT = 0x16,
    AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM = 0x2b,
    AHARDWAREBUFFER_FORMAT_D16_UNORM = 0x30,
    AHARDWAREBUFFER_FORMAT_D24_UNORM = 0x31,
    AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT = 0x32,
    AHARDWAREBUFFER_FORMAT_D32_FLOAT = 0x33,
    AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT = 0x34,
    AHARDWAREBUFFER_FORMAT_S8_UINT = 0x35,
    AHARDWAREBUFFER_FORMAT_BLOB = 0x21
} AHardwareBufferFormat;

typedef enum AHardwareBufferUsage {
    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE = 0x100,
    AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT = 0x200,
    AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP = 0x2000000,
    AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE = 0x4000000,
    AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT = 0x4000,
    AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER = 0x1000000
} AHardwareBufferUsage;

typedef struct AHardwareBuffer_Desc {
    uint32_t format;  //	   One of AHARDWAREBUFFER_FORMAT_*.
    uint32_t height;  //	   Height in pixels.
    uint32_t layers;  //	   Number of images in an image array.
    uint32_t rfu0;    //	   Initialize to zero, reserved for future use.
    uint64_t rfu1;    //	   Initialize to zero, reserved for future use.
    uint32_t stride;  //	   Row stride in pixels, ignored for AHardwareBuffer_allocate()
    uint64_t usage;   //	   Combination of AHARDWAREBUFFER_USAGE_*.
    uint32_t width;   //	   Width in pixels.
} AHardwareBuffer_Desc;

// Minimal NDK fxn stubs to allow testing on ndk-less platform
static inline int AHardwareBuffer_allocate(const AHardwareBuffer_Desc *ahbDesc, AHardwareBuffer **buffer) {
    size_t size = ahbDesc->height * ahbDesc->width * 8;  // Alloc for largest (64 bpp) format
    if (size < sizeof(AHardwareBuffer_Desc)) size = sizeof(AHardwareBuffer_Desc);
    *buffer = (AHardwareBuffer *)malloc(size);
    memcpy((void *)(*buffer), (void *)ahbDesc, sizeof(AHardwareBuffer_Desc));
    return 0;
}

static inline void AHardwareBuffer_release(AHardwareBuffer *buffer) {
    if (buffer) free(buffer);
}

static inline void AHardwareBuffer_describe(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) {
    if (buffer && outDesc) {
        memcpy((void *)outDesc, (void *)buffer, sizeof(AHardwareBuffer_Desc));
    }
    return;
}

#endif  // __ANDROID__

#endif  // VK_USE_PLATFORM_ANDROID_KHR

#endif  // ANDROID_NDK_TYPES_H_
