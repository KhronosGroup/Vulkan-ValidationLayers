/***************************************************************************
 *
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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
 ****************************************************************************/

// The layer library itself is only this file (plus the Tracy hooks in profiling_layer.cpp). Everything else is in the vvl_core
// static library, so that tests can link the layer internals directly. Because nothing else in the layer library references
// vvl_core, these exports are what pull the rest of the layer in at link time.

#include <cassert>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#include "chassis/chassis.h"

// Include new / delete overrides if using mimalloc. This needs to be included exactly once, in a file that is part of the
// layer library but not vvl_core. Otherwise anything linking vvl_core, like the tests, would also replace its new / delete.
#if defined(USE_MIMALLOC) && defined(_WIN64)
#include "mimalloc-new-delete.h"
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define VVL_EXPORT __attribute__((visibility("default")))
#else
#define VVL_EXPORT
#endif

// The following functions need to match the `/DEF` and `--version-script` files
// for consistency across platforms that don't accept those linker options.
extern "C" {

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* funcName) {
    return vulkan_layer_chassis::GetInstanceProcAddr(instance, funcName);
}

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char* funcName) {
    return vulkan_layer_chassis::GetDeviceProcAddr(dev, funcName);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount,
                                                                                 VkExtensionProperties* pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
    assert(pVersionStruct != nullptr);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vulkan_layer_chassis::GetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vulkan_layer_chassis::GetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vulkan_layer_chassis::GetPhysicalDeviceProcAddr;
    }

    return VK_SUCCESS;
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount,
                                                                           VkLayerProperties* pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                               const char* pLayerName, uint32_t* pCount,
                                                                               VkExtensionProperties* pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}
#endif

}  // extern "C"
